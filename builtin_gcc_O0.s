	.file	"builtin.c"
	.option nopic
	.attribute arch, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"%s"
	.text
	.align	1
	.globl	print
	.type	print, @function
print:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	sd	a0,-24(s0)
	ld	a1,-24(s0)
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	printf
	nop
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	print, .-print
	.align	1
	.globl	println
	.type	println, @function
println:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	sd	a0,-24(s0)
	ld	a0,-24(s0)
	call	puts
	nop
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	println, .-println
	.section	.rodata
	.align	3
.LC1:
	.string	"%d"
	.text
	.align	1
	.globl	printInt
	.type	printInt, @function
printInt:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	mv	a5,a0
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	mv	a1,a5
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	printf
	nop
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	printInt, .-printInt
	.section	.rodata
	.align	3
.LC2:
	.string	"%d\n"
	.text
	.align	1
	.globl	printlnInt
	.type	printlnInt, @function
printlnInt:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	mv	a5,a0
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	mv	a1,a5
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	printf
	nop
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	printlnInt, .-printlnInt
	.align	1
	.globl	getString
	.type	getString, @function
getString:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	li	a0,256
	call	malloc
	sd	a0,-24(s0)
	ld	a1,-24(s0)
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	scanf
	ld	a5,-24(s0)
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	getString, .-getString
	.align	1
	.globl	getInt
	.type	getInt, @function
getInt:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	addi	a5,s0,-20
	mv	a1,a5
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	scanf
	lw	a5,-20(s0)
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	getInt, .-getInt
	.align	1
	.globl	builtin_memset
	.type	builtin_memset, @function
builtin_memset:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	sd	a0,-24(s0)
	mv	a5,a1
	mv	a4,a2
	sw	a5,-28(s0)
	mv	a5,a4
	sw	a5,-32(s0)
	lw	a4,-32(s0)
	lw	a5,-28(s0)
	mv	a2,a4
	mv	a1,a5
	ld	a0,-24(s0)
	call	memset
	mv	a5,a0
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	builtin_memset, .-builtin_memset
	.align	1
	.globl	builtin_memcpy
	.type	builtin_memcpy, @function
builtin_memcpy:
	addi	sp,sp,-48
	sd	ra,40(sp)
	sd	s0,32(sp)
	addi	s0,sp,48
	sd	a0,-24(s0)
	sd	a1,-32(s0)
	mv	a5,a2
	sw	a5,-36(s0)
	lw	a5,-36(s0)
	mv	a2,a5
	ld	a1,-32(s0)
	ld	a0,-24(s0)
	call	memcpy
	mv	a5,a0
	mv	a0,a5
	ld	ra,40(sp)
	ld	s0,32(sp)
	addi	sp,sp,48
	jr	ra
	.size	builtin_memcpy, .-builtin_memcpy
	.ident	"GCC: (SiFive GCC 8.3.0-2020.04.1) 8.3.0"
