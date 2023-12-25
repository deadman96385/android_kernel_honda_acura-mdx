#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/acpi.h>
#include <linux/gpio/consumer.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
extern unsigned char  emmc_ext_csd_version;

#include "max20038-i2c.h"

static struct usb_drv_data *max20038_drv = NULL;
static inline int i2c_write_le8(struct i2c_client *client, unsigned addr,
            unsigned data)
{
  int ret = i2c_smbus_write_byte_data(client, addr, data);
  if (ret < 0)
    dev_err(&client->dev, "Failed to write 0x%02x to 0x%02x,"
        "ret = %d", data, addr, ret);
  return ret;
}

static inline int i2c_read_le8(struct i2c_client *client, unsigned addr)
{
  int ret = i2c_smbus_read_byte_data(client, addr);
  if (ret < 0)
    dev_err(&client->dev, "Failed to read 0x%02x, ret = %d", addr, ret);
  return ret;
}

int get_max20038_SETUP_3 (void)
{
  int ret = -EIO;
  struct device *dev = NULL;
  if (max20038_drv == NULL){
    printk(KERN_ERR "usb_drv_data is NULL pointer\n");
    return -1;
  }
  dev = &max20038_drv->max_i2c_client->dev;
  ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_3);
  if (ret < 0){
    dev_err(dev,"failed to read SETUP_3\n");
    return ret;
  }
  dev_info(dev,"current SETUP_3 = %02hhx\n",ret);
  return ret;
}
int set_max20038_SETUP_3 (size_t value)
{
  int ret = -EIO;
  struct device *dev = NULL;
  if (max20038_drv == NULL){
    printk(KERN_ERR "usb_drv_data is NULL pointer\n");
    return -1;
  }
  dev = &max20038_drv->max_i2c_client->dev;
  ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_3);
  dev_info(dev," config  SETUP_3 from %02hhx to %02hhx\n",ret,value);
  ret = i2c_write_le8(max20038_drv->max_i2c_client, SETUP_3, value);
  if (ret < 0){
    dev_err(dev,"failed to write to SETUP_3\n");
    return ret;
  }

  return 0;
}

int set_max20038_vbus_on(void)
{
  int ret = -1;
  struct device *dev = NULL;
  dev = &max20038_drv->max_i2c_client->dev;

  /*set usb otg charger power on*/
  if (gpiod_get_value(max20038_drv->gpiod_usb_pwr) !=
      max20038_drv->vbus_power_value){
    msleep(100);
    gpiod_set_value(max20038_drv->gpiod_usb_pwr,
        max20038_drv->vbus_power_value);
    if (gpiod_get_value(max20038_drv->gpiod_usb_pwr) !=
        max20038_drv->vbus_power_value)
    {
      dev_err(dev, "failed to set usb power to %d\n",
          max20038_drv->vbus_power_value);
    }
    else {
      dev_info(dev, "set usb power to %d\n",
          max20038_drv->vbus_power_value);
      ret = 1;
    }
  }
  else {
    dev_info(dev, "usb vbus state is expected %d\n",
         max20038_drv->vbus_power_value);
    ret = 1;
  }

  return ret;

}

static ssize_t set_usb_pwr_state(struct device *dev,
     struct device_attribute *attr, const char *buf, size_t count)
{
  uint8_t gpio_value = 0;
  int rc;
  rc = sscanf(buf, "%hhx", &gpio_value);

  if (rc < 1){
    dev_err(dev, "failed to parse set value in buf\n");
    return -1;
  }

  if (max20038_drv->gain_value_set){
    gpiod_set_value(max20038_drv->gpiod_usb_pwr, gpio_value);
    dev_info(dev, "set_usb_pwr_state to %d\n",gpio_value);
  }
  else {
    max20038_drv->vbus_power_value = gpio_value;
    dev_info(dev, "failed to set_usb_pwr_state to %d"
        " SETUP_2 is not ready\n",gpio_value);
  }
  return count;
}

static ssize_t show_usb_pwr_state(struct device *dev,
        struct device_attribute *attr, char *buf)
{
  ssize_t len;
  len = scnprintf(buf, PAGE_SIZE, "%d\n",
      gpiod_get_value(max20038_drv->gpiod_usb_pwr));
  return len;

}

static DEVICE_ATTR(usb_pwr_state, S_IRUGO | S_IWUSR,
      show_usb_pwr_state, set_usb_pwr_state);

static struct attribute *usb_pwr_attributes[] = {
  &dev_attr_usb_pwr_state.attr,
  NULL
};
static const struct attribute_group usb_pwr_attr_group = {
  .attrs = usb_pwr_attributes,
};

static void read_nor_usb_gain_worker(struct work_struct *work)
{

  unsigned setup_2 = TYAA_USB_GAIN;
  unsigned setup_4 = USB_OV_VALUE_0;
  struct device *dev = &max20038_drv->max_i2c_client->dev;
  int ret;

  /*read setup_2 value from NOR*/
#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
  if(read_pdr(&max20038_drv->pdr) == 0 &&
      (max20038_drv->oem = find_oem_entry((struct intel_pdr_header *)max20038_drv->pdr)) != NULL)
  {
    max20038_drv->hwinfo = (denso_otp_data_t *) (((uintptr_t) max20038_drv->pdr) + max20038_drv->oem->offset);
#else
    if(read_oem(&max20038_drv->hwinfo) == 0)
    {
#endif
      dev_info(dev, "successfully read nor\n");
    }
    else {
      dev_err(dev, "failed to read nor\n");
      max20038_drv->retry--;
      if (max20038_drv->retry > 0)
        queue_delayed_work(max20038_drv->usb_workq,
            &max20038_drv->read_nor_usb_gain_work,
            max20038_drv->read_nor_delay);
      return;
    }
    if (max20038_drv->hwinfo->usb_gain < MIN_USB_GAIN ||
        max20038_drv->hwinfo->usb_gain > MAX_USB_GAIN ){
      if (max20038_drv->hwinfo->usb_gain == 0xFF )
        dev_warn (dev,"USB gain value is unset in NOR\n");
      else
        dev_err(dev, "invalid usb register value %02hhx in NOR\n",
            max20038_drv->hwinfo->usb_gain);
    }
    else {
      setup_2 = max20038_drv->hwinfo->usb_gain;
    }

    ret = i2c_write_le8(max20038_drv->max_i2c_client, SETUP_2, setup_2);
    if (ret < 0)
      dev_err(dev,"failed to write to SETUP_2\n");
    ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_2);
    dev_info(dev," SETUP_2 = %02hhx\n",ret);
    max20038_drv->gain_value_set = 1;

    /*set over current register according to eMMC version
     *eMMC5.0  0x16  eMMC5.1  0x17
     * */
    if (emmc_ext_csd_version == 0)
      dev_err(dev,"failed to get emmc_ext_csd_version\n");
    else if (emmc_ext_csd_version > 7)
      setup_4 = USB_OV_VALUE_1;

    ret = i2c_write_le8(max20038_drv->max_i2c_client, SETUP_4, setup_4);
    if (ret < 0)
    dev_err(dev,"failed to write to SETUP_4\n");
    ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_4);
    dev_info(dev," SETUP_4 = %02hhx\n",ret);


#ifdef CONFIG_DENSO_HW_CTRL_USE_PDR
  if(max20038_drv->pdr)
    kzfree(max20038_drv->pdr);
#else
  if(max20038_drv->hwinfo)
    kzfree(max20038_drv->hwinfo);
#endif

  }

static int max20038_i2c_probe(struct i2c_client *client,
          const struct i2c_device_id *id)
{
  int ret = -EIO;
  struct device *dev = &client->dev;
  max20038_drv = devm_kzalloc(dev, sizeof(*max20038_drv), GFP_KERNEL);
  if (!max20038_drv)
    return -ENOMEM;
  dev_set_drvdata(dev, max20038_drv);
  if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
  {
    dev_err(&client->dev, "i2c_check_functionality error!");
    return ret;
  }

  max20038_drv->gpiod_usb_pwr = devm_gpiod_get(&client->dev,
      "usb_otg_pwr", GPIOD_OUT_LOW);
  if (IS_ERR(max20038_drv->gpiod_usb_pwr)) {
    dev_err(&client->dev, "Not found usb power  gpio%ld\n",
        PTR_ERR(max20038_drv->gpiod_usb_pwr));
    return ret;
  }
  max20038_drv->max_i2c_client = client;

  /*set register
   *SETUP_1   0x23
   *SETUP_2   0x0E or set to value stored in NOR
   *SETUP_3   0x0C device mode  0x4C host mode
   *SETUP_4   0x16 on emmc5.0 FCB  0x17 on emmc5.1 FCB
   */

  /*default value is 0x23*/
  ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_1);
  dev_info(dev," SETUP_1 = %02hhx\n",ret);

  ret = i2c_write_le8(max20038_drv->max_i2c_client, SETUP_3, 0x0c);
  if (ret < 0)
    dev_err(dev,"failed to write to SETUP_3\n");
  ret = i2c_read_le8(max20038_drv->max_i2c_client, SETUP_3);
  dev_info(dev," SETUP_3 = %02hhx\n",ret);


  max20038_drv->read_nor_delay = msecs_to_jiffies(300);
  max20038_drv->retry = 10;

  max20038_drv->vbus_power_value = 1;
  max20038_drv->gain_value_set = 0;

  max20038_drv->usb_workq = create_singlethread_workqueue("usb_nor_gain");
  if (max20038_drv->usb_workq == NULL)
    dev_err(&client->dev,"failed to create usb workqueue \n");

  INIT_DELAYED_WORK(&max20038_drv->read_nor_usb_gain_work,
      read_nor_usb_gain_worker);

  ret = sysfs_create_group(&client->dev.kobj, &usb_pwr_attr_group);
  if ( ret < 0 )
    dev_err(&client->dev,"failed to create usb attr gruop \n");

  queue_delayed_work(max20038_drv->usb_workq,
      &max20038_drv->read_nor_usb_gain_work,
      max20038_drv->read_nor_delay);

  dev_info(&client->dev,"max20038 probe done\n");

  return 0;
}

static int max20038_i2c_remove(struct i2c_client *client)
{
  gpiod_set_value(max20038_drv->gpiod_usb_pwr, 0);
  sysfs_remove_group(&client->dev.kobj, &usb_pwr_attr_group);
  cancel_delayed_work_sync(&max20038_drv->read_nor_usb_gain_work);
  destroy_workqueue(max20038_drv->usb_workq);
  dev_set_drvdata(&client->dev, NULL);
  if (max20038_drv)
    kfree(max20038_drv);
  return 0;
}


static const struct acpi_device_id max20038_acpi_id[] = {
    { "MAX0038", 0  },
      {  }

};

MODULE_DEVICE_TABLE(acpi, max20038_acpi_id);

static const struct i2c_device_id max20038_id[] = {
  { "max20038", 0  },
  {  },
};


static struct i2c_driver max20038_i2c_driver = {
  .driver = {
    .owner  = THIS_MODULE,
    .name = "max20038",
    .acpi_match_table = ACPI_PTR(max20038_acpi_id),
  },
  .id_table = max20038_id,
  .probe    = max20038_i2c_probe,
  .remove   = max20038_i2c_remove,

};

module_i2c_driver(max20038_i2c_driver);

MODULE_LICENSE("GPL");
