#include "ASX340.h"

enum CmdPool 
{
	CP_POOL_0,
	CP_POOL_1 = 2,
	CP_POOL_2 = 4,
	CP_POOL_3 = 6,
	CP_POOL_4 = 8,
	CP_POOL_5 = 10,
	CP_POOL_6 = 12,
	CP_POOL_7 = 14,
};

enum CmdCode 
{
	CC_REG = 0x0040,
	CC_HANDLER = 0x001f
};

enum FlashCode 
{
	FC_LOCK = 0x8500,
	FC_LOCK_STATUS,
	FC_UNLOCK,
	FC_CONFIG,
	FC_READ,
	FC_WRITE,
	FC_ERASE_BLOCK,
	FC_ERASE_ALL,
	FC_QUERY_DEVICE,
	FC_DEVICE_STATUS,
	FC_CONFIG_DEVICE
};

isp::Asx340::Asx340(std::shared_ptr<fbd::Base> device)
	:Base(device)
{

}

isp::Asx340::~Asx340()
{
}

bool isp::Asx340::initialize()
{
	bool result = false, success = true;
	do 
	{
		if (sensor_type_ == SensorType::ASX340_EEPROM_UB934Q_UNIVERSAL) {
			uint8_t addr[] = { 0x4c, 0x58, 0x5c, 0x5d, 0x65 };
			uint8_t data[] = { 0x01, 0x58, 0xb0, 0x90, 0x90 };
			if (sizeof(addr) != sizeof(data)) {
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr); i++) {
				if (!device_->i2cAddrWrite(addr[i], 1, &data[i], 1, true, SLAVE_ADDR_934)) {
					success = false;
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}

			if (!success || !initEeprom()) {
				break;
			}
		}
		else if (sensor_type_ == SensorType::ASX340_EEPROM_NULL_UNIVERSAL) {
			if (!initEeprom()) {
				break;
			}
		}
		else if (sensor_type_ == SensorType::ASX340_FLASH_NULL_UNIVERSAL) {
			if (!initFlash()) {
				break;
			}
		}
		else {
			setLastError("不支持的烧录模式");
			break;
		}

		if (!success) {
			break;
		}
		result = true;
	} while (false);
	return result;
}


bool isp::Asx340::lock()
{
	bool result = false;
	do
	{
		if (!sendHostCmd(FC_LOCK)) {
			setLastError("上锁失败,%s", last_error_.c_str());
			break;
		}

		if (!sendHostCmd(FC_LOCK_STATUS)) {
			setLastError("获取锁状态失败,%s", last_error_.c_str());
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::unlock()
{
	bool result = false;
	do
	{
		if (!sendHostCmd(FC_UNLOCK)) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!lock()) {
			break;
		}

		for (uint32_t i = 0; i < size; i += 16) {
			if (!readData(i, data + i, size - i > 16 ? 16 : size - i)) {
				success = false;
				break;
			}
			*percent = getPercent(i, size);
		}

		if (!success || !unlock()) {
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		for (uint32_t i = 0; i < size; i += 8) {
			if (!writeData(i, data + i, size - i > 8 ? 8 : size - i)) {
				success = false;
				break;
			}
			*percent = getPercent(i, size);
		}

		if (!success || !unlock()) {
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	return readData(address, data, size);
}

bool isp::Asx340::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	return writeData(address, data, size);
}

bool isp::Asx340::setXyOffset(int x, int y)
{
	bool result = false;
	do
	{
		uint8_t data[4] = { 0 };
		uint16_t address = 0xc85c;
		data[0] = x;
		data[1] = 0;
		if (!device_->i2cAddrWrite(address, 2, data, 2)) {
			setLastError(device_->getLastError());
			break;
		}

		Sleep(100);

		address = 0xc85d;
		data[0] = y;
		data[1] = 0;
		if (!device_->i2cAddrWrite(address, 2, data, 2)) {
			setLastError(device_->getLastError());
			break;
		}

		Sleep(100);

		address = 0xfc00;
		data[0] = 0x28;
		data[1] = 0x00;
		if (!device_->i2cAddrWrite(address, 2, data, 2)) {
			setLastError(device_->getLastError());
			break;
		}

		Sleep(100);

		if (!sendHostCmd(0x8100)) {
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::getXyOffset(int& x, int& y)
{
	return false;
}

bool isp::Asx340::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	bool result = false, success = true;
	do 
	{
		uint8_t buffer[512] = { 0 };
		buffer[0] = (address >> 24) & 0xff;
		buffer[1] = (address >> 16) & 0xff;
		buffer[2] = (address >> 8) & 0xff;
		buffer[3] = (address >> 0) & 0xff;
		buffer[4] = (uint8_t)size;
		memcpy(buffer + 5, data, size);

		uint32_t start = GetTickCount64();
		while (true) {
			if (!isBusy()) {
				break;
			}

			if (GetTickCount64() - start > 3000) {
				success = false;
				setLastError("写入超时");
				break;
			}
		}

		if (!success) {
			break;
		}

		if (!device_->i2cAddrWrite(getAddress(), 2, buffer, size + 5)) {
			setLastError(device_->getLastError());
			break;
		}

		if (!sendHostCmd(FC_WRITE)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	bool result = false, success = true;
	do 
	{
		if (!device_->i2cAddrWrite(getAddress(CP_POOL_0), 2, {
			uint8_t((address >> 24) & 0xff),
			uint8_t((address >> 16) & 0xff) })) {
			setLastError(device_->getLastError());
			break;
		}

		if (!device_->i2cAddrWrite(getAddress(CP_POOL_1), 2, {
			uint8_t((address >> 8) & 0xff),
			uint8_t((address >> 0) & 0xff) })) {
			setLastError(device_->getLastError());
			break;
		}

		if (!device_->i2cAddrWrite(getAddress(CP_POOL_2), 2, { 0x10, 0x00 })) {
			setLastError(device_->getLastError());
			break;
		}

		if (!sendHostCmd(FC_READ)) {
			break;
		}

		if (!sendHostCmd(FC_DEVICE_STATUS)) {
			break;
		}

		for (int i = 0; i < size; i += 8) {
			if (!device_->i2cAddrRead(getAddress(i), 2, data + i, size - i > 8 ? 8 : size - i)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

uint16_t isp::Asx340::getAddress(int cmdPool) const
{
	return 0x8000 | (CC_HANDLER << 10) | cmdPool;
}

bool isp::Asx340::sendHostCmd(uint16_t cmd)
{
	bool result = false, success = true;
	do
	{
		if (!device_->i2cAddrWrite(0x0040, 2, { (uint8_t)(cmd >> 8),(uint8_t)cmd })) {
			setLastError(device_->getLastError());
			break;
		}

		uint8_t data[4] = { 0 };
		uint32_t start = GetTickCount64();
		while (true) {
			memset(data, 0, sizeof(data));
			if (!device_->i2cAddrRead(0x0040, 2, data, 2)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}

			if (!(MAKEWORD(data[1], data[0]) & 0x8000)) {
				break;
			}

			if (GetTickCount64() - start > 3000) {
				success = false;
				setLastError("发送命令0x%x超时", cmd);
				break;
			}
		}

		if (!success) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::reset()
{
	bool result = false;
	do 
	{
		if (!device_->i2cAddrWrite(0x001a, 2, { 0x00,0x05 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!device_->i2cAddrWrite(0x001a, 2, { 0x00,0x04 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::isBusy()
{
	bool result = false;
	do 
	{
		if (!sendHostCmd(FC_DEVICE_STATUS)) {
			result = true;
		}
	} while (false);
	return result;
}

bool isp::Asx340::query()
{
	bool result = false, success = true;
	do
	{
		if (!sendHostCmd(FC_QUERY_DEVICE) || !sendHostCmd(FC_DEVICE_STATUS)) {
			break;
		}

		for (int i = 0; i < 8; i++) {
			uint8_t temp[32] = { 0 };
			if (!device_->i2cAddrRead(getAddress(CP_POOL_0 + 2 * i), 2, temp, 2)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		if (!success) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::initFlash()
{
	bool result = false;
	do
	{
		if (!reset() || !lock() || !query() || !configI2c()) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::initEeprom()
{
	bool result = false, success = true, right = false;
	do
	{
		if (!device_->i2cAddrWrite(0x001c, 2, { 0xa2,0x30 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x001a, 2, { 0x08,0x15 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x001a, 2, { 0x08,0x14 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		uint8_t data[32] = { 0 };
		for (int i = 0; i < 10; i++) {
			memset(data, 0, sizeof(data));
			if (!device_->i2cAddrRead(0x001c, 2, data, 2)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}

			if (data[0] == 0x30 && data[1] == 0x30) {
				right = true;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		if (!success) {
			break;
		}

		if (!right) {
			setLastError("未收到正确值0x30");
			break;
		}

		if (!device_->i2cAddrWrite(0xdc07, 2, { 0x03 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x001c, 2, { 0x30,0x00 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!lock() || !configE2p()) {
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::configI2c()
{
	bool result = false;
	do 
	{
		if (!device_->i2cAddrWrite(getAddress(), 2, { 0x01, 0x00, 0x03, 0x18, 0x00, 0x10, 0x00, 0x00 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!sendHostCmd(FC_CONFIG_DEVICE)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Asx340::configE2p()
{
	bool result = false;
	do
	{
		if (!device_->i2cAddrWrite(getAddress(), 2, { 0x02, 0x00, 0x02, 0x10, 0x00, 0x00, 0xff, 0xff })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!sendHostCmd(FC_CONFIG_DEVICE)) {
			break;
		}
		result = true;
	} while (false);
	return result;
}
