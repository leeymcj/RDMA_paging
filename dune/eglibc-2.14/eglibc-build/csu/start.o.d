$(common-objpfx)csu/start.o: \
 ../sysdeps/x86_64/elf/start.S /usr/include/stdc-predef.h \
 ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h ../sysdeps/generic/bp-sym.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:

../sysdeps/generic/bp-sym.h:
