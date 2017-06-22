#include <wiringPi.h>
#include "cryptoauthlib.h"
#include "atca_helpers.h"

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys
// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command 
// which uses gpio export for setup for wiringPiSetupSys
#define	LED	17

int main(void)
{
	//wiringPiSetupGpio();

	const ATCAIfaceCfg *atca_cfg;
	atca_cfg = &cfg_ateccx08a_i2c_default;

	ATCA_STATUS status;
	uint8_t revision[4];
	uint8_t serial[ATCA_SERIAL_NUM_SIZE];

	status = atcab_init(atca_cfg);
	if (status != ATCA_SUCCESS) {
		//LOG(LL_ERROR, ("ATCA: Library init failed"));
		goto out;
	}

	status = atcab_info(revision);
	if (status != ATCA_SUCCESS) {
		//LOG(LL_ERROR, ("ATCA: Failed to get chip info"));
		goto out;
	}
	atcab_printbin_label((const uint8_t*)"revision", revision, 4);

	status = atcab_read_serial_number(serial);
	if (status != ATCA_SUCCESS) {
		//LOG(LL_ERROR, ("ATCA: Failed to get chip serial number"));
		goto out;
	}
	atcab_printbin_label((const uint8_t*)"serial", serial, ATCA_SERIAL_NUM_SIZE);

	bool config_is_locked, data_is_locked;
	status = atcab_is_locked(LOCK_ZONE_CONFIG, &config_is_locked);
	status = atcab_is_locked(LOCK_ZONE_DATA, &data_is_locked);
	if (status != ATCA_SUCCESS) {
		//LOG(LL_ERROR, ("ATCA: Failed to get chip zone lock status"));
		goto out;
	}

	size_t configSize, otpSize, dataSize[16];
	atcab_get_zone_size(ATCA_ZONE_CONFIG, 0, &configSize);
	atcab_get_zone_size(ATCA_ZONE_OTP, 0, &otpSize);
	for (uint16_t i = 0; i < 16; ++i)
	{
		atcab_get_zone_size(ATCA_ZONE_DATA, i, &dataSize[i]);
	}

	uint8_t configData[ATCA_CONFIG_SIZE];
	atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, 0, configData, ATCA_CONFIG_SIZE);
	atcab_printbin_label((const uint8_t*)"config", configData, sizeof(configData));

	uint8_t num[32];
	status = atcab_random(num);
	atcab_printbin_label((const uint8_t*)"random" ,num, sizeof(num));

out:
	return 0;
}