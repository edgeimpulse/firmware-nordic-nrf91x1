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
LOG_MODULE_REGISTER(memory, CONFIG_EI_LOG_LEVEL);
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include "flash_memory.h"

#define EXTERNAL_FLASH_DEVICE_SIZE_BITS     DT_PROP(DT_INST(0, jedec_spi_nor), size)    /*< configured in device tree of the board >*/
#define EXTERNAL_FLASH_DEVICE_SIZE_BYTES    ((EXTERNAL_FLASH_DEVICE_SIZE_BITS)/8)       /*< On board Flash size, size in DT is in bits >*/



uint32_t EiFlashMemory::read_data(uint8_t *data, uint32_t address, uint32_t num_bytes)
{
    int ret;

    if(address + num_bytes > this->memory_size) {
        num_bytes = this->memory_size - address;
    }

    ret = flash_read(this->flash_dev, address, data, num_bytes);
    if(ret) {
        LOG_ERR("flash_read failed: %d", ret);
        return 0;
    }

    return num_bytes;
}

uint32_t EiFlashMemory::write_data(const uint8_t *data, uint32_t address, uint32_t num_bytes)
{
    int ret;

    if(address + num_bytes > this->memory_size) {
        LOG_ERR("write_data out of bounds");
        return 0;
    }

    ret = flash_write(this->flash_dev, address, data, num_bytes);
    if(ret) {
        LOG_ERR("flash_write failed: %d", ret);
        return 0;
    }

    return num_bytes;
}

uint32_t EiFlashMemory::erase_data(uint32_t address, uint32_t num_bytes)
{
    int ret;
    struct flash_pages_info info;
    uint32_t page_offset;
    uint32_t bytes_to_erase = num_bytes;

    if(address + num_bytes > this->memory_size) {
        LOG_ERR("erase_data out of bounds");
        return 0;
    }

    // Check if address is page aligned
    ret = flash_get_page_info_by_offs(this->flash_dev, address, &info);
    if(ret) {
        LOG_ERR("page_info failed: %d", ret);
        return 0;
    }

    // calculate page offset and increase number of bytes to erase
    page_offset = address - info.start_offset;
    bytes_to_erase += page_offset;

    // check if numkber of bytes to erase is a multiple of page size
    if(bytes_to_erase % info.size != 0) {
        bytes_to_erase += info.size - (bytes_to_erase % info.size);
    }

    ret = flash_erase(this->flash_dev, info.start_offset, bytes_to_erase);
    if(ret) {
        LOG_ERR("flash_erase failed: %d", ret);
        return 0;
    }

    return num_bytes;
}

EiFlashMemory::EiFlashMemory(uint32_t config_size):
    EiDeviceMemory(
        config_size,
        10,
        EXTERNAL_FLASH_DEVICE_SIZE_BYTES,
        CONFIG_SPI_NOR_FLASH_LAYOUT_PAGE_SIZE),
    flash_dev(DEVICE_DT_GET_ONE(jedec_spi_nor))
    {
    if (!device_is_ready(this->flash_dev)) {
		LOG_ERR("%s: device not ready.", this->flash_dev->name);
	}
}
