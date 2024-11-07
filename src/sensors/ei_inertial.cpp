/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(inertial, CONFIG_EI_LOG_LEVEL);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "ei_inertial.h"

const struct device *const ism330dhcx = DEVICE_DT_GET_ONE(st_ism330dhcx);

static void ism330dhcx_config(const struct device *ism330dhcx)
{
	struct sensor_value odr_attr, fs_attr;

	/* set ISM330DHCX sampling frequency to 416 Hz */
	odr_attr.val1 = 416;
	odr_attr.val2 = 0;

	if (sensor_attr_set(ism330dhcx, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		LOG_ERR("Cannot set sampling frequency for ISM330DHCX accel");
		return;
	}

	sensor_g_to_ms2(16, &fs_attr);

	if (sensor_attr_set(ism330dhcx, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_FULL_SCALE, &fs_attr) < 0) {
		LOG_ERR("Cannot set fs for ISM330DHCX accel");
		return;
	}

	/* set ISM330DHCX gyro sampling frequency to 208 Hz */
	odr_attr.val1 = 208;
	odr_attr.val2 = 0;

	if (sensor_attr_set(ism330dhcx, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		LOG_ERR("Cannot set sampling frequency for ISM330DHCX gyro");
		return;
	}

	sensor_degrees_to_rad(250, &fs_attr);

	if (sensor_attr_set(ism330dhcx, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_FULL_SCALE, &fs_attr) < 0) {
		LOG_ERR("Cannot set fs for ISM330DHCX gyro");
		return;
	}
}

bool ei_inertial_init(void)
{
	if (!device_is_ready(ism330dhcx)) {
		LOG_ERR("%s: device not ready.", ism330dhcx->name);
		return false;
	}

    ism330dhcx_config(ism330dhcx);

    if(ei_add_sensor_to_fusion_list(inertial_sensor) == false) {
        LOG_ERR("ERR: failed to register inertial sensor!");
        return false;
    }

    return true;
}

float *ei_fusion_inertial_read_data(int n_samples)
{
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
    static float inertial[INERTIAL_AXIS_SAMPLED];

    memset(inertial, 0, INERTIAL_AXIS_SAMPLED * sizeof(float));

    if (sensor_sample_fetch(ism330dhcx) < 0) {
        LOG_ERR("ISM330DHCX IMU Sensor sample update error");
    }
    else {
        sensor_channel_get(ism330dhcx, SENSOR_CHAN_ACCEL_XYZ, accel);
		sensor_channel_get(ism330dhcx, SENSOR_CHAN_GYRO_XYZ, gyro);
        inertial[0] = sensor_value_to_double(&accel[0]);
        inertial[1] = sensor_value_to_double(&accel[1]);
        inertial[2] = sensor_value_to_double(&accel[2]);
        inertial[3] = sensor_value_to_double(&gyro[0]);
        inertial[4] = sensor_value_to_double(&gyro[1]);
        inertial[5] = sensor_value_to_double(&gyro[2]);
    }

    return inertial;
}