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

	.text
	.globl function..is_accepting.1        # -- Begin function function..is_accepting.1
	.p2align 2
	.type function..is_accepting.1,@function
function..is_accepting.1:        # @function..is_accepting.1
	addi	t0, x10, -3
	sltiu	x30, t0, 1
	bnez	x30, .L1
	j .L2
.L0:
	mv	a0, x30
	ret
.L1:
	li	x30, 1
	j .L0
.L2:
	li	x30, 0
	j .L0
	.text
	.globl function..transition.1        # -- Begin function function..transition.1
	.p2align 2
	.type function..transition.1,@function
function..transition.1:        # @function..transition.1
	sltiu	x30, x10, 1
	bnez	x30, .L4
	j .L5
.L3:
	mv	a0, x30
	ret
.L4:
	addi	t0, x11, -97
	sltiu	x30, t0, 1
	bnez	x30, .L6
	j .L7
.L5:
	addi	t0, x10, -1
	sltiu	x30, t0, 1
	bnez	x30, .L8
	j .L9
.L6:
	li	x30, 1
	j .L3
.L7:
	li	x30, 0
	j .L3
.L8:
	addi	t0, x11, -98
	sltiu	x30, t0, 1
	bnez	x30, .L10
	j .L11
.L9:
	addi	t0, x10, -2
	sltiu	x30, t0, 1
	bnez	x30, .L14
	j .L15
.L10:
	li	x30, 2
	j .L3
.L11:
	addi	t0, x11, -97
	sltiu	x30, t0, 1
	beqz	x30, .L13
.L12:
	li	x30, 1
	j .L3
.L13:
	li	x30, 0
	j .L3
.L14:
	addi	t0, x11, -99
	sltiu	x30, t0, 1
	bnez	x30, .L16
	j .L17
.L15:
	li	x30, 3
	j .L3
.L16:
	li	x30, 3
	j .L3
.L17:
	addi	t0, x11, -97
	sltiu	x30, t0, 1
	beqz	x30, .L19
.L18:
	li	x30, 1
	j .L3
.L19:
	li	x30, 0
	j .L3
	.text
	.globl function..match_pattern.1        # -- Begin function function..match_pattern.1
	.p2align 2
	.type function..match_pattern.1,@function
function..match_pattern.1:        # @function..match_pattern.1
	addi	sp, sp, -288
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	addi	t1, sp, 48
	sd	t1, 128(sp)
	ld	t0, 128(sp)
	sd	a0, 280(sp)
	sd	ra, 272(sp)
	mv	a0, t0
	addi	a1, sp, 136
	li	a2, 80
	call	builtin_memcpy
	li	x20, 0
	li	x21, 0
	ld	a0, 280(sp)
	ld	ra, 272(sp)
	j .L22
.L20:
	mv	a0, x9
	ld	s1, 0(sp)
	ld	s2, 8(sp)
	ld	s3, 16(sp)
	ld	s4, 24(sp)
	ld	s5, 32(sp)
	addi	sp, sp, 288
	ret
.L21:
	ld	t0, 128(sp)
	slli	t1, x20, 3
	add x9, t0, t1
	ld	x18, 0(x9)
	sd	a0, 280(sp)
	sd	ra, 272(sp)
	mv	x10, x21
	mv	x11, x18
	call	function..transition.1
	mv	x19, a0
	addiw	x9, x20, 1
	mv	x20, x9
	mv	x21, x19
	ld	a0, 280(sp)
	ld	ra, 272(sp)
.L22:
	slt	x9, x20, x10
	bnez	x9, .L21
	j .L23
.L23:
	addi	t0, x21, -3
	sltiu	x9, t0, 1
	bnez	x9, .L25
	j .L26
.L24:
	mv	x9, x18
	j .L20
.L25:
	li	x18, 1
	j .L24
.L26:
	li	x18, 0
	j .L24
	.text
	.globl main        # -- Begin function main
	.p2align 2
	.type main,@function
main:        # @main
	addi	sp, sp, -272
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	addi	t1, sp, 32
	sd	t1, 192(sp)
	ld	t0, 192(sp)
	mv	x9, t0
	li	t1, 1
	sd	t1, 0(x9)
	addi	x18, x9, 8
	li	t1, 2
	sd	t1, 0(x18)
	addi	x9, x18, 8
	li	t1, 3
	sd	t1, 0(x9)
	addi	x18, x9, 8
	li	t1, 4
	sd	t1, 0(x18)
	addi	x19, x18, 8
	li	t1, 97
	sd	t1, 0(x19)
	addi	x9, x19, 8
	li	t1, 98
	sd	t1, 0(x9)
	addi	x18, x9, 8
	li	t1, 99
	sd	t1, 0(x18)
	addi	x9, x18, 8
	li	t1, 5
	sd	t1, 0(x9)
	addi	x18, x9, 8
	li	t1, 6
	sd	t1, 0(x18)
	addi	x9, x18, 8
	li	t1, 7
	sd	t1, 0(x9)
	ld	t0, 192(sp)
	sd	ra, 264(sp)
	addi	a0, sp, 112
	mv	a1, t0
	li	a2, 80
	call	builtin_memcpy
	addi	a0, sp, -152
	addi	a1, sp, 112
	li	a2, 80
	call	builtin_memcpy
	li	x10, 10
	call	function..match_pattern.1
	mv	x9, a0
	mv	x10, x9
	call	printlnInt
	ld	ra, 264(sp)
.L27:
	li	a0, 0
	ld	s1, 0(sp)
	ld	s2, 8(sp)
	ld	s3, 16(sp)
	addi	sp, sp, 272
	ret
