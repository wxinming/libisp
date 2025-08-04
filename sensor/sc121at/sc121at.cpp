#include "SC121AT.h"
#include "../../utils/ihex/kk_ihex_read.h"

isp::Sc121At::Sc121At(std::shared_ptr<fbd::Base> device)
	:Sc220At(device)
{
}

isp::Sc121At::~Sc121At()
{
}

bool isp::Sc121At::openBridge()
{
	bool result = false;
	do 
	{
		uint8_t slave = 0x60;
		uint8_t reg1[3] = { 0x30, 0x32, 0x10 };
		uint8_t reg2[3] = { 0x36, 0x41, 0x82 };
		if (device_->i2cWrite(reg1, 3, slave) != 3) {
			setLastError(device_->getLastError());
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (device_->i2cWrite(reg2, 3, slave) != 3) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

bool isp::Sc121At::closeBridge()
{
	return Sc220At::closeBridge();
}

bool isp::Sc121At::enableWp()
{
	return Sc220At::enableWp();
}

bool isp::Sc121At::disableWp()
{
	bool result = false;
	do 
	{
		uint8_t slave = 0x60;
		uint8_t reg1[3] = { 0x30, 0x16, 0x50 };
		uint8_t reg2[3] = { 0x30, 0x17, 0x03 };
		if (device_->i2cWrite(reg1, 3, slave) != 3) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (device_->i2cWrite(reg2, 3, slave) != 3) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

