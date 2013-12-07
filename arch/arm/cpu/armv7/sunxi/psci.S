/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Based on code by Carl van Schaik <carl@ok-labs.com>.
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
#include <asm/psci.h>
#include <asm/arch/cpu.h>

	.pushsection ._secure.text, "ax"

	.arch_extension sec

#define	TEN_MS			(10 * CONFIG_SYS_CLK_FREQ / 1000)

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	adr	r0, _target_pc
	str	r2, [r0]
	dsb

	movw	r0, #(SUNXI_CPUCFG_BASE & 0xffff)
	movt	r0, #(SUNXI_CPUCFG_BASE >> 16)

	@ CPU mask
	and	r1, r1, #3	@ only care about first cluster
	mov	r4, #1
	lsl	r4, r4, r1

	adr	r6, _sunxi_cpu_entry
	str	r6, [r0, #0x1a4] @ PRIVATE_REG (boot vector)

	@ Assert reset on target CPU
	mov	r6, #0
	lsl	r5, r1, #6	@ 64 bytes per CPU
	add	r5, r5, #0x40	@ Offset from base
	add	r5, r5, r0	@ CPU control block
	str	r6, [r5]	@ Reset CPU

	@ l1 invalidate
	ldr	r6, [r0, #0x184]
	bic	r6, r6, r4
	str	r6, [r0, #0x184]

	@ Lock CPU
	ldr	r6, [r0, #0x1e4]
	bic	r6, r6, r4
	str	r6, [r0, #0x1e4]

	@ Release power clamp
	movw	r6, #0x1ff
	movt	r6, #0
1:	lsrs	r6, r6, #1
	str	r6, [r0, #0x1b0]
	bne	1b

	@ Write CNTP_TVAL : 10ms @ 24MHz (240000 cycles)
	movw	r1, #(TEN_MS & 0xffff)
	movt	r1, #(TEN_MS >> 16)
	mcr	p15, 0, r1, c14, c2, 0
	isb
	@ Enable physical timer, mask interrupt
	mov	r1, #3
	mcr	p15, 0, r1, c14, c2, 1
	@ Poll physical timer until ISTATUS is on
1:	isb
	mrc	p15, 0, r1, c14, c2, 1
	ands	r1, r1, #4
	bne	1b
	@ Disable timer
	mov	r1, #0
	mcr	p15, 0, r1, c14, c2, 1
	isb

	@ Clear power gating
	ldr	r6, [r0, #0x1b4]
	bic	r6, r6, #1
	str	r6, [r0, #0x1b4]

	@ Deassert reset on target CPU
	mov	r6, #3
	str	r6, [r5]

	@ Unlock CPU
	ldr	r6, [r0, #0x1e4]
	orr	r6, r6, r4
	str	r6, [r0, #0x1e4]

	mov	r0, #ARM_PSCI_RET_SUCCESS	@ Return PSCI_RET_SUCCESS
	mov	pc, lr

_target_pc:
	.word	0

_sunxi_cpu_entry:
	@ Set SMP bit
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1
	isb

	bl	_nonsec_init

	adr	r0, _target_pc
	ldr	r0, [r0]
	b	_do_nonsec_entry

	.popsection
