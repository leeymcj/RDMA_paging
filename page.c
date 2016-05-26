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
#include "time.h"


#define PAGE_SIZE ( 1<<12 )
static char *page;
static int page_nr = 10;
static int page_len;

static pthread_mutex_t _syscall_mtx;


void usage(const char *argv0)
{
  fprintf(stderr, "usage:%s  ip port num_page\n", argv0);
  exit(1);
}

static void recover(void)
{
        printf("hello: recovered from pagefault\n");
        exit(0);
}


static void pgflt_handler(uintptr_t addr, uint64_t fec, struct dune_tf *tf)
{
	//printf("caught page fault!\n");

	//printf("%s\n", page); 	//this will generate infinite page fault
	int ret;
	ptent_t *pte;

	//printf("page-in\n");

	pthread_mutex_lock(&_syscall_mtx);
	ret = dune_vm_lookup(pgroot, (void *) addr, CREATE_NORMAL, &pte);
	assert(ret==0);
	long offset = 0;//for single page
	
	*pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *) addr));
	rdma_read_offset((void *)addr, (__u64)addr - (__u64) page );
	pthread_mutex_unlock(&_syscall_mtx);

}


int main(int argc, char *argv[])
{
	
	int i,n;
	volatile int ret;
	char* val;
	struct timeval t0, t1, t2, t3;

	ret = dune_init_and_enter();

	if (ret) {
		printf("failed to initialize dune\n");
		return ret;
	}

	dune_register_pgflt_handler(pgflt_handler);
	

	if (argc != 4){
		usage(argv[0]);
	}

	page_nr = atoi(argv[3]);
	page_len = PAGE_SIZE*page_nr;

	//launch_server(2, page_nr);
	//ret = rdma_init("cp-2", "41406", PAGE_SIZE);
	rdma_init(argv[1], argv[2] , PAGE_SIZE);


	 gettimeofday(&t0, 0);	
	//allocated a memory
	if (posix_memalign((void **)&page, PAGE_SIZE, page_len)) {
		perror("init page memory failed");
		return -1;
	}

	memset( page,0, page_len);
	val= (char*) malloc(page_len);
	memcpy(val, page, page_len);


	/*page-out*/	
	pthread_mutex_lock(&_syscall_mtx);
	//remote write
	for (i=0; i<page_nr; i++)
	rdma_write_offset( &page[i*PAGE_SIZE], i*PAGE_SIZE);

	//unmap physical address
	dune_vm_unmap(pgroot, (void *)page, page_len);
	pthread_mutex_unlock(&_syscall_mtx);

	//printf("page-out\n");

	//generate page fault
		
	//printf("page fault!\n");
	gettimeofday(&t1, 0);
        assert(!memcmp(val, page, page_len));
        gettimeofday(&t2, 0);


	//printf("page fault handled\n");
	rdma_done();
	long pagefault = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
        long total = (t2.tv_sec-t0.tv_sec)*1000000 + t2.tv_usec-t0.tv_usec;
        fprintf(stdout, "%ld\t%ld\n", pagefault, total);
        return 0;


}

