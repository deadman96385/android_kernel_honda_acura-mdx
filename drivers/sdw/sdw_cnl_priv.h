/*
 *  sdw_cnl_priv.h - Private definition for intel master controller driver.
 *
 *  Copyright (C) 2014-2015 Intel Corp
 *  Author:  Hardik Shah  <hardik.t.shah@intel.com>
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */

#ifndef _LINUX_SDW_CNL_PRIV_H
#define _LINUX_SDW_CNL_PRIV_H

#define SDW_CNL_PM_TIMEOUT	3000 /* ms */
#define SDW_CNL_SLAVES_STAT_UPPER_DWORD_SHIFT 32
#define SDW_CNL_SLAVE_STATUS_BITS	4
#define SDW_CNL_CMD_WORD_LEN	4
#define SDW_CNL_DEFAULT_SSP_INTERVAL	0x18
#define SDW_CNL_DEFAULT_CLK_DIVIDER	0

#if IS_ENABLED(CONFIG_SND_SOC_INTEL_CNL_FPGA)
#define SDW_CNL_DEFAULT_SYNC_PERIOD	0x257F
#define SDW_CNL_DEFAULT_FRAME_SHAPE	0x48
#else
#define SDW_CNL_DEFAULT_SYNC_PERIOD	0x176F
#define SDW_CNL_DEFAULT_FRAME_SHAPE	0x30
#endif

#define SDW_CNL_PORT_REG_OFFSET		0x80
#define CNL_SDW_SCP_ADDR_REGS		0x2
#define SDW_CNL_PCM_PDI_NUM_OFFSET	0x2
#define SDW_CNL_PDM_PDI_NUM_OFFSET     0x6

#define SDW_CNL_CTMCTL_REG_OFFSET	0x60
#define SDW_CNL_IOCTL_REG_OFFSET	0x60
#define SDW_CNL_PCMSCAP_REG_OFFSET	0x60
#define SDW_CNL_PCMSCHC_REG_OFFSET	0x60
#define SDW_CNL_PDMSCAP_REG_OFFSET	0x60
#define SDW_CNL_PCMSCHM_REG_OFFSET	0x60
#define SDW_CNL_SNDWWAKEEN_REG_OFFSET   0x190
#define SDW_CNL_SNDWWAKESTS_REG_OFFSET   0x192


#define SDW_CNL_MCP_CONFIG			0x0
#define MCP_CONFIG_BRELENABLE_MASK		0x1
#define MCP_CONFIG_BRELENABLE_SHIFT		0x6
#define MCP_CONFIG_MAXCMDRETRY_SHIFT		24
#define MCP_CONFIG_MAXCMDRETRY_MASK		0xF
#define MCP_CONFIG_MAXPREQDELAY_SHIFT		16
#define MCP_CONFIG_MAXPREQDELAY_MASK		0x1F
#define MCP_CONFIG_MMMODEEN_SHIFT		0x7
#define MCP_CONFIG_MMMODEEN_MASK		0x1
#define MCP_CONFIG_SNIFFEREN_SHIFT		0x5
#define MCP_CONFIG_SNIFFEREN_MASK		0x1
#define MCP_CONFIG_SSPMODE_SHIFT		0x4
#define MCP_CONFIG_SSPMODE_MASK			0x1
#define MCP_CONFIG_CMDMODE_SHIFT		0x3
#define MCP_CONFIG_CMDMODE_MASK			0x1

#define MCP_CONFIG_OPERATIONMODE_MASK		0x7
#define MCP_CONFIG_OPERATIONMODE_SHIFT		0x0
#define MCP_CONFIG_OPERATIONMODE_NORMAL		0x0

#define SDW_CNL_MCP_CONTROL			0x4
#define MCP_CONTROL_RESETDELAY_SHIFT		0x8
#define MCP_CONTROL_CMDRST_SHIFT		0x7
#define MCP_CONTROL_CMDRST_MASK			0x1
#define MCP_CONTROL_SOFTRST_SHIFT		0x6
#define MCP_CONTROL_SOFTCTRLBUSRST_SHIFT	0x5
#define MCP_CONTROL_HARDCTRLBUSRST_MASK		0x1
#define MCP_CONTROL_HARDCTRLBUSRST_SHIFT	0x4
#define MCP_CONTROL_CLOCKPAUSEREQ_SHIFT		0x3
#define MCP_CONTROL_CLOCKSTOPCLEAR_SHIFT	0x2
#define MCP_CONTROL_CLOCKSTOPCLEAR_MASK		0x1
#define MCP_CONTROL_CMDACCEPTMODE_MASK		0x1
#define MCP_CONTROL_CMDACCEPTMODE_SHIFT		0x1
#define MCP_CONTROL_BLOCKWAKEUP_SHIFT		0x0
#define MCP_CONTROL_BLOCKWAKEUP_MASK		0x1


#define MCP_SLAVEINTMASK0_MASK			0xFFFFFFFF
#define MCP_SLAVEINTMASK1_MASK			0x0000FFFF

#define SDW_CNL_MCP_CMDCTRL			0x8
#define SDW_CNL_MCP_SSPSTAT			0xC
#define SDW_CNL_MCP_FRAMESHAPE			0x10
#define SDW_CNL_MCP_FRAMESHAPEINIT		0x14
#define SDW_CNL_MCP_CONFIGUPDATE		0x18
#define MCP_CONFIGUPDATE_CONFIGUPDATE_SHIFT	0x0
#define MCP_CONFIGUPDATE_CONFIGUPDATE_MASK	0x1

#define SDW_CNL_MCP_PHYCTRL			0x1C
#define SDW_CNL_MCP_SSPCTRL0			0x20
#define SDW_CNL_MCP_SSPCTRL1			0x28
#define SDW_CNL_MCP_CLOCKCTRL0			0x30
#define SDW_CNL_MCP_CLOCKCTRL1			0x38
#define SDW_CNL_MCP_STAT			0x40
#define SDW_CNL_MCP_INTSTAT			0x44
#define MCP_INTSTAT_IRQ_SHIFT			31
#define MCP_INTSTAT_IRQ_MASK			1
#define MCP_INTSTAT_WAKEUP_SHIFT		16
#define MCP_INTSTAT_SLAVE_STATUS_CHANGED_SHIFT	12
#define MCP_INTSTAT_SLAVE_STATUS_CHANGED_MASK	0xF
#define MCP_INTSTAT_SLAVENOTATTACHED_SHIFT	12
#define MCP_INTSTAT_SLAVEATTACHED_SHIFT		13
#define MCP_INTSTAT_SLAVEALERT_SHIFT		14
#define MCP_INTSTAT_SLAVERESERVED_SHIFT		15

#define MCP_INTSTAT_DPPDIINT_SHIFT		11
#define MCP_INTSTAT_DPPDIINTMASK		0x1
#define MCP_INTSTAT_CONTROLBUSCLASH_SHIFT	10
#define MCP_INTSTAT_CONTROLBUSCLASH_MASK	0x1
#define MCP_INTSTAT_DATABUSCLASH_SHIFT		9
#define MCP_INTSTAT_DATABUSCLASH_MASK		0x1
#define MCP_INTSTAT_CMDERR_SHIFT		7
#define MCP_INTSTAT_CMDERR_MASK			0x1
#define MCP_INTSTAT_TXE_SHIFT			1
#define MCP_INTSTAT_TXE_MASK			0x1
#define MCP_INTSTAT_RXWL_SHIFT			2
#define MCP_INTSTAT_RXWL_MASK			1

#define SDW_CNL_MCP_INTMASK			0x48
#define MCP_INTMASK_IRQEN_SHIFT			31
#define MCP_INTMASK_IRQEN_MASK			0x1
#define MCP_INTMASK_WAKEUP_SHIFT		16
#define MCP_INTMASK_WAKEUP_MASK			0x1
#define MCP_INTMASK_SLAVERESERVED_SHIFT		15
#define MCP_INTMASK_SLAVERESERVED_MASK		0x1
#define MCP_INTMASK_SLAVEALERT_SHIFT		14
#define MCP_INTMASK_SLAVEALERT_MASK		0x1
#define MCP_INTMASK_SLAVEATTACHED_SHIFT		13
#define MCP_INTMASK_SLAVEATTACHED_MASK		0x1
#define MCP_INTMASK_SLAVENOTATTACHED_SHIFT	12
#define MCP_INTMASK_SLAVENOTATTACHED_MASK	0x1
#define MCP_INTMASK_DPPDIINT_SHIFT		11
#define MCP_INTMASK_DPPDIINT_MASK		0x1
#define MCP_INTMASK_CONTROLBUSCLASH_SHIFT	10
#define MCP_INTMASK_CONTROLBUSCLASH_MASK	1
#define MCP_INTMASK_DATABUSCLASH_SHIFT		9
#define MCP_INTMASK_DATABUSCLASH_MASK		1
#define MCP_INTMASK_CMDERR_SHIFT		7
#define MCP_INTMASK_CMDERR_MASK			0x1
#define MCP_INTMASK_TXE_SHIFT			1
#define MCP_INTMASK_TXE_MASK			0x1
#define MCP_INTMASK_RXWL_SHIFT			2
#define MCP_INTMASK_RXWL_MASK			0x1

#define SDW_CNL_MCP_INTSET			0x4C
#define SDW_CNL_MCP_STAT			0x40
#define MCP_STAT_ACTIVE_BANK_MASK		0x1
#define MCP_STAT_ACTIVE_BANK_SHIT		20
#define MCP_STAT_CLOCKSTOPPED_MASK		0x1
#define MCP_STAT_CLOCKSTOPPED_SHIFT		16

#define SDW_CNL_MCP_SLAVESTAT			0x50
#define MCP_SLAVESTAT_MASK			0x3

#define SDW_CNL_MCP_SLAVEINTSTAT0		0x54
#define MCP_SLAVEINTSTAT_NOT_PRESENT_MASK	0x1
#define MCP_SLAVEINTSTAT_ATTACHED_MASK		0x2
#define MCP_SLAVEINTSTAT_ALERT_MASK		0x4
#define MCP_SLAVEINTSTAT_RESERVED_MASK		0x8

#define SDW_CNL_MCP_SLAVEINTSTAT1		0x58
#define SDW_CNL_MCP_SLAVEINTMASK0		0x5C
#define SDW_CNL_MCP_SLAVEINTMASK1		0x60
#define SDW_CNL_MCP_PORTINTSTAT			0x64
#define SDW_CNL_MCP_PDISTAT			0x6C

#define SDW_CNL_MCP_FIFOLEVEL			0x78
#define SDW_CNL_MCP_FIFOSTAT			0x7C
#define MCP_RX_FIFO_AVAIL_MASK			0x3F
#define SDW_CNL_MCP_COMMAND_BASE		0x80
#define SDW_CNL_MCP_RESPONSE_BASE		0x80
#define SDW_CNL_MCP_COMMAND_LENGTH		0x20

#define MCP_COMMAND_SSP_TAG_MASK		0x1
#define MCP_COMMAND_SSP_TAG_SHIFT		31
#define MCP_COMMAND_COMMAND_MASK		0x7
#define MCP_COMMAND_COMMAND_SHIFT		28
#define MCP_COMMAND_DEV_ADDR_MASK		0xF
#define MCP_COMMAND_DEV_ADDR_SHIFT		24
#define MCP_COMMAND_REG_ADDR_H_MASK		0x7
#define MCP_COMMAND_REG_ADDR_H_SHIFT		16
#define MCP_COMMAND_REG_ADDR_L_MASK		0xFF
#define MCP_COMMAND_REG_ADDR_L_SHIFT		8
#define MCP_COMMAND_REG_DATA_MASK		0xFF
#define MCP_COMMAND_REG_DATA_SHIFT		0x0

#define MCP_RESPONSE_RDATA_MASK			0xFF
#define MCP_RESPONSE_RDATA_SHIFT		8
#define MCP_RESPONSE_ACK_MASK			0x1
#define MCP_RESPONSE_ACK_SHIFT			0
#define MCP_RESPONSE_NACK_MASK			0x2

#define SDW_CNL_DPN_CONFIG0			0x100
#define SDW_CNL_DPN_CHANNELEN0		0x104
#define SDW_CNL_DPN_SAMPLECTRL0		0x108
#define SDW_CNL_DPN_OFFSETCTRL0		0x10C
#define SDW_CNL_DPN_HCTRL0			0x110
#define SDW_CNL_DPN_ASYNCCTRL0		0x114

#define SDW_CNL_DPN_CONFIG1			0x118
#define SDW_CNL_DPN_CHANNELEN1		0x11C
#define SDW_CNL_DPN_SAMPLECTRL1		0x120
#define SDW_CNL_DPN_OFFSETCTRL1		0x124
#define SDW_CNL_DPN_HCTRL1			0x128

#define SDW_CNL_PORTCTRL			0x130
#define PORTCTRL_PORT_DIRECTION_SHIFT		0x7
#define PORTCTRL_PORT_DIRECTION_MASK		0x1
#define PORTCTRL_BANK_INVERT_SHIFT		0x8
#define PORTCTRL_BANK_INVERT_MASK		0x1

#define SDW_CNL_PDINCONFIG0			0x1100
#define SDW_CNL_PDINCONFIG1			0x1108
#define PDINCONFIG_CHANNEL_MASK_SHIFT		0x8
#define PDINCONFIG_CHANNEL_MASK_MASK		0xFF
#define PDINCONFIG_PORT_NUMBER_SHIFT		0x0
#define PDINCONFIG_PORT_NUMBER_MASK		0x1F
#define PDINCONFIG_PORT_SOFT_RESET_SHIFT	0x18
#define PDINCONFIG_PORT_SOFT_RESET		0x1F

#define DPN_CONFIG_WL_SHIFT			0x8
#define DPN_CONFIG_WL_MASK			0x1F
#define DPN_CONFIG_PF_MODE_SHIFT		0x0
#define DPN_CONFIG_PF_MODE_MASK			0x3
#define DPN_CONFIG_PD_MODE_SHIFT		0x2
#define DPN_CONFIG_PD_MODE_MASK			0x3
#define DPN_CONFIG_BPM_MASK			0x1
#define DPN_CONFIG_BPM_SHIFT			0x12
#define DPN_CONFIG_BGC_MASK			0x3
#define DPN_CONFIG_BGC_SHIFT			0x10

#define DPN_SAMPLECTRL_SI_MASK			0xFFFF
#define DPN_SAMPLECTRL_SI_SHIFT			0x0

#define DPN_OFFSETCTRL0_OF1_MASK		0xFF
#define DPN_OFFSETCTRL0_OF1_SHIFT		0x0
#define DPN_OFFSETCTRL0_OF2_MASK		0xFF
#define DPN_OFFSETCTRL0_OF2_SHIFT		0x8

#define DPN_HCTRL_HSTOP_MASK			0xF
#define DPN_HCTRL_HSTOP_SHIFT			0x0
#define DPN_HCTRL_HSTART_MASK			0xF
#define DPN_HCTRL_HSTART_SHIFT			0x4
#define DPN_HCTRL_LCONTROL_MASK			0x7
#define DPN_HCTRL_LCONTROL_SHIFT		0x8

/* SoundWire Shim registers */
#define SDW_CNL_LCAP				0x0
#define SDW_CNL_LCTL				0x4
#define CNL_LCTL_CPA_SHIFT			8
#define CNL_LCTL_SPA_SHIFT			0
#define CNL_LCTL_CPA_MASK			0x1
#define CNL_LCTL_SPA_MASK			0x1

#define SDW_CMDSYNC_SET_MASK			0xF0000
#define SDW_CNL_IPPTR				0x8
#define SDW_CNL_SYNC				0xC
#define CNL_SYNC_CMDSYNC_MASK			0x1
#define CNL_SYNC_CMDSYNC_SHIFT			16
#define CNL_SYNC_SYNCGO_MASK			0x1
#define CNL_SYNC_SYNCGO_SHIFT			0x18
#define CNL_SYNC_SYNCPRD_MASK			0x7FFF
#define CNL_SYNC_SYNCPRD_SHIFT			0x0
#define CNL_SYNC_SYNCCPU_MASK			0x8000
#define CNL_SYNC_SYNCCPU_SHIFT			0xF

#define SDW_CNL_CTLSCAP				0x10
#define SDW_CNL_CTLS0CM				0x12
#define SDW_CNL_CTLS1CM				0x14
#define SDW_CNL_CTLS2CM				0x16
#define SDW_CNL_CTLS3CM				0x18

#define SDW_CNL_PCMSCAP				0x20
#define CNL_PCMSCAP_BSS_SHIFT			8
#define CNL_PCMSCAP_BSS_MASK			0x1F
#define CNL_PCMSCAP_OSS_SHIFT			4
#define CNL_PCMSCAP_OSS_MASK			0xF
#define CNL_PCMSCAP_ISS_SHIFT			0
#define CNL_PCMSCAP_ISS_MASK			0xF

#define SDW_CNL_PCMSCHM				0x22
#define CNL_PCMSYCM_DIR_SHIFT			15
#define CNL_PCMSYCM_DIR_MASK			0x1
#define CNL_PCMSYCM_STREAM_SHIFT		8
#define CNL_PCMSYCM_STREAM_MASK			0x3F
#define CNL_PCMSYCM_HCHAN_SHIFT			4
#define CNL_PCMSYCM_HCHAN_MASK			0xF
#define CNL_PCMSYCM_LCHAN_SHIFT			0
#define CNL_PCMSYCM_LCHAN_MASK			0xF

#define SDW_CNL_PCMSCHC				0x42

#define SDW_CNL_PDMSCAP				0x62
#define CNL_PDMSCAP_BSS_SHIFT			8
#define CNL_PDMSCAP_BSS_MASK			0x1F
#define CNL_PDMSCAP_OSS_SHIFT			4
#define CNL_PDMSCAP_OSS_MASK			0xF
#define CNL_PDMSCAP_ISS_SHIFT			0
#define CNL_PDMSCAP_ISS_MASK			0xF
#define CNL_PDMSCAP_CPSS_SHIFT			13
#define CNL_PDMSCAP_CPSS_MASK			0x7
#define SDW_CNL_PDMSCM

#define SDW_CNL_IOCTL				0x6C
#define CNL_IOCTL_MIF_SHIFT			0x0
#define CNL_IOCTL_MIF_MASK			0x1
#define CNL_IOCTL_CO_SHIFT			0x1
#define CNL_IOCTL_CO_MASK			0x1
#define CNL_IOCTL_COE_SHIFT			0x2
#define CNL_IOCTL_COE_MASK			0x1
#define CNL_IOCTL_DO_SHIFT			0x3
#define CNL_IOCTL_DO_MASK			0x1
#define CNL_IOCTL_DOE_SHIFT			0x4
#define CNL_IOCTL_DOE_MASK			0x1
#define CNL_IOCTL_BKE_SHIFT			0x5
#define CNL_IOCTL_BKE_MASK			0x1
#define CNL_IOCTL_WPDD_SHIFT			0x6
#define CNL_IOCTL_WPDD_MASK			0x1
#define CNL_IOCTL_CIBD_SHIFT			0x8
#define CNL_IOCTL_CIBD_MASK			0x1
#define CNL_IOCTL_DIBD_SHIFT			0x9
#define CNL_IOCTL_DIBD_MASK			0x1

#define SDW_CNL_CTMCTL_OFFSET			0x60
#define SDW_CNL_CTMCTL				0x6E
#define CNL_CTMCTL_DACTQE_SHIFT			0x0
#define CNL_CTMCTL_DACTQE_MASK			0x1
#define CNL_CTMCTL_DODS_SHIFT			0x1
#define CNL_CTMCTL_DODS_MASK			0x1
#define CNL_CTMCTL_DOAIS_SHIFT			0x3
#define CNL_CTMCTL_DOAIS_MASK			0x3

#define ALH_CNL_STRMZCFG_BASE			0x4
#define ALH_CNL_STRMZCFG_OFFSET			0x4
#define CNL_STRMZCFG_DMAT_SHIFT			0x0
#define CNL_STRMZCFG_DMAT_MASK			0xFF
#define CNL_STRMZCFG_DMAT_VAL			0x3
#define CNL_STRMZCFG_CHAN_SHIFT			16
#define CNL_STRMZCFG_CHAN_MASK			0xF

#define SDW_BRA_HEADER_SIZE_PDI             12 /* In bytes */
#define SDW_BRA_HEADER_CRC_SIZE_PDI         4 /* In bytes */
#define SDW_BRA_DATA_CRC_SIZE_PDI           4 /* In bytes */
#define SDW_BRA_HEADER_RESP_SIZE_PDI        4 /* In bytes */
#define SDW_BRA_FOOTER_RESP_SIZE_PDI        4 /* In bytes */
#define SDW_BRA_PADDING_SZ_PDI              4 /* In bytes */
#define SDW_BRA_HEADER_TOTAL_SZ_PDI         16 /* In bytes */

#define SDW_BRA_SOP_EOP_PDI_STRT_VALUE	0x4
#define SDW_BRA_SOP_EOP_PDI_END_VALUE	0x2
#define SDW_BRA_SOP_EOP_PDI_MASK	0x1F
#define SDW_BRA_SOP_EOP_PDI_SHIFT	5

#define SDW_BRA_STRM_ID_BLK_OUT		3
#define SDW_BRA_STRM_ID_BLK_IN		4

#define SDW_BRA_PDI_TX_ID		0
#define SDW_BRA_PDI_RX_ID		1

#define SDW_BRA_SOFT_RESET		0x1
#define SDW_BRA_BULK_ENABLE		1
#define SDW_BRA_BLK_EN_MASK		0xFFFEFFFF
#define SDW_BRA_BLK_EN_SHIFT		16

#define SDW_BRA_ROLLINGID_PDI_INDX	3
#define SDW_BRA_ROLLINGID_PDI_MASK	0xF
#define SDW_BRA_ROLLINGID_PDI_SHIFT	0

#define SDW_PCM_STRM_START_INDEX	0x2

#endif /* _LINUX_SDW_CNL_H */