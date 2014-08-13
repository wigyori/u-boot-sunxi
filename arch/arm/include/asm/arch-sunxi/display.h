/*
 * Sunxi platform display cntroller register and constant defines
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DISPLAY_H
#define _SUNXI_DISPLAY_H

struct sunxi_de_be_reg {
	u8 res0[0x800];		/* 0x000 */
	u32 mode;		/* 0x800 */
	u32 backcolor;		/* 0x804 */
	u32 disp_size;		/* 0x808 */
	u8 res1[0x4];		/* 0x80c */
	u32 layer0_size;	/* 0x810 */
	u32 layer1_size;	/* 0x814 */
	u32 layer2_size;	/* 0x818 */
	u32 layer3_size;	/* 0x81c */
	u32 layer0_pos;		/* 0x820 */
	u32 layer1_pos;		/* 0x824 */
	u32 layer2_pos;		/* 0x828 */
	u32 layer3_pos;		/* 0x82c */
	u8 res2[0x10];		/* 0x830 */
	u32 layer0_stride;	/* 0x840 */
	u32 layer1_stride;	/* 0x844 */
	u32 layer2_stride;	/* 0x848 */
	u32 layer3_stride;	/* 0x84c */
	u32 layer0_addr_low32b;	/* 0x850 */
	u32 layer1_addr_low32b;	/* 0x854 */
	u32 layer2_addr_low32b;	/* 0x858 */
	u32 layer3_addr_low32b;	/* 0x85c */
	u32 layer0_addr_high4b;	/* 0x860 */
	u32 layer1_addr_high4b;	/* 0x864 */
	u32 layer2_addr_high4b;	/* 0x868 */
	u32 layer3_addr_high4b;	/* 0x86c */
	u32 reg_ctrl;		/* 0x870 */
	u8 res3[0xc];		/* 0x874 */
	u32 color_key_max;	/* 0x880 */
	u32 color_key_min;	/* 0x884 */
	u32 color_key_config;	/* 0x888 */
	u8 res4[0x4];		/* 0x88c */
	u32 layer0_attr0_ctrl;	/* 0x890 */
	u32 layer1_attr0_ctrl;	/* 0x894 */
	u32 layer2_attr0_ctrl;	/* 0x898 */
	u32 layer3_attr0_ctrl;	/* 0x89c */
	u32 layer0_attr1_ctrl;	/* 0x8a0 */
	u32 layer1_attr1_ctrl;	/* 0x8a4 */
	u32 layer2_attr1_ctrl;	/* 0x8a8 */
	u32 layer3_attr1_ctrl;	/* 0x8ac */
};

struct sunxi_tcon_reg {
	u8 res0[0x04];		/* 0x00 */
};

struct sunxi_hdmi_reg {
	u8 res0[0x04];		/* 0x00 */
};

/*
 * DE-BE register constants.
 */
#define SUNXI_DE_BE_MODE_ENABLE			(1 << 0)
#define SUNXI_DE_BE_MODE_START			(1 << 1)
#define SUNXI_DE_BE_MODE_LAYER0_ENABLE		(1 << 8)
#define SUNXI_DE_BE_WIDTH(x)			(((x) - 1) << 0)
#define SUNXI_DE_BE_HEIGHT(y)			(((y) - 1) << 16)
#define SUNXI_DE_BE_LAYER_STRIDE(x)		((x) << 5)
#define SUNXI_DE_BE_REG_CTRL_LOAD_REGS		(1 << 0)
#define SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888	(0x09 << 8)

/*
 * HDMI register constants.
 */
#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PAD_CTRL0_HDP	0x7e80000f
#define SUNXI_HDMI_PAD_CTRL0_RUN	0x7e8000ff
#else
#define SUNXI_HDMI_PAD_CTRL0_HDP	0xfe800000
#define SUNXI_HDMI_PAD_CTRL0_RUN	0xfe800000
#endif

#ifdef CONFIG_MACH_SUN4I
#define SUNXI_HDMI_PAD_CTRL1		0x00d8c820
#elif defined CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PAD_CTRL1		0x01ded030
#else
#define SUNXI_HDMI_PAD_CTRL1		0x00d8c830
#endif

#ifdef CONFIG_MACH_SUN6I
#define SUNXI_HDMI_PLL_CTRL		0xba48a308
#else
#define SUNXI_HDMI_PLL_CTRL		0xfa4ef708
#endif

#define SUNXI_HDMI_PLL_DBG0_PLL3	0
#define SUNXI_HDMI_PLL_DBG0_PLL7	0x00200000

#ifdef CONFIG_VIDEO_DT_SIMPLEFB
void sunxi_simplefb_setup(void *blob);
#endif

#endif /* _SUNXI_DISPLAY_H */
