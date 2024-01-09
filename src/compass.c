#include "compass.h"
#include "math.h"
#include "zephyr/drivers/sensor.h"
#include "zephyr/kernel.h"
#include "zephyr/sys/util.h"

/**
 * @brief
 * convert reading from channel SENSOR_CHAN_MAGN_XYZ to angle in 2d plane
 *
 * @param magXyz array of sensor values obtained through the channel
 * SENSOR_CHAN_MAGN_XYZ
 **/
double compass_gauss_to_heading2d(
    const struct compass_calibration_data *calibration_data,
    const struct sensor_value *magXyz) {

  double xGauss = sensor_value_to_double(magXyz),
         yGauss = sensor_value_to_double(magXyz + 1);

  return atan2(yGauss, xGauss);
}

/**
 * @brief Update calibration with sampled data
 *
 **/
static void compass_calibration_data_fetch(
    const struct device *compass_dev,
    struct compass_calibration_data *calibration_data) {
  sensor_sample_fetch(compass_dev);

  struct sensor_value mag_reads[3];

  sensor_channel_get(compass_dev, SENSOR_CHAN_MAGN_XYZ, mag_reads);

  calibration_data->xMax =
      MAX(calibration_data->xMax, sensor_value_to_double(mag_reads));

  calibration_data->yMax =
      MAX(calibration_data->yMax, sensor_value_to_double(mag_reads + 1));

  calibration_data->xMin =
      MIN(calibration_data->xMin, sensor_value_to_double(mag_reads));

  calibration_data->yMin =
      MIN(calibration_data->yMin, sensor_value_to_double(mag_reads + 1));
}
