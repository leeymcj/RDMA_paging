#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

//#include <netdb.h>
//#include <stdio.h>
//#include <linux/stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <rdma/rdma_cma.h>
#include <rdma_cma.h>
#include <linux/kthread.h>
//#include <sys/time.h>
#include <linux/semaphore.h>

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

enum mode {
  M_WRITE,
  M_READ
};


void die(const char *reason);

void build_connection(struct rdma_cm_id *id);
void build_params(struct rdma_conn_param *params);
void destroy_connection(void *context);
void on_connect(void *context);
void send_mr(void *context);
//void set_mode(enum mode m);
void time_stamp(int i);
void time_calculate(void);
void _rdma_done(void*);
void _rdma_read(void*, void*);
void _rdma_read_offset(void*, void*, uint64_t); //with offset
void _rdma_write_offset(void*, void*, uint64_t); //context, buf, offset

#endif
