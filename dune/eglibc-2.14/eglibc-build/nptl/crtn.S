#APP
	
#include "defs.h"
	
#if defined __i686 && defined __ASSEMBLER__
	
#undef __i686
	
#define __i686 __i686
	
#endif
	
/*@HEADER_ENDS*/
/*@_init_EPILOG_BEGINS*/
# 0 "" 2
# 92 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	.section .init
# 0 "" 2
#NO_APP
	addq	$8, %rsp
	ret
#APP
	END_INIT
	
/*@_init_EPILOG_ENDS*/
/*@_fini_EPILOG_BEGINS*/
# 0 "" 2
# 121 "../nptl/sysdeps/pthread/pt-initfini.c" 1
	.section .fini
# 0 "" 2
#NO_APP
	addq	$8, %rsp
	ret
#APP
	END_FINI
	
/*@_fini_EPILOG_ENDS*/
/*@TRAILER_BEGINS*/
	.hidden	__pthread_initialize_minimal_internal
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-4)"
	.section	.note.GNU-stack,"",@progbits
