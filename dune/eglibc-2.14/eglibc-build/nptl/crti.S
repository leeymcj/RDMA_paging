#APP
	
#include "defs.h"
	
#if defined __i686 && defined __ASSEMBLER__
	
#undef __i686
	
#define __i686 __i686
	
#endif
	
/*@HEADER_ENDS*/
/*@_init_PROLOG_BEGINS*/
	.section .init
#NO_APP
	.section	.init,"ax",@progbits
	.p2align 4,,15
	.globl	_init
	.type	_init, @function
_init:
	subq	$8, %rsp
	call	__pthread_initialize_minimal_internal
#APP
# 87 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	ALIGN
# 0 "" 2
# 88 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	END_INIT
# 0 "" 2
# 90 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	
/*@_init_PROLOG_ENDS*/
/*@_fini_PROLOG_BEGINS*/
	.section .fini
#NO_APP
	.section	.fini,"ax",@progbits
	.p2align 4,,15
	.globl	_fini
	.type	_fini, @function
_fini:
	subq	$8, %rsp
#APP
# 107 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	ALIGN
# 0 "" 2
# 108 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	END_FINI
# 0 "" 2
# 109 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	
/*@_fini_PROLOG_ENDS*/
/*@TRAILER_BEGINS*/
	.hidden	__pthread_initialize_minimal_internal
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-4)"
	.section	.note.GNU-stack,"",@progbits
