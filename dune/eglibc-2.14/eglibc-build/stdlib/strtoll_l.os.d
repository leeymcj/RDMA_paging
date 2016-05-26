$(common-objpfx)stdlib/strtoll_l.os: \
 ../sysdeps/wordsize-64/strtoll_l.c /usr/include/stdc-predef.h \
 ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:
