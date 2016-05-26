#include <stdio.h>
#include <stdlib.h>

#include "/usr/src/dune/libdune/dune.h"
#include "/usr/src/dune/libdune/cpu-x86.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <asm/prctl.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <utime.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <linux/sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <err.h>
#include <linux/futex.h>
#include <linux/unistd.h>
#include "rdma-client.h"


#define PAGE_SIZE  1<<12 

static char *page;
//static int PAGE_SIZE;
static int page_len;

static pthread_mutex_t _syscall_mtx;

long offset = 0;//for single page

void usage(const char *argv0)
{
  fprintf(stderr, "usage:%s  page_size(KB) num_page\n", argv0);
  exit(1);
}

static void recover(void)
{
        printf("hello: recovered from pagefault\n");
        exit(0);
}


static void pgflt_handler(uintptr_t addr, uint64_t fec, struct dune_tf *tf)
{
	printf("caught page fault!\n");

	//printf("%s\n", page); 	//this will generate infinite page fault
	int ret;
	ptent_t *pte;

//	printf("page-in\n");

	pthread_mutex_lock(&_syscall_mtx);
	ret = dune_vm_lookup(pgroot, (void *) addr, CREATE_NORMAL, &pte);
	assert(ret==0);
	*pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *) addr));
	rdma_read_offset((void *)addr, offset);
	pthread_mutex_unlock(&_syscall_mtx);

}


int main(int argc, char *argv[])
{
	
	int i;
	volatile int ret;
	char* val = (char *) malloc(page_len);
	FILE* server_list;
	char port[100][20];
	char buf[20];

	//input size
	if (argc != 3){
		usage(argv[0]);
	}
	//PAGE_SIZE= atoi(argv[1])*( 2 << 10 );
	page_len = PAGE_SIZE*atoi(argv[1]);
	
	//get server list
	if ( ( server_list=fopen("server_list.txt", "r") ) == NULL)
	{
		perror("fopen failed\n");
		return -1;
	}
	for(i=0; fgets(buf, sizeof(buf), server_list) != NULL; i++)
	{
		strcpy(port[i], buf);
		//printf("%s\n", port[i]);
	}
	
	int n=1;
	char hostname[20];
	snprintf(hostname, 10,"cp-%d", n+1);
	printf("%s\n", hostname);

	rdma_init( "192.168.0.2", "47366" , PAGE_SIZE);


	ret = dune_init_and_enter();

	if (ret) {
		printf("failed to initialize dune\n");
		return ret;
	}
	

	dune_register_pgflt_handler(pgflt_handler);
	

	

	//allocated a memory
	if (posix_memalign((void **)&page, PAGE_SIZE, page_len)) {
		perror("init page memory failed");
		return -1;
	}

	//for(i=0; i<page_len; i++)
	//	page[i]=rand()%255+1;
	//memset(page, 0, page_len);
	memcpy(val, page, page_len);

	/*page-out*/	
	pthread_mutex_lock(&_syscall_mtx);
	//remote write
	rdma_write_offset(page, offset);
	//unmap physical address
	dune_vm_unmap(pgroot, (void *)page, page_len);
	pthread_mutex_unlock(&_syscall_mtx);

//	printf("page-out\n");

	//generate page fault	
	assert(!memcmp(val, page, page_len));


	printf("page fault handled\n");
	rdma_done();
	return 0;

}

