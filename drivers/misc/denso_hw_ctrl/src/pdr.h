/* Copyright (c) 2019 DENSO Corporation.  All Rights Reserved. */
#ifndef _PDR_H_INCLUDED_
#define _PDR_H_INCLUDED_

/* See Gordon Peak Security Subsystem High Level Design
   Document Number: 576709, Rev. 1.12
   Appendix C-2
*/

#define PDR_OFFSET (0x1000)
#define PDR_SIZE (1020 * 1024)
#define PDR_HEADER_MAGIC (0x44504324)

struct intel_pdr_header {
	uint32_t	magic;
	uint32_t	num_entries;
	uint8_t		header_version; /* 1 */
	uint8_t		entry_version; /* 1 */
	uint8_t		header_length; /* should be 16 (0x10) */
	/* 8-bit XOR checksum of the sub-partition directory (from
		1 st byte of header to last byte of last partition directory
		entry) */
	uint8_t		checksum; /* 8-bit XOR checksum */
	char		name[4];
} __attribute__((__packed__));
/* NOTE: checksum spans 1st byte of header to last byte
   of last partition directory entry */

struct intel_pdr_dirent {
	char		name[12];
	uint32_t	offset;
	uint32_t	length;
	uint32_t	reserved; /* 0 */
} __attribute__((__packed__));

struct intel_pdr_dirent *find_oem_entry(struct intel_pdr_header *header);
int read_pdr(uint8_t **pdr);

#endif /* _PDR_H_INCLUDED_ */
