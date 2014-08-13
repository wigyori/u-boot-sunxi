/*
 * Display driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/display.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/fb.h>
#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

struct sunxi_display {
	GraphicDevice graphic_device[1];
	int enabled;
} sunxi_display[1];

/*
 * Convenience functions to ease readability, and to provide an easy
 * comparison with the sunxi kms driver.
 */
static unsigned int
sunxi_io_read(void *base, int offset)
{
	return readl(base + offset);
}

static void
sunxi_io_write(void *base, int offset, unsigned int value)
{
	writel(value, base + offset);
}

static void
sunxi_io_mask(void *base, int offset, unsigned int value, unsigned int mask)
{
	unsigned int tmp = readl(base + offset);

	tmp &= ~mask;
	tmp |= value & mask;

	writel(tmp, base + offset);
}

/*
 * LCDC, what allwinner calls a CRTC, so timing controller and serializer.
 */
#define SUNXI_LCDC_ENABLE		0x000
#define SUNXI_LCDC_INT0			0x004
#define SUNXI_LCDC_INT1			0x008
#define SUNXI_LCDC_TCON0_DOTCLOCK	0x044
#define SUNXI_LCDC_TCON0_IO_TRI		0x08c
#define SUNXI_LCDC_TCON1_ENABLE		0x090
#define SUNXI_LCDC_TCON1_TIMING_SRC	0x094
#define SUNXI_LCDC_TCON1_TIMING_SCALE	0x098
#define SUNXI_LCDC_TCON1_TIMING_OUT	0x09c
#define SUNXI_LCDC_TCON1_TIMING_H	0x0a0
#define SUNXI_LCDC_TCON1_TIMING_V	0x0a4
#define SUNXI_LCDC_TCON1_TIMING_SYNC	0x0a8
#define SUNXI_LCDC_TCON1_IO_TRI		0x0f4

/*
 * HDMI regs.
 */
#define SUNXI_HDMI_CTRL			0x004
#define SUNXI_HDMI_INT_CTRL		0x008
#define SUNXI_HDMI_HPD			0x00c
#define SUNXI_HDMI_VIDEO_CTRL		0x010
#define SUNXI_HDMI_VIDEO_SIZE		0x014
#define SUNXI_HDMI_VIDEO_BP		0x018
#define SUNXI_HDMI_VIDEO_FP		0x01c
#define SUNXI_HDMI_VIDEO_SPW		0x020
#define SUNXI_HDMI_VIDEO_POLARITY	0x024
#define SUNXI_HDMI_TX_DRIVER0		0x200
#define SUNXI_HDMI_TX_DRIVER1		0x204
#define SUNXI_HDMI_TX_DRIVER2		0x208
#define SUNXI_HDMI_TX_DRIVER3		0x20C

static int
sunxi_hdmi_hpd_detect(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	void *hdmi = (void *) SUNXI_HDMI_BASE;

	/* Set pll3 to 300MHz */
	clock_set_pll3(300000000);

	/* Set hdmi parent to pll3 */
	clrsetbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_PLL_MASK,
			CCM_HDMI_CTRL_PLL3);

	/* Set ahb gating to pass */
#ifdef CONFIG_MACH_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);

	/* Clock on */
	setbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);

	sunxi_io_write(hdmi, SUNXI_HDMI_CTRL, 0x80000000);
	sunxi_io_write(hdmi, SUNXI_HDMI_TX_DRIVER0, SUNXI_HDMI_PAD_CTRL0_HDP);

	udelay(1000);

	if (sunxi_io_read(hdmi, SUNXI_HDMI_HPD) & 0x01)
		return 1;

	/* No need to keep these running */
	sunxi_io_write(hdmi, SUNXI_HDMI_CTRL, 0);
	clrbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);
	clrbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);
#ifdef CONFIG_MACH_SUN6I
	clrbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	clock_set_pll3(0);

	return 0;
}

/*
 * This is the entity that mixes and matches the different layers and inputs.
 * Allwinner calls it the back-end, but i like composer better.
 */
static void
sunxi_composer_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
#ifdef CONFIG_MACH_SUN6I
	int pll = clock_get_pll6() * 2;
#else
	int pll = clock_get_pll5p();
#endif
	int i, div = 1;

	while ((pll / div) > 300000000)
		div++;

#ifdef CONFIG_MACH_SUN6I
	/* AHB reset off */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DE_BE0);
#endif

	/* AHB clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DE_BE0);
	setbits_le32(&ccm->dram_clk_gate, 1 << CCM_DRAM_GATE_OFFSET_DE_BE0);

	/* Set clock source to 2 (pll5p (sun4i) / pll6x2 (sun6i)), set div,
	   clear reset and enable */
	writel(CCM_DE_BE0_CTRL_GATE | CCM_DE_BE0_CTRL_RST |
	       CCM_DE_BE0_CTRL_PLL(2) | CCM_DE_BE0_CTRL_M(div),
	       &ccm->be0_clk_cfg);

	/* Engine bug, clear registers after reset */
	for (i = 0x0800; i < 0x1000; i += 4)
		writel(0, SUNXI_DE_BE0_BASE + i);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_ENABLE);
}

static void
sunxi_composer_mode_set(struct fb_videomode *mode, unsigned int address)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;

	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->disp_size);
	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->layer0_size);
	writel(SUNXI_DE_BE_LAYER_STRIDE(mode->xres), &de_be->layer0_stride);
	writel(address << 3, &de_be->layer0_addr_low32b);
	writel(address >> 29, &de_be->layer0_addr_high4b);
	writel(SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888, &de_be->layer0_attr1_ctrl);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_LAYER0_ENABLE);
}

static void
sunxi_lcdc_pll_set(int dotclock, int *clk_div, int *clk_double)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int value, n, m, diff;
	int best_n = 0, best_m = 0, best_diff = 0x0FFFFFFF;
	int best_double = 0;

	/*
	 * Find the lowest divider resulting in a matching clock, if there
	 * is no match, pick the closest lower clock, as monitors tend to
	 * not sync to higher frequencies.
	 */
	for (m = 15; m > 0; m--) {
		n = (m * dotclock) / 3000;

		if ((n >= 9) && (n <= 127)) {
			value = (3000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 0;
			}
		}

		/* These are just duplicates */
		if (!(m & 1))
			continue;

		n = (m * dotclock) / 6000;
		if ((n >= 9) && (n <= 127)) {
			value = (6000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 1;
			}
		}
	}

#if 1
	printf("dotclock: %dkHz = %dkHz: (%d * 3MHz * %d) / %d\n",
	       dotclock, (best_double + 1) * 3000 * best_n / best_m,
	       best_double + 1, best_n, best_m);
#endif

	clock_set_pll3(best_n * 3000000);

	writel(CCM_LCD_CH1_CTRL_GATE |
	    (best_double ? CCM_LCD_CH1_CTRL_PLL3_2X : CCM_LCD_CH1_CTRL_PLL3) |
	    CCM_LCD_CH1_CTRL_M(best_m), &ccm->lcd0_ch1_clk_cfg);

	*clk_div = best_m;
	*clk_double = best_double;
}

static void
sunxi_lcdc_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	void *lcdc = (void *) SUNXI_LCD0_BASE;

	/* Reset off */
#ifdef CONFIG_MACH_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);
#else
	setbits_le32(&ccm->lcd0_ch0_clk_cfg, CCM_LCD_CH0_CTRL_RST);
#endif

	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);

	/* Init lcdc */
	sunxi_io_write(lcdc, SUNXI_LCDC_ENABLE, 0);
	sunxi_io_write(lcdc, SUNXI_LCDC_INT0, 0);
	sunxi_io_write(lcdc, SUNXI_LCDC_INT1, 0x20);

	/* Disable tcon0 dot clock (set the divider to 0) */
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON0_DOTCLOCK, 0xF0000000);

	/* Set all io lines to tristate */
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON0_IO_TRI, 0xffffffff);
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_IO_TRI, 0xffffffff);
}

#ifdef CONFIG_MACH_SUN6I
static void
sunxi_drc_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* On sun6i the drc must be clocked even when in pass-through mode */
	writel(0x80000003, &ccm->iep_drc0_clk_cfg);
	setbits_le32(&ccm->ahb_reset1_cfg, /* 1 << AHB_RESET_OFFSET_DRC0 */ 0x02001800);
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DRC0);
}
#endif

static void
sunxi_lcdc_mode_set(struct fb_videomode *mode, int *clk_div, int *clk_double)
{
	void *lcdc = (void *) SUNXI_LCD0_BASE;
	int total;

	/* Use tcon1 */
	sunxi_io_mask(lcdc, SUNXI_LCDC_ENABLE, 0x01, 0x01);

	/* Enabled, 0x1E start delay */
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_ENABLE, 0x800001E0);

	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_SRC,
		       ((mode->xres - 1) << 16) | (mode->yres - 1));
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_SCALE,
		       ((mode->xres - 1) << 16) | (mode->yres - 1));
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_OUT,
		       ((mode->xres - 1) << 16) | (mode->yres - 1));

	total = mode->left_margin + mode->xres + mode->right_margin +
		mode->hsync_len;
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_H,
		       ((total - 1) << 16) |
		       (mode->hsync_len + mode->left_margin - 1));

	total = mode->upper_margin + mode->yres + mode->lower_margin +
		mode->vsync_len;
	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_V,
		       ((total *  2) << 16) |
		       (mode->vsync_len + mode->upper_margin - 1));

	sunxi_io_write(lcdc, SUNXI_LCDC_TCON1_TIMING_SYNC,
		       ((mode->hsync_len - 1) << 16) | (mode->vsync_len - 1));

	sunxi_lcdc_pll_set(mode->pixclock, clk_div, clk_double);
}

static void
sunxi_hdmi_mode_set(struct fb_videomode *mode, int clk_div, int clk_double)
{
	void *hdmi = (void *) SUNXI_HDMI_BASE;
	int h, v;

	sunxi_io_write(hdmi, SUNXI_HDMI_INT_CTRL, 0xFFFFFFFF);
	sunxi_io_write(hdmi, SUNXI_HDMI_VIDEO_POLARITY, 0x03e00000);

	sunxi_io_write(hdmi, SUNXI_HDMI_TX_DRIVER0, SUNXI_HDMI_PAD_CTRL0_RUN);
	sunxi_io_write(hdmi, SUNXI_HDMI_TX_DRIVER1, SUNXI_HDMI_PAD_CTRL1);
	sunxi_io_write(hdmi, SUNXI_HDMI_TX_DRIVER2, SUNXI_HDMI_PLL_CTRL);
	sunxi_io_write(hdmi, SUNXI_HDMI_TX_DRIVER3, SUNXI_HDMI_PLL_DBG0_PLL3);

	/* Setup clk div and doubler */
#ifdef CONFIG_MACH_SUN6I
	sunxi_io_mask(hdmi, SUNXI_HDMI_TX_DRIVER2, (clk_div -1) << 4, 0xf0);
#else
	sunxi_io_mask(hdmi, SUNXI_HDMI_TX_DRIVER2, clk_div << 4, 0xf0);
#endif
	if (clk_double)
		sunxi_io_mask(hdmi, SUNXI_HDMI_TX_DRIVER1, 0, 0x40);
	else
		sunxi_io_mask(hdmi, SUNXI_HDMI_TX_DRIVER1, 0x40, 0x40);

	sunxi_io_write(hdmi, SUNXI_HDMI_VIDEO_SIZE,
		       ((mode->yres - 1) << 16) | (mode->xres - 1));

	h = mode->hsync_len + mode->left_margin;
	v = mode->vsync_len + mode->upper_margin;
	sunxi_io_write(hdmi, SUNXI_HDMI_VIDEO_BP, ((v - 1) << 16) | (h - 1));

	h = mode->right_margin;
	v = mode->lower_margin;
	sunxi_io_write(hdmi, SUNXI_HDMI_VIDEO_FP, ((v - 1) << 16) | (h - 1));

	h = mode->hsync_len;
	v = mode->vsync_len;
	sunxi_io_write(hdmi, SUNXI_HDMI_VIDEO_SPW, ((v - 1) << 16) | (h - 1));

	if (mode->sync & FB_SYNC_HOR_HIGH_ACT)
		sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_POLARITY, 0x01, 0x01);
	else
		sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_POLARITY, 0, 0x01);

	if (mode->sync & FB_SYNC_VERT_HIGH_ACT)
		sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_POLARITY, 0x02, 0x02);
	else
		sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_POLARITY, 0, 0x02);
}

static void
sunxi_engines_init(void)
{
	sunxi_composer_init();
	sunxi_lcdc_init();
#ifdef CONFIG_MACH_SUN6I
	sunxi_drc_init();
#endif
}

static void
sunxi_mode_set(struct fb_videomode *mode, unsigned int address)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
	void *lcdc = (void *) SUNXI_LCD0_BASE;
	void *hdmi = (void *) SUNXI_HDMI_BASE;
	int clk_div, clk_double;
	int retries = 3;

retry:
	sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_CTRL, 0, 0x80000000);
	sunxi_io_mask(lcdc, SUNXI_LCDC_ENABLE, 0, 0x80000000);
	clrbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_START);

	sunxi_composer_mode_set(mode, address);
	sunxi_lcdc_mode_set(mode, &clk_div, &clk_double);
	sunxi_hdmi_mode_set(mode, clk_div, clk_double);

	setbits_le32(&de_be->reg_ctrl, SUNXI_DE_BE_REG_CTRL_LOAD_REGS);
	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_START);

	udelay(1000000 / mode->refresh + 500);

	sunxi_io_mask(lcdc, SUNXI_LCDC_ENABLE, 0x80000000, 0x80000000);
	sunxi_io_mask(lcdc, SUNXI_LCDC_TCON1_IO_TRI, 0x00000000, 0x03000000);

	udelay(1000000 / mode->refresh + 500);

	sunxi_io_mask(hdmi, SUNXI_HDMI_VIDEO_CTRL, 0x80000000, 0x80000000);

	udelay(1000000 / mode->refresh + 500);

	/* Sometimes the display pipeline does not sync up properly, if
	   this happens the hdmi fifo underrun or overrun bits are set */
	if (sunxi_io_read(hdmi, SUNXI_HDMI_INT_CTRL) & 0x03) {
		if (retries--)
			goto retry;
		eprintf("HDMI fifo under or overrun\n");
	}
}

void *
video_hw_init(void)
{
	static GraphicDevice *graphic_device = sunxi_display->graphic_device;
#if 0
	/*
	 * Vesa standard 1024x768@60
	 * 65.0  1024 1032 1176 1344  768 771 777 806  -hsync -vsync
	 */
	struct fb_videomode mode = {
		.name = "1024x768",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 65000,
		.left_margin = 160,
		.right_margin = 24,
		.upper_margin = 29,
		.lower_margin = 3,
		.hsync_len = 136,
		.vsync_len = 6,
		.sync = 0,
		.vmode = 0,
		.flag = 0,
	};
#elif 0
	/* 1920x1080@60 */
	struct fb_videomode mode = {
		.name = "1920x1080",
		.refresh = 60,
		.xres = 1920,
		.yres = 1080,
		.pixclock = 148500,
		.left_margin = 88,
		.right_margin = 148,
		.upper_margin = 36,
		.lower_margin = 4,
		.hsync_len = 44,
		.vsync_len = 5,
		.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0,
	};
#else
	/* 1280x720@50 */
	struct fb_videomode mode = {
		.name = "1280x720",
		.refresh = 50,
		.xres = 1280,
		.yres = 720,
		.pixclock = 74250,
		.left_margin = 440,
		.right_margin = 220,
		.upper_margin = 20,
		.lower_margin = 5,
		.hsync_len = 40,
		.vsync_len = 5,
		.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0,
	};
#endif
	int ret;

	memset(sunxi_display, 0, sizeof(struct sunxi_display));

	printf("Reserved %dkB of RAM for Framebuffer.\n",
	       CONFIG_SUNXI_FB_SIZE >> 10);
	gd->fb_base = gd->ram_top;

	ret = sunxi_hdmi_hpd_detect();
	if (!ret)
		return NULL;

	printf("HDMI connected.\n");
	sunxi_display->enabled = 1;

	printf("Setting up a %s console.\n", mode.name);
	sunxi_engines_init();
	sunxi_mode_set(&mode, gd->fb_base - CONFIG_SYS_SDRAM_BASE);

	/*
	 * These are the only members of this structure that are used. All the
	 * others are driver specific. There is nothing to decribe pitch or
	 * stride, but we are lucky with our hw.
	 */
	graphic_device->frameAdrs = gd->fb_base;
	graphic_device->gdfIndex = GDF_32BIT_X888RGB;
	graphic_device->gdfBytesPP = 4;
	graphic_device->winSizeX = mode.xres;
	graphic_device->winSizeY = mode.yres;

	return graphic_device;
}
