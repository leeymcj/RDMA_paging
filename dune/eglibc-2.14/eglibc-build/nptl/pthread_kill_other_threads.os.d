$(common-objpfx)nptl/pthread_kill_other_threads.os: \
 pthread_kill_other_threads.c /usr/include/stdc-predef.h \
 ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h ../include/shlib-compat.h \
 $(common-objpfx)abi-versions.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:

../include/shlib-compat.h:

$(common-objpfx)abi-versions.h: