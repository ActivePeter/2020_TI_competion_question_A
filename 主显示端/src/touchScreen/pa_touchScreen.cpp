
#include "pa_touchScreen.h"

pa_touchScreen pa_touchScreen::instance = pa_touchScreen();

pa_touchScreen::pa_touchScreen() {}

uint16_t pa_touchScreen::spiRead()
{
	uint8_t i = 12;
	uint16_t value = 0;

	while (1)
	{

		Hardware_setCLK(1);
		// pa_delayMs(1);
		Hardware_setCLK(0);
		// pa_delayMs(1);

		if (Hardware_ReadMISO() != 0)
		{
			value |= 1;
		}
		i--;
		if (i == 0)
		{
			break;
		}
		value <<= 1;
	};

	return value;
}

void pa_touchScreen::spiWrite(uint8_t value)
{
	uint8_t i = 0x08;

	Hardware_setCLK(0);
	// HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);

	while (i > 0)
	{
		if ((value & 0x80) != 0x00)
		{
			Hardware_setMOSI(1);
			// HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_SET);
		}
		else
		{
			Hardware_setMOSI(0);
			// HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_RESET);
		}

		value <<= 1;
		Hardware_setCLK(1);
		// pa_delayMs(1);
		Hardware_setCLK(0);
		// pa_delayMs(1);
		// HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
		// HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
		i--;
	};
}

uint8_t pa_touchScreen::isPressed()
{
	return !Hardware_ReadIRQ();
}

uint8_t pa_touchScreen::readRaw(uint16_t Coordinates[2])
{
	uint16_t rawx, rawy = 0;
	uint32_t calculating_x, calculating_y = 0;

	uint32_t samples = config.SampleCount;
	uint32_t counted_samples = 0;
	if (!isPressed())
	{
		Coordinates[0] = 0;
		Coordinates[1] = 0;
		return 0;
	}
	Hardware_SetCS(0);
	spiWrite(CMDs::CMD_RDY);
	spiRead();
	spiWrite(CMDs::CMD_RDX);
	spiRead();
	while ((samples > 0) && isPressed())
	{
		spiWrite(CMDs::CMD_RDY);

		rawy = spiRead();
		calculating_y += rawy;

		spiWrite(CMDs::CMD_RDX);
		rawx = spiRead();
		calculating_x += rawx;
		samples--;
		counted_samples++;
	};
	// spiWrite(0xD0);
	// spiRead();
	// spiWrite(0);
	// spiRead();

	Hardware_SetCS(1);
	if (!rawx || !rawy)
	{
		Coordinates[0] = 0;
		Coordinates[1] = 0;
		return 0;
	}
	if ((counted_samples == config.SampleCount) && isPressed())
	{

		calculating_x /= counted_samples;
		calculating_y /= counted_samples;

		rawx = calculating_x;
		rawy = calculating_y;

		// if(rawx<16384){
		// 	rawx+65536;
		// }
		// rawx-=16384;
		//CONVERTING 16bit Value to Screen coordinates
		// 65535/273 = 240!
		// 65535/204 = 320!
		Coordinates[0] = rawx;
		Coordinates[1] = rawy;

		return 1;
	}
	else
	{
		Coordinates[0] = 0;
		Coordinates[1] = 0;
		return 0;
	}
}

void pa_touchScreen::turnRawToScreen(uint16_t Coordinates[2])
{
	int x = Coordinates[0];
	int y = Coordinates[1];
	x = 1.0 * config.screenW * (x - config.xBeginRaw) / (config.xEndRaw - config.xBeginRaw);
	y = 1.0 * config.screenH * (y - config.yBeginRaw) / (config.yEndRaw - config.yBeginRaw);
	if (x < 0)
		x = 0;
	if (x > config.screenW)
		x = config.screenW;
	if (y < 0)
		y = 0;
	if (y > config.screenH)
		y = config.screenH;
	Coordinates[0] = x;
	Coordinates[1] = y;
}

void pa_touchScreen::init(
	int screenW,
	int screenH,
	int xBeginRaw,
	int xEndRaw,
	int yBeginRaw,
	int yEndRaw, int sampleTime = 2)
{

	config.SampleCount = sampleTime;
	config.xBeginRaw = xBeginRaw;
	config.xEndRaw = xEndRaw;
	config.yBeginRaw = yBeginRaw;
	config.yEndRaw = yEndRaw;
	config.screenH = screenH;
	config.screenW = screenW;

	Hardware_Init();
	Hardware_SetCS(1);
}
