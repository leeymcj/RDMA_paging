$(common-objpfx)csu/elf-init.oS: elf-init.c \
 /usr/include/stdc-predef.h ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h \
 /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stddef.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:

/usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stddef.h:
