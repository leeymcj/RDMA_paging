	.file	"initfini.c"
#APP
	
#include "defs.h"

/*@HEADER_ENDS*/

/*@_init_PROLOG_BEGINS*/
	.align 4
	.type	call_gmon_start,@function
call_gmon_start:
	subq	$8, %rsp
	movq	__gmon_start__@GOTPCREL(%rip), %rax
	testq	%rax, %rax
	je	.L22
	call	*%rax
.L22:
	addq	$8, %rsp
	ret

	.section .init
	.align 4
.globl _init
	.type	_init,@function
_init:
	subq	$8, %rsp
	call	call_gmon_start
	ALIGN
	END_INIT

/*@_init_PROLOG_ENDS*/

/*@_init_EPILOG_BEGINS*/
	.section .init
	addq	$8, %rsp
	ret
	END_INIT

/*@_init_EPILOG_ENDS*/

/*@_fini_PROLOG_BEGINS*/
	.section .fini
	.align 4
.globl _fini
	.type	_fini,@function
_fini:
	subq	$8, %rsp
	ALIGN
	END_FINI

/*@_fini_PROLOG_ENDS*/
	call	i_am_not_a_leaf@PLT

/*@_fini_EPILOG_BEGINS*/
	.section .fini
	addq	$8, %rsp
	ret
	END_FINI

/*@_fini_EPILOG_ENDS*/

/*@TRAILER_BEGINS*/
	.weak	__gmon_start__

	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-4)"
	.section	.note.GNU-stack,"",@progbits
