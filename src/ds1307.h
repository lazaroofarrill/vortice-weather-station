#ifndef VORTICE_V2_DS1307_H_
#define VORTICE_V2_DS1307_H_

#include "zephyr/device.h"
#include "zephyr/drivers/i2c.h"
#include <stdint.h>
#include <time.h>

struct RtcDs1307 {
  const struct device *dev;
  uint8_t addr;
  uint16_t timestamp;
};

struct RtcDs1307 *ds1307_create(const struct device *device,
                                const uint8_t addr);

int ds1307_time_fetch(const struct RtcDs1307 *rtc);

int ds1307_time_set(const struct RtcDs1307 *rtc, struct tm *time);

int __ds1307_setup(struct RtcDs1307 *rtc, const struct device *device,
                   const uint8_t addr);

int ds1307_enable(const struct RtcDs1307 *rtc);

#endif
