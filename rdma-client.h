#ifndef RDMA_CLIENT_H
#define RDMA_CLIENT_H

#include "rdma-common.h"


int rdma_init(char* ip, char* port, long msg_size); //blocking
int rdma_read(void* buf);  //blocking
int rdma_read_offset(void* buf, uint64_t offset);  //with offset
int rdma_write_offset(void* buf, uint64_t offset);
int rdma_done(void);  //blocking


#endif
