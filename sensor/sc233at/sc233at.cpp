#include "SC233AT.h"
#include "../../utils/ihex/kk_ihex_read.h"

isp::Sc233At::Sc233At(std::shared_ptr<fbd::Base> device)
	:Sc220At(device)
{
}

isp::Sc233At::~Sc233At()
{
}

bool isp::Sc233At::openBridge()
{
	return Sc220At::openBridge();
}

bool isp::Sc233At::closeBridge()
{
	return Sc220At::closeBridge();
}

bool isp::Sc233At::enableWp()
{
	if (sensor_type_ == SensorType::SC233AT_EEPROM_9296A_UNIVERSAL) {
		if (!device_->i2cAddrWrite(0x02d3, 2, { 0x10 }, false, 0x80)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_NS6603_UNIVERSAL) {
		uint8_t data[] = 
		{ 
			0x40, 0x0d, 0x40, 0x00, 0x40,
			0x00, 0x48, 0x40, 0x0e, 0x00,
			0x00, 0x40, 0x00, 0x78, 0x7d
		};

		if (!device_->i2cAddrWrite(0xe048, 2, data, sizeof(data), false, 0x52)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_M65Q68_UNIVERSAL) {
		if (!device_->i2cAddrWrite(0x0628, 2, { 0x98 }, false, 0x80)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_TI954_UNIVERSAL) {
		//什么都不做
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return true;
}

bool isp::Sc233At::disableWp()
{
	if (sensor_type_ == SensorType::SC233AT_EEPROM_9296A_UNIVERSAL) {
		//0x00改为0x04 2025.5.21
		if (!device_->i2cAddrWrite(0x02d3, 2, { 0x04 }, false, 0x80)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_NS6603_UNIVERSAL) {
		uint8_t data[] = 
		{
			0x40, 0x0d, 0x00, 0x00, 0x40,
			0x00, 0x48, 0x40, 0x0e, 0x00,
			0x00, 0x40, 0x00, 0xb5, 0x78
		};

		if (!device_->i2cAddrWrite(0xe048, 2, data, sizeof(data), false, 0x52)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_M65Q68_UNIVERSAL) {
		if (!device_->i2cAddrWrite(0x0628, 2, { 0x88 }, false, 0x80)) {
			setLastError(device_->getLastError());
			return false;
		}
	}
	else if (sensor_type_ == SensorType::SC233AT_EEPROM_TI954_UNIVERSAL) {
		//什么都不做
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return true;
}

