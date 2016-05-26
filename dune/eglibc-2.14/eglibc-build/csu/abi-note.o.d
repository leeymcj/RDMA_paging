$(common-objpfx)csu/abi-note.o: abi-note.S \
 /usr/include/stdc-predef.h ../include/libc-symbols.h \
 $(common-objpfx)config.h \
 ../sysdeps/generic/symbol-hacks.h \
 $(common-objpfx)csu/abi-tag.h

/usr/include/stdc-predef.h:

../include/libc-symbols.h:

$(common-objpfx)config.h:

../sysdeps/generic/symbol-hacks.h:

$(common-objpfx)csu/abi-tag.h:
