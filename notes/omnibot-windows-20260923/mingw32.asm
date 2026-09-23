	.file	"probe.cpp"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	_probe
	.def	_probe;	.scl	2;	.type	32;	.endef
_probe:
LFB14:
	.cfi_startproc
	sub	esp, 12
	.cfi_def_cfa_offset 16
	mov	ecx, DWORD PTR [esp+16]
	mov	eax, DWORD PTR [ecx]
	call	[DWORD PTR [eax]]
	add	esp, 12
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
LFE14:
	.ident	"GCC: (Rev8, Built by MSYS2 project) 15.2.0"
