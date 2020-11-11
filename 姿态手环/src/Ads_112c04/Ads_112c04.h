
#ifndef __Ads_112c04_H__
#define __Ads_112c04_H__
extern "C"{
	#include "pa_HardwareIIC/pa_HardwareIIC.h"
}

class Ads_112c04
{
public:
	enum SpeedOfSample //采样速度
	{
		SPS_20 = 0,
		SPS_45,
		SPS_90,
		SPS_175,
		SPS_330,
		SPS_600,
		SPS_1000
	};
	enum Mode //倍频模式
	{
		Mode_Normal = 0,
		Mode_Turbo //速度翻倍
	};
	enum ConvMode //转换模式
	{
		Singleshot = 0,
		Continuous //持续转换
	};
	enum Gain //增益
	{
		GAIN_1 = 0,
		GAIN_2,
		GAIN_4,
		GAIN_8,
		GAIN_16,
		GAIN_32,
		GAIN_64,
		GAIN_128
	};
	enum AxState //地址选择脚状态
	{
		DGND = 0,
		DVDD,
		SDA,
		SCL,
	};
	enum CMD
	{
		CMD_RESET = 0x06,
		CMD_START = 0x08,
		CMD_RDATA = 0x10,
		CMD_RREG = 0x20,
		CMD_WREG = 0x40
	};
	Ads_112c04();
	static Ads_112c04 instance;
	void init(AxState A0, AxState A1);
	//配置寄存器0
	void configRegister0(Gain gain);
	//配置寄存器1
	void configRegister1(SpeedOfSample speedOfSample, Mode mode, ConvMode convMode);
	double readADC();
	void reset();
	//开始转换
	void startConv();
	//为0时表示转换完成
	char getDrdyState();

private:
	void initHardware();
	char I2C_ADDRESS;
};

#endif