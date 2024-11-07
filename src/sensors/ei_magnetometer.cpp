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
LOG_MODULE_REGISTER(magnetometer, CONFIG_EI_LOG_LEVEL);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "ei_magnetometer.h"

const struct device *const iis2mdc = DEVICE_DT_GET_ONE(st_iis2mdc);

static void iis2mdc_config(const struct device *iis2mdc)
{
	struct sensor_value odr_attr;

	/* set IIS2MDC sampling frequency to 100 Hz */
	odr_attr.val1 = 100;
	odr_attr.val2 = 0;

	if (sensor_attr_set(iis2mdc, SENSOR_CHAN_ALL,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
		LOG_ERR("Cannot set sampling frequency for IIS2MDC");
		return;
	}
}

bool ei_magnetometer_init(void)
{
	if (!device_is_ready(iis2mdc)) {
		LOG_ERR("%s: device not ready.", iis2mdc->name);
		return false;
	}

    iis2mdc_config(iis2mdc);

    if(ei_add_sensor_to_fusion_list(magnetometer_sensor) == false) {
        LOG_ERR("ERR: failed to register magnetometer sensor!");
        return false;
    }

    return true;
}

float *ei_fusion_magnetometer_read_data(int n_samples)
{
    struct sensor_value magn[MAG_AXIS_SAMPLED];
    static float magn_mt[MAG_AXIS_SAMPLED];

    memset(magn_mt, 0, MAG_AXIS_SAMPLED * sizeof(float));

    if (sensor_sample_fetch(iis2mdc) < 0) {
        LOG_ERR("IIS2MDC Magn Sensor sample update error");
    }
    else {
        sensor_channel_get(iis2mdc, SENSOR_CHAN_MAGN_XYZ, magn);
        // convert gauss to militesla
        magn_mt[0] = sensor_value_to_double(&magn[0]) / 10.0f;
        magn_mt[1] = sensor_value_to_double(&magn[1]) / 10.0f;
        magn_mt[2] = sensor_value_to_double(&magn[2]) / 10.0f;
    }

    return magn_mt;
}