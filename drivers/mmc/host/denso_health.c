/*
 * Definitions of eMMC health data functions (Micron CMD56)
 *
 * Copyright (c) 2015-2019 DENSO INTERNATIONAL AMERICA, INC. ALL RIGHTS RESERVED.
 */
#define DEBUG
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include "denso_health.h"

/*
 * See Micron TN-FC-42: e.MMC Device Health Report
 * CMD56 Health Report Command Parameter (32 bits)
 * RRRRRRRR 22222222 11111111 CCCCCCCM
 * 0     : Mode:  1=Read, 0=Write
 * 1-7   : Command Index: CMD_*
 * 8-15  : Arg 1
 * 16-23 : Arg 2
 * 24-31 : Reserved, 0
 */
#define MKCMD(cidx,arg1,arg2,mode)           (((arg2)<<16)|((arg1)<<8)|(((cidx)<<1)|mode))
#define MODE_READ                            1
#define MODE_WRITE                           0
#define CMD_BAD_BLOCK_COUNTERS               0x08    /* 0b0001000 */
#define CMD_BLOCK_ERASE_COUNT_ENHANCED       0x11    /* 0b0010001 */ /* SLC */
#define CMD_BLOCK_ERASE_COUNT_NORMAL         0x12    /* 0b0010010 */ /* SLC and MLC */
#define CMD_SECURE_SMART_REPORT              0x1c    /* 0b0011100 */
#define CMD56_BLOCKLEN                       (512)
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

static struct denso_health_attribute health_report[] = {
    MK_HEALTH_ATTR("Initial Bad Block Count",                   0x00, sizeof(u16), true),
    MK_HEALTH_ATTR("Run-time Bad Block Count",                  0x02, sizeof(u16), true),
    MK_HEALTH_ATTR("Remaining Spare Block Count",               0x04, sizeof(u16), true),
    MK_HEALTH_ATTR("MLC Minimum Block Erasures",                0x10, sizeof(u32), true),
    MK_HEALTH_ATTR("MLC Maximum Block Erasures",                0x14, sizeof(u32), true),
    MK_HEALTH_ATTR("MLC Average Block Erasures",                0x18, sizeof(u32), true),
    MK_HEALTH_ATTR("MLC Total Block Erasures",                  0x1c, sizeof(u32), true),
    MK_HEALTH_ATTR("SLC Minimum Block Erasures",                0x20, sizeof(u32), true),
    MK_HEALTH_ATTR("SLC Maximum Block Erasures",                0x24, sizeof(u32), true),
    MK_HEALTH_ATTR("SLC Average Block Erasures",                0x28, sizeof(u32), true),
    MK_HEALTH_ATTR("SLC Total Block Erasures",                  0x2c, sizeof(u32), true),
    MK_HEALTH_ATTR("SLC Cumulative Initialization",             0x30, sizeof(u32), false),
    MK_HEALTH_ATTR("Read Reclaim Count",                        0x34, sizeof(u32), false),
    MK_HEALTH_ATTR("Written 100MB Data Count",                  0x44, sizeof(u32), false),
    MK_HEALTH_ATTR("Power Loss Count",                          0xc0, sizeof(u32), false),
    MK_HEALTH_ATTR("Auto-Initiated Refresh # Read Ops",         0xd0, sizeof(u32), false),
    MK_HEALTH_ATTR("Auto-Initiated Refresh Read Scan Progress", 0xd4, sizeof(u32), false),
    { NULL }
};

static void bind_mmc_structs(struct mmc_request *mrq, struct mmc_command *cmd,
                            struct mmc_data *data)
{
    mrq->cmd = cmd;
    /* cmd->mrq = mrq;    assigned in core layer */
    if(data) {
        data->mrq = mrq;
        mrq->data = data;
    }
}

static void init_mmc_structs(struct mmc_request *mrq, struct mmc_command *cmd,
                            struct mmc_data *data)
{
    memset(mrq, 0, sizeof(*mrq));
    memset(cmd, 0, sizeof(*cmd));
    memset(data, 0, sizeof(*data));
    data->blksz = CMD56_BLOCKLEN;
    data->blocks = 1;
    data->flags = MMC_DATA_READ;
    bind_mmc_structs(mrq, cmd, data);
}

static int exec_mmc_request(struct mmc_host *mmc, struct mmc_request *mrq, int setblklen)
{
    struct mmc_command cmd;
    int rc = 0;

    dev_dbg(mmc_dev(mmc), "+%s()\n", __func__);
    if(setblklen)
        rc = mmc_set_blocklen(mmc->card, CMD56_BLOCKLEN);
    if(!rc) {
        if(mrq->data) {
            mrq->data->bytes_xfered = 0;
			mrq->data->flags = MMC_DATA_READ;
            mmc_set_data_timeout(mrq->data, mmc->card);
        }
        mmc_wait_for_req(mmc, mrq);
        dev_dbg(mmc_dev(mmc), ".%s() request completed.\n",
                __func__);
        rc = mrq->cmd->error;
        if(!rc && mrq->data)
            rc =  mrq->data->error;
        if(rc) {
            dev_warn(mmc_dev(mmc),
                    "!%s() '%s' .cmd=%d, rc=%d\n",
                    __func__, mmc_hostname(mmc),
                    mrq->cmd->opcode, rc);
        }
    } else {
        dev_warn(mmc_dev(mmc), "!%s() could not SET_BLOCKLEN=%u\n",
                 __func__, cmd.arg);
    }
    dev_dbg(mmc_dev(mmc), "-%s() = %d\n", __func__, rc);
    return rc;
}

/*
 * Reads size bytes starting at buf[off], shifting into
 * a u32 result. Endian-ness is indicated by be.  At most
 * sizeof(u32) (i.e. 4) bytes are read.
 * Returns the result.
 */
static u32 valueAt(u8 *buf, off_t off, size_t size, bool be)
{
    u32 res = 0;
    register int i;

    for(i = 0; i < (int)MIN(sizeof(res), size); i++)
        if(be) {
            res <<= 8;
            res |= buf[off + i];
        } else {
            res |= buf[off + i] << (i * 8);
        }

    return res;
}

/**
 * Retrieve the device's 'Secure Smart Report' data, and use it to write a
 * report to the given seq_file.
 *
 * Returns 0 if successful; -ENOMEM if work buffers cannot be allocated, or
 * -EREMOTEIO if any error occurs while trying to communicate with the device.
 */
int denso_write_health_report(struct mmc_host *host, struct seq_file *s)
{
    struct mmc_card *card = host->card;
    int rc = 0;
    struct mmc_request  mrq;
    struct mmc_command  cmd;
    struct mmc_data     data;
    struct scatterlist  sg = { 0 };
    u8 *buf;
    int i;

    dev_dbg(mmc_dev(host), "+%s()\n", __func__);
    buf = kzalloc(CMD56_BLOCKLEN, GFP_KERNEL);
    if(buf) {
        init_mmc_structs(&mrq, &cmd, &data);
        sg_init_one(&sg, buf, CMD56_BLOCKLEN);
        data.sg = &sg;
        data.sg_len = 1;
        mmc_set_data_timeout(&data, card);
        cmd.opcode = MMC_GEN_CMD;
        cmd.arg = MKCMD(CMD_SECURE_SMART_REPORT, 0, 0, MODE_READ);
        cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;
        mmc_claim_host(host);
        rc = exec_mmc_request(host, &mrq, TRUE);
        mmc_release_host(host);
        if (!rc) {
            for(i = 0; health_report[i].name; i++)
                seq_printf(s, "%-45s: %llu\n", health_report[i].name,
                    valueAt(buf, health_report[i].offset, health_report[i].size,
                             health_report[i].big_endian));
        } else {
            dev_warn(mmc_dev(host),
                    "!%s() cannot get health report data from device (%d)\n",
                    __func__, rc);
            rc = -EREMOTEIO;
        }
        kfree(buf);
    } else {
        rc = -ENOMEM;
        dev_warn(mmc_dev(host), "!%s() cannot kzalloc\n", __func__);
    }
    dev_dbg(mmc_dev(host), "-%s() = %d\n", __func__, rc);

    return rc;
}
