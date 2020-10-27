#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// 우리는 하나의 코어만 사용하고자 하므로 나머지 코어들을 무한루프 상태에 둔다.
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	adr	x0, bss_begin // bss 영역 0으로 초기화
	adr	x1, bss_end
	sub	x1, x1, x0 // bss_end - bss_begin
	bl 	memzero // memzero(bss_begin, bss_end - bss_begin)

	mov	sp, #LOW_MEMORY   // sp 초기화
	bl	kernel_main   // 실행을 kernel_main 함수로 넘겨준다.
	b 	proc_hang		// should never come here
