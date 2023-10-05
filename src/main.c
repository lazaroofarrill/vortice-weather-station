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

  k_msleep(1000);
  gpio_pin_toggle_dt(&led);
}

// A build error here means the board is not supported
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main() {
  int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return 0;
  }

  const struct device *const i2c_dev = DEVICE_DT_GET(DT_ALIAS(board_i2c));
  const struct device *const as5600_dev = DEVICE_DT_GET(DT_ALIAS(vane));
  const struct device *const icm20948_dev =
      DEVICE_DT_GET_ANY(invensense_icm20948);

  printk("\n");

  if (!device_is_ready(i2c_dev)) {
    printk("Could not get i2c device\n");
    return -1;
  }

  printk("Using board %s\n", CONFIG_BOARD);
  printk("Scanning i2c:\n");
  scan_i2c(i2c_dev, led);
  printk("\n");

  //  struct RtcDs1307 *rtc = ds1307_create(i2c_dev, 0x68);

  while (1) {
    struct sensor_value angle;
    struct sensor_value accel_x;

    //    sensor_sample_fetch(as5600_dev);
    //    sensor_channel_get(as5600_dev, SENSOR_CHAN_ROTATION, &angle);

    sensor_sample_fetch(icm20948_dev);
    sensor_channel_get(icm20948_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
    //    printk("angle: %d.%06d\n", angle.val1, angle.val2);
    // ds1307_time_fetch(rtc);
    //
    printk("x: %d.%d", accel_x.val1, accel_x.val2);

    k_sleep(K_MSEC(1000));
    gpio_pin_toggle_dt(&led);
  }

  return 0;
}
