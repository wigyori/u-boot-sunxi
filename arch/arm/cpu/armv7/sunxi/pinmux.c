/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

int sunxi_gpio_set_cfgpin(u32 pin, u32 val)
{
	u32 cfg;
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_CFG_INDEX(pin);
	u32 offset = GPIO_CFG_OFFSET(pin);
	struct sunxi_gpio *pio =
	    &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

	cfg = readl(&pio->cfg[0] + index);
	cfg &= ~(0xf << offset);
	cfg |= val << offset;

	writel(cfg, &pio->cfg[0] + index);

	return 0;
}

int sunxi_gpio_get_cfgpin(u32 pin)
{
	u32 cfg;
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_CFG_INDEX(pin);
	u32 offset = GPIO_CFG_OFFSET(pin);
	struct sunxi_gpio *pio =
	    &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

	cfg = readl(&pio->cfg[0] + index);
	cfg >>= offset;

	return cfg & 0xf;
}

int sunxi_gpio_set_drv(u32 pin, u32 val)
{
	u32 drv;
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_DRV_INDEX(pin);
	u32 offset = GPIO_DRV_OFFSET(pin);
	struct sunxi_gpio *pio =
	    &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

	drv = readl(&pio->drv[0] + index);
	drv &= ~(0x3 << offset);
	drv |= val << offset;

	writel(drv, &pio->drv[0] + index);

	return 0;
}

int sunxi_gpio_set_pull(u32 pin, u32 val)
{
	u32 pull;
	u32 bank = GPIO_BANK(pin);
	u32 index = GPIO_PULL_INDEX(pin);
	u32 offset = GPIO_PULL_OFFSET(pin);
	struct sunxi_gpio *pio =
	    &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

	pull = readl(&pio->pull[0] + index);
	pull &= ~(0x3 << offset);
	pull |= val << offset;

	writel(pull, &pio->pull[0] + index);

	return 0;
}
