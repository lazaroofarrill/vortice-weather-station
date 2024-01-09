#ifndef SRC_SENSORS_COMPASS_H_
#define SRC_SENSORS_COMPASS_H_
#include "zephyr/drivers/sensor.h"

struct compass_calibration_data {
  double xMax;
  double xMin;
  double yMax;
  double yMin;
};

double compass_gauss_to_heading2d(
    const struct compass_calibration_data *calibration_data,
    const struct sensor_value *magXyz);

#endif
