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
#include <errno.h>
#include <stdint.h>
#include <sys/_timespec.h>

LOG_MODULE_REGISTER(DS1307, LOG_LEVEL_DBG);

struct RtcDs1307 *ds1307_create(const struct device *device,
                                const uint8_t addr) {

  struct RtcDs1307 *rtc = k_malloc(sizeof(struct RtcDs1307));
  __ds1307_setup(rtc, device, addr);
  ds1307_enable(rtc);

  LOG_INF("RTC Instance created");

  return rtc;
}

const static uint8_t kRtcBitmasks[7] = {0x7F, 0x7F, 0x3F, 0x07,
                                        0x3F, 0x1F, 0xFF};

/***
 * @param rtc pointer of type const struct RtcDs1307
 * @param buff pointer of type uint8_t to store the read buffer.
 * The pointer must be longer than 7 bytes to store all the information.
 *
 * @retval 0 if successful.
 * @retval -EINVAL if buffer is shorter than 7 bytes.
 ***/
int __ds1307_reg_load(const struct RtcDs1307 *rtc, uint8_t *buff) {
  uint8_t write_buff[1] = {0x00};
  const uint8_t kRegSize = 7;

  int err = i2c_write_read(rtc->dev, rtc->addr, write_buff, 1, buff, kRegSize);
  if (err != 0) {
    LOG_ERR("Error reading data from RTC");
    return err;
  }

  return 0;
}

int ds1307_time_set(const struct RtcDs1307 *rtc, struct tm *time) {
  uint8_t write_buff[8] = {0};

  // Disable rtc
  int err = i2c_reg_update_byte(rtc->dev, rtc->addr, 0x00, 0x80, 0xFF);
  if (err != 0) {
    LOG_ERR("Could not stop RTC");
    return err;
  }

  LOG_DBG("RTC Disabled");

  err = __ds1307_reg_load(rtc, write_buff + 1);
  if (err != 0) {
    return err;
  }

  for (int i = 0; i < 7; i++) {
    write_buff[i + 1] = write_buff[i + 1] & kRtcBitmasks[i - 1];
  }

  write_buff[0] = 0x00;
  write_buff[1] = write_buff[1] | bin2bcd(time->tm_sec);
  write_buff[2] = write_buff[2] | bin2bcd(time->tm_min);
  write_buff[3] = write_buff[3] | bin2bcd(time->tm_hour);
  write_buff[4] = write_buff[4] | bin2bcd(time->tm_wday + 1);
  write_buff[5] = write_buff[5] | bin2bcd(time->tm_mday);
  write_buff[6] = write_buff[6] | bin2bcd(time->tm_mon + 1);
  write_buff[7] = write_buff[7] | bin2bcd(time->tm_year - 100);

  err = i2c_write(rtc->dev, write_buff, sizeof(write_buff), rtc->addr);
  if (err != 0) {
    return err;
  }

  err = ds1307_enable(rtc);
  if (err != 0) {
    return err;
  }

  return 0;
}

int ds1307_time_fetch(const struct RtcDs1307 *rtc) {
  uint8_t read_buff[7] = {0};

  // Read from register 0x00 to register 0x06
  int ret = __ds1307_reg_load(rtc, read_buff);
  if (ret != 0) {
    return ret;
  }

  uint8_t seconds, minutes, hours, day, date, month, year;
  seconds = bcd2bin(read_buff[0] & kRtcBitmasks[0]);
  minutes = bcd2bin(read_buff[1] & kRtcBitmasks[1]);
  hours = bcd2bin(read_buff[2] & kRtcBitmasks[2]);
  day = bcd2bin(read_buff[3] & kRtcBitmasks[3]);
  date = bcd2bin(read_buff[4] & kRtcBitmasks[4]);
  month = bcd2bin(read_buff[5] & kRtcBitmasks[5]);
  year = bcd2bin(read_buff[6] & kRtcBitmasks[6]);

  struct tm time = {.tm_min = minutes,
                    .tm_sec = seconds,
                    .tm_mon = month - 1, // months in DS1307 go from 1-12
                    .tm_hour = hours,
                    .tm_mday = date,
                    .tm_wday = day - 1,
                    .tm_year = (int)year + 100}; // RTC returns years from 2000

  time_t unix_time = timeutil_timegm(&time);

  printk("reg 0: %02X\n"
         "reg 2: %02X\n\n",
         read_buff[0], read_buff[2]);
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

  if (ret != 0) {
    return ret;
  }

  return 0;
}

int __ds1307_setup(struct RtcDs1307 *rtc, const struct device *device,
                   const uint8_t addr) {
  rtc->dev = device;
  rtc->addr = addr;
  rtc->timestamp = 0;

  int err = i2c_reg_update_byte(rtc->dev, rtc->addr, 0x02, 0x40, 0xFF);
  if (err != 0) {
    LOG_ERR("RTC could not be set to 24H mode");
    return err;
  } else {
    LOG_INF("RTC set in 24H mode.");
  }
  return 0;
}

int ds1307_enable(const struct RtcDs1307 *rtc) {
  // Enable oscillator
  int err = i2c_reg_update_byte(rtc->dev, rtc->addr, 0x00, 0x80, 0x00);
  if (err != 0) {
    LOG_ERR("RTC could not be enabled.");
    return err;
  }

  return 0;
}
