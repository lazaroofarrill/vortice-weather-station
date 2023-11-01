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

#define LED0_NODE DT_ALIAS(led0)
#define SLEEP_TIME_MS 1000

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

  printk("\n");

  if (!device_is_ready(i2c_dev)) {
    printk("Could not get i2c device\n");
    return -1;
  }

  if (!device_is_ready(icm20948_dev)) {
    printk("Could not get icm20948 sensor");
    return -1;
  }

  printk("Using board %s\n", CONFIG_BOARD);
  printk("Scanning i2c:\n");
  scan_i2c(i2c_dev, led);
  printk("\n");

  //  struct RtcDs1307 *rtc = ds1307_create(i2c_dev, 0x68);

  char axis[] = {'x', 'y', 'z'};

  while (1) {
    struct sensor_value accel_xyz[3];
    struct sensor_value temp;
    float temp_deg = sensor_value_to_float(&temp);
    float accel_xyz_f[3];
    for (int i = 0; i < 3; i++) {
      accel_xyz_f[i] = sensor_value_to_float(accel_xyz + i);
    }

    sensor_sample_fetch(icm20948_dev);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_DIE_TEMP, &temp);

    printk("temp: %.2f\t", temp_deg);
    printk("acc[ ");
    for (int i = 0; i < 3; i++) {
      printk("%c: %20.6f\t", axis[i], accel_xyz_f[i]);
    }
    printk(" ]");
    printk("\n");

    k_sleep(K_MSEC(1000));
    gpio_pin_toggle_dt(&led);
  }

  return 0;
}
