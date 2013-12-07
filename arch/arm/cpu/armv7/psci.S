/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

	.arch_extension	sec

	.align	5
	.globl _psci_vectors
_psci_vectors:
	adr	pc, .		@ reset
	adr	pc, .		@ undef
	adr	pc, _smc_psci	@ smc
	adr	pc, .		@ pabort
	adr	pc, .		@ dabort
	adr	pc, .		@ hyp
	adr	pc, .		@ irq
	adr	pc, .		@ fiq

ENTRY(psci_cpu_suspend)
ENTRY(psci_cpu_off)
ENTRY(psci_cpu_on)
ENTRY(psci_migrate)
	mov	r0, #ARM_PSCI_RET_NI	@ Return -1 (Not Implemented)
	mov	pc, lr
ENDPROC(psci_migrate)
ENDPROC(psci_cpu_on)
ENDPROC(psci_cpu_off)
ENDPROC(psci_cpu_suspend)
.weak psci_cpu_suspend
.weak psci_cpu_off
.weak psci_cpu_on
.weak psci_migrate

_psci_table:
	.word	ARM_PSCI_FN_CPU_SUSPEND
	.word	psci_cpu_suspend
	.word	ARM_PSCI_FN_CPU_OFF
	.word	psci_cpu_off
	.word	ARM_PSCI_FN_CPU_ON
	.word	psci_cpu_on
	.word	ARM_PSCI_FN_MIGRATE
	.word	psci_migrate
	.word	0
	.word	0

_secure_stacks:			@ Enough to save 16 registers per CPU
	.skip	16*4*CONFIG_ARMV7_PSCI_NR_CPUS
_secure_stack_base:

_smc_psci:
	@ Switch to secure mode
	mrc	p15, 0, sp, c1, c1, 0
	bic	sp, sp, #1
	mcr	p15, 0, sp, c1, c1, 0

	adr	sp, _secure_stack_base
	mcr	p15, 0, r0, c13, c0, 4	@ use TPIDRPRW as a tmp reg
	mcr	p15, 0, r1, c13, c0, 3	@ use TPIDRURO as a tmp reg
	mrc	p15, 0, r0, c0, c0, 5	@ MPIDR
	and	r1, r0, #3		@ cpu number in cluster
	lsr	r0, r0, #8
	and	r0, r0, #3		@ cluster number
	mul	r1, r1, r0		@ absolute cpu nr
	sbc	sp, sp, r1, lsl #6	@ sp = sp_base - 64*cpunr

	mrc	p15, 0, r0, c13, c0, 4	@ restore r0
	mrc	p15, 0, r1, c13, c0, 3	@ restore r1

	push	{r4-r12,lr}

	adr	r4, _psci_table
1:	ldr	r5, [r4]	@ Load PSCI function ID
	ldr	r6, [r4, #4]	@ Load target PC
	cmp	r5, #0		@ If reach the end, bail out
	mvneq	r0, #0		@ Return -1 (Not Implemented)
	beq	2f
	cmp	r0, r5		@ If not matching, try next entry
	addne	r4, r4, #8
	bne	1b
	cmp	r6, #0		@ Not implemented
	moveq	r0, #ARM_PSCI_RET_NI
	beq	2f

	blx	r6		@ Execute PSCI function

2:	pop	{r4-r12, lr}

	@ Back to non-secure
	mrc	p15, 0, sp, c1, c1, 0
	orr	sp, sp, #1
	mcr	p15, 0, sp, c1, c1, 0
	movs	pc, lr		@ Return to the kernel

	.popsection
