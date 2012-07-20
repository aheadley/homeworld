	.file	"Matrix.c"
.globl IdentityHMatrix
	.section	.rodata
	.align 32
	.type	IdentityHMatrix, @object
	.size	IdentityHMatrix, 64
IdentityHMatrix:
	.long	1065353216
	.long	0
	.long	0
	.long	0
	.long	0
	.long	1065353216
	.long	0
	.long	0
	.long	0
	.long	0
	.long	1065353216
	.long	0
	.long	0
	.long	0
	.long	0
	.long	1065353216
.globl IdentityMatrix
	.align 32
	.type	IdentityMatrix, @object
	.size	IdentityMatrix, 36
IdentityMatrix:
	.long	1065353216
	.long	0
	.long	0
	.long	0
	.long	1065353216
	.long	0
	.long	0
	.long	0
	.long	1065353216
	.text
	.p2align 4,,15
.globl hmatMakeHMatFromMat
	.type	hmatMakeHMatFromMat, @function
hmatMakeHMatFromMat:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	8(%ebp), %eax
	movl	(%ecx), %ebx
	movl	%ebx, (%eax)
	movl	4(%ecx), %ebx
	movl	%ebx, 4(%eax)
	movl	8(%ecx), %ebx
	movl	%ebx, 8(%eax)
	movl	$0x0, %ebx
	movl	%ebx, 12(%eax)
	movl	12(%ecx), %edx
	movl	%edx, 16(%eax)
	movl	16(%ecx), %edx
	movl	%edx, 20(%eax)
	movl	20(%ecx), %edx
	movl	%ebx, 28(%eax)
	movl	%edx, 24(%eax)
	movl	24(%ecx), %edx
	movl	%edx, 32(%eax)
	movl	28(%ecx), %edx
	movl	%edx, 36(%eax)
	movl	32(%ecx), %edx
	movl	$0x3f800000, %ecx
	movl	%edx, 40(%eax)
	movl	%ebx, 44(%eax)
	movl	%ebx, 48(%eax)
	movl	%ebx, 52(%eax)
	movl	%ebx, 56(%eax)
	movl	%ecx, 60(%eax)
	popl	%ebx
	popl	%ebp
	ret
	.size	hmatMakeHMatFromMat, .-hmatMakeHMatFromMat
	.p2align 4,,15
.globl hmatMakeHMatFromMatAndVec
	.type	hmatMakeHMatFromMatAndVec, @function
hmatMakeHMatFromMatAndVec:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	8(%ebp), %eax
	movl	16(%ebp), %esi
	movl	(%ecx), %ebx
	movl	%ebx, (%eax)
	movl	4(%ecx), %ebx
	movl	%ebx, 4(%eax)
	movl	8(%ecx), %ebx
	movl	%ebx, 8(%eax)
	movl	$0x0, %ebx
	movl	%ebx, 12(%eax)
	movl	12(%ecx), %edx
	movl	%edx, 16(%eax)
	movl	16(%ecx), %edx
	movl	%edx, 20(%eax)
	movl	20(%ecx), %edx
	movl	%ebx, 28(%eax)
	movl	%edx, 24(%eax)
	movl	24(%ecx), %edx
	movl	%edx, 32(%eax)
	movl	28(%ecx), %edx
	movl	%edx, 36(%eax)
	movl	32(%ecx), %edx
	movl	%ebx, 44(%eax)
	movl	%edx, 40(%eax)
	movl	(%esi), %ecx
	movl	%ecx, 48(%eax)
	movl	4(%esi), %ebx
	movl	%ebx, 52(%eax)
	movl	8(%esi), %ecx
	movl	%ecx, 56(%eax)
	movl	$0x3f800000, %ecx
	movl	%ecx, 60(%eax)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.size	hmatMakeHMatFromMatAndVec, .-hmatMakeHMatFromMatAndVec
	.p2align 4,,15
.globl hmatCreateHMatFromHVecs
	.type	hmatCreateHMatFromHVecs, @function
hmatCreateHMatFromHVecs:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	8(%ebp), %edx
	movl	16(%ebp), %ebx
	movl	20(%ebp), %esi
	movl	(%ecx), %eax
	movl	24(%ebp), %edi
	movl	%eax, (%edx)
	movl	4(%ecx), %eax
	movl	%eax, 4(%edx)
	movl	8(%ecx), %eax
	movl	%eax, 8(%edx)
	movl	12(%ecx), %eax
	movl	%eax, 12(%edx)
	movl	(%ebx), %ecx
	movl	%ecx, 16(%edx)
	movl	4(%ebx), %ecx
	movl	%ecx, 20(%edx)
	movl	8(%ebx), %ecx
	movl	%ecx, 24(%edx)
	movl	12(%ebx), %ecx
	movl	%ecx, 28(%edx)
	movl	(%esi), %ebx
	movl	%ebx, 32(%edx)
	movl	4(%esi), %ecx
	movl	%ecx, 36(%edx)
	movl	8(%esi), %ebx
	movl	%ebx, 40(%edx)
	movl	12(%esi), %ecx
	movl	%ecx, 44(%edx)
	movl	(%edi), %esi
	movl	%esi, 48(%edx)
	movl	4(%edi), %ebx
	movl	%ebx, 52(%edx)
	movl	8(%edi), %ecx
	movl	%ecx, 56(%edx)
	movl	12(%edi), %ecx
	movl	%ecx, 60(%edx)
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	hmatCreateHMatFromHVecs, .-hmatCreateHMatFromHVecs
	.p2align 4,,15
.globl hmatMultiplyHMatByHMat
	.type	hmatMultiplyHMatByHMat, @function
hmatMultiplyHMatByHMat:
	pushl	%ebp
	xorl	%edx, %edx
	movl	%esp, %ebp
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	16(%ebp), %eax
	movl	8(%ebp), %ebx
	.p2align 4,,7
.L9:
	flds	(%ecx,%edx,4)
	flds	16(%ecx,%edx,4)
	fld	%st(1)
	fld	%st(1)
	fmuls	4(%eax)
	flds	32(%ecx,%edx,4)
	fxch	%st(2)
	fmuls	(%eax)
	flds	48(%ecx,%edx,4)
	fxch	%st(1)
	faddp	%st, %st(2)
	fld	%st(2)
	fmuls	8(%eax)
	faddp	%st, %st(2)
	fld	%st(0)
	fmuls	12(%eax)
	faddp	%st, %st(2)
	fld	%st(3)
	fxch	%st(2)
	fstps	(%ebx,%edx,4)
	fld	%st(4)
	fxch	%st(2)
	fmuls	20(%eax)
	fxch	%st(2)
	fmuls	16(%eax)
	faddp	%st, %st(2)
	fld	%st(2)
	fmuls	24(%eax)
	faddp	%st, %st(2)
	fld	%st(0)
	fmuls	28(%eax)
	faddp	%st, %st(2)
	fld	%st(3)
	fxch	%st(2)
	fstps	16(%ebx,%edx,4)
	fld	%st(4)
	fxch	%st(2)
	fmuls	36(%eax)
	fxch	%st(2)
	fmuls	32(%eax)
	faddp	%st, %st(2)
	fld	%st(2)
	fmuls	40(%eax)
	faddp	%st, %st(2)
	fld	%st(0)
	fmuls	44(%eax)
	faddp	%st, %st(2)
	fxch	%st(1)
	fstps	32(%ebx,%edx,4)
	fxch	%st(3)
	fmuls	48(%eax)
	fxch	%st(2)
	fmuls	52(%eax)
	fxch	%st(1)
	fmuls	56(%eax)
	fxch	%st(2)
	faddp	%st, %st(1)
	fxch	%st(2)
	fmuls	60(%eax)
	fxch	%st(2)
	faddp	%st, %st(1)
	faddp	%st, %st(1)
	fstps	48(%ebx,%edx,4)
	incl	%edx
	cmpl	$3, %edx
	jle	.L9
	popl	%ebx
	popl	%ebp
	ret
	.size	hmatMultiplyHMatByHMat, .-hmatMultiplyHMatByHMat
	.p2align 4,,15
.globl hmatMultiplyHMatByHVec
	.type	hmatMultiplyHMatByHVec, @function
hmatMultiplyHMatByHVec:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	movl	16(%ebp), %edx
	movl	8(%ebp), %ecx
	flds	16(%eax)
	flds	4(%edx)
	flds	(%edx)
	fmuls	(%eax)
	flds	8(%edx)
	fxch	%st(3)
	fmul	%st(2), %st
	flds	12(%edx)
	fxch	%st(2)
	faddp	%st, %st(1)
	flds	32(%eax)
	fmul	%st(4), %st
	faddp	%st, %st(1)
	flds	48(%eax)
	fmul	%st(2), %st
	faddp	%st, %st(1)
	fstps	(%ecx)
	flds	4(%eax)
	flds	(%edx)
	fxch	%st(3)
	fmuls	20(%eax)
	flds	36(%eax)
	fxch	%st(2)
	fmul	%st(4), %st
	fxch	%st(2)
	fmul	%st(5), %st
	fxch	%st(2)
	faddp	%st, %st(1)
	faddp	%st, %st(1)
	flds	52(%eax)
	fmul	%st(2), %st
	faddp	%st, %st(1)
	fstps	4(%ecx)
	flds	8(%eax)
	flds	24(%eax)
	flds	4(%edx)
	fmul	%st, %st(1)
	fxch	%st(2)
	fmul	%st(4), %st
	fxch	%st(5)
	fmuls	40(%eax)
	fxch	%st(5)
	faddp	%st, %st(1)
	flds	56(%eax)
	fmul	%st(3), %st
	fxch	%st(1)
	faddp	%st, %st(5)
	faddp	%st, %st(4)
	fxch	%st(3)
	fstps	8(%ecx)
	fxch	%st(1)
	fmuls	12(%eax)
	flds	8(%edx)
	fxch	%st(3)
	fmuls	28(%eax)
	fxch	%st(3)
	fmuls	44(%eax)
	fxch	%st(1)
	faddp	%st, %st(3)
	fxch	%st(1)
	fmuls	60(%eax)
	fxch	%st(2)
	faddp	%st, %st(1)
	faddp	%st, %st(1)
	fstps	12(%ecx)
	popl	%ebp
	ret
	.size	hmatMultiplyHMatByHVec, .-hmatMultiplyHMatByHVec
	.p2align 4,,15
.globl hmatMultiplyHVecByHMat
	.type	hmatMultiplyHVecByHMat, @function
hmatMultiplyHVecByHMat:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %edx
	movl	16(%ebp), %eax
	movl	8(%ebp), %ecx
	flds	4(%edx)
	flds	(%eax)
	fld	%st(1)
	fmuls	4(%eax)
	flds	8(%edx)
	fxch	%st(2)
	fmuls	(%edx)
	flds	12(%edx)
	fxch	%st(1)
	faddp	%st, %st(2)
	fld	%st(2)
	fmuls	8(%eax)
	faddp	%st, %st(2)
	fld	%st(0)
	fmuls	12(%eax)
	faddp	%st, %st(2)
	fxch	%st(1)
	fstps	(%ecx)
	flds	(%edx)
	fld	%st(0)
	fxch	%st(4)
	fmuls	20(%eax)
	fld	%st(3)
	fxch	%st(5)
	fmuls	16(%eax)
	fxch	%st(5)
	fmuls	24(%eax)
	fxch	%st(5)
	faddp	%st, %st(1)
	faddp	%st, %st(4)
	fld	%st(1)
	fmuls	28(%eax)
	faddp	%st, %st(4)
	fld	%st(0)
	fxch	%st(4)
	fstps	4(%ecx)
	flds	4(%edx)
	fld	%st(0)
	fmuls	36(%eax)
	fxch	%st(5)
	fmuls	32(%eax)
	fxch	%st(4)
	fmuls	40(%eax)
	fxch	%st(4)
	faddp	%st, %st(5)
	fld	%st(2)
	fmuls	44(%eax)
	fxch	%st(5)
	faddp	%st, %st(4)
	fxch	%st(3)
	faddp	%st, %st(4)
	fxch	%st(3)
	fstps	8(%ecx)
	fxch	%st(2)
	fmuls	48(%eax)
	flds	56(%eax)
	fxch	%st(2)
	fmuls	52(%eax)
	fxch	%st(2)
	fmuls	8(%edx)
	fxch	%st(1)
	faddp	%st, %st(2)
	fxch	%st(2)
	fmuls	60(%eax)
	fxch	%st(1)
	faddp	%st, %st(2)
	faddp	%st, %st(1)
	fstps	12(%ecx)
	popl	%ebp
	ret
	.size	hmatMultiplyHVecByHMat, .-hmatMultiplyHVecByHMat
	.p2align 4,,15
.globl hmatTranspose
	.type	hmatTranspose, @function
hmatTranspose:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	16(%eax), %ecx
	movl	4(%eax), %edx
	movl	%edx, 16(%eax)
	movl	%ecx, 4(%eax)
	movl	8(%eax), %edx
	movl	32(%eax), %ecx
	movl	%ecx, 8(%eax)
	movl	%edx, 32(%eax)
	movl	48(%eax), %ecx
	movl	12(%eax), %edx
	movl	%edx, 48(%eax)
	movl	%ecx, 12(%eax)
	movl	24(%eax), %edx
	movl	36(%eax), %ecx
	movl	%ecx, 24(%eax)
	movl	%edx, 36(%eax)
	movl	52(%eax), %ecx
	movl	28(%eax), %edx
	movl	%edx, 52(%eax)
	movl	%ecx, 28(%eax)
	movl	44(%eax), %edx
	movl	56(%eax), %ecx
	movl	%ecx, 44(%eax)
	movl	%edx, 56(%eax)
	popl	%ebp
	ret
	.size	hmatTranspose, .-hmatTranspose
	.p2align 4,,15
.globl hmatCopyAndTranspose
	.type	hmatCopyAndTranspose, @function
hmatCopyAndTranspose:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movl	(%eax), %ecx
	movl	%ecx, (%edx)
	movl	4(%eax), %ecx
	movl	%ecx, 16(%edx)
	movl	8(%eax), %ecx
	movl	%ecx, 32(%edx)
	movl	12(%eax), %ecx
	movl	%ecx, 48(%edx)
	movl	16(%eax), %ecx
	movl	%ecx, 4(%edx)
	movl	20(%eax), %ecx
	movl	%ecx, 20(%edx)
	movl	24(%eax), %ecx
	movl	%ecx, 36(%edx)
	movl	28(%eax), %ecx
	movl	%ecx, 52(%edx)
	movl	32(%eax), %ecx
	movl	%ecx, 8(%edx)
	movl	36(%eax), %ecx
	movl	%ecx, 24(%edx)
	movl	40(%eax), %ecx
	movl	%ecx, 40(%edx)
	movl	44(%eax), %ecx
	movl	%ecx, 56(%edx)
	movl	48(%eax), %ecx
	movl	%ecx, 12(%edx)
	movl	52(%eax), %ecx
	movl	%ecx, 28(%edx)
	movl	56(%eax), %ecx
	movl	%ecx, 44(%edx)
	movl	60(%eax), %ecx
	movl	%ecx, 60(%edx)
	popl	%ebp
	ret
	.size	hmatCopyAndTranspose, .-hmatCopyAndTranspose
	.p2align 4,,15
.globl hmatMakeRotAboutZ
	.type	hmatMakeRotAboutZ, @function
hmatMakeRotAboutZ:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	16(%ebp), %ecx
	movl	8(%ebp), %eax
	movl	%ecx, %edx
	movl	12(%ebp), %ebx
	xorl	$-2147483648, %edx
	movl	%ebx, (%eax)
	movl	%edx, 16(%eax)
	movl	%ebx, 20(%eax)
	movl	$0x0, %edx
	movl	$0x3f800000, %ebx
	movl	%edx, 32(%eax)
	movl	%edx, 48(%eax)
	movl	%ecx, 4(%eax)
	movl	%edx, 36(%eax)
	movl	%edx, 52(%eax)
	movl	%edx, 8(%eax)
	movl	%edx, 24(%eax)
	movl	%ebx, 40(%eax)
	movl	%edx, 56(%eax)
	movl	%edx, 12(%eax)
	movl	%edx, 28(%eax)
	movl	%edx, 44(%eax)
	movl	%ebx, 60(%eax)
	popl	%ebx
	popl	%ebp
	ret
	.size	hmatMakeRotAboutZ, .-hmatMakeRotAboutZ
	.p2align 4,,15
.globl hmatMakeRotAboutX
	.type	hmatMakeRotAboutX, @function
hmatMakeRotAboutX:
	pushl	%ebp
	movl	$0x0, %edx
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	movl	$0x3f800000, %edi
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	16(%ebp), %ebx
	movl	12(%ebp), %esi
	movl	%ebx, %ecx
	movl	%esi, 20(%eax)
	xorl	$-2147483648, %ecx
	movl	%edi, (%eax)
	movl	%edx, 16(%eax)
	movl	%edx, 32(%eax)
	movl	%edx, 48(%eax)
	movl	%edx, 4(%eax)
	movl	%ecx, 36(%eax)
	movl	%edx, 52(%eax)
	movl	%edx, 8(%eax)
	movl	%ebx, 24(%eax)
	movl	%esi, 40(%eax)
	movl	%edx, 56(%eax)
	movl	%edx, 12(%eax)
	movl	%edx, 28(%eax)
	movl	%edx, 44(%eax)
	movl	%edi, 60(%eax)
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	hmatMakeRotAboutX, .-hmatMakeRotAboutX
	.p2align 4,,15
.globl hmatMakeRotAboutY
	.type	hmatMakeRotAboutY, @function
hmatMakeRotAboutY:
	pushl	%ebp
	movl	$0x0, %edx
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	$0x3f800000, %ebx
	movl	8(%ebp), %eax
	movl	16(%ebp), %ecx
	movl	12(%ebp), %esi
	movl	%ecx, 32(%eax)
	movl	%esi, (%eax)
	xorl	$-2147483648, %ecx
	movl	%edx, 16(%eax)
	movl	%edx, 48(%eax)
	movl	%edx, 4(%eax)
	movl	%ebx, 20(%eax)
	movl	%edx, 36(%eax)
	movl	%edx, 52(%eax)
	movl	%ecx, 8(%eax)
	movl	%edx, 24(%eax)
	movl	%esi, 40(%eax)
	movl	%edx, 56(%eax)
	movl	%edx, 12(%eax)
	movl	%edx, 28(%eax)
	movl	%edx, 44(%eax)
	movl	%ebx, 60(%eax)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.size	hmatMakeRotAboutY, .-hmatMakeRotAboutY
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC13:
	.string	"(%8f %8f %8f %8f)\n"
	.text
	.p2align 4,,15
.globl hmatPrintHMatrix
	.type	hmatPrintHMatrix, @function
hmatPrintHMatrix:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$48, %esp
	movl	8(%ebp), %ebx
	flds	48(%ebx)
	fstpl	24(%esp)
	flds	32(%ebx)
	fstpl	16(%esp)
	flds	16(%ebx)
	fstpl	8(%esp)
	flds	(%ebx)
	fstpl	(%esp)
	pushl	$.LC13
	call	printf
	popl	%ecx
	flds	52(%ebx)
	fstpl	24(%esp)
	flds	36(%ebx)
	fstpl	16(%esp)
	flds	20(%ebx)
	fstpl	8(%esp)
	flds	4(%ebx)
	fstpl	(%esp)
	pushl	$.LC13
	call	printf
	popl	%edx
	flds	56(%ebx)
	fstpl	24(%esp)
	flds	40(%ebx)
	fstpl	16(%esp)
	flds	24(%ebx)
	fstpl	8(%esp)
	flds	8(%ebx)
	fstpl	(%esp)
	pushl	$.LC13
	call	printf
	flds	60(%ebx)
	popl	%eax
	fstpl	24(%esp)
	flds	44(%ebx)
	fstpl	16(%esp)
	flds	28(%ebx)
	fstpl	8(%esp)
	flds	12(%ebx)
	fstpl	(%esp)
	pushl	$.LC13
	call	printf
	movl	-4(%ebp), %ebx
	leave
	ret
	.size	hmatPrintHMatrix, .-hmatPrintHMatrix
	.p2align 4,,15
.globl matGetMatFromHMat
	.type	matGetMatFromHMat, @function
matGetMatFromHMat:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %edx
	movl	8(%ebp), %ecx
	movl	(%edx), %eax
	movl	%eax, (%ecx)
	movl	4(%edx), %eax
	movl	%eax, 4(%ecx)
	movl	8(%edx), %eax
	movl	%eax, 8(%ecx)
	movl	16(%edx), %eax
	movl	%eax, 12(%ecx)
	movl	20(%edx), %eax
	movl	%eax, 16(%ecx)
	movl	24(%edx), %eax
	movl	%eax, 20(%ecx)
	movl	32(%edx), %eax
	movl	%eax, 24(%ecx)
	movl	36(%edx), %eax
	movl	%eax, 28(%ecx)
	movl	40(%edx), %eax
	movl	%eax, 32(%ecx)
	popl	%ebp
	ret
	.size	matGetMatFromHMat, .-matGetMatFromHMat
	.p2align 4,,15
.globl matCreateCoordSysFromHeading
	.type	matCreateCoordSysFromHeading, @function
matCreateCoordSysFromHeading:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	fldz
	fld1
	fxch	%st(1)
	subl	$52, %esp
	movl	12(%ebp), %edx
	movl	8(%ebp), %ebx
	fsts	-24(%ebp)
	fsts	-20(%ebp)
	fxch	%st(1)
	fsts	-16(%ebp)
	movl	(%edx), %ecx
	movl	%ecx, -40(%ebp)
	movl	4(%edx), %ecx
	flds	-40(%ebp)
	movl	%ecx, -36(%ebp)
	fucom	%st(2)
	fnstsw	%ax
	andb	$69, %ah
	movl	8(%edx), %ecx
	xorb	$64, %ah
	movl	%ecx, -32(%ebp)
	jne	.L29
	flds	-36(%ebp)
	fucom	%st(3)
	fnstsw	%ax
	fstp	%st(3)
	andb	$69, %ah
	xorb	$64, %ah
	jne	.L31
	flds	-32(%ebp)
	fucom	%st(2)
	fnstsw	%ax
	fstp	%st(2)
	andb	$69, %ah
	xorb	$64, %ah
	jne	.L32
	fxch	%st(2)
	fxch	%st(1)
	movl	$0x3df40269, %ecx
	movl	$0x3f7e2d2d, %edx
	movl	%ecx, -24(%ebp)
	movl	%edx, -16(%ebp)
	jmp	.L25
	.p2align 4,,7
.L29:
	fstp	%st(1)
	fstp	%st(1)
	flds	-36(%ebp)
.L30:
	flds	-32(%ebp)
.L25:
	flds	-16(%ebp)
	flds	-20(%ebp)
	fld	%st(3)
	fld	%st(3)
	fmul	%st(2), %st
	subl	$12, %esp
	fld	%st(4)
	fxch	%st(2)
	fmul	%st(4), %st
	fxch	%st(4)
	leal	-56(%ebp), %edx
	pushl	%edx
	fmul	%st(7), %st
	fxch	%st(4)
	fsubp	%st, %st(1)
	flds	-24(%ebp)
	fxch	%st(3)
	fmul	%st(7), %st
	fxch	%st(2)
	fmul	%st(3), %st
	fxch	%st(1)
	fsts	-56(%ebp)
	fxch	%st(3)
	fmul	%st(6), %st
	fxch	%st(1)
	fsubp	%st, %st(4)
	fsubrp	%st, %st(1)
	fld	%st(2)
	fsts	-52(%ebp)
	fld	%st(1)
	fsts	-48(%ebp)
	fxch	%st(2)
	fmul	%st(6), %st
	fxch	%st(2)
	fmul	%st(7), %st
	fxch	%st(1)
	fmulp	%st, %st(7)
	fxch	%st(3)
	fmul	%st(4), %st
	fxch	%st(4)
	fmul	%st(2), %st
	fxch	%st(4)
	fsubp	%st, %st(1)
	fxch	%st(1)
	fmulp	%st, %st(4)
	fxch	%st(1)
	fsubp	%st, %st(2)
	fxch	%st(2)
	fsubp	%st, %st(3)
	fxch	%st(1)
	fstps	-24(%ebp)
	fstps	-20(%ebp)
	fstps	-16(%ebp)
	call	vecNormalize
	leal	-24(%ebp), %ecx
	movl	%ecx, (%esp)
	call	vecNormalize
	movl	-24(%ebp), %edx
	movl	%edx, (%ebx)
	movl	-20(%ebp), %ecx
	movl	%ecx, 4(%ebx)
	movl	-16(%ebp), %edx
	movl	%edx, 8(%ebx)
	movl	-56(%ebp), %ecx
	movl	%ecx, 12(%ebx)
	movl	-52(%ebp), %edx
	movl	%edx, 16(%ebx)
	movl	-48(%ebp), %ecx
	movl	%ecx, 20(%ebx)
	movl	-40(%ebp), %edx
	movl	%edx, 24(%ebx)
	movl	-36(%ebp), %ecx
	movl	%ecx, 28(%ebx)
	movl	-32(%ebp), %edx
	movl	%edx, 32(%ebx)
	movl	-4(%ebp), %ebx
	leave
	ret
	.p2align 4,,7
.L31:
	fstp	%st(1)
	fxch	%st(1)
	jmp	.L30
	.p2align 4,,7
.L32:
	fxch	%st(2)
	fxch	%st(1)
	jmp	.L25
	.size	matCreateCoordSysFromHeading, .-matCreateCoordSysFromHeading
	.p2align 4,,15
.globl matCreateMatFromVecs
	.type	matCreateMatFromVecs, @function
matCreateMatFromVecs:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	8(%ebp), %edx
	movl	16(%ebp), %ebx
	movl	20(%ebp), %esi
	movl	(%ecx), %eax
	movl	%eax, (%edx)
	movl	4(%ecx), %eax
	movl	%eax, 4(%edx)
	movl	8(%ecx), %eax
	movl	%eax, 8(%edx)
	movl	(%ebx), %ecx
	movl	%ecx, 12(%edx)
	movl	4(%ebx), %ecx
	movl	%ecx, 16(%edx)
	movl	8(%ebx), %ecx
	movl	%ecx, 20(%edx)
	movl	(%esi), %ebx
	movl	%ebx, 24(%edx)
	movl	4(%esi), %ecx
	movl	%ecx, 28(%edx)
	movl	8(%esi), %ecx
	movl	%ecx, 32(%edx)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.size	matCreateMatFromVecs, .-matCreateMatFromVecs
	.p2align 4,,15
.globl matMultiplyMatByMat
	.type	matMultiplyMatByMat, @function
matMultiplyMatByMat:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %ecx
	movl	16(%ebp), %edx
	movl	8(%ebp), %ebx
#APP
	    pushl     %ebp
    movl      $-3, %eax
    jmp       mat_x_mat_l1
    .align    4
mat_x_mat_l2:
    fstps     32(%ebx, %eax, 4)
mat_x_mat_l1:
    flds      36(%ecx, %eax, 4)
    fmuls     8(%edx)
    flds      (%edx)
    fmuls     12(%ecx, %eax, 4)
    faddp     %st, %st(1)
    flds      4(%edx)
    fmuls     24(%ecx, %eax, 4)
    faddp     %st, %st(1)
    movl      12(%ecx, %eax, 4), %ebp
    movl      24(%ecx, %eax, 4), %esi
    flds      24(%ecx, %eax, 4)
    flds      24(%ecx, %eax, 4)
    flds      12(%ecx, %eax, 4)
    flds      12(%ecx, %eax, 4)
    flds      36(%ecx, %eax, 4)
    flds      36(%ecx, %eax, 4)
    fxch      %st(6)
    movl      36(%ecx, %eax, 4), %edi
    fstps     12(%ebx, %eax, 4)
    fmuls     20(%edx)
    fxch      %st(4)
    fmuls     16(%edx)
    faddp     %st, %st(4)
    fxch      %st(1)
    fmuls     12(%edx)
    faddp     %st, %st(3)
    fxch      %st(2)
    fstps     24(%ebx, %eax, 4)
    fmuls     28(%edx)
    fxch      %st(2)
    fmuls     32(%edx)
    faddp     %st, %st(2)
    fmuls     24(%edx)
    faddp     %st, %st(1)
    incl      %eax
    jne       mat_x_mat_l2
    fstps     32(%ebx, %eax, 4)
    popl      %ebp

#NO_APP
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	matMultiplyMatByMat, .-matMultiplyMatByMat
	.p2align 4,,15
.globl matMultiplyMatByVec
	.type	matMultiplyMatByVec, @function
matMultiplyMatByVec:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %edi
	movl	16(%ebp), %esi
	movl	8(%ebp), %ebx
#APP
	    flds    0*4(%esi)
    fmuls   (0+0*3)*4(%edi)
    flds    1*4(%esi)
    fmuls   (0+1*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (0+2*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    0*4(%esi)
    fmuls   (1+0*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    1*4(%esi)
    fmuls   (1+1*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (1+2*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(1)
    flds    0*4(%esi)
    fmuls   (2+0*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    1*4(%esi)
    fmuls   (2+1*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (2+2*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(1)
    fxch    %st(2)
    fstps   1*4(%ebx)
    fxch    %st(1)
    faddp   %st, %st(1)
    fxch    %st(1)
    fstps   0*4(%ebx)
    fstps   2*4(%ebx)

#NO_APP
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	matMultiplyMatByVec, .-matMultiplyMatByVec
	.p2align 4,,15
.globl matMultiplyVecByMat
	.type	matMultiplyVecByMat, @function
matMultiplyVecByMat:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %esi
	movl	16(%ebp), %edi
	movl	8(%ebp), %ebx
#APP
	    flds    0*4(%esi)
    fmuls   (0+0*3)*4(%edi)
    flds    1*4(%esi)
    fmuls   (1+0*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (2+0*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    0*4(%esi)
    fmuls   (0+1*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    1*4(%esi)
    fmuls   (1+1*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (2+1*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(1)
    flds    0*4(%esi)
    fmuls   (0+2*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(2)
    flds    1*4(%esi)
    fmuls   (1+2*3)*4(%edi)
    flds    2*4(%esi)
    fmuls   (2+2*3)*4(%edi)
    fxch    %st(1)
    faddp   %st, %st(1)
    fxch    %st(2)
    fstps   1*4(%ebx)
    fxch    %st(1)
    faddp   %st, %st(1)
    fxch    %st(1)
    fstps   0*4(%ebx)
    fstps   2*4(%ebx)

#NO_APP
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	matMultiplyVecByMat, .-matMultiplyVecByMat
	.p2align 4,,15
.globl matTranspose
	.type	matTranspose, @function
matTranspose:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	4(%eax), %ecx
	movl	12(%eax), %edx
	movl	%edx, 4(%eax)
	movl	%ecx, 12(%eax)
	movl	24(%eax), %edx
	movl	8(%eax), %ecx
	movl	%ecx, 24(%eax)
	movl	%edx, 8(%eax)
	movl	20(%eax), %ecx
	movl	28(%eax), %edx
	movl	%edx, 20(%eax)
	movl	%ecx, 28(%eax)
	popl	%ebp
	ret
	.size	matTranspose, .-matTranspose
	.p2align 4,,15
.globl matCopyAndTranspose
	.type	matCopyAndTranspose, @function
matCopyAndTranspose:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	movl	(%edx), %eax
	movl	%eax, (%ecx)
	movl	4(%edx), %eax
	movl	%eax, 12(%ecx)
	movl	8(%edx), %eax
	movl	%eax, 24(%ecx)
	movl	12(%edx), %eax
	movl	%eax, 4(%ecx)
	movl	16(%edx), %eax
	movl	%eax, 16(%ecx)
	movl	20(%edx), %eax
	movl	%eax, 28(%ecx)
	movl	24(%edx), %eax
	movl	%eax, 8(%ecx)
	movl	28(%edx), %eax
	movl	%eax, 20(%ecx)
	movl	32(%edx), %eax
	movl	%eax, 32(%ecx)
	popl	%ebp
	ret
	.size	matCopyAndTranspose, .-matCopyAndTranspose
	.p2align 4,,15
.globl matMakeRotAboutZ
	.type	matMakeRotAboutZ, @function
matMakeRotAboutZ:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	16(%ebp), %ecx
	movl	8(%ebp), %eax
	movl	%ecx, %edx
	movl	12(%ebp), %ebx
	xorl	$-2147483648, %edx
	movl	%ecx, 4(%eax)
	movl	%edx, 12(%eax)
	movl	$0x3f800000, %ecx
	movl	$0x0, %edx
	movl	%ebx, (%eax)
	movl	%edx, 24(%eax)
	movl	%ebx, 16(%eax)
	movl	%edx, 28(%eax)
	movl	%edx, 8(%eax)
	movl	%edx, 20(%eax)
	movl	%ecx, 32(%eax)
	popl	%ebx
	popl	%ebp
	ret
	.size	matMakeRotAboutZ, .-matMakeRotAboutZ
	.p2align 4,,15
.globl matMakeRotAboutX
	.type	matMakeRotAboutX, @function
matMakeRotAboutX:
	pushl	%ebp
	movl	$0x3f800000, %ecx
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	16(%ebp), %ebx
	movl	%ebx, %edx
	movl	12(%ebp), %esi
	xorl	$-2147483648, %edx
	movl	%ecx, (%eax)
	movl	$0x0, %ecx
	movl	%esi, 16(%eax)
	movl	%ecx, 12(%eax)
	movl	%ecx, 24(%eax)
	movl	%ecx, 4(%eax)
	movl	%edx, 28(%eax)
	movl	%ecx, 8(%eax)
	movl	%ebx, 20(%eax)
	movl	%esi, 32(%eax)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.size	matMakeRotAboutX, .-matMakeRotAboutX
	.p2align 4,,15
.globl matMakeRotAboutY
	.type	matMakeRotAboutY, @function
matMakeRotAboutY:
	pushl	%ebp
	movl	$0x0, %ecx
	movl	%esp, %ebp
	movl	$0x3f800000, %edx
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	16(%ebp), %ebx
	movl	12(%ebp), %esi
	movl	%ebx, 24(%eax)
	movl	%esi, (%eax)
	xorl	$-2147483648, %ebx
	movl	%ecx, 12(%eax)
	movl	%ecx, 4(%eax)
	movl	%edx, 16(%eax)
	movl	%ecx, 28(%eax)
	movl	%ebx, 8(%eax)
	movl	%ecx, 20(%eax)
	movl	%esi, 32(%eax)
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.size	matMakeRotAboutY, .-matMakeRotAboutY
	.section	.rodata.str1.1
.LC26:
	.string	"(%8f %8f %8f)\n"
	.text
	.p2align 4,,15
.globl matPrintmatrix
	.type	matPrintmatrix, @function
matPrintmatrix:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$32, %esp
	movl	8(%ebp), %ebx
	flds	24(%ebx)
	fstpl	16(%esp)
	flds	12(%ebx)
	fstpl	8(%esp)
	flds	(%ebx)
	fstpl	(%esp)
	pushl	$.LC26
	call	printf
	flds	28(%ebx)
	popl	%eax
	fstpl	16(%esp)
	flds	16(%ebx)
	fstpl	8(%esp)
	flds	4(%ebx)
	fstpl	(%esp)
	pushl	$.LC26
	call	printf
	flds	32(%ebx)
	popl	%eax
	fstpl	16(%esp)
	flds	20(%ebx)
	fstpl	8(%esp)
	flds	8(%ebx)
	fstpl	(%esp)
	pushl	$.LC26
	call	printf
	movl	-4(%ebp), %ebx
	leave
	ret
	.size	matPrintmatrix, .-matPrintmatrix
	.p2align 4,,15
.globl matCopyAndScale
	.type	matCopyAndScale, @function
matCopyAndScale:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$60, %esp
	movl	IdentityMatrix+4, %edx
	movl	IdentityMatrix, %esi
	movl	%edx, -68(%ebp)
	movl	%esi, -72(%ebp)
	movl	IdentityMatrix+16, %edx
	movl	IdentityMatrix+8, %edi
	movl	%edx, -56(%ebp)
	movl	IdentityMatrix+12, %esi
	movl	IdentityMatrix+28, %edx
	movl	12(%ebp), %ecx
	movl	%edi, -64(%ebp)
	movl	%esi, -60(%ebp)
	movl	%edx, -44(%ebp)
	movl	IdentityMatrix+20, %edi
	movl	IdentityMatrix+24, %esi
	movl	IdentityMatrix+32, %edx
	movl	%edx, -40(%ebp)
	movl	%edi, -52(%ebp)
	movl	%esi, -48(%ebp)
	movl	8(%ebp), %ebx
	flds	16(%ebp)
	flds	(%ecx)
	fmul	%st(1), %st
	leal	-72(%ebp), %edx
	fstps	(%ebx)
	flds	4(%ecx)
	fmul	%st(1), %st
	fstps	4(%ebx)
	flds	8(%ecx)
	fmul	%st(1), %st
	fstps	8(%ebx)
	flds	12(%ecx)
	fmul	%st(1), %st
	fstps	12(%ebx)
	flds	16(%ecx)
	fmul	%st(1), %st
	fstps	16(%ebx)
	flds	20(%ecx)
	fmul	%st(1), %st
	fstps	20(%ebx)
	flds	24(%ecx)
	fmul	%st(1), %st
	fstps	24(%ebx)
	flds	28(%ecx)
	fmul	%st(1), %st
	fstps	28(%ebx)
	flds	32(%ecx)
	fmul	%st(1), %st
	fstps	32(%ebx)
	flds	-72(%ebp)
	fmul	%st(1), %st
	fstps	-72(%ebp)
	flds	-56(%ebp)
	fmul	%st(1), %st
	fxch	%st(1)
	fmuls	-40(%ebp)
	fxch	%st(1)
	fstps	-56(%ebp)
	fstps	-40(%ebp)
#APP
	    pushl     %ebp
    movl      $-3, %eax
    jmp       mat_x_mat_l1
    .align    4
mat_x_mat_l2:
    fstps     32(%ebx, %eax, 4)
mat_x_mat_l1:
    flds      36(%ecx, %eax, 4)
    fmuls     8(%edx)
    flds      (%edx)
    fmuls     12(%ecx, %eax, 4)
    faddp     %st, %st(1)
    flds      4(%edx)
    fmuls     24(%ecx, %eax, 4)
    faddp     %st, %st(1)
    movl      12(%ecx, %eax, 4), %ebp
    movl      24(%ecx, %eax, 4), %esi
    flds      24(%ecx, %eax, 4)
    flds      24(%ecx, %eax, 4)
    flds      12(%ecx, %eax, 4)
    flds      12(%ecx, %eax, 4)
    flds      36(%ecx, %eax, 4)
    flds      36(%ecx, %eax, 4)
    fxch      %st(6)
    movl      36(%ecx, %eax, 4), %edi
    fstps     12(%ebx, %eax, 4)
    fmuls     20(%edx)
    fxch      %st(4)
    fmuls     16(%edx)
    faddp     %st, %st(4)
    fxch      %st(1)
    fmuls     12(%edx)
    faddp     %st, %st(3)
    fxch      %st(2)
    fstps     24(%ebx, %eax, 4)
    fmuls     28(%edx)
    fxch      %st(2)
    fmuls     32(%edx)
    faddp     %st, %st(2)
    fmuls     24(%edx)
    faddp     %st, %st(1)
    incl      %eax
    jne       mat_x_mat_l2
    fstps     32(%ebx, %eax, 4)
    popl      %ebp

#NO_APP
	addl	$60, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	matCopyAndScale, .-matCopyAndScale
	.ident	"GCC: (GNU) 3.3 (Mandrake Linux 9.2 3.3-2mdk)"
