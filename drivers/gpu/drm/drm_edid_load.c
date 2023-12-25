/*
   drm_edid_load.c: use a built-in EDID data set or load it via the firmware
		    interface

   Copyright (C) 2012 Carsten Emde <C.Emde@osadl.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#include <linux/module.h>
#include <linux/firmware.h>
#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <linux/delay.h>

#include "../../misc/denso_hw_ctrl/src/nor_attributes.h"
#include "../../misc/denso_hw_ctrl/src/mtd_api.h"
#include "../../misc/denso_hw_ctrl/src/pdr.h"

static char edid_firmware[PATH_MAX];
module_param_string(edid_firmware, edid_firmware, sizeof(edid_firmware), 0644);
MODULE_PARM_DESC(edid_firmware, "Do not probe monitor, use specified EDID blob "
	"from built-in data or /lib/firmware instead. ");

#define GENERIC_EDIDS 3
static const char * const generic_edid_name[GENERIC_EDIDS] = {
	"edid/1920x720.bin",
    "edid/1280x690.bin",
	"edid/honda-10in.bin",
};

static const u8 generic_edid[GENERIC_EDIDS][256] = {
  { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x53, 0x0e, 0x29, 0x09, 0x01, 0x00, 0x00, 0x00,
    0x1c, 0x18, 0x01, 0x03, 0x80, 0x1d, 0x0b, 0x78,
    0x02, 0xec, 0x18, 0xa3, 0x54, 0x46, 0x98, 0x25,
    0x0f, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x34, 0x20,
    0x80, 0x40, 0x70, 0xd0, 0x09, 0x20, 0x36, 0x06,
    0x33, 0x00, 0x80, 0xd0, 0x72, 0x00, 0x00, 0x18,
    0x00, 0x00, 0x00, 0xfd, 0x00, 0x38, 0x3a, 0x1e,
    0x32, 0x09, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x54,
    0x49, 0x2d, 0x44, 0x53, 0x39, 0x30, 0x55, 0x78,
    0x39, 0x32, 0x39, 0x0a, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xbd,
    0x02, 0x03, 0x15, 0x40, 0x41, 0x84, 0x23, 0x09,
    0x7f, 0x05, 0x83, 0x01, 0x00, 0x00, 0x66, 0x03,
    0x0c, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28
  },
  {
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x53, 0x0e, 0x29, 0x09, 0x01, 0x00, 0x00, 0x00,
    0x1c, 0x18, 0x01, 0x04, 0x81, 0x17, 0x0a, 0x78,
    0x02, 0xec, 0x18, 0xa3, 0x54, 0x46, 0x98, 0x25,
    0x0f, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x25, 0x1d,
    0x00, 0xdc, 0x50, 0xb2, 0x8b, 0x20, 0x46, 0x28,
    0xf6, 0x0c, 0xe6, 0x64, 0x00, 0x00, 0x00, 0x18,
    0x00, 0x00, 0x00, 0xfd, 0x00, 0x38, 0x3a, 0x1e,
    0x32, 0x09, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x54,
    0x49, 0x2d, 0x44, 0x53, 0x39, 0x30, 0x55, 0x78,
    0x39, 0x32, 0x39, 0x0a, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xeb,
    0x02, 0x03, 0x15, 0x40, 0x41, 0x84, 0x23, 0x09,
    0x7f, 0x05, 0x83, 0x01, 0x00, 0x00, 0x66, 0x03,
    0x0c, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28
  },
  {
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x53, 0x0e, 0x29, 0x09, 0x01, 0x00, 0x00, 0x00,
    0x1c, 0x18, 0x01, 0x03, 0x80, 0x18, 0x09, 0x78,
    0x02, 0xec, 0x18, 0xa3, 0x54, 0x46, 0x98, 0x25,
    0x0f, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x34, 0x20,
    0x80, 0x40, 0x70, 0xd0, 0x09, 0x20, 0x36, 0x06,
    0x33, 0x00, 0x80, 0xd0, 0x72, 0x00, 0x00, 0x18,
    0x00, 0x00, 0x00, 0xfd, 0x00, 0x38, 0x3a, 0x1e,
    0x32, 0x09, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x54,
    0x49, 0x2d, 0x44, 0x53, 0x39, 0x30, 0x55, 0x78,
    0x39, 0x32, 0x39, 0x0a, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc4,
    0x02, 0x03, 0x15, 0x40, 0x41, 0x84, 0x23, 0x09,
    0x7f, 0x05, 0x83, 0x01, 0x00, 0x00, 0x66, 0x03,
    0x0c, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28
  },
 };

static  int display_type_in_nor = -1;

static int edid_size(const u8 *edid, int data_size)
{
	if (data_size < EDID_LENGTH)
		return 0;

	return (edid[0x7e] + 1) * EDID_LENGTH;
}

static int get_display_type(void)
{
	denso_otp_data_t *hwinfo = NULL;
	struct intel_pdr_dirent *oem = NULL;
	uint8_t *pdr = NULL;

	do{
	/* By detault, DENSO_HW_CTRL_READ_PDR=n, and read_oem()
	 * is used. If =y, the oem region will be located via the PDR
	 * directoryi via find_oem_entry().
	 * */
	#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
	if(read_pdr(&pdr) == 0 &&
	  (oem = find_oem_entry((struct intel_pdr_header *)pdr)) != NULL)
	{
		hwinfo = (denso_otp_data_t *) (((uintptr_t) pdr) + oem->offset);
	#else
		if(read_oem(&hwinfo) == 0)
		{
	#endif
			DRM_INFO("get_display_type 0x%02x:successfully read nor\n",
					hwinfo->display_id);
			display_type_in_nor = hwinfo->display_id;
		}
		else {
			DRM_INFO("get_display_type:failed to read nor\n");
			break;
		}
	} while(0);
	#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
	if(pdr)
		kzfree(pdr);
	#else
	if(hwinfo)
		kzfree(hwinfo);
	#endif
	return display_type_in_nor;
}



static void *edid_load(struct drm_connector *connector, const char *name,
			const char *connector_name)
{
	const struct firmware *fw = NULL;
	const u8 *fwdata;
	u8 *edid;
	int fwsize, builtin;
	int i, valid_extensions = 0;
	int retry = 10;
	bool print_bad_edid = !connector->bad_edid_counter || (drm_debug & DRM_UT_KMS);

	/*obtain display type saved in NOR*/
	while (display_type_in_nor == -1 && retry--){
		msleep(200);
		display_type_in_nor = get_display_type();
	}

	do  {
		if (retry <= 0)
			DRM_ERROR("failed to get display type\n");
		else {
		/*display type      value
		* TJBX              0X1
		* TYAA              0X2
		*
		* if display type did not set in NOR or failed to read
		* display type from NOR more than 2 seconds.
		* Use TYAA display type as default
		*/
			if (display_type_in_nor == 0x1){
				builtin = match_string(generic_edid_name, GENERIC_EDIDS,
				"edid/honda-10in.bin");

				DRM_INFO("10 in. display\n");
				break;
			}
			else if (display_type_in_nor == 0x2)
			{
				builtin = match_string(generic_edid_name, GENERIC_EDIDS, name);
				DRM_INFO("12 in. display\n");
				break;
			}
			else if (display_type_in_nor == 0xff){
				DRM_INFO("display type was not set in NOR\n");
			}
		}
		builtin = match_string(generic_edid_name, GENERIC_EDIDS, name);
	} while (0);


	if (builtin >= 0) {
		fwdata = generic_edid[builtin];
		fwsize = sizeof(generic_edid[builtin]);
	} else {
		struct platform_device *pdev;
		int err;

		pdev = platform_device_register_simple(connector_name, -1, NULL, 0);
		if (IS_ERR(pdev)) {
			DRM_ERROR("Failed to register EDID firmware platform device "
				  "for connector \"%s\"\n", connector_name);
			return ERR_CAST(pdev);
		}

		err = request_firmware(&fw, name, &pdev->dev);
		platform_device_unregister(pdev);
		if (err) {
			DRM_ERROR("Requesting EDID firmware \"%s\" failed (err=%d)\n",
				  name, err);
			return ERR_PTR(err);
		}

		fwdata = fw->data;
		fwsize = fw->size;
	}

	if (edid_size(fwdata, fwsize) != fwsize) {
		DRM_ERROR("Size of EDID firmware \"%s\" is invalid "
			  "(expected %d, got %d\n", name,
			  edid_size(fwdata, fwsize), (int)fwsize);
		edid = ERR_PTR(-EINVAL);
		goto out;
	}

	edid = kmemdup(fwdata, fwsize, GFP_KERNEL);
	if (edid == NULL) {
		edid = ERR_PTR(-ENOMEM);
		goto out;
	}

	if (!drm_edid_block_valid(edid, 0, print_bad_edid,
				  &connector->edid_corrupt)) {
		connector->bad_edid_counter++;
		DRM_ERROR("Base block of EDID firmware \"%s\" is invalid ",
		    name);
		kfree(edid);
		edid = ERR_PTR(-EINVAL);
		goto out;
	}

	for (i = 1; i <= edid[0x7e]; i++) {
		if (i != valid_extensions + 1)
			memcpy(edid + (valid_extensions + 1) * EDID_LENGTH,
			    edid + i * EDID_LENGTH, EDID_LENGTH);
		if (drm_edid_block_valid(edid + i * EDID_LENGTH, i,
					 print_bad_edid,
					 NULL))
			valid_extensions++;
	}

	if (valid_extensions != edid[0x7e]) {
		u8 *new_edid;

		edid[EDID_LENGTH-1] += edid[0x7e] - valid_extensions;
		DRM_INFO("Found %d valid extensions instead of %d in EDID data "
		    "\"%s\" for connector \"%s\"\n", valid_extensions,
		    edid[0x7e], name, connector_name);
		edid[0x7e] = valid_extensions;

		new_edid = krealloc(edid, (valid_extensions + 1) * EDID_LENGTH,
				    GFP_KERNEL);
		if (new_edid)
			edid = new_edid;
	}

	DRM_INFO("Got %s EDID base block and %d extension%s from "
	    "\"%s\" for connector \"%s\"\n", (builtin >= 0) ? "built-in" :
	    "external", valid_extensions, valid_extensions == 1 ? "" : "s",
	    name, connector_name);

out:
	release_firmware(fw);
	return edid;
}

int drm_load_edid_firmware(struct drm_connector *connector)
{
	const char *connector_name = connector->name;
	char *edidname, *last, *colon, *fwstr, *edidstr, *fallback = NULL;
	int ret;
	struct edid *edid;

	if (edid_firmware[0] == '\0')
		return 0;

	/*
	 * If there are multiple edid files specified and separated
	 * by commas, search through the list looking for one that
	 * matches the connector.
	 *
	 * If there's one or more that doesn't specify a connector, keep
	 * the last one found one as a fallback.
	 */
	fwstr = kstrdup(edid_firmware, GFP_KERNEL);
	edidstr = fwstr;

	while ((edidname = strsep(&edidstr, ","))) {
		colon = strchr(edidname, ':');
		if (colon != NULL) {
			if (strncmp(connector_name, edidname, colon - edidname))
				continue;
			edidname = colon + 1;
			break;
		}

		if (*edidname != '\0') /* corner case: multiple ',' */
			fallback = edidname;
	}

	if (!edidname) {
		if (!fallback) {
			kfree(fwstr);
			return 0;
		}
		edidname = fallback;
	}

	last = edidname + strlen(edidname) - 1;
	if (*last == '\n')
		*last = '\0';

	edid = edid_load(connector, edidname, connector_name);
	kfree(fwstr);

	if (IS_ERR_OR_NULL(edid))
		return 0;

	drm_mode_connector_update_edid_property(connector, edid);
	ret = drm_add_edid_modes(connector, edid);
	drm_edid_to_eld(connector, edid);
	kfree(edid);

	return ret;
}
