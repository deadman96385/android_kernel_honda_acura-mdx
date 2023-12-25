/*
 * Copyright (c) 2015-2019 DENSO CORPORATION.  ALL RIGHTS RESERVED.
 * The purpose of the driver is to abstract the DENSO specific hardware
 * attributes for access to the User Space.
 */
/* system headers */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/printk.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/crc32.h>

/* local headers */
#include "nor_attributes.h"
#include "mtd_api.h"
#include "pdr.h"

#define MODULE_NAME "denso_hw_ctrl"
MODULE_ALIAS(MODULE_NAME);

#define FILE_PERMS (S_IRUGO | S_IWUSR | S_IWGRP)
#define MAXUINT64 (0xffffffffffffffff)
/* constant for invalid data */
static const char *INVALID = "INVALID";
static const char *INVALID_ASCII = "????????????????????????????????";
/* Alphabet for Base 16 and 'special' Base-33 (for VIN) */
static const char *b36 = "0123456789ABCDEFGHJKLMNPRSTUVWXYZ";  /* I, O, Q removed (VIN) */
/* alphabet for Base64 */
static const char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#define B64OFFSET(c) offsetOf(b64, 64, (c))
#define OFFSET(c) offsetOf(b36, 33, (c))

/*
 * Returns the offset of c in list of length len, or -1 if
 * c is not in list.
 */
static int offsetOf(const char *list, int len, char c)
{
	register int i;
	for(i = 0; i < len; i++)
		if(list[i] == c)
			return i;

	return -1;
}

/*
 * Parses source as a hexadecimal string, placing
 * the result in target.  Skips leading whitespace, if any.
 * Returns the number of bytes written to target.
 */
static int fromHex(uint8_t *target, const char *source)
{
	const char *p = source;
	int count;
	int off1, off2;

	while(isspace(*p))
		p++;

	for(count = 0 ; *p; p += 2, count++) {
		if(':' == *p || '-' == *p)
			p++;
		off1 = OFFSET(toupper(*p));
		off2 = OFFSET(toupper(*(p+1)));
		if(off1 < 0 || off1 > 15 || off2 < 0 || off2 > 15)
			break;
		target[count] = (off1 << 4) | off2;
	}

	return count;
}

/*
 * Writes a NULL-terminated hexadecimal string representation
 * of count bytes from source to target.
 * Returns a pointer to the end (i.e. the NULL) of the string in target.
 */
static char *toHex(char *target, uint8_t *source, size_t count)
{
    register size_t i;
	char *end = target;
	for(i = 0; i < count; i++) {
	    *(end++) = b36[source[i] >> 4];
	    *(end++) = b36[source[i] & 0x0f];
	}
	*end = 0x00;
    return end;
}

/*
 * Writes a NULL-terminated Base-64 string representation of
 * length bytes from source to target.
 * Returns the number of characters written to target.
 */
static size_t toBase64(char *target, uint8_t *source, size_t length)
{
	size_t icount = 0;
	size_t ocount = 0;
	uint8_t b1, b2, b3;
	int n;

	while(icount < length) {
		n = 3;
		b1 = source[icount++];
		if(icount < length) {
			b2 = source[icount++];
		} else {
			b2 = 0x00;
			n--;
		}
		if(icount < length) {
			b3 = source[icount++];
		} else {
			b3 = 0x00;
			n--;
		}
		target[ocount++] = b64[(b1 & 0xfc) >> 2];
		target[ocount++] = b64[((b1 & 0x03) << 4) | ((b2 & 0xf0) >> 4)];
		target[ocount++] = (n > 1) ? b64[((b2 & 0x0f) << 2) | ((b3 & 0xc0) >> 6)] : '=';
		target[ocount++] = (n > 2) ? b64[b3 & 0x3f] : '=';
	}
	target[ocount] = 0x00;

	return ocount;
}

/*
 * Parses the NULL-terminated source string as Base-64, writing
 * the result to target.
 * Returns the number of bytes written to target, or -EINVAL
 * if there is an error parsing the input.
 */
static size_t fromBase64(uint8_t *target, char *source)
{
	int off;
	size_t i;
	uint8_t *t = target;
	size_t len = strlen(source);

	for(i = 0; i < len; i += 4) {
		if((off = B64OFFSET(source[i])) < 0)
			return -EINVAL;
		*t = off << 2;
		if((off = B64OFFSET(source[i+1])) < 0)
			return -EINVAL;
		*t |= (off & 0x30) >> 4;
		t++;

		if('=' == source[i+2] || 0x00 == source[i+2])
			break;
		*t = (off & 0x0f) << 4;
		if((off = B64OFFSET(source[i+2])) < 0)
			return -EINVAL;
		*t |= (off & 0x3c) >> 2;
		t++;

		if('=' == source[i+3] || 0x00 == source[i+3])
			break;
		*t = (off & 0x03) << 6;
		if((off = B64OFFSET(source[i+3])) < 0)
			return -EINVAL;
		*t |= off & 0x3f;
		t++;
	}

	return (size_t) (t - target);
}

/*
 * Writes a string reqresentation of source into target, using
 * base as the radix.  Base can be 2 through 36.
 * Returns the number of characters written to target.
 */
static int u64tostr(char *target, uint64_t source, int base)
{
	int count = 0;
	char buf[65];
	char *p = &buf[63];
	buf[64] = 0x00;

	if(base < 2 || base > 36)
		return -1;
	while(source) {
		*(p--) = b36[source % base];
		source /= base;
		count++;
	}
	strcpy(target, p+1);
	return count;
}

/*
 * Parses source as an encoded uint64_t using base as the radix,
 * and placing the value in result.  If not NULL, endptr is
 * updated to point to where parsing stopped.  Base can be 2
 * through 36.
 * If the 'special' base of 0 is given, then the string
 * is interpreted to determine an actual base: if the prefix
 * is '0x', then base 16 is used; if the prefix is '0' then
 * base 8 is used, else base 10 is used.
 * Returns
 *    0 if source is fully parsed/consumed
 *    -EINVAL if a parse error occurs (invalid char)
 *    -ERANGE if continued parsing would overflow the result.
 *
 * This function is similar to the standard kstrtoull, except
 * kstrto*() functions support a maximum radix of 16.
 */
static int strtou64(uint64_t *result, const char *source, char **endptr, int base)
{
	const char *p = source;
	int val;
	int rc = 0;

	*result = 0;
	if(base == 0 || (base >= 2 && base <= 36)) {
		while(isspace(*p))
			p++;
		if(!base && '0' == *p && 'x' == *(p+1)) {
			base = 16;
			p += 2;
		} else if(!base && '0' == *p) {
			base = 8;
		} else if(!base) {
			base = 10;
		}
		while(*p) {
			val = OFFSET(toupper(*p));
			if(val >= base) {
				rc = -EINVAL;
				break;
			}
			if(val < 0)
				break;
			if(*result > (MAXUINT64 / base)) { /* shift would overflow */
				rc = -ERANGE;
				break;
			}
			*result *= base;
			*result += val;
			p++;
		}
		if(NULL != endptr)
			*endptr = p;
	} else {
		rc = -EINVAL;
	}

	return rc;
}

/*
 * Checks and stores source as NULL-terminated ASCII to
 * to target with given (max) size.  Zero's target's full
 * size prior to writing.
 * Returns 0 if successful, -EINVAL if source's length is
 * invalid or -ERANGE if it cannot fit in target.
 */
static int check_str(uint8_t *target, const char *source, size_t length, size_t size)
{
	if(length > size)
		return -ERANGE;
	if(length != strlen(source))
		return -EINVAL;

	memset(target, 0, size);
	strcpy(target, source);

	return 0;
}

/*
 * Returns a CRC32 of the given denso_otp_data_t,
 * using all fields except version, update_count, and
 * checksum in its calculation.
 */
static uint32_t calc_checksum(denso_otp_data_t *hwinfo)
{
	size_t skip = sizeof(hwinfo->version) + sizeof(hwinfo->update_count) + sizeof(hwinfo->checksum);
	uintptr_t buf = ((uintptr_t) hwinfo) + skip;
	return (uint32_t) crc32(0, (void *) buf, sizeof(denso_otp_data_t) - skip);
}

/*
 * Verify the header pointed to.  Returns 1 if valid, or 0 if not.
 * TODO: verify checksum, too.
 */
static int verify_pdr_header(struct intel_pdr_header *header)
{
	pr_debug(".%s() magic=0x%08x, hver=%u, ever=%u, len=%u\n",
		__func__, header->magic, header->header_version,
		header->entry_version, header->header_length);
	return (PDR_HEADER_MAGIC == header->magic && \
			1 == header->header_version && \
			1 == header->entry_version && \
			16 == header->header_length);
}

/*
 * Finds an entry in the PDR header/directory by name.
 * Returns a pointer to the directory entry structure, or NULL
 * if no match is found.
 */
static struct intel_pdr_dirent *find_entry(struct intel_pdr_header *header, const char *name)
{
	struct intel_pdr_dirent *entry;
	uint32_t i;

	if(verify_pdr_header(header)) {
		entry = (struct intel_pdr_dirent *) (((uintptr_t) header) + header->header_length);
		for(i = 0; i < header->num_entries; i++, entry++) {
			pr_debug(".%s() %u: name=%s, off=0x%x, len=%u\n",
				__func__, i, entry[i].name, entry[i].offset, entry[i].length);
			if(!strcmp(entry[i].name, name))
				return &entry[i];
		}
	}

	pr_err("No entry in PDR for '%s'\n", name);

	return NULL;
}

/*
 * Returns a pointer to the "oem" directory entry in the given header, or
 * NULL if it is not found.
 */
struct intel_pdr_dirent *find_oem_entry(struct intel_pdr_header *header)
{
	return find_entry(header, "oem");
}

/* OEM Offset determined from previous use of find_oem_entry() */
#define OEM_OFFSET (0xd000)
#define OEM_SIZE sizeof(denso_otp_data_t)
/*
 * Reads the HW control data from a fixed location
 * (OEM_OFFSET).  Allocates enough memory to contain
 * a denso_otp_data_t, which the caller is resonsible
 * for freeing.
 *
 * Returns 0 if successful, or
 *  -ENOMEM if memory cannot be allocated.
 *  -EIO if NOR cannot be read
 */
int read_oem(denso_otp_data_t **oem)
{
	int rc;

	if((*oem = (denso_otp_data_t *) kzalloc(OEM_SIZE, GFP_KERNEL)) == NULL) {
		pr_err("Cannot allocate buffer for quick OEM");
		return -ENOMEM;
	}
	if((rc = read_nor(PDR_OFFSET + OEM_OFFSET, (char *) *oem, OEM_SIZE)) != OEM_SIZE) {
		kfree(*oem);
		return -EIO;
	}

	return 0;
}
/*
 * Reads/loads the PDR from NOR, allocating memory to hold its contents.
 * If successful, the given pointer is updated to point to the buffer,
 * which the caller is responsible for kfree-ing.
 * Returns
 *  0 if successful
 * -ENOMEM if memory cannot be allocated
 * -EIO if NOR cannot be read
 */
int read_pdr(uint8_t **pdr)
{
	int rc;

	if((*pdr= kzalloc(PDR_SIZE, GFP_KERNEL)) == NULL) {
		pr_err("Cannot allocate buffer for PDR");
		return -ENOMEM;
	}
	if((rc = read_nor(PDR_OFFSET, *pdr, PDR_SIZE)) != PDR_SIZE) {
		kfree(*pdr);
		return -EIO;
	}

	return 0;
}

/*
 * Erases and Writes one erase page size (4096 bytes) from
 * the OEM portion of the given PDR to the corresponding
 * location in NOR.
 * Returns:
 *  0 if successful
 * -EIO if NOR cannot be written
 * -EINVAL if the OEM directory entry cannot be found
 */
static int write_oem(uint8_t *pdr)
{
	struct intel_pdr_dirent *oem;
	denso_otp_data_t *hwinfo;
	uint32_t erase_size;
	int rc = nor_erase_size(&erase_size);

	pr_debug(".%s() rc=%d, erase size=%u\n", __func__, rc, erase_size);

	if(!rc) {
		oem = find_oem_entry((struct intel_pdr_header *) pdr);
		if(oem) {
			hwinfo = (denso_otp_data_t *) &pdr[oem->offset];
			hwinfo->version = NOR_VERSION;
			hwinfo->update_count = (0xffffffff == hwinfo->update_count) ? \
									1 : hwinfo->update_count + 1;
			hwinfo->checksum = calc_checksum(hwinfo);
			pr_warn("NOR update count: %u\n", hwinfo->update_count);
			if(erase_nor(PDR_OFFSET + oem->offset, erase_size) ||
					write_nor(PDR_OFFSET + oem->offset, (const char *) hwinfo, erase_size) <= 0)
				rc = -EIO;
		} else {
			rc = -EINVAL;
		}
	}

	return rc;
}

/*
 * SYSFS attribute store function.
 * Returns the number of input chars consumed, or <0 if an error occurs.
 */
static ssize_t attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	denso_otp_data_t *hwinfo = NULL;
	struct intel_pdr_dirent *oem = NULL;
	uint8_t *pdr = NULL;
	uint32_t check;
	ssize_t rc = 0;

/* By detault, DENSO_HW_CTRL_READ_PDR=n, and read_oem()
   is used. If =y, the oem region will be located via the PDR
   directoryi via find_oem_entry().  See Kconfig.
*/
#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
	if(read_pdr(&pdr) == 0 && (oem = find_oem_entry((struct intel_pdr_header *) pdr)) != NULL) {
		hwinfo = (denso_otp_data_t *) (((uintptr_t) pdr) + oem->offset);
#else
	if((rc = read_oem(&hwinfo)) == 0) {
#endif
		pr_debug(".%s() hwinfo@%p\n", __func__, hwinfo);
		if((check = calc_checksum(hwinfo)) == hwinfo->checksum)
			pr_debug("NOR Data Checksum OK (0x%08x)\n", hwinfo->checksum);
		else
			pr_warning("NOR Data Corrupted?! (expected 0x%08x, got 0x%08x)\n",
					hwinfo->checksum, check);
		if(!strcmp(attr->attr.name, __stringify(PRODUCT))) {
			rc = scnprintf(buf, PAGE_SIZE, "%u\n", hwinfo->product);
		} else if(!strcmp(attr->attr.name, __stringify(PRODUCT_REVISION))) {
			rc = scnprintf(buf, PAGE_SIZE, "%u\n", hwinfo->revision);
		} else if(!strcmp(attr->attr.name, __stringify(MODEL))) {
			rc = scnprintf(buf, PAGE_SIZE, "%u\n", hwinfo->model);
		} else if(!strcmp(attr->attr.name, __stringify(CURRENT_VIN))) {
			if(strlen(hwinfo->vin_current) != LENGTH_VIN)
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", INVALID_ASCII);
			else
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", hwinfo->vin_current);
		} else if(!strcmp(attr->attr.name, __stringify(ORIGINAL_VIN))) {
			if(strlen(hwinfo->vin_original) != LENGTH_VIN)
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", INVALID_ASCII);
			else
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", hwinfo->vin_original);
		} else if(!strcmp(attr->attr.name, __stringify(BLUETOOTH_ADDRESS))) {
			toHex(buf, hwinfo->bt_address, LENGTH_BT_ADDR);
			rc = strlen(buf);
		} else if(!strcmp(attr->attr.name, __stringify(SERIAL_NUMBER))) {
			rc = scnprintf(buf, PAGE_SIZE, "%u\n", hwinfo->serial);
		} else if(!strcmp(attr->attr.name, __stringify(MANUFACTURE_DATE))) {
			rc = scnprintf(buf, PAGE_SIZE, "%llu\n", hwinfo->mfg_date);
		} else if(!strcmp(attr->attr.name, __stringify(MANUFACTURE_COUNT))) {
			rc = scnprintf(buf, PAGE_SIZE, "%llu\n", hwinfo->mfg_count);
		} else if(!strcmp(attr->attr.name, __stringify(HERE_ID))) {
			if(strlen(hwinfo->here_id) != LENGTH_HERE_ID)
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", INVALID_ASCII);
			else
				rc = scnprintf(buf, PAGE_SIZE, "%s\n", hwinfo->here_id);
		} else if(!strcmp(attr->attr.name, __stringify(PRODUCTION_HISTORY))) {
			rc = scnprintf(buf, PAGE_SIZE, "%08llX\n", hwinfo->history);
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_LOW))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->temp_low);
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_ROOM))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->temp_room);
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_HIGH))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->temp_high);
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_LOW))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->gyro_low);
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_ROOM))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->gyro_room);
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_HIGH))) {
			rc = scnprintf(buf, PAGE_SIZE, "%d\n", hwinfo->gyro_high);
		} else if(!strcmp(attr->attr.name, __stringify(UPDATE_COUNT))) {
			rc = scnprintf(buf, PAGE_SIZE, "%u\n", hwinfo->update_count);
		} else if(!strcmp(attr->attr.name, __stringify(CHECKSUM))) {
			rc = scnprintf(buf, PAGE_SIZE, "%08x\n", hwinfo->checksum);
		} else if(!strcmp(attr->attr.name, __stringify(USB_GAIN))) {
			rc = scnprintf(buf, PAGE_SIZE, "%08x\n", hwinfo->usb_gain);
		} else if(!strcmp(attr->attr.name, __stringify(DISPLAY_ID))) {
			rc = scnprintf(buf, PAGE_SIZE, "%08x\n", hwinfo->display_id);
		} else {
			rc = scnprintf(buf, PAGE_SIZE, "%s\n", INVALID_ASCII);
		}
	} else {
		pr_err("!%s() cannot open/read NOR: %ld\n", __func__, rc);
		rc = 0;
	}


#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
	if(pdr)
		kzfree(pdr);
#else
	if(hwinfo)
		kzfree(hwinfo);
#endif

	return rc;
}

/*
 * Removes leading and trailing whitespace from buf of
 * length count.  Returns an updated pointer to the
 * (non-whitespace) start of buf.
 */
static char *trim(char *buf, size_t count)
{
	char *p = buf;

	buf[count] = 0x00;
	if((p = strchr(buf, '\n')) != NULL)
		*p = 0x00;
	while(isspace(*buf))
		buf++;
	for(p = &buf[strlen(buf)]; p > buf; p--)
		if(isspace(*p))
			*p= 0x00;

	return buf;
}

/*
 * Parses count chars from buf as unsigned numeric input (up to 64 bits),
 * checking validity and bounds, and storing the result in uval.  Input
 * format can be base octal, decimal, or hexadecimal.
 * Returns count if successful, or -EINVAL if parsing or bounds check fail.
 */
static int check_input(uint64_t *uval, const char *buf, size_t count, uint64_t min, uint64_t max)
{
	int rc = -EINVAL;
	if(!strtou64(uval, (char *) buf, NULL, 0))
		if((*uval >= min) && (*uval <= max))
			rc = count;

	return rc;
}

/*
 * Parses count chars from buf as signed numeric input (single byte),
 * checking validity and bounds, and storing the result in val.  Input
 * format can be octal, decimal, or hexadecimal.
 * Returns count if successful, or -EINVAL if parsing or bounds check fail.
 */
static int check_temp(int8_t *val, const char *buf, size_t count, int8_t min, int8_t max)
{
	int rc = -EINVAL;
	long long res;
	if(!kstrtoll(buf, 0, &res))
		if((res >= min) && (res <= max)) {
			*val = (int8_t) res;
			rc = count;
		}

	return rc;
}

/*
 * SYSFS attribute store function.
 * Returns the number of input chars consumed, or <0 if an error occurs.
 */
static ssize_t attr_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	denso_otp_data_t *hwinfo;
	struct intel_pdr_dirent *oem;
	uint8_t *pdr = NULL;
	uint64_t uval = 0;
	int8_t ival = 0;
	uint32_t check;
	int rc = 0;
	char *p;
	ssize_t ret = count;

	if((p = strnchr(buf, count, '\n')) != NULL)
		*p = 0x00;
	if(read_pdr(&pdr) == 0 && (oem = find_oem_entry((struct intel_pdr_header *) pdr)) != NULL) {
		hwinfo = (denso_otp_data_t *) (((uintptr_t) pdr) + oem->offset);
		if((check = calc_checksum(hwinfo)) == hwinfo->checksum)
			pr_debug("NOR Data Checksum OK (0x%08x)\n", hwinfo->checksum);
		else
			pr_err("NOR Data Corrupted?! (expected 0x%08x, got 0x%08x)\n",
					hwinfo->checksum, check);
		if(!strcmp(attr->attr.name, __stringify(PRODUCT))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_PRODUCT)) == count)
				hwinfo->product = uval;
		} else if(!strcmp(attr->attr.name, __stringify(PRODUCT_REVISION))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_REVISION)) == count)
				hwinfo->revision = uval;
		} else if(!strcmp(attr->attr.name, __stringify(MODEL))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_MODEL)) == count)
				hwinfo->model = uval;
		} else if(!strcmp(attr->attr.name, __stringify(CURRENT_VIN))) {
			buf = trim((char *) buf, count);
			if(check_str(hwinfo->vin_current, buf, LENGTH_VIN, sizeof(hwinfo->vin_current)))
				ret = -EINVAL;
		} else if(!strcmp(attr->attr.name, __stringify(ORIGINAL_VIN))) {
			buf = trim((char *) buf, count);
			if(check_str(hwinfo->vin_original, buf, LENGTH_VIN, sizeof(hwinfo->vin_original)))
				ret = -EINVAL;
		} else if(!strcmp(attr->attr.name, __stringify(BLUETOOTH_ADDRESS))) {
			if(fromHex(hwinfo->bt_address, buf) != LENGTH_BT_ADDR)
				ret = -EINVAL;
		} else if(!strcmp(attr->attr.name, __stringify(SERIAL_NUMBER))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_SERIAL)) == count)
				hwinfo->serial = uval;
		} else if(!strcmp(attr->attr.name, __stringify(MANUFACTURE_DATE))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_DATE)) == count)
				hwinfo->mfg_date = uval;
		} else if(!strcmp(attr->attr.name, __stringify(MANUFACTURE_COUNT))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_COUNT)) == count)
				hwinfo->mfg_count = uval;
		} else if(!strcmp(attr->attr.name, __stringify(HERE_ID))) {
			buf = trim((char *) buf, count);
			if(check_str(hwinfo->here_id, buf, LENGTH_HERE_ID, sizeof(hwinfo->here_id)))
				ret = -EINVAL;
		} else if(!strcmp(attr->attr.name, __stringify(PRODUCTION_HISTORY))) {
			if((ret = check_input(&uval, buf, count, 0, MAX_HISTORY)) == count)
				hwinfo->history = uval;
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_LOW))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->temp_low = ival;
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_ROOM))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->temp_room = ival;
		} else if(!strcmp(attr->attr.name, __stringify(TEMP_SENSOR_HIGH))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->temp_high= ival;
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_LOW))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->gyro_low = ival;
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_ROOM))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->gyro_room = ival;
		} else if(!strcmp(attr->attr.name, __stringify(GYRO_TEMP_HIGH))) {
			if((ret = check_temp(&ival, buf, count, MIN_TEMP, MAX_TEMP)) == count)
				hwinfo->gyro_high = ival;
		} else if(!strcmp(attr->attr.name, __stringify(USB_GAIN))) {
			if((ret = check_temp(&ival, buf, count, 0, MAX_USB_GAIN_VALUE)) == count)
				hwinfo->usb_gain = ival;
		} else if(!strcmp(attr->attr.name, __stringify(DISPLAY_ID))) {
			if((ret = check_temp(&ival, buf, count, 0, MAX_DISPLAY_ID)) == count)
				hwinfo->display_id = ival;
		} else {
			ret = -EINVAL;
		}
		if(count == ret) {
			if(write_oem(pdr))
				ret = -EIO;
		} else {
			printk(KERN_ERR "%s: %s", INVALID, buf);
		}
		kzfree(pdr);
	} else {
		printk(KERN_ERR ".%s() Cannot Read NOR/PDR (%d).\n", __func__, rc);
		ret = -EIO;
	}

	return ret;
}

/* Define SYSFS attributes */
static struct kobj_attribute kobj_attr_product           = __ATTR(PRODUCT, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_product_revision  = __ATTR(PRODUCT_REVISION, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_model             = __ATTR(MODEL, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_current_vin       = __ATTR(CURRENT_VIN, FILE_PERMS, attr_show,attr_store);
static struct kobj_attribute kobj_attr_original_vin      = __ATTR(ORIGINAL_VIN, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_bt_addr           = __ATTR(BLUETOOTH_ADDRESS, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_serial_num        = __ATTR(SERIAL_NUMBER, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_mfg_date          = __ATTR(MANUFACTURE_DATE, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_mfg_count         = __ATTR(MANUFACTURE_COUNT, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_here              = __ATTR(HERE_ID, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_prod_hist         = __ATTR(PRODUCTION_HISTORY, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_tslow             = __ATTR(TEMP_SENSOR_LOW, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_tsroom            = __ATTR(TEMP_SENSOR_ROOM, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_tshigh            = __ATTR(TEMP_SENSOR_HIGH, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_gtlow             = __ATTR(GYRO_TEMP_LOW, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_gtroom            = __ATTR(GYRO_TEMP_ROOM, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_gthigh            = __ATTR(GYRO_TEMP_HIGH, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_updcnt            = __ATTR(UPDATE_COUNT, S_IRUGO, attr_show, NULL);
static struct kobj_attribute kobj_attr_chksum            = __ATTR(CHECKSUM, S_IRUGO, attr_show, NULL);
static struct kobj_attribute kobj_attr_usb_reg            = __ATTR(USB_GAIN, FILE_PERMS, attr_show, attr_store);
static struct kobj_attribute kobj_attr_display_id        = __ATTR(DISPLAY_ID, FILE_PERMS, attr_show, attr_store);

static struct attribute *nor_attributes[] = {
	&kobj_attr_product.attr,
	&kobj_attr_product_revision.attr,
	&kobj_attr_model.attr,
	&kobj_attr_current_vin.attr,
	&kobj_attr_original_vin.attr,
	&kobj_attr_bt_addr.attr,
	&kobj_attr_serial_num.attr,
	&kobj_attr_mfg_date.attr,
	&kobj_attr_mfg_count.attr,
	&kobj_attr_here.attr,
	&kobj_attr_prod_hist.attr,
	&kobj_attr_tslow.attr,
	&kobj_attr_tsroom.attr,
	&kobj_attr_tshigh.attr,
	&kobj_attr_gtlow.attr,
	&kobj_attr_gtroom.attr,
	&kobj_attr_gthigh.attr,
	&kobj_attr_updcnt.attr,
	&kobj_attr_chksum.attr,
	&kobj_attr_usb_reg.attr,
	&kobj_attr_display_id.attr,
	NULL
};

static struct attribute_group nor_attribute_group = {
	.attrs = nor_attributes,
};

static struct kobject *denso_hw_ctrl_kobj;

static int __init denso_hw_ctrl_init(void)
{
	int retval;

	printk(KERN_DEBUG "+%s()\n", __func__);
	/*
	 * Create a simple kobject with the name of MODULE_ALIAS,
	 * located under /sys/firmware/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	denso_hw_ctrl_kobj = kobject_create_and_add(MODULE_NAME, firmware_kobj);
	if (!denso_hw_ctrl_kobj) {
		retval = -ENOMEM;
	} else {
		/* Create the files associated with this kobject */
		retval = sysfs_create_group(denso_hw_ctrl_kobj, &nor_attribute_group);
		if (retval)
			kobject_put(denso_hw_ctrl_kobj);
	}
	printk(KERN_DEBUG "-%s() = %d\n", __func__, retval);
	return retval;
}

static void __exit denso_hw_ctrl_exit(void)
{
	kobject_put(denso_hw_ctrl_kobj);
}

late_initcall(denso_hw_ctrl_init);
module_exit(denso_hw_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert Peters <robert_peters@denso-diam.com>");
MODULE_DESCRIPTION("Driver to control custom parts of DENSO TYAA boards");
