CFLAGS = -Wall -I /usr/include $(EXTRA_CFLAGS)
LDLIBS := ${LDLIBS} /usr/src/dune/libdune -ldune
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread
BINARIES = page base

all:  $(BINARIES)

page : page.o /usr/src/dune/libdune/libdune.a rdma-client.o rdma-common.o
	$(CC) $(CFLAGS) $(<) -o page rdma-client.o rdma-common.o -L ${LDLIBS}

base : base.o /usr/src/dune/libdune/libdune.a rdma-client.o rdma-common.o
	$(CC) $(CFLAGS) $(<) -o base rdma-client.o rdma-common.o -L ${LDLIBS}

clean:
	$(RM) $(BINARIES) *.o
