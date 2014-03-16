/*
 * (C) Copyright 2012-2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some board init for the Allwinner A10-evb board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#ifdef CONFIG_AXP209_POWER
#include <axp209.h>
#endif
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/mmc.h>

DECLARE_GLOBAL_DATA_PTR;

/* add board specific code here */
int board_init(void)
{
	int id_pfr1;

	gd->bd->bi_boot_params = (PHYS_SDRAM_0 + 0x100);

	asm volatile("mrc p15, 0, %0, c0, c1, 1" : "=r"(id_pfr1));
	debug("id_pfr1: 0x%08x\n", id_pfr1);
	/* Generic Timer Extension available? */
	if ((id_pfr1 >> 16) & 0xf) {
		debug("Setting CNTFRQ\n");
		/* CNTFRQ == 24 MHz */
		asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(24000000));
	}

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_0, PHYS_SDRAM_0_SIZE);

	return 0;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT);
#if !defined (CONFIG_SPL_BUILD) && defined (CONFIG_MMC_SUNXI_SLOT_EXTRA)
	sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT_EXTRA);
#endif

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void sunxi_board_init(void)
{
	int power_failed = 0;
	unsigned long ramsize;

	printf("DRAM:");
	ramsize = sunxi_dram_init();
	printf(" %lu MiB\n", ramsize >> 20);
	if (!ramsize)
		hang();

#ifdef CONFIG_AXP209_POWER
	power_failed |= axp209_init();
	power_failed |= axp209_set_dcdc2(1400);
	power_failed |= axp209_set_dcdc3(1250);
	power_failed |= axp209_set_ldo2(3000);
	power_failed |= axp209_set_ldo3(2800);
	power_failed |= axp209_set_ldo4(2800);
#endif

	/*
	 * Only clock up the CPU to full speed if we are reasonably
	 * assured it's being powered with suitable core voltage
	 */
	if (!power_failed)
#ifdef CONFIG_SUN7I
		clock_set_pll1(912000000);
#else
		clock_set_pll1(1008000000);
#endif
	else
		printf("Failed to set core voltage! Can't set CPU frequency\n");
}
#endif
