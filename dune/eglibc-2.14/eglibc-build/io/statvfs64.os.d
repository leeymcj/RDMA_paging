$(common-objpfx)io/statvfs64.os: \
 ../sysdeps/unix/sysv/linux/wordsize-64/statvfs64.c \
 /usr/include/stdc-predef.h ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:
