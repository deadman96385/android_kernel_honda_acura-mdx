/* Copyright (c) DENSO CORPORATION.  ALL RIGHTS RESERVED. */
#ifndef _NOR_ATTRIBUTES_H_
#define _NOR_ATTRIBUTES_H_

/* Maximum size in bytes for each attribute */
#define NOR_MAX_DATASIZE   (32)
#define NOR_INT_DATASIZE   (sizeof(int))
#define NOR_VERSION        (2)

/* all the nor attributes, in one easy to change place */
#define PRODUCT                  product
#define PRODUCT_REVISION         product_revision
#define CURRENT_VIN              current_vin
#define ORIGINAL_VIN             original_vin
#define BLUETOOTH_ADDRESS        bluetooth_address
#define SERIAL_NUMBER            serial_number
#define MANUFACTURE_DATE         manufacture_date
#define MANUFACTURE_COUNT        manufacture_count
#define HERE_ID                  here_id
#define MODEL                    model
#define PRODUCTION_HISTORY       production_history
#define TEMP_SENSOR_LOW          temp_sensor_low
#define TEMP_SENSOR_ROOM         temp_sensor_room
#define TEMP_SENSOR_HIGH         temp_sensor_high
#define GYRO_TEMP_LOW            gyro_temp_low
#define GYRO_TEMP_ROOM           gyro_temp_room
#define GYRO_TEMP_HIGH           gyro_temp_high
#define UPDATE_COUNT             update_count
#define CHECKSUM                 checksum
#define USB_GAIN                 usb_gain_value
#define DISPLAY_ID               display_id

#define LENGTH_HERE_ID          (22)
#define LENGTH_VIN              (17)
#define LENGTH_BT_ADDR          (6)
/* limits */
#define MIN_TEMP                (-128)
#define MAX_TEMP                (127)
#define MAX_DATE                (0xffff)
#define MAX_COUNT               (0xffff)
#define MAX_SERIAL              (0xffffffff)
#define MAX_HISTORY             (0xffffffff)
#define MAX_PRODUCT             (0x0f)
#define MAX_MODEL               (0x0f)
#define MAX_REVISION            (0xff)
#define MAX_USB_GAIN_VALUE      (0x7f)
#define MAX_DISPLAY_ID          (0x7f)

typedef struct __attribute__((packed)) {
	uint32_t    version;
	uint32_t    update_count;
	uint32_t    checksum;
	uint32_t    product;
	uint32_t    model;
	uint32_t    revision;
	uint32_t    serial;
	uint8_t     bt_address[NOR_MAX_DATASIZE];
	uint8_t     here_id[NOR_MAX_DATASIZE];
	uint64_t    mfg_date;
	uint64_t    mfg_count;
	uint8_t     vin_original[NOR_MAX_DATASIZE];
	uint8_t     vin_current[NOR_MAX_DATASIZE];
	int8_t      temp_low;
	int8_t      temp_high;
	int8_t      temp_room;
	int8_t      gyro_low;
	int8_t      gyro_high;
	int8_t      gyro_room;
	uint64_t    history;
	uint8_t     usb_gain;
	uint8_t     display_id;
} denso_otp_data_t;

int read_oem(denso_otp_data_t **);

#endif /* #ifndef _NOR_ATTRIBUTES_H_ */
