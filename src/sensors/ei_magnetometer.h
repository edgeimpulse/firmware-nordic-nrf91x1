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

#ifndef EI_MAGNETOMETER_H
#define EI_MAGNETOMETER_H

#include "firmware-sdk/ei_fusion.h"

#define MAG_AXIS_SAMPLED          3

bool ei_magnetometer_init(void);
float *ei_fusion_magnetometer_read_data(int n_samples);

static const ei_device_fusion_sensor_t magnetometer_sensor = {
    // name of sensor module to be displayed in fusion list
    "Magnetometer",
    // number of sensor module axis
    MAG_AXIS_SAMPLED,
    // sampling frequencies
    { 20.0f, 50.0f, 100.0f },
    // axis name and units payload (must be same order as read in)
    { {"magX", "mT"}, {"magY", "mT"}, {"magZ", "mT"} },
    // reference to read data function
    &ei_fusion_magnetometer_read_data,
    0
};

#endif /* EI_MAGNETOMETER_H */