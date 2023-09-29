#include "errno.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/drivers/i2c.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/sys/printk.h"
#include <stdint.h>

#define LED0_NODE DT_ALIAS(led0)
#define SLEEP_TIME_MS 1000

int i2c_ping(const struct device *i2c_dev, uint16_t addr) {
  const uint8_t msg[1] = {0};

  return i2c_write(i2c_dev, msg, 1, addr);
}

// A build error here means the board is not supported
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

LOG_MODULE_REGISTER(imu_logger, LOG_LEVEL_INF);

int main() {
  printk("hello world");
  int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return 0;
  }

  //  while (1) {
  //    ret = gpio_pin_toggle_dt(&led);
  //    if (ret < 0) {
  //      return 0;
  //    }
  //    k_msleep(SLEEP_TIME_MS);
  //  }

  const struct device *const i2c_dev = DEVICE_DT_GET(DT_ALIAS(board_i2c));
  printk("\n");

  if (!device_is_ready(i2c_dev)) {
    printk("Could not get i2c device\n");
    return -1;
  }

  printk("Using board %s\n", CONFIG_BOARD);
  printk("Scanning i2c:\n");

  while (true) {
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
  return 0;
}
