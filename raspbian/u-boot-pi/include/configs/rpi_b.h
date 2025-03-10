/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* Architecture, CPU, etc.*/
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_ARM1176
#define CONFIG_BCM2835
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_DCACHE_OFF
/*
 * 2835 is a SKU in a series for which the 2708 is the first or primary SoC,
 * so 2708 has historically been used rather than a dedicated 2835 ID.
 */
#define CONFIG_MACH_TYPE		MACH_TYPE_BCM2708

/* Memory layout */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_TEXT_BASE		0x00008000
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE
/*
 * The board really has 256M. However, the VC (VideoCore co-processor) shares
 * the RAM, and uses a configurable portion at the top. We tell U-Boot that a
 * smaller amount of RAM is present in order to avoid stomping on the area
 * the VC uses.
 */
#define CONFIG_SYS_SDRAM_SIZE		SZ_128M
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_SDRAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MALLOC_LEN		SZ_4M
#define CONFIG_SYS_MEMTEST_START	0x00100000
#define CONFIG_SYS_MEMTEST_END		0x00200000
#define CONFIG_LOADADDR			0x00200000

/* Flash */
#define CONFIG_SYS_NO_FLASH

/* Devices */
/* GPIO */
#define CONFIG_BCM2835_GPIO
/* LCD */
#define CONFIG_LCD
#define CONFIG_LCD_DT_SIMPLEFB
#define LCD_BPP				LCD_COLOR16
/*
 * Prevent allocation of RAM for FB; the real FB address is queried
 * dynamically from the VideoCore co-processor, and comes from RAM
 * not owned by the ARM CPU.
 */
#define CONFIG_FB_ADDR			0
#define CONFIG_VIDEO_BCM2835
#define CONFIG_SYS_WHITE_ON_BLACK

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_MMC_SDHCI_IO_ACCESSORS
#define CONFIG_BCM2835_SDHCI

/* Console UART */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK		3000000
#define CONFIG_PL01x_PORTS		{ (void *)0x20201000 }
#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200

/* Console configuration */
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +		\
					 sizeof(CONFIG_SYS_PROMPT) + 16)

/* Environment */
#define CONFIG_ENV_SIZE			SZ_16K
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_SYS_LOAD_ADDR		0x1000000
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_PREBOOT \
	"if load mmc 0:1 ${loadaddr} /uEnv.txt; then " \
		"env import -t ${loadaddr} ${filesize}; " \
	"fi"

#define ENV_DEVICE_SETTINGS \
	"stdin=serial,lcd\0" \
	"stdout=serial,lcd\0" \
	"stderr=serial,lcd\0"

/*
 * Memory layout for where various images get loaded by boot scripts:
 *
 * scriptaddr can be pretty much anywhere that doesn't conflict with something
 *   else. Put it low in memory to avoid conflicts.
 *
 * pxefile_addr_r can be pretty much anywhere that doesn't conflict with
 *   something else. Put it low in memory to avoid conflicts.
 *
 * kernel_addr_r must be within the first 128M of RAM in order for the
 *   kernel's CONFIG_AUTO_ZRELADDR option to work. Since the kernel will
 *   decompress itself to 0x8000 after the start of RAM, kernel_addr_r
 *   should not overlap that area, or the kernel will have to copy itself
 *   somewhere else before decompression. Similarly, the address of any other
 *   data passed to the kernel shouldn't overlap the start of RAM. Pushing
 *   this up to 16M allows for a sizable kernel to be decompressed below the
 *   compressed load address.
 *
 * fdt_addr_r simply shouldn't overlap anything else. Choosing 32M allows for
 *   the compressed kernel to be up to 16M too.
 *
 * ramdisk_addr_r simply shouldn't overlap anything else. Choosing 33M allows
 *   for the FDT/DTB to be up to 1M, which is hopefully plenty.
 */
#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00000000\0" \
	"pxefile_addr_r=0x00100000\0" \
	"kernel_addr_r=0x01000000\0" \
	"fdt_addr_r=0x02000000\0" \
	"fdtfile=bcm2835-rpi-b.dtb\0" \
	"ramdisk_addr_r=0x02100000\0" \

#define BOOTCMDS_MMC \
	"mmc_boot=" \
		"setenv devtype mmc; " \
		"if mmc dev ${devnum}; then " \
			"run scan_boot; " \
		"fi\0" \
	"bootcmd_mmc0=setenv devnum 0; run mmc_boot;\0"
#define BOOT_TARGETS_MMC "mmc0"

#define BOOTCMD_INIT_USB "run usb_init; "
#define BOOTCMDS_USB \
	"usb_init=" \
		"if ${usb_need_init}; then " \
			"set usb_need_init false; " \
			"usb start 0; " \
		"fi\0" \
	\
	"usb_boot=" \
		"setenv devtype usb; " \
		BOOTCMD_INIT_USB \
		"if usb dev ${devnum}; then " \
			"run scan_boot; " \
		"fi\0" \
	\
	"bootcmd_usb0=setenv devnum 0; run usb_boot;\0"
#define BOOT_TARGETS_USB "usb0"

#define BOOTCMDS_DHCP \
	"bootcmd_dhcp=" \
		BOOTCMD_INIT_USB \
		"if dhcp ${scriptaddr} boot.scr.uimg; then "\
			"source ${scriptaddr}; " \
		"fi\0"
#define BOOT_TARGETS_DHCP "dhcp"

#define BOOTCMDS_PXE \
	"bootcmd_pxe=" \
		BOOTCMD_INIT_USB \
		"dhcp; " \
		"if pxe get; then " \
			"pxe boot; " \
		"fi\0"
#define BOOT_TARGETS_PXE "pxe"

#define BOOTCMDS_COMMON \
	"dhcpuboot=usb start; dhcp u-boot.uimg; bootm\0" \
	"rootpart=1\0" \
	\
	"do_script_boot="                                                 \
		"load ${devtype} ${devnum}:${rootpart} "                  \
			"${scriptaddr} ${prefix}${script}; "              \
		"source ${scriptaddr}\0"                                  \
	\
	"script_boot="                                                    \
		"for script in ${boot_scripts}; do "                      \
			"if test -e ${devtype} ${devnum}:${rootpart} "    \
					"${prefix}${script}; then "       \
				"echo Found ${prefix}${script}; "         \
				"run do_script_boot; "                    \
				"echo SCRIPT FAILED: continuing...; "     \
			"fi; "                                            \
		"done\0"                                                  \
	\
	"do_sysboot_boot="                                                \
		"sysboot ${devtype} ${devnum}:${rootpart} any "           \
			"${scriptaddr} ${prefix}extlinux/extlinux.conf\0" \
	\
	"sysboot_boot="                                                   \
		"if test -e ${devtype} ${devnum}:${rootpart} "            \
				"${prefix}extlinux/extlinux.conf; then "  \
			"echo Found ${prefix}extlinux/extlinux.conf; "    \
			"run do_sysboot_boot; "                           \
			"echo SCRIPT FAILED: continuing...; "             \
		"fi\0"                                                    \
	\
	"scan_boot="                                                      \
		"echo Scanning ${devtype} ${devnum}...; "                 \
		"for prefix in ${boot_prefixes}; do "                     \
			"run sysboot_boot; "                              \
			"run script_boot; "                               \
		"done\0"                                                  \
	\
	"boot_targets=" \
		BOOT_TARGETS_MMC " " \
		BOOT_TARGETS_USB " " \
		BOOT_TARGETS_PXE " " \
		BOOT_TARGETS_DHCP " " \
		"\0" \
	\
	"boot_prefixes=/\0" \
	\
	"boot_scripts=boot.scr.uimg\0" \
	\
	BOOTCMDS_MMC \
	BOOTCMDS_USB \
	BOOTCMDS_DHCP \
	BOOTCMDS_PXE

#define CONFIG_BOOTCOMMAND \
	"set usb_need_init; " \
	"for target in ${boot_targets}; do run bootcmd_${target}; done"

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_DEVICE_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTCMDS_COMMON

#define CONFIG_BOOTDELAY 2

/* Shell */
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_COMMAND_HISTORY

/* Commands */
#include <config_cmd_default.h>
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_MMC
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART

/* Device tree support */
#define CONFIG_OF_BOARD_SETUP
/* ATAGs support for bootm/bootz */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

/* USB support */
#define CONFIG_USB_DWC2_OTG
#define CONFIG_USB_DWC2_REG_ADDR 0x20980000
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_STORAGE
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_CMD_USB
#define CONFIG_MISC_INIT_R

#include <config_distro_defaults.h>

/* Some things don't make sense on this HW or yet */
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SAVEENV

#endif
