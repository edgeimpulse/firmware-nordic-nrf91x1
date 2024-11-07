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

#ifndef EI_DEVICE_NRF91_H
#define EI_DEVICE_NRF91_H

#include "firmware-sdk/ei_device_info_lib.h"
#include "firmware-sdk/ei_device_memory.h"
#include "ei_fusion_sensors_config.h"

class EiDeviceNRF91 : public EiDeviceInfo {
private:
    EiDeviceNRF91() = delete;
    std::string mac_address = "00:11:22:33:44:55:66";
    EiState state;
    int standalone_sensor_num = 0;
    static const int standalone_sensor_num_max = 1;
    ei_device_sensor_t standalone_sensor_list[standalone_sensor_num_max];

public:
    std::string get_mac_address(void);

    EiDeviceNRF91(EiDeviceMemory* mem);
    ~EiDeviceNRF91();

    void (*sample_read_callback)(void);
    void init_device_id(void);
    bool start_sample_thread(void (*sample_read_cb)(void), float sample_interval_ms) override;
    bool stop_sample_thread(void) override;

    void set_state(EiState state) override;
    EiState get_state(void);

    const std::string& get_device_id(void) override;

    bool get_sensor_list(const ei_device_sensor_t **sensor_list, size_t *sensor_list_size) override;
    bool set_sensor_list(EiDeviceMemory* mem);
    uint32_t get_data_output_baudrate(void) override;

#if MULTI_FREQ_ENABLED == 1
    bool start_multi_sample_thread(void (*sample_multi_read_cb)(uint8_t), float* multi_sample_interval_ms, uint8_t num_fusioned) override;
    void (*sample_multi_read_callback)(uint8_t);
#endif
    void set_microphone_present(bool present);
};

#endif /* EI_DEVICE_NRF91_H */
