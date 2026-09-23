	.file	"probe.cpp"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	probe
	.def	probe;	.scl	2;	.type	32;	.endef
	.seh_proc	probe
probe:
.LFB14:
	sub	rsp, 40
	.seh_stackalloc	40
	.seh_endprologue
	mov	rax, QWORD PTR [rcx]
	call	[QWORD PTR [rax]]
	add	rsp, 40
	ret
	.seh_endproc
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.2.0"
