#include "ds1307.h"
#include "stdlib.h"
#include "time.h"
#include "zephyr/drivers/i2c.h"
#include "zephyr/kernel.h"
#include "zephyr/logging/log.h"
#include "zephyr/sys/printk.h"
#include "zephyr/sys/sys_heap.h"
#include "zephyr/sys/timeutil.h"
#include "zephyr/sys/util.h"
#include <stdint.h>
#include <sys/_timespec.h>

struct RtcDs1307 *ds1307_create(const struct device *device,
                                const uint8_t addr) {

  struct RtcDs1307 *rtc = k_malloc(sizeof(struct RtcDs1307));
  __ds1307_setup(rtc, device, addr);
  return rtc;
}

int ds1307_time_set(const struct RtcDs1307 *rtc, struct tm *time) { return 0; }

int ds1307_time_fetch(const struct RtcDs1307 *rtc) {
  uint8_t read_buff[7] = {0}, write_buff[1] = {0x00};

  // Read from register 0x00 to register 0x06
  int ret = i2c_write_read(rtc->dev, rtc->addr, write_buff, sizeof(write_buff),
                           read_buff, sizeof(read_buff));

  if (ret != 0) {
    return ret;
  }

  uint8_t seconds, minutes, hours, day, date, month, year;
  seconds = bcd2bin(read_buff[0] & 0x7F);
  minutes = bcd2bin(read_buff[1] & 0x7F);
  hours = bcd2bin(read_buff[2] & 0x7E);
  day = bcd2bin(read_buff[3] & 0x07);
  date = bcd2bin(read_buff[4] & 0x7E);
  month = bcd2bin(read_buff[5] & 0x7D);
  year = bcd2bin(read_buff[6]);

  struct tm time = {.tm_min = minutes,
                    .tm_sec = seconds,
                    .tm_mon = month,
                    .tm_hour = hours,
                    .tm_mday = date,
                    .tm_wday = day,
                    .tm_year = year};

  time_t unix_time = timeutil_timegm(&time);

  printk("seconds: %d\n"
         "minutes: %d\n"
         "hours: %d\n"
         "day: %d\n"
         "date: %d\n"
         "month: %d\n"
         "year: %d\n",
         seconds, minutes, hours, day, date, month, year);
  printk("timestamp: %lld\n", unix_time);
  printk("\n\n");

  uint8_t config = 0;
  ret = i2c_reg_read_byte(rtc->dev, rtc->addr, 0x00, &config);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

void __ds1307_setup(struct RtcDs1307 *rtc, const struct device *device,
                    const uint8_t addr) {
  rtc->dev = device;
  rtc->addr = addr;
  rtc->timestamp = 0;
}

void ds1307_enable(const struct RtcDs1307 *rtc) {
  int ret = i2c_reg_update_byte(rtc->dev, rtc->addr, 0x00, 0x08, 0x00);

  if (ret != 0) {
    printk("ERROR when enabling clock");
  }
}
