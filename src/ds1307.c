#include "ds1307.h"
#include "stdlib.h"

struct RtcDs1307 *ds1307_create(const struct device *device,
                                const uint8_t addr) {
  struct RtcDs1307 *rtc = (struct RtcDs1307*)malloc(sizeof(struct RtcDs1307);
  ds1307_setup(rtc, device, addr);
  return rtc;
}

void ds1307_time_fetch(const struct RtcDs1307 *rtc) {}

void ds1307_setup(struct RtcDs1307 *rtc, const struct device *device,
                  const uint8_t addr) {
  rtc->i2c_dev = device;
  rtc->address = addr;
}

void ds1307_enable(const struct RtcDs1307 *rtc) {
  int ret = i2c_reg_update_byte(rtc->i2c_dev, rtc->address, 0x00, 0x08, 0x00);
}
