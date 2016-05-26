#include "rdma-client.h"

#include <linux/ip.h>
#include <linux/in.h>
#include <linux/netdevice.h>

const int TIMEOUT_IN_MS = 500; /* ms */

static int on_addr_resolved(struct rdma_cm_id *id);
static int on_connection(struct rdma_cm_id *id);
static int on_disconnect(struct rdma_cm_id *id);
static int on_event(struct rdma_cm_id *id, struct rdma_cm_event *event);
static int on_route_resolved(struct rdma_cm_id *id);
static int create_rdma(void*);


long RDMA_BUFFER_SIZE;
void* conn_context;
long ip;
int port;
//pthread_t init_thread;
//static struct task_struct * init_thread = NULL;
//static struct semaphore* init_wait;

/* run in a thread */ //why thread? for concurrency? Let's assume connection initialized at the beginning in container
int rdma_init(char* s_ip, char* s_port, long msg_size){ //connection initialized a priori

  int ret = 0;
  RDMA_BUFFER_SIZE = msg_size;
  ret = kstrtol(s_ip, 0, &ip);
  ret = kstrtoint(s_port, 0, &port);
//  memset(s_ip , 0, 20);
//  memset(s_port, 0, 20);
//  strcpy(s_ip, ip);
//  strcpy(s_port, port);
// sema_init(init_wait, 0);
  kthread_run(  create_rdma , NULL, "init_thread");

  /* Wait for kthread_stop*/	
  set_current_state(TASK_INTERRUPTIBLE);
  while (!kthread_should_stop()) {
    schedule();
    set_current_state(TASK_INTERRUPTIBLE);
  }
  
  
  //TEST_NZ(pthread_detach(init_thread));
  //down(init_wait);
  return 0;
}

static int create_rdma(void *ctx)
{

  //  struct addrinfo *addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *id = NULL;
  rdma_cm_event_handler event_handler = NULL;

  /*
  if (strcmp(argv[1], "write") == 0)
    set_mode(M_WRITE);
  else if (strcmp(argv[1], "read") == 0)
    set_mode(M_READ);
  else
    usage(argv[0]);
  */


 // TEST_NZ(getaddrinfo(s_ip, s_port, NULL, &addr));
  struct sockaddr_in addr = { 0 };
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(ip);
  addr.sin_port = htons(port);


  //TEST_Z(ec = rdma_create_event_channel()); 
  TEST_NZ( id = rdma_create_id( event_handler, NULL, RDMA_PS_TCP, IB_QPT_RC ) ); //WHAT QP?
  TEST_NZ( rdma_resolve_addr(id, NULL, (struct sockaddr*) &addr, TIMEOUT_IN_MS) );

//  kfree(addr);


  time_stamp(0);


  while ( event_handler(id, event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
//    rdma_ack_cm_event(event);


    if (on_event(id, &event_copy))
      break;
  }

  rdma_destroy_id(id);

  time_stamp(9);
  time_calculate();
  return 0;
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

//event handler
int on_event(struct rdma_cm_id *id, struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_ADDR_RESOLVED)
    r = on_addr_resolved(id);
  else if (event->event == RDMA_CM_EVENT_ROUTE_RESOLVED)
    r = on_route_resolved(id);
  else if (event->event == RDMA_CM_EVENT_ESTABLISHED)
    r = on_connection(id);
  else if (event->event == RDMA_CM_EVENT_DISCONNECTED)
    r = on_disconnect(id);
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

