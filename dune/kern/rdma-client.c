#include "rdma-client.h"

const int TIMEOUT_IN_MS = 500; /* ms */

static int on_addr_resolved(struct rdma_cm_id *id);
static int on_connection(struct rdma_cm_id *id);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_event *event);
static int on_route_resolved(struct rdma_cm_id *id);
static void * create_rdma(void*);
long RDMA_BUFFER_SIZE;
void* conn_context;
char s_ip[20];
char s_port[20];
kthread_t init_thread;
sem_t init_wait;

/* run in a thred */
int rdma_init(char* ip, char* port, long msg_size){
  //printf("2\n");
  RDMA_BUFFER_SIZE = msg_size;
  memset(s_ip , 0, 20);
  memset(s_port, 0, 20);
  strcpy(s_ip, ip);
  strcpy(s_port, port);
  sem_init(&init_wait, 0, 0);
  TEST_NZ(kthread_create(&init_thread, NULL, create_rdma, NULL));
  TEST_NZ(kthread_detach(init_thread));
  sem_wait(&init_wait);
  return 0;
}

void * create_rdma(void *ctx)
{
  struct addrinfo *addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *conn= NULL;
  struct rdma_event_channel *ec = NULL;
  /*
  if (strcmp(argv[1], "write") == 0)
    set_mode(M_WRITE);
  else if (strcmp(argv[1], "read") == 0)
    set_mode(M_READ);
  else
    usage(argv[0]);
  */


  TEST_NZ(getaddrinfo(s_ip, s_port, NULL, &addr));

  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
  TEST_NZ(rdma_resolve_addr(conn, NULL, addr->ai_addr, TIMEOUT_IN_MS));

  freeaddrinfo(addr);


  time_stamp(0);
  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);

    if (on_event(&event_copy))
      break;
  }

  rdma_destroy_event_channel(ec);

  time_stamp(9);
  time_calculate();
  return NULL;
}


int on_addr_resolved(struct rdma_cm_id *id)
{
  //printf("address resolved.\n");

  build_connection(id);
  time_stamp(13);   
  //sprintf(get_local_message_region(id->context), "message from active/client side with pid %d", getpid());
  TEST_NZ(rdma_resolve_route(id, TIMEOUT_IN_MS));
  time_stamp(14);

  return 0;
}

int on_connection(struct rdma_cm_id *id)
{
  on_connect(id->context);
  time_stamp(1);
  time_stamp(4);
  //send_mr(id->context);
  conn_context = (void*)id->context;
  return 0;
}

int on_disconnect(struct rdma_cm_id *id)
{
  //printf("disconnected.\n");

  destroy_connection(id->context);
  return 1; /* exit event loop */
}

int on_event(struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_ADDR_RESOLVED)
    r = on_addr_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ROUTE_RESOLVED)
    r = on_route_resolved(event->id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED)
    r = on_connection(event->id);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(event->id);
  else {
     pr_debug("on_event: %d\n", event->event);
   die("on_event: unknown event.");
  }

  return r;
}

int on_route_resolved(struct rdma_cm_id *id)
{
  struct rdma_conn_param cm_params;
  time_stamp(15);
  //printf("route resolved.\n");
  build_params(&cm_params);
  TEST_NZ(rdma_connect(id, &cm_params));
  time_stamp(16);

  return 0;
}

/*
 *J: RDMA APIs
 */
int rdma_read(void* buf){
  //printf("6\n");
  _rdma_read(conn_context, buf);
  return 0;
}

/* J: offset = page_size * page_num; */
int rdma_read_offset(void* buf, uint64_t offset){
  _rdma_read_offset(conn_context, buf, offset);
  return 0;
}

/* J: offset = page_size * page_num; */
int rdma_write_offset(void* buf, uint64_t offset){
  _rdma_write_offset(conn_context, buf, offset);
  return 0;
}

int rdma_done(void){
  //printf("9\n");
  _rdma_done(conn_context);
  return 0;
}

