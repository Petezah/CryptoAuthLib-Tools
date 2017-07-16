#include <wiringPi.h>
#include "cryptoauthlib.h"
#include "atca_helpers.h"

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys
// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command 
// which uses gpio export for setup for wiringPiSetupSys
#define	LED	17

uint8_t g_dev_config[ATCA_CONFIG_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0xc0, 0x01, 0x00,
	0xc0, 0x00, 0xaa, 0x00,
	0x8f, 0x20, 0xc4, 0x44,
	0x83, 0x20, 0x87, 0x30,
	0x8f, 0x0f, 0x83, 0xb0,
	0x83, 0x80, 0x83, 0x4f,
	0x0f, 0x0f, 0xcf, 0x4f,
	0xcf, 0x0f, 0x0f, 0x4f,
	0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f,
	0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x55, 0x55,
	0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x33, 0x00, 0x1c, 0x00,
	0x33, 0x00, 0x13, 0x00,
	0x3c, 0x00, 0x33, 0x00,
	0x33, 0x00, 0x33, 0x00,
	0x3c, 0x00, 0x3c, 0x00,
	0x3c, 0x00, 0x3c, 0x00,
	0x3c, 0x00, 0x3c, 0x00,
	0x3c, 0x00, 0x3c, 0x00
};

uint8_t g_otp_data[ATCA_OTP_SIZE] = "This is test data that can be read back out of the chip!huzzah!";

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

	if (!config_is_locked)
	{
		atcab_write_config_zone(g_dev_config);

		uint8_t testConfigData[ATCA_CONFIG_SIZE];
		atcab_read_config_zone(testConfigData);

		if (0 == memcmp(g_dev_config + 16, testConfigData + 16, ATCA_CONFIG_SIZE - 16))
		{
			printf("Config was successfully written!  Locking config zone!");

			uint16_t configCrc = 0;
			atCRC(ATCA_CONFIG_SIZE, g_dev_config, (uint8_t*)&configCrc);
			uint8_t lockResponse = 0;
			if (atcab_lock_config_zone(&lockResponse) == ATCA_SUCCESS)
			{
				printf("Success!\n");
			}
		}
	}

	if (!data_is_locked)
	{
		atcab_write_bytes_zone(ATCA_ZONE_OTP, 0, 0, g_otp_data, ATCA_OTP_SIZE);

		uint16_t otpCrc = 0;
		atCRC(ATCA_ZONE_OTP, g_dev_config, (uint8_t*)&otpCrc);
		uint8_t lockResponse = 0;
		if (atcab_lock_data_zone(&lockResponse) == ATCA_SUCCESS)
		{
			printf("Successfully write and locked OTP!\n");
		}
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

	uint8_t otpData[ATCA_CONFIG_SIZE];
	atcab_read_bytes_zone(ATCA_ZONE_OTP, 0, 0, otpData, ATCA_OTP_SIZE);
	printf("Data in OTP: %s\n", otpData);

	uint8_t num[32];
	status = atcab_random(num);
	atcab_printbin_label((const uint8_t*)"random" ,num, sizeof(num));

out:
	return 0;
}