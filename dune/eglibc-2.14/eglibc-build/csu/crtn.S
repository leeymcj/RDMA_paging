#APP
	
#include "defs.h"

/*@HEADER_ENDS*/
/*@_init_EPILOG_BEGINS*/
	.section .init
	addq	$8, %rsp
	ret
	END_INIT

/*@_init_EPILOG_ENDS*/
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
