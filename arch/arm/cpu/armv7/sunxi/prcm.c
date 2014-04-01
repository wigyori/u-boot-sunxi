/*
 * Sunxi A31 Power Management Unit
 *
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 * http://linux-sunxi.org
 *
 * Based on sun6i sources and earlier U-Boot Allwiner A10 SPL work
 *
 * (C) Copyright 2006-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/prcm.h>
#include <asm/arch/sys_proto.h>

void prcm_init_apb0(void)
{
	struct sunxi_prcm_reg *prcm =
		(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;
	u32 reg_val;

	reg_val = readl(&prcm->apb0_gate);
	reg_val |= PRCM_APB0_GATE_P2WI | PRCM_APB0_GATE_PIO;
	writel(reg_val, &prcm->apb0_gate);

	reg_val = readl(&prcm->apb0_reset);
	reg_val |= PRCM_APB0_RESET_P2WI | PRCM_APB0_RESET_PIO;
	writel(reg_val, &prcm->apb0_reset);
}
