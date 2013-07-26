/*
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/p2wi.h>
#include <axp221.h>

static u8 axp221_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp221_set_dcdc1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1600, 3400, 100);

	ret = p2wi_write(AXP221_DCDC1_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL2, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 7;
	return p2wi_write(AXP221_OUTPUT_CTRL2, cfg);
}

int axp221_set_dcdc2(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	return p2wi_write(AXP221_DCDC2_CTRL, cfg);
}

int axp221_set_dcdc3(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1860, 20);

	return p2wi_write(AXP221_DCDC3_CTRL, cfg);
}

int axp221_set_dcdc4(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	return p2wi_write(AXP221_DCDC4_CTRL, cfg);
}

int axp221_set_dcdc5(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1000, 2550, 50);

	return p2wi_write(AXP221_DCDC5_CTRL, cfg);
}

int axp221_set_dldo1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_DLDO1_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL2, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 3;
	return p2wi_write(AXP221_OUTPUT_CTRL2, cfg);
}

int axp221_set_dldo2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_DLDO2_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL2, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 4;
	return p2wi_write(AXP221_OUTPUT_CTRL2, cfg);
}

int axp221_set_dldo3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_DLDO3_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL2, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 5;
	return p2wi_write(AXP221_OUTPUT_CTRL2, cfg);
}

int axp221_set_dldo4(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_DLDO4_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL2, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 6;
	return p2wi_write(AXP221_OUTPUT_CTRL2, cfg);
}

int axp221_set_aldo1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_ALDO1_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL1, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 6;
	return p2wi_write(AXP221_OUTPUT_CTRL1, cfg);
}

int axp221_set_aldo2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_ALDO2_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL1, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 7;
	return p2wi_write(AXP221_OUTPUT_CTRL1, cfg);
}

int axp221_set_aldo3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = p2wi_write(AXP221_ALDO3_CTRL, cfg);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_OUTPUT_CTRL3, &cfg);
	if (ret)
		return ret;

	cfg |= 1 << 7;
	return p2wi_write(AXP221_OUTPUT_CTRL3, cfg);
}

int axp221_init(void)
{
	u8 axp_chip_id;
	int ret;

	p2wi_init();
	ret = p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR, AXP221_CTRL_ADDR,
				       AXP221_INIT_DATA);
	if (ret)
		return ret;

	ret = p2wi_read(AXP221_CHIP_ID, &axp_chip_id);
	if (ret)
		return ret;

	if (!(axp_chip_id == 0x6 || axp_chip_id == 0x7 || axp_chip_id == 0x17))
		return -ENODEV;

	return 0;
}
