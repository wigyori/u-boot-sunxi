/*
 * (C) Copyright 2012-2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_COMMON_CONFIG_H
#define _SUNXI_COMMON_CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SUNXI		/* sunxi family */
#ifdef CONFIG_SPL_BUILD
#ifndef CONFIG_SPL_FEL
#define CONFIG_SYS_THUMB_BUILD	/* Thumbs mode to save space in SPL */
#endif
#endif

#include <asm/arch/cpu.h>	/* get chip and board defs */

#define CONFIG_SYS_TEXT_BASE		0x4a000000

/*
 * Display CPU information
 */
#define CONFIG_DISPLAY_CPUINFO

/* Serial & console */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
/* ns16550 reg in the low bits of cpu reg */
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(24000000)
#define CONFIG_SYS_NS16550_COM1		SUNXI_UART0_BASE
#define CONFIG_SYS_NS16550_COM2		SUNXI_UART1_BASE
#define CONFIG_SYS_NS16550_COM3		SUNXI_UART2_BASE
#define CONFIG_SYS_NS16550_COM4		SUNXI_UART3_BASE

/* DRAM Base */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* 32 KiB */

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_0			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_0_SIZE		0x80000000 /* 2 GiB */

#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_SETEXPR

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING

/* mmc config */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_MMC_SUNXI
#define CONFIG_MMC_SUNXI_SLOT		0
#define CONFIG_MMC_SUNXI_USE_DMA
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0	/* first detected MMC controller */

/*
 * Size of malloc() pool
 * 1MB = 0x100000, 0x100000 = 1024 * 1024
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (1 << 20))

/* Flat Device Tree (FDT/DT) support */
#define CONFIG_OF_LIBFDT
#define CONFIG_SYS_BOOTMAPSZ		(256 << 20)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER	/* use "hush" command parser    */
#define CONFIG_CMD_ECHO
#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		0x50000000 /* default load address */

/* standalone support */
#define CONFIG_STANDALONE_LOAD_ADDR	0x50000000

#define CONFIG_SYS_HZ			1000

/* baudrate */
#define CONFIG_BAUDRATE			115200

/* The stack sizes are set up in start.S using the settings below */
#define CONFIG_STACKSIZE		(256 << 10)	/* 256 KiB */

/* FLASH and environment organization */

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* 512 KiB */
#define CONFIG_IDENT_STRING		" Allwinner Technology"

#define CONFIG_ENV_OFFSET		(544 << 10) /* (8 + 24 + 512) KiB */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */

#define CONFIG_BOOTDELAY	3
#define CONFIG_SYS_BOOT_GET_CMDLINE
#define CONFIG_AUTO_COMPLETE

#include <config_cmd_default.h>

/* Accept zimage + raw ramdisk without mkimage headers */
#define CONFIG_CMD_BOOTZ
#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT		/* with this we can access fat bootfs */
#define CONFIG_FAT_WRITE	/* enable write access */
#define CONFIG_CMD_EXT2		/* with this we can access ext2 bootfs */
#define CONFIG_CMD_EXT4		/* with this we can access ext4 bootfs */

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#ifdef CONFIG_SPL_FEL

#define CONFIG_SPL
#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv7/sunxi/u-boot-spl-fel.lds"
#define CONFIG_SPL_START_S_PATH "arch/arm/cpu/armv7/sunxi"
#define CONFIG_SPL_TEXT_BASE		0x2000
#define CONFIG_SPL_MAX_SIZE		0x4000		/* 16 KiB */

#else /* CONFIG_SPL */

#define CONFIG_SPL_BSS_START_ADDR	0x50000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KiB */

#define CONFIG_SPL_TEXT_BASE		0x20		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x5fe0		/* 24KB on sun4i/sun7i */

#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT

#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv7/sunxi/u-boot-spl.lds"

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	80	/* 40KiB */
#define CONFIG_SPL_PAD_TO		32768		/* decimal for 'dd' */

#endif /* CONFIG_SPL */

/* end of 32 KiB in sram */
#define LOW_LEVEL_SRAM_STACK		0x00008000
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

/* I2C */
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_HARD_I2C
#define CONFIG_SUNXI_I2C
#define CONFIG_SYS_I2C_SLAVE		0x7f
#define CONFIG_CMD_I2C

/* PMU */
#if defined CONFIG_AXP152_POWER || defined CONFIG_AXP209_POWER
#define CONFIG_SPL_POWER_SUPPORT
#endif

#ifndef CONFIG_CONS_INDEX
#define CONFIG_CONS_INDEX              1       /* UART0 */
#endif

#if !defined CONFIG_ENV_IS_IN_MMC && \
    !defined CONFIG_ENV_IS_IN_NAND && \
    !defined CONFIG_ENV_IS_IN_FAT && \
    !defined CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_NOWHERE
#endif

#endif /* _SUNXI_COMMON_CONFIG_H */
