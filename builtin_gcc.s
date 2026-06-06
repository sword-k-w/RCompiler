	.file	"builtin.c"
	.option nopic
	.attribute arch, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	print
	.type	print, @function
print:
	mv	a1,a0
	lui	a0,%hi(.LC0)
	addi	a0,a0,%lo(.LC0)
	tail	printf
	.size	print, .-print
	.align	1
	.globl	println
	.type	println, @function
println:
	tail	puts
	.size	println, .-println
	.align	1
	.globl	printInt
	.type	printInt, @function
printInt:
	mv	a1,a0
	lui	a0,%hi(.LC1)
	addi	a0,a0,%lo(.LC1)
	tail	printf
	.size	printInt, .-printInt
	.align	1
	.globl	printlnInt
	.type	printlnInt, @function
printlnInt:
	mv	a1,a0
	lui	a0,%hi(.LC2)
	addi	a0,a0,%lo(.LC2)
	tail	printf
	.size	printlnInt, .-printlnInt
	.align	1
	.globl	getString
	.type	getString, @function
getString:
	addi	sp,sp,-16
	li	a0,256
	sd	ra,8(sp)
	sd	s0,0(sp)
	call	malloc
	mv	s0,a0
	mv	a1,a0
	lui	a0,%hi(.LC0)
	addi	a0,a0,%lo(.LC0)
	call	scanf
	mv	a0,s0
	ld	ra,8(sp)
	ld	s0,0(sp)
	addi	sp,sp,16
	jr	ra
	.size	getString, .-getString
	.align	1
	.globl	getInt
	.type	getInt, @function
getInt:
	addi	sp,sp,-32
	lui	a0,%hi(.LC1)
	addi	a1,sp,12
	addi	a0,a0,%lo(.LC1)
	sd	ra,24(sp)
	call	scanf
	ld	ra,24(sp)
	lw	a0,12(sp)
	addi	sp,sp,32
	jr	ra
	.size	getInt, .-getInt
	.align	1
	.globl	builtin_memset
	.type	builtin_memset, @function
builtin_memset:
	tail	memset
	.size	builtin_memset, .-builtin_memset
	.align	1
	.globl	builtin_memcpy
	.type	builtin_memcpy, @function
builtin_memcpy:
	tail	memcpy
	.size	builtin_memcpy, .-builtin_memcpy
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"%s"
	.zero	5
.LC1:
	.string	"%d"
	.zero	5
.LC2:
	.string	"%d\n"
	.ident	"GCC: (SiFive GCC 8.3.0-2020.04.1) 8.3.0"
