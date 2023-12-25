#ifndef  __MAX2008_I2C_H
#define  __MAX2008_I2C_H
#include "denso_hw_ctrl/src/nor_attributes.h"
#include "denso_hw_ctrl/src/mtd_api.h"
#include "denso_hw_ctrl/src/pdr.h"

#define  SETUP_1       0x00
#define  SETUP_2       0x01
#define  SETUP_3       0x02
#define  SETUP_4       0x03
#define  STATUS        0x07
#define  CDP_MODE      0x4c
#define  SDP_MODE      0x0c
#define  MAX_USB_GAIN  0x1F
#define  MIN_USB_GAIN  0x01
#define  TYAA_USB_GAIN 0x0E
#define  USB_OV_VALUE_0  0x16
#define  USB_OV_VALUE_1  0x17

struct usb_drv_data {
  struct i2c_client *max_i2c_client;
  struct gpio_desc* gpiod_usb_pwr;
  struct workqueue_struct *usb_workq;
  struct delayed_work read_nor_usb_gain_work;
  unsigned long read_nor_delay;
  denso_otp_data_t *hwinfo;
  struct intel_pdr_dirent *oem;
  uint8_t *pdi;
  uint32_t retry;
  uint32_t vbus_power_value;
  uint32_t gain_value_set;
};

int set_max20038_SETUP_3 (size_t value);
int get_max20038_SETUP_3 (void);
int set_max20038_vbus_on(void);
#endif /*__MAX2008_I2C_H*/


