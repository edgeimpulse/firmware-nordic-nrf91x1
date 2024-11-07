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
LOG_MODULE_REGISTER(main, CONFIG_EI_LOG_LEVEL);

#include <zephyr/kernel.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/uart.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "ei_device_nrf91.h"
#include "ei_at_handlers.h"
#include "ei_ws_client.h"
#include "inference/ei_run_impulse.h"
#include "sensors/ei_accelerometer.h"
#include "sensors/ei_inertial.h"
#include "sensors/ei_magnetometer.h"
#include "sensors/ei_microphone.h"

const struct device *uart;

void ei_putchar(char c)
{
    // it is ~30% faster than printf
    uart_poll_out(uart, c);
}

void nrf_modem_fault_handler(struct nrf_modem_fault_info *fault_info)
{
	LOG_ERR("Modem error: 0x%x, PC: 0x%x", fault_info->reason, fault_info->program_counter);
}

#ifdef CONFIG_REMOTE_INGESTION
void pdn_event_handler(uint8_t cid, enum pdn_event event, int reason)
{
	switch (event) {
	case PDN_EVENT_CNEC_ESM:
		LOG_DBG("PDP context %d error, %s", cid, pdn_esm_strerror(reason));
		break;
	case PDN_EVENT_ACTIVATED:
		LOG_DBG("PDP context %d activated", cid);
		break;
	case PDN_EVENT_DEACTIVATED:
		LOG_DBG("PDP context %d deactivated", cid);
		break;
	case PDN_EVENT_NETWORK_DETACH:
		LOG_DBG("PDP context %d network detached", cid);
		break;
	case PDN_EVENT_IPV6_UP:
		LOG_DBG("PDP context %d IPv6 up", cid);
		break;
	case PDN_EVENT_IPV6_DOWN:
		LOG_DBG("PDP context %d IPv6 down", cid);
		break;
	default:
		LOG_DBG("PDP context %d, unknown event %d", cid, event);
		break;
	}
}

bool modem_connect(void)
{
	int ret;

	LOG_DBG("Modem Lib init...");
	ret = nrf_modem_lib_init();
	if (ret < 0) {
		LOG_ERR("Unable to init modem library (%d)", ret);
		return false;
	}
	LOG_DBG("Modem Lib init... OK");

	/* Setup a callback for the default PDP context (zero).
	 * Do this before switching to function mode 1 (CFUN=1)
	 * to receive the first activation event.
	 */
	LOG_DBG("PDN context init...");
	ret = pdn_default_ctx_cb_reg(pdn_event_handler);
	if (ret) {
		LOG_ERR("pdn_default_ctx_cb_reg() failed, err %d", ret);
		return false;
	}
	LOG_DBG("PDN context init... OK");

	LOG_DBG("Waiting for network...");
	ret = lte_lc_init_and_connect();
	if (ret) {
		LOG_ERR("Failed to connect to the LTE network, err %d", ret);
		return false;
	}
	LOG_DBG("Waiting for network... OK");

	return true;
}
#endif

int main(void)
{
    char rcv_char;
    ATServer *at;

    /* This is needed so that output of printf
       is output immediately without buffering
    */
    setvbuf(stdout, NULL, _IONBF, 0);

    uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
    if (!device_is_ready(uart)) {
		LOG_ERR("%s: device not ready\n", uart->name);
	}

    EiDeviceNRF91* dev = static_cast<EiDeviceNRF91*>(EiDeviceInfo::get_device());

#ifdef CONFIG_REMOTE_INGESTION
	if(!modem_connect()) {
		LOG_ERR("Modem connection failed!");
	}
	else {
    	ei_ws_client_start(dev, at_sample_start);
	}
#else
	// we need modem library to get IMEI and obtain unique device ID
	int ret = nrf_modem_lib_init();
	if (ret < 0) {
		LOG_ERR("Unable to init modem library (%d)", ret);
	}
#endif

	// based on accelerometer presence we can see if the shield is present,
	// and know if the microphone is present since it cannot be detected by the driver SDK
    dev->set_microphone_present(ei_accelerometer_init());
    ei_inertial_init();
    ei_magnetometer_init();
    ei_microphone_init();
	dev->set_sensor_list(EiDeviceInfo::get_device()->get_memory());

    dev->set_state(eiStateFinished);

    // Init AT Server and print initial prompt
    at = ei_at_init(dev);
    ei_printf("Type AT+HELP to see a list of commands.\r\n");
    at->print_prompt();

    while(true) {
        while(uart_fifo_read(uart, (uint8_t*)&rcv_char, 1) == 1) {
            if(is_inference_running() && rcv_char == 'b') {
                ei_stop_impulse();
                at->print_prompt();
                continue;
            }
            at->handle(rcv_char);
        }
        if(is_inference_running() == true) {
            ei_run_impulse();
        }
        k_sleep(K_MSEC(1));
    }

    return 0;
}
