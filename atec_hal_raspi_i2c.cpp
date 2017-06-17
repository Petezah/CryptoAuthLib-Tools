
#include <wiringPi.h>
#include <inttypes.h>
#include "gnublin.h"

#include "cryptoauthlib.h"

static gnublin_i2c* i2c = nullptr;

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
	(void)hal;
	i2c = new gnublin_i2c((cfg->atcai2c.slave_address >> 1));
	if(!i2c->fail())
	{
		return ATCA_SUCCESS;
	}

	return ATCA_COMM_FAIL;
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
	(void)iface;
	return ATCA_SUCCESS;
}

bool i2c_write(uint8_t *txdata, int len)
{
	int result = -1;
	if (len > 1)
	{
		result = i2c->send(txdata[0], txdata + 1, len - 1);
	}
	else
	{
		result = i2c->send(txdata, len);
	}
	if (result < 0)
	{
		const char *msg = i2c->getErrorMessage();
		printf("Error: %s\n", msg);
		return false;
	}

	return true;
}

bool i2c_read(uint8_t *rxdata, int len)
{
	int result = i2c->receive(rxdata, len);
	if (result < 0)
	{
		const char *msg = i2c->getErrorMessage();
		printf("Error: %s\n", msg);
		return false;
	}

	return true;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
	const ATCAIfaceCfg *cfg = atgetifacecfg(iface);
	void *i2c = atgetifacehaldat(iface);

	ATCA_STATUS status = ATCA_TX_TIMEOUT;

	if (txlength > 0) {
		txdata[0] = 0x03; /* Word Address Value = Command */
	}
	else {
		/* This is our (raspi) extension. In this mode WAV is set by wake or idle. */
	}
	txlength++; /* Include Word Address value in txlength */

	if (i2c_write(txdata, txlength))
	{
		status = ATCA_SUCCESS;
	}
	else 
	{
		status = ATCA_TX_FAIL;
	}
	return status;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
	const ATCAIfaceCfg *cfg = atgetifacecfg(iface);
	struct mgos_i2c *i2c = (struct mgos_i2c *) atgetifacehaldat(iface);

	ATCA_STATUS status = ATCA_RX_TIMEOUT;

	int retries = cfg->rx_retries;

	while (retries-- > 0) {
		if (!i2c_read(rxdata, *rxlength))
		{
			continue;
		}
		status = ATCA_SUCCESS;
		break;
	}
	/*
	* rxlength is a pointer, which suggests that the actual number of bytes
	* received should be returned in the value pointed to, but none of the
	* existing HAL implementations do it.
	*/
	return status;
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
	const ATCAIfaceCfg *cfg = atgetifacecfg(iface);
	struct mgos_i2c *i2c = (struct mgos_i2c *) atgetifacehaldat(iface);

	ATCA_STATUS status = ATCA_WAKE_FAILED;

	uint8_t response[4] = { 0x00, 0x00, 0x00, 0x00 };
	uint8_t expected_response[4] = { 0x04, 0x11, 0x33, 0x43 };

	/*
	* ATCA devices define "wake up" token as START, 80 us of SDA low, STOP.
	* Simulate this by trying to send 0 bytes to address 0 @ 100 kHz. This will
	* fail, but we're not expecting an ACK, so don't check for return value.
	*/
	i2c_write(response, 1);

	/* After wake signal we need to wait some time for device to init. */
	atca_delay_us(cfg->wake_delay);

	/* Receive the wake response. */
	uint16_t len = sizeof(response);
	status = hal_i2c_receive(iface, response, &len);
	if (status == ATCA_SUCCESS) {
		if (memcmp(response, expected_response, 4) != 0) {
			status = ATCA_WAKE_FAILED;
		}
	}

	return status;
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
	uint8_t idle_cmd = 0x02;
	return i2c_write(&idle_cmd, 1) ? ATCA_SUCCESS : ATCA_TX_FAIL;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
	uint8_t sleep_cmd = 0x01;
	return i2c_write(&sleep_cmd, 1) ? ATCA_SUCCESS : ATCA_TX_FAIL;
}

ATCA_STATUS hal_i2c_release(void *hal_data)
{
	(void)hal_data;
	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
	return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int busNum, ATCAIfaceCfg *cfg, int *found)
{
	return ATCA_UNIMPLEMENTED;
}

void atca_delay_ms(uint32_t ms) 
{
	delay(ms);
}

void atca_delay_us(uint32_t us) 
{
	delayMicroseconds(us);
}