#include "compass.h"
#include "ds1307.h"
#include "errno.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/drivers/i2c.h"
#include "zephyr/drivers/sensor.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/logging/log_core.h"
#include "zephyr/sys/printk.h"
#include <stdint.h>

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

// BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console),
// zephyr_cdc_acm_uart),
//              "Console device is not ACM CDC UART device");

#define LED0_NODE DT_ALIAS(led0)

LOG_MODULE_REGISTER(app);

int i2c_ping(const struct device *i2c_dev, uint16_t addr) {
  const uint8_t msg[1] = {0};

  return i2c_write(i2c_dev, msg, 1, addr);
}

void scan_i2c(const struct device *i2c_dev, const struct gpio_dt_spec led) {
  for (uint8_t i = 3; i < 0x78; i++) {
    int result = i2c_ping(i2c_dev, i);

    if (i % 16 == 0)
      printk("\n%.2x:", i);

    if (result == 0) {
      printk(" %.2x", i);
    } else
      printk(" --");
  }

  gpio_pin_toggle_dt(&led);
}

// A build error here means the board is not supported
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main() {
  int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return 0;
  }

  const struct device *const i2c_dev = DEVICE_DT_GET(DT_ALIAS(i2c0));
  const struct device *const icm20948_dev = DEVICE_DT_GET(DT_ALIAS(compass));

  struct compass_calibration_data compass_calibration_data;

  printk("\n");

  if (!device_is_ready(i2c_dev)) {
    printk("Could not get i2c device\n");
    return -1;
  }

  scan_i2c(i2c_dev, led);
  k_msleep(1000);

  if (!device_is_ready(icm20948_dev)) {
    printk("Could not get icm20948 sensor");
    return -1;
  }

  printk("Using board %s\n", CONFIG_BOARD);
  printk("\n");

  //  struct RtcDs1307 *rtc = ds1307_create(i2c_dev, 0x68);

  char axis[] = {'x', 'y', 'z'};

  while (1) {

    struct sensor_value accel_xyz[3];
    struct sensor_value gyro_xyz[3];
    struct sensor_value mag_xyz[3];
    struct sensor_value temp;

    int err = sensor_sample_fetch(icm20948_dev);
    if (err) {
      k_msleep(100);
      continue;
    }
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_GYRO_XYZ, gyro_xyz);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_MAGN_XYZ, mag_xyz);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_DIE_TEMP, &temp);

    printk("acc( ");
    for (int i = 0; i < 3; i++) {
      printk("%c: %20.6f\t", axis[i], sensor_value_to_float(accel_xyz + i));
    }
    printk(" )\n");

    printk("gyro( ");
    for (int i = 0; i < 3; i++) {
      printk("%c: %20.6f\t", axis[i], sensor_value_to_float(gyro_xyz + i));
    }
    printk(" )\n");

    printk("mag( ");
    for (int i = 0; i < 3; i++) {
      printk("%c: %20.6f\t", axis[i], sensor_value_to_float(mag_xyz + i));
    }
    printk(" )");
    printk("\n");

    double angle =
        compass_gauss_to_heading2d(&compass_calibration_data, mag_xyz);

    // printk("Angle is: %6lf\n", angle);

    k_sleep(K_MSEC(1000));
    gpio_pin_toggle_dt(&led);
  }

  return 0;
}
