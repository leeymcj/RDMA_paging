#include "rdma-common.h"

extern long RDMA_BUFFER_SIZE;
struct timeval ts[17];
static const int TS_SELF = 0;
sem_t read_ops;
sem_t write_ops;
sem_t done_ops;
extern sem_t init_wait;

struct message {
  enum {
    MSG_MR,
    MSG_DONE
  } type;

  union {
    struct ibv_mr mr;
  } data;
};

struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

struct connection {
  struct rdma_cm_id *id;
  struct ibv_qp *qp;

  int connected;

  struct ibv_mr *recv_mr;
  struct ibv_mr *send_mr;
  struct ibv_mr *rdma_local_mr;  //for read region
  struct ibv_mr *rdma_remote_mr; //for write region

  struct ibv_mr peer_mr;

  struct message *recv_msg;
  struct message *send_msg;

  char *rdma_local_region;
  char *rdma_remote_region;

  enum {
    SS_INIT,
    SS_DONE_SENT
  } send_state;

  enum {
    RS_INIT,
    RS_MR_RECV,
    RS_DONE_RECV
  } recv_state;
};

static void build_context(struct ibv_context *verbs);
static void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
//static char * get_local_message_region(struct connection *conn);
static void on_completion(struct ibv_wc *);
static void * poll_cq(void *);
static void post_receives(struct connection *conn);
static void register_memory(struct connection *conn);
static void send_message(struct connection *conn);

static struct context *s_ctx = NULL;
//static enum mode s_mode = M_WRITE;

void die(const char *reason)
{
  fprintf(stderr, "%s\n", reason);
  exit(EXIT_FAILURE);
}

void build_connection(struct rdma_cm_id *id)
{
  struct connection *conn;
  struct ibv_qp_init_attr qp_attr;

  //init semaphores
  sem_init(&read_ops, 0, 0);
  sem_init(&done_ops, 0, 0);
  sem_init(&write_ops, 0, 1);

  build_context(id->verbs);
  build_qp_attr(&qp_attr);

  TEST_NZ(rdma_create_qp(id, s_ctx->pd, &qp_attr));

  id->context = conn = (struct connection *)malloc(sizeof(struct connection));

  conn->id = id;
  conn->qp = id->qp;

  conn->send_state = SS_INIT;
  conn->recv_state = RS_INIT;

  conn->connected = 0;

  register_memory(conn);
  post_receives(conn);
}

void build_context(struct ibv_context *verbs)
{
  if (s_ctx) {
    if (s_ctx->ctx != verbs)
      die("cannot handle events in more than one context.");

    return;
  }

  s_ctx = (struct context *)malloc(sizeof(struct context));

  s_ctx->ctx = verbs;

  TEST_Z(s_ctx->pd = ibv_alloc_pd(s_ctx->ctx));
  TEST_Z(s_ctx->comp_channel = ibv_create_comp_channel(s_ctx->ctx));
  TEST_Z(s_ctx->cq = ibv_create_cq(s_ctx->ctx, 10, NULL, s_ctx->comp_channel, 0)); /* cqe=10 is arbitrary */
  TEST_NZ(ibv_req_notify_cq(s_ctx->cq, 0));

  TEST_NZ(pthread_create(&s_ctx->cq_poller_thread, NULL, poll_cq, NULL));
}

void build_params(struct rdma_conn_param *params)
{
  memset(params, 0, sizeof(*params));

  params->initiator_depth = params->responder_resources = 1;
  params->rnr_retry_count = 7; /* infinite retry */
}

void build_qp_attr(struct ibv_qp_init_attr *qp_attr)
{
  memset(qp_attr, 0, sizeof(*qp_attr));

  qp_attr->send_cq = s_ctx->cq;
  qp_attr->recv_cq = s_ctx->cq;
  qp_attr->qp_type = IBV_QPT_RC;

  qp_attr->cap.max_send_wr = 10;
  qp_attr->cap.max_recv_wr = 10;
  qp_attr->cap.max_send_sge = 1;
  qp_attr->cap.max_recv_sge = 1;
}

void destroy_connection(void *context)
{
  struct connection *conn = (struct connection *)context;

  rdma_destroy_qp(conn->id);

  ibv_dereg_mr(conn->send_mr);
  ibv_dereg_mr(conn->recv_mr);
  ibv_dereg_mr(conn->rdma_local_mr);
  ibv_dereg_mr(conn->rdma_remote_mr);

  free(conn->send_msg);
  free(conn->recv_msg);
  free(conn->rdma_local_region);
  free(conn->rdma_remote_region);

  rdma_destroy_id(conn->id);

  free(conn);
}

/*
char * get_local_message_region(struct connection *conn)
{
  return conn->rdma_local_region;
}
*/

void on_completion(struct ibv_wc *wc)
{
  struct connection *conn = (struct connection *)(uintptr_t)wc->wr_id;

  if (wc->status != IBV_WC_SUCCESS){
    if (wc->opcode == IBV_WC_RDMA_WRITE)
        printf("IBV_WC_RDMA_WRITE failed!\n");
    die("on_completion: status is not IBV_WC_SUCCESS.");
  }

  if (wc->opcode & IBV_WC_RECV) {
    if ((conn->recv_state == RS_INIT) && (conn->recv_msg->type == MSG_MR)) {  //RECV Server MR
      memcpy(&conn->peer_mr, &conn->recv_msg->data.mr, sizeof(conn->peer_mr));
      post_receives(conn); /* only rearm for MSG_MR */
      conn->recv_state = RS_MR_RECV;
      if (conn->send_state == SS_INIT && conn->recv_state == RS_MR_RECV) {
        sem_post(&init_wait);  //end of init
      }
    }else if (conn->recv_state == RS_MR_RECV){  //RECV Server Done msg
        conn->recv_state = RS_DONE_RECV;
    }
  }else { //Send completion
    if ((conn->send_state == SS_INIT) && (conn->recv_state == RS_MR_RECV)) {
      if (wc->opcode == IBV_WC_RDMA_READ){
  	time_stamp(7);
        sem_post(&read_ops);
      }
      if (wc->opcode == IBV_WC_SEND){ //Send Done msg
        conn->send_state = SS_DONE_SENT;
        sem_post(&done_ops);
      }
      if (wc->opcode == IBV_WC_RDMA_WRITE){
        sem_post(&write_ops);
          time_stamp(3);
      }
    }
  }

  if ((conn->send_state == SS_DONE_SENT) && (conn->recv_state == RS_DONE_RECV)) {
    //printf("remote buffer: %s\n", get_local_message_region(conn));
    rdma_disconnect(conn->id);
  }
}

void _rdma_read(void *context, void* buf)
{
  struct connection *conn = (struct connection *)context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  //printf("7\n");
  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_RDMA_READ;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr.rdma.remote_addr = (uintptr_t)conn->peer_mr.addr;
  wr.wr.rdma.rkey = conn->peer_mr.rkey;

  sge.addr = (uintptr_t)conn->rdma_local_region;
  sge.length = RDMA_BUFFER_SIZE;
  sge.lkey = conn->rdma_local_mr->lkey;

  time_stamp(2);
  time_stamp(6);
  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
  //printf("8\n");
  sem_wait(&read_ops);
  memcpy((char*)buf, conn->rdma_local_region, RDMA_BUFFER_SIZE);
  time_stamp(3);
}

void _rdma_read_offset(void *context, void* buf, uint64_t offset)
{
  struct connection *conn = (struct connection *)context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  //printf("7\n");
  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_RDMA_READ;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr.rdma.remote_addr = (uintptr_t)conn->peer_mr.addr + offset;
  wr.wr.rdma.rkey = conn->peer_mr.rkey;

  sge.addr = (uintptr_t)conn->rdma_local_region;
  sge.length = RDMA_BUFFER_SIZE;
  sge.lkey = conn->rdma_local_mr->lkey;

  time_stamp(2);
  time_stamp(6);
  //read lock
  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
  //printf("8\n");
  sem_wait(&read_ops);
  memcpy((char*)buf, conn->rdma_local_region, RDMA_BUFFER_SIZE);
  time_stamp(3);
}

void _rdma_write_offset(void *context, void* buf, uint64_t offset)
{
  struct connection *conn = (struct connection *)context;
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;
  //printf("7\n");
  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_RDMA_WRITE;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr.rdma.remote_addr = (uintptr_t)conn->peer_mr.addr + offset;
  wr.wr.rdma.rkey = conn->peer_mr.rkey;

  sge.addr = (uintptr_t)conn->rdma_remote_region;
  sge.length = RDMA_BUFFER_SIZE;
  sge.lkey = conn->rdma_remote_mr->lkey;

  time_stamp(2);
  sem_wait(&write_ops);
  memcpy(conn->rdma_remote_region, (char*)buf, RDMA_BUFFER_SIZE);
  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));

}

void _rdma_done(void *context)
{
  struct connection *conn = (struct connection *)context;
  conn->send_msg->type = MSG_DONE;
  send_message(conn);
  //printf("10\n");
  sem_wait(&done_ops);
}

void on_connect(void *context)
{
  ((struct connection *)context)->connected = 1;
}

void * poll_cq(void *ctx)
{
  struct ibv_cq *cq;
  struct ibv_wc wc;

  while (1) {
    TEST_NZ(ibv_get_cq_event(s_ctx->comp_channel, &cq, &ctx));
    ibv_ack_cq_events(cq, 1);
    TEST_NZ(ibv_req_notify_cq(cq, 0));

    while (ibv_poll_cq(cq, 1, &wc))
      on_completion(&wc);
  }

  return NULL;
}

void post_receives(struct connection *conn)
{
  struct ibv_recv_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  wr.wr_id = (uintptr_t)conn;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;

  sge.addr = (uintptr_t)conn->recv_msg;
  sge.length = sizeof(struct message);
  sge.lkey = conn->recv_mr->lkey;

  TEST_NZ(ibv_post_recv(conn->qp, &wr, &bad_wr));
}

void register_memory(struct connection *conn)
{
  conn->send_msg = malloc(sizeof(struct message));
  conn->recv_msg = malloc(sizeof(struct message));

  conn->rdma_local_region = malloc(RDMA_BUFFER_SIZE);
  conn->rdma_remote_region = malloc(RDMA_BUFFER_SIZE);
  /*
  time_stamp(10);
  memset(conn->rdma_local_region, 0, RDMA_BUFFER_SIZE);
  memset(conn->rdma_remote_region, 0, RDMA_BUFFER_SIZE);
  time_stamp(11);
  */

  TEST_Z(conn->send_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->send_msg, 
    sizeof(struct message), 
    IBV_ACCESS_LOCAL_WRITE));

  TEST_Z(conn->recv_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->recv_msg, 
    sizeof(struct message), 
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));

  TEST_Z(conn->rdma_local_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->rdma_local_region, 
    RDMA_BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE));

  TEST_Z(conn->rdma_remote_mr = ibv_reg_mr(
    s_ctx->pd, 
    conn->rdma_remote_region, 
    RDMA_BUFFER_SIZE, 
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));
}

void send_message(struct connection *conn)
{
  struct ibv_send_wr wr, *bad_wr = NULL;
  struct ibv_sge sge;

  memset(&wr, 0, sizeof(wr));

  wr.wr_id = (uintptr_t)conn;
  wr.opcode = IBV_WR_SEND;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.send_flags = IBV_SEND_SIGNALED;

  sge.addr = (uintptr_t)conn->send_msg;
  sge.length = sizeof(struct message);
  sge.lkey = conn->send_mr->lkey;

  while (!conn->connected);

  TEST_NZ(ibv_post_send(conn->qp, &wr, &bad_wr));
}

void send_mr(void *context)
{
  struct connection *conn = (struct connection *)context;

  conn->send_msg->type = MSG_MR;
  memcpy(&conn->send_msg->data.mr, conn->rdma_remote_mr, sizeof(struct ibv_mr));

  send_message(conn);
}

/*
void set_mode(enum mode m)
{
  s_mode = m;
}
*/
void time_stamp(int i){
	struct timezone tzp;
	gettimeofday(ts+i, &tzp);
	//printf("---time stamp:%d---\n", i);
}


void time_calculate(void){
  long tmp[4] = {0,0,0,0};
  int i;
  for (i=0; i<8; i+=2){
    tmp[i/2] = (long)(ts[i+1].tv_sec * 1000000 + ts[i+1].tv_usec) - (long)(ts[i].tv_sec * 1000000 + ts[i].tv_usec);
  }
  //output : build time, malloc, mr_exchange, post, cleanup, total, before_malloc, after_malloc
  printf("%ld %ld %ld %ld\n", tmp[0], tmp[1],tmp[2], tmp[3]);
}

/*
void time_calculate(void){
	long tmp[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int i;
	for (i=0; i<10; i+=2){
		tmp[i/2] = (long)(ts[i+1].tv_sec * 1000000 + ts[i+1].tv_usec) - (long)(ts[i].tv_sec * 1000000 + ts[i].tv_usec);
	}
	//tmp5 is the total time
	tmp[5] = (long)(ts[9].tv_sec * 1000000 + ts[9].tv_usec) - (long)(ts[0].tv_sec * 1000000 + ts[0].tv_usec) - TS_SELF*8;
	//tmp6 is tp[0]-tp[2]--before malloc
	tmp[6] = (long)(ts[2].tv_sec * 1000000 + ts[2].tv_usec) - (long)(ts[0].tv_sec * 1000000 + ts[0].tv_usec) - TS_SELF*8;
	//tmp7 is tp[1]-tp[3]--after malloc
	tmp[7] = (long)(ts[1].tv_sec * 1000000 + ts[1].tv_usec) - (long)(ts[3].tv_sec * 1000000 + ts[3].tv_usec) - TS_SELF*8;
	tmp[8] = (long)(ts[11].tv_sec * 1000000 + ts[11].tv_usec) - (long)(ts[10].tv_sec * 1000000 + ts[10].tv_usec) - TS_SELF*8;
	tmp[9] = (long)(ts[12].tv_sec * 1000000 + ts[12].tv_usec) - (long)(ts[11].tv_sec * 1000000 + ts[11].tv_usec) - TS_SELF*8;
	tmp[10] = (long)(ts[14].tv_sec * 1000000 + ts[14].tv_usec) - (long)(ts[13].tv_sec * 1000000 + ts[13].tv_usec) - TS_SELF*8;
	tmp[11] = (long)(ts[15].tv_sec * 1000000 + ts[15].tv_usec) - (long)(ts[14].tv_sec * 1000000 + ts[14].tv_usec) - TS_SELF*8;
	tmp[12] = (long)(ts[16].tv_sec * 1000000 + ts[16].tv_usec) - (long)(ts[15].tv_sec * 1000000 + ts[15].tv_usec) - TS_SELF*8;
	tmp[13] = (long)(ts[1].tv_sec * 1000000 + ts[1].tv_usec) - (long)(ts[16].tv_sec * 1000000 + ts[16].tv_usec) - TS_SELF*8;
	//output : build time, malloc, mr_exchange, post, cleanup, total, before_malloc, after_malloc
	printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", tmp[0], tmp[1],tmp[2],tmp[3],tmp[4],tmp[5], tmp[6], tmp[7], tmp[8], tmp[9], tmp[10], tmp[11], tmp[12], tmp[13]);
}
*/
