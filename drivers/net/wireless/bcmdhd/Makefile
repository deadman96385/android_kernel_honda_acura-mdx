# bcmdhd
#####################
# Basic features
#####################

DHDCFLAGS += -Wall -Wstrict-prototypes -Dlinux -DLINUX -DBCMDRIVER            \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DBCMFILEIMAGE            \
	-DDHDTHREAD -DSHOW_EVENTS -DBCMDBG -DWLP2P                            \
	-DWIFI_ACT_FRAME -DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT            \
	-DEMBEDDED_PLATFORM -DPNO_SUPPORT                                     \
	-DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT                           \
	-DCUSTOMER_HW2 -DGET_CUSTOM_MAC_ENABLE			              \
	-DSEC_ENHANCEMENT

# Cypress:For VTS
DHDCFLAGS += -DDBG_PKT_MON -DNDO_CONFIG_SUPPORT

# ARP offload feature
#DHDCFLAGS += -DARP_OFFLOAD_SUPPORT

# FW_COREDUMP
#DHDCFLAGS += -DDHD_FW_COREDUMP

# Enable below macro if __DATE__ & __TIME__ are deprecated by compiler
DHDCFLAGS += -DDHD_VERSION_NO_DATE_TIME

#################
# Common feature
#################
# Brakepad release: change ANROID_O to WL_VIF_SUPPORT
# Clutch release comment out VIF support but Denso bsp need to support VIF
## SWAUTO-1902 - disable VIF and ACS.  ##
# Multiple virtual interface support for all
DHDCFLAGS += -DWL_VIF_SUPPORT

# Adding Auto Channel Selection Support as default
#Clutch release changed to support ACS
DHDCFLAGS += -DWL_SUPPORT_AUTO_CHANNEL

# Enabling WAPI as default for all.
DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

# DT-based architecture
#DHDCFLAGS += -DCONFIG_DTS

DHDCFLAGS += -DWL_VIRTUAL_APSTA

DHDCFLAGS += -DWL_CFG80211
# Print out kernel panic point of file and line info when assertion happened
DHDCFLAGS += -DBCMASSERT_LOG

# ARCH specific flags
# In case of x86 architechture, pls enable below flag
#DHDCFLAGS += -D__X86

# CUSTOMER4 flags
DHDCFLAGS += -DDHD_PKTID_AUDIT_ENABLED
DHDCFLAGS += -DSUPPORT_HIDDEN_AP -DDHD_LOSSLESS_ROAMING
DHDCFLAGS += -DPASS_ALL_MCAST_PKTS -DESCAN_BUF_OVERFLOW_MGMT -DPKTPRIO_OVERRIDE
DHDCFLAGS += -DUSE_INITIAL_SHORT_DWELL_TIME -DWL_NEWCFG_PRIVCMD_SUPPORT
DHDCFLAGS += -DENABLE_BCN_LI_BCN_WAKEUP -DSOFTAP_UAPSD_OFF
DHDCFLAGS += -DSUPPORT_LTECX -DSUPPORT_2G_VHT
DHDCFLAGS += -DSUPPORT_WL_TXPOWER -DBLOCK_IPV6_PACKET -DSUPPORT_DEEP_SLEEP
DHDCFLAGS += -DSUPPORT_AMPDU_MPDU_CMD -DSOFTAP_SEND_HANGEVT -DNUM_SCB_MAX_PROBE=3
DHDCFLAGS += -DCUSTOM_AMSDU_AGGSF=2
DHDCFLAGS += -DCUSTOM_IBSS_AMPDU_BA_WSIZE=16
DHDCFLAGS += -DCUSTOM_TCPACK_DELAY_TIME=10
DHDCFLAGS += -DDEBUGFS_CFG80211
DHDCFLAGS += -DSUPPORT_SOFTAP_WPAWPA2_MIXED

# keepalive
DHDCFLAGS += -DCUSTOM_KEEP_ALIVE_SETTING=28000

DHDCFLAGS += -DVSDB

# For p2p connection issue
DHDCFLAGS += -DWL_SCB_TIMEOUT=10

# Max allowed sta connections
DHDCFLAGS += -DMAX_CONNECTIONS=7

# TDLS enable
#DHDCFLAGS += -DWLTDLS -DWLTDLS_AUTO_ENABLE
# For TDLS tear down inactive time 40 sec
#DHDCFLAGS += -DCUSTOM_TDLS_IDLE_MODE_SETTING=10000
# for TDLS RSSI HIGH for establishing TDLS link
#DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_HIGH=-80
# for TDLS RSSI HIGH for tearing down TDLS link
#DHDCFLAGS += -DCUSTOM_TDLS_RSSI_THRESHOLD_LOW=-85

# Roaming
DHDCFLAGS += -DROAM_AP_ENV_DETECTION
DHDCFLAGS += -DROAM_ENABLE -DROAM_CHANNEL_CACHE -DROAM_API
DHDCFLAGS += -DENABLE_FW_ROAM_SUSPEND
# Roaming trigger
DHDCFLAGS += -DCUSTOM_ROAM_TRIGGER_SETTING=-75
DHDCFLAGS += -DCUSTOM_ROAM_DELTA_SETTING=10
# Set PM 2 always regardless suspend/resume
DHDCFLAGS += -DSUPPORT_PM2_ONLY

# For special PNO Event keep wake lock for 10sec
DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=10
DHDCFLAGS += -DMIRACAST_AMPDU_SIZE=8

# Gscan suppport
DHDCFLAGS += -DGSCAN_SUPPORT

# Early suspend
DHDCFLAGS += -DDHD_USE_EARLYSUSPEND

# For Scan result patch
DHDCFLAGS += -DESCAN_RESULT_PATCH

# For Static Buffer
ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
  DHDCFLAGS += -DENHANCED_STATIC_BUF
  DHDCFLAGS += -DSTATIC_WL_PRIV_STRUCT
endif
ifneq ($(CONFIG_DHD_USE_SCHED_SCAN),)
DHDCFLAGS += -DWL_SCHED_SCAN
endif

# Ioctl timeout 5000ms
DHDCFLAGS += -DIOCTL_RESP_TIMEOUT=5000

# Prevent rx thread monopolize
DHDCFLAGS += -DWAIT_DEQUEUE

# Config PM Control
DHDCFLAGS += -DDHD_PM_CONTROL_FROM_FILE

# idle count
DHDCFLAGS += -DDHD_USE_IDLECOUNT

# SKB TAILPAD to avoid out of boundary memory access
DHDCFLAGS += -DDHDENABLE_TAILPAD

# Wi-Fi Direct
DHDCFLAGS += -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
DHDCFLAGS += -DWL_CFG80211_STA_EVENT
DHDCFLAGS += -DWL_IFACE_COMB_NUM_CHANNELS
DHDCFLAGS += -DWL_ENABLE_P2P_IF
#SCAN time
DHDCFLAGS += -DCUSTOM_SET_SHORT_DWELL_TIME

ifeq (,$(findstring 3.10.20,$(LINUXVER)))
  DHDCFLAGS += -DCUSTOM_FORCE_NODFS_FLAG
endif

# SoftAP
#DHDCFLAGS += -DDISABLE_11H_SOFTAP
DHDCFLAGS += -DSET_RANDOM_MAC_SOFTAP
DHDCFLAGS += -DWL_CFG80211_ACL

# Brakepad release: add WL_CFG80211_PRIORITIZE_AP_CREATING support
# Uncomment the below line to prioritize AGO creation.
#DHDCFLAGS += -DWL_CFG80211_PRIORITIZE_AP_CREATING

# Uncomment the below line to disable 11H
#DHDCFLAGS += -DDISABLE_11H

# Disable 11ac support
#DHDCFLAGS += -DDISABLE_11AC

#Disable FRAMEBURST on VSDB
#Need to Remove this DEF, as it fails CERT test.  disabled by Brakepad release
#DHDCFLAGS += -DDISABLE_FRAMEBURST_VSDB

# Uncomment the below line to disable 11K
#DHDCFLAGS += -DDISABLE_11K

# Prevent send hang event even though p2p discovery interface is not removed.
DHDCFLAGS += -DEXPLICIT_DISCIF_CLEANUP

# Clutch release added
# Uncomment the below line for AP to receive disconnect management frame.
DHDCFLAGS += -DWL_CFG80211_AP_RX_MGMT_DISCONNECT

##########################
# driver type
# m: module type driver
# y: built-in type driver
##########################
DRIVER_TYPE ?= $(CONFIG_BCMDHD)
DRIVERX_TYPE ?= $(CONFIG_BCMDHDX)

#########################
# Chip dependent feature
#########################

ifneq ($(filter y, $(CONFIG_BCM4359)),)
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
  DHDCFLAGS += -DCUSTOM_PSPRETEND_THR=30
  DHDCFLAGS += -DMAX_AP_CLIENT_CNT=10
  DHDCFLAGS += -DMAX_GO_CLIENT_CNT=5
# New Features
  DHDCFLAGS += -DWL11U
  DHDCFLAGS += -DMFP
# Enable below macro for Dynamic Link Power Control
#  DHDCFLAGS += -DDHD_ENABLE_LPC
ifeq (,$(findstring 3.10.20,$(LINUXVER)))
  DHDCFLAGS += -DCUSTOM_COUNTRY_CODE
endif
  DHDCFLAGS += -DSAR_SUPPORT
# debug info
  DHDCFLAGS += -DDHD_WAKE_STATUS

ifneq ($(CONFIG_BCMDHD_PCIE),)
  DHDCFLAGS += -DPCIE_FULL_DONGLE -DBCMPCIE -DCUSTOM_DPC_PRIO_SETTING=-1
# tput enhancement
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=48
  DHDCFLAGS += -DCUSTOM_AMPDU_RELEASE=32
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB
# Disable watchdog thread
  DHDCFLAGS += -DCUSTOM_DHD_WATCHDOG_MS=0
  DHDCFLAGS += -DMAX_CNTL_TX_TIMEOUT=1
  DHDCFLAGS += -DSUPPORT_LINKDOWN_RECOVERY
# Add Load Balance Feature here
  DHDCFLAGS += -DDHD_LB
  DHDCFLAGS += -DDHD_LB_RXP
  DHDCFLAGS += -DDHD_LB_STATS
# Bandsteer support
  DHDCFLAGS += -DDHD_BANDSTEER
endif

ifneq ($(CONFIG_BCMDHD_SDMMC),)
  DHDCFLAGS += -DBDC -DDHD_BCMEVENTS -DMMC_SDIO_ABORT
# Macros for HW-OOB
  DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB
# Enable Below macro for In-Band interrupt
#  DHDCFLAGS += -DSDIO_ISR_THREAD
  DHDCFLAGS += -DBCMSDIO -DBCMLXSDMMC -DUSE_SDIOFIFO_IOVAR
  DHDCFLAGS += -DPROP_TXSTATUS
  DHDCFLAGS += -DCUSTOM_AMPDU_MPDU=16
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64
# tput enhancement
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=256
  DHDCFLAGS += -DBCMSDIOH_TXGLOM -DCUSTOM_TXGLOM=1 -DBCMSDIOH_TXGLOM_HIGHSPEED
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DREPEAT_READFRAME
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=40
  DHDCFLAGS += -DMAX_HDR_READ=128
  DHDCFLAGS += -DDHD_FIRSTREAD=128
  # bcn_timeout
  DHDCFLAGS += -DCUSTOM_BCN_TIMEOUT=5
  DHDCFLAGS += -DWLFC_STATE_PREALLOC
endif


ifneq ($(CONFIG_BCMDHD_SDIOH_STD),)
  DHDCFLAGS += -DPROP_TXSTATUS -DBCMSDIO -DBCMSDIOH_STD -DBCMSDIOH_TXGLOM -DRXFRAME_THREAD -DSDHOST3=1
  DHDCFLAGS += -DBCMFILEIMAGE -DSEC_ENHANCEMENT -DDHD_VERSION_NO_DATE_TIME  -DDHD_FW_COREDUMP -DSDTEST -DDHD_IFDEBUG -DSI_ERROR_ENFORCE
  DHDCFLAGS += -DBDC -DTOE -DDHD_BCMEVENTS -DOEM_ANDROID -DWL11ULB
  DHDCFLAGS += -DWLAIBSS -DWLAIBSS_PS -DWL_RELMCAST -DWL_SUPPORT_AUTO_CHANNEL -DIPV6_NDO_SUPPORT
  DHDCFLAGS += -DTSQ_MULTIPLIER=10 -DWL11U
  DHDCFLAGS += -DWLP2P
  DHDCFLAGS += -DHT40_GO -DWL_SDO  -DAMPDU_HOSTREORDER
endif


ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
  DHDCFLAGS += -DDHD_USE_STATIC_IOCTLBUF
endif
  DHDCFLAGS += -DDONGLE_ENABLE_ISOLATION
# Print 802.1X packets
  DHDCFLAGS += -DDHD_8021X_DUMP
# Print DHCP packets
  DHDCFLAGS += -DDHD_DHCP_DUMP
endif

ifneq ($(CONFIG_BCM4339),)
  DHDCFLAGS += -DBCM4339_CHIP -DHW_OOB

  # tput enhancement
  DHDCFLAGS += -DCUSTOM_GLOM_SETTING=8 -DCUSTOM_RXCHAIN=1
  DHDCFLAGS += -DUSE_DYNAMIC_F2_BLKSIZE -DDYNAMIC_F2_BLKSIZE_FOR_NONLEGACY=128
  DHDCFLAGS += -DBCMSDIOH_TXGLOM -DCUSTOM_TXGLOM=1 -DBCMSDIOH_TXGLOM_HIGHSPEED
  DHDCFLAGS += -DDHDTCPACK_SUPPRESS
  DHDCFLAGS += -DUSE_WL_TXBF
  DHDCFLAGS += -DUSE_WL_FRAMEBURST
  DHDCFLAGS += -DRXFRAME_THREAD
  DHDCFLAGS += -DCUSTOM_AMPDU_BA_WSIZE=64
  DHDCFLAGS += -DCUSTOM_DPC_CPUCORE=0
  DHDCFLAGS += -DPROP_TXSTATUS_VSDB
  DHDCFLAGS += -DCUSTOM_MAX_TXGLOM_SIZE=32

  # New Features
  DHDCFLAGS += -DWL11U
  DHDCFLAGS += -DDHD_ENABLE_LPC
  DHDCFLAGS += -DCUSTOM_PSPRETEND_THR=30
endif


#EXTRA_LDFLAGS += --strip-debug

ifeq ($(DRIVER_TYPE),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif

ifeq ($(DRIVERX_TYPE),y)
  DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
  DHDCFLAGS += -DUSE_LATE_INITCALL_SYNC
endif

ifneq ($(DRIVERX_TYPE),)
  DHDCFLAGS += -DBCMDHDX
endif

DHDOFILES := dhd_pno.o dhd_common.o dhd_ip.o dhd_custom_gpio.o \
	dhd_linux.o dhd_linux_sched.o dhd_cfg80211.o dhd_linux_wq.o aiutils.o bcmevent.o \
	bcmutils.o bcmwifi_channels.o hndpmu.o linux_osl.o sbutils.o siutils.o \
	wl_android.o wl_roam.o wl_cfg80211.o wl_cfgp2p.o wl_cfg_btcoex.o wldev_common.o wl_linux_mon.o  \
	dhd_linux_platdev.o dhd_pno.o  hnd_pktq.o hnd_pktpool.o

# RTT support
#DHDCFLAGS += -DRTT_SUPPORT -DRTT_DEBUG
#DHDOFILES += dhd_rtt.o bcmxtlv.o bcm_app_utils.o

# Vendor Extension support for Android
# if the kernel has support for vendor-extension then enable this flag in kernel \
	config.
ifeq ($(CONFIG_BCMDHD_VENDOR_EXT),y)
	DHDCFLAGS += -DWL_VENDOR_EXT_SUPPORT
	DHDOFILES += wl_cfgvendor.o
endif

# Bandsteer support
#DHDCFLAGS += -DDHD_BANDSTEER
#DHDOFILES +=  dhd_bandsteer.o

# DRIVER start/stop support
#DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD -DKEEP_WIFION_OPTION

ifneq ($(CONFIG_BCMDHD_PCIE),)
  DHDOFILES += dhd_pcie.o dhd_pcie_linux.o dhd_msgbuf.o dhd_flowring.o
  DHDOFILES += pcie_core.o
# Bandsteer support
  DHDOFILES += dhd_bandsteer.o
endif


ifneq ($(CONFIG_BCMDHD_SDMMC),)
  DHDOFILES += bcmsdh.o bcmsdh_linux.o dhd_sdio.o dhd_cdc.o dhd_wlfc.o
  DHDOFILES += bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o
endif

ifneq ($(CONFIG_BCMDHD_SDIOH_STD),)
  DHDOFILES += bcmsdh.o bcmsdh_linux.o dhd_sdio.o dhd_cdc.o dhd_wlfc.o
  DHDOFILES += bcmsdstd.o bcmsdstd_linux.o
endif

ifneq ($(CONFIG_BCMDHD),)
  bcmdhd-objs := $(DHDOFILES)
  obj-$(DRIVER_TYPE)   += bcmdhd.o
endif

ifneq ($(CONFIG_BCMDHDX),)
  bcmdhdx-objs := $(DHDOFILES)
  obj-$(DRIVERX_TYPE)   += bcmdhdx.o
endif

EXTRA_CFLAGS += $(DHDCFLAGS) -DDHD_DEBUG
EXTRA_CFLAGS += -DSRCBASE=\"$(src)\"
EXTRA_CFLAGS += -I$(src)/include/ -I$(src)/
KBUILD_CFLAGS += -I$(LINUXDIR)/include -I$(shell pwd)

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -rf *.o *.ko *.mod.c *~ .*.cmd *.o.cmd .*.o.cmd \
	Module.symvers modules.order .tmp_versions modules.builtin

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
