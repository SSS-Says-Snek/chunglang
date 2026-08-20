	.file	"<module sus>"
	.text
	.globl	main                            # -- Begin function main
	.p2align	4
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	$4, 16(%rsp)
	movq	$.L__unnamed_1, 8(%rsp)
	leaq	8(%rsp), %rdi
	callq	print_string@PLT
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.type	.L__unnamed_1,@object           # @0
	.section	.rodata.cst4,"aM",@progbits,4
.L__unnamed_1:
	.ascii	"gurt"
	.size	.L__unnamed_1, 4

	.section	".note.GNU-stack","",@progbits
