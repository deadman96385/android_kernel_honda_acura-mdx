#ifndef _DENSO_MMC_HEALTH_H
#define _DENSO_MMC_HEALTH_H
/*
 * Declarations and constants for eMMC health
 * data (Micron CMD56)
 *
 * Copyright (c) 2015-2019 DENSO INTERNATIONAL AMERICA, INC. ALL RIGHTS RESERVED.
 */

/* name of the debugfs file which reports eMMC block erasure statistics */
#define DENSO_HEALTH_DEBUGFS_ERASURES       "health_erasure_detail"
#define DENSO_HEALTH_DEBUGFS_LIFETIME       "health_lifetime"
#define DENSO_HEALTH_DEBUGFS_HEALTH_REPORT  "health_report"

struct denso_health_attribute {
    const char      *name;
    off_t           offset;
    size_t          size;
    bool            big_endian;
};

#define MK_HEALTH_ATTR(_name, _offset, _size, _big_endian) { \
    .name = _name, \
    .offset = (_offset), \
    .size = (_size), \
    .big_endian = (_big_endian) \
    }

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

int denso_write_health_report(struct mmc_host *, struct seq_file *);
#endif /* _DENSO_MMC_HEALTH_H */
