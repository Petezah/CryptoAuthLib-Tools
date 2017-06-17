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
	printf("revision\n");
	atcab_printbin(revision, 4, true);

	status = atcab_read_serial_number(serial);
	if (status != ATCA_SUCCESS) {
		//LOG(LL_ERROR, ("ATCA: Failed to get chip serial number"));
		goto out;
	}
	printf("serial\n");
	atcab_printbin(serial, ATCA_SERIAL_NUM_SIZE, true);

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

	uint8_t num[32];
	status = atcab_random(num);
	printf("random\n");
	atcab_printbin(num, sizeof(num), true);

out:
	return 0;
}