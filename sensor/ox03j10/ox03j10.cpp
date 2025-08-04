#include "OX03J10.h"

isp::Ox03J10::Ox03J10(std::shared_ptr<fbd::Base> device)
	: Base(device)
{

}

isp::Ox03J10::~Ox03J10()
{

}

bool isp::Ox03J10::initialize()
{
	bool result = false, success = true;
	do
	{
		if (sensor_type_ == SensorType::OX03J10_FLASH_9296A_UNIVERSAL)
		{
			std::string registerText =
			R"(
				0x90,0x0001,0x01,0x1608
				0x90,0x0010,0x31,0x1608
				0x90,0x0313,0x00,0x1608
				0x00,0x0000,0xff,0x0808
				0x80,0x0318,0x5e,0x1608
				0x90,0x0330,0x04,0x1608
				0x90,0x044a,0xd0,0x1608
				0x90,0x0051,0x02,0x1608
				0x90,0x0052,0x01,0x1608
				0x90,0x0332,0x30,0x1608
				0x90,0x0325,0x80,0x1608
				0x90,0x0333,0x4e,0x1608
				0x90,0x0320,0x2c,0x1608
				0x00,0x0000,0xff,0x0808
				0x90,0x03e0,0x00,0x1608
				0x90,0x03e2,0x20,0x1608
				0x90,0x03e7,0x0c,0x1608
				0x90,0x03e6,0xb7,0x1608
				0x90,0x03e5,0x35,0x1608
				0x90,0x03ea,0x00,0x1608
				0x90,0x03eb,0x00,0x1608
				0x90,0x03ef,0xc2,0x1608
				0x90,0x03f1,0x40,0x1608
				0x80,0x03f0,0x51,0x1608
				0x80,0x03f1,0x09,0x1608
				0x80,0x0570,0x20,0x1608
				0x80,0x02be,0x10,0x1608
				0x00,0x0000,0xff,0x0808
				0x80,0x02d6,0x80,0x1608
				0x80,0x02d6,0x90,0x1608
				0x00,0x0000,0xff,0x0808
				0x00,0x0000,0xff,0x0808
				0x90,0x0313,0x02,0x1608
			)";

			std::vector<uint8_t> slave;
			std::vector<uint16_t> reg;
			std::vector<uint8_t> reglen;
			std::vector<uint8_t> data;
			parseRlgRegisterText(registerText, slave, reg, reglen, data);
			for (int i = 0; i < slave.size(); ++i) {
				if (slave[i] != 0) {
					if (!device_->i2cAddrWrite(reg[i], reglen[i], &data[i], 1, false, slave[i])) {
						success = false;
						setLastError(device_->getLastError());
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(0xff));
				}
			}

			if (!success) {
				break;
			}
		}
		else
		{
			setLastError("不支持的烧录模式");
			break;
		}

		if (!initChip()) {
			break;
		}

		if (!success) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!device_->i2cAddrWrite(0xa10a, 2, { 0x00 }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa11e, 2, { 0x7f }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa11d, 2, { 0x00 }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa110, 2, { 0x80 }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa10f, 2, { 0x18 }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa10e, 2, { 0x4c }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		if (!device_->i2cAddrWrite(0xa10d, 2, { 0x00 }))
		{
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(OVX1E_DELAY));

		for (uint32_t i = 0; i < size; i += 4096) {
			if (!readData(i, data + i, size - i > 4096 ? 4096 : size - i)) {
				success = false;
				break;
			}
			*percent = getPercent(i, size);
		}

		if (!success) {
			break;
		}
		uninitChip();
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		for (uint32_t i = 0; i < size; i += 4096)
		{
			if (!writeData(i, data + i, size - i > 4096 ? 4096 : size - i))
			{
				success = false;
				break;
			}
			*percent = getPercent(i, size);
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	return readData(address, data, size);
	bool result = false;
	do
	{
		auto nearAddress = findNearAddress(address, 256);
		uint8_t nearData[256] = { 0 };
		if (!readData(nearAddress, nearData, sizeof(nearData))) {
			break;
		}
		auto offset = address - nearAddress;
		memcpy(data, &nearData[offset], size);
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	bool result = false;
	do
	{
		auto nearAddress = findNearAddress(address, 256);
		uint8_t nearData[256] = { 0 };
		if (!readData(nearAddress, nearData, sizeof(nearData))) {
			break;
		}

		auto offset = address - nearAddress;
		memcpy(&nearData[offset], data, size);

		if (!writeData(nearAddress, nearData, sizeof(nearData))) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	bool result = false, success = true;
	do
	{
		uint8_t buffer[512] = { 0 };
		uint32_t segment = 0;
		for (uint32_t i = 0; i < size; i += 0x100) {
			segment = size - i > 0x100 ? 0x100 : size - i;
			buffer[0] = 0x00;
			buffer[1] = 0x00;
			buffer[2] = 0x81;
			buffer[3] = 0x00;
			buffer[4] = 0x00;
			buffer[5] = 0x01;
			buffer[6] = 0x12;
			buffer[7] = ((address + i) >> 16) & 0xff;
			buffer[8] = ((address + i) >> 8) & 0xff;
			buffer[9] = ((address + i) >> 0) & 0xff;
			buffer[10] = (segment >> 8) & 0xff;
			buffer[11] = (segment >> 0) & 0xff;

			if (!sendHostCmd(buffer, 12, 0x9000)) {
				success = false;
				break;
			}

			for (uint32_t j = 0; j < segment; j += 32) {
				if (!device_->i2cAddrRead(0x9300 + j, 2, data + i + j,
					segment - j > 32 ? 32 : segment - j))
				{
					success = false;
					setLastError(device_->getLastError());
					break;
				}
			}

			if (!success)
			{
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

bool isp::Ox03J10::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	bool result = false, success = true;
	do
	{
		uint8_t buffer[512] = { 0 };
		buffer[0] = 0x00;
		buffer[1] = 0x00;
		buffer[2] = 0x80;
		buffer[3] = 0x00;
		buffer[4] = 0x00;
		buffer[5] = 0x05;
		buffer[6] = 0x00;
		buffer[7] = 0x00;
		buffer[8] = (address >> 16) & 0xff;
		buffer[9] = (address >> 8) & 0xff;
		buffer[10] = (address >> 0) & 0xff;
		if (!sendHostCmd(buffer, 11, 0x9000)) {
			break;
		}

		uint32_t segment = 0;
		for (uint32_t i = 0; i < size; i += 0x100) {
			segment = size - i > 0x100 ? 0x100 : size - i;
			memset(buffer, 0, sizeof(buffer));
			buffer[0] = 0x00;
			buffer[1] = 0x00;
			buffer[2] = 0x81;
			buffer[3] = 0x00;
			buffer[4] = 0x00;
			buffer[5] = 0x01;
			buffer[6] = 0x11;
			buffer[7] = ((address + i) >> 16) & 0xff;
			buffer[8] = ((address + i) >> 8) & 0xff;
			buffer[9] = ((address + i) >> 0) & 0xff;
			buffer[10] = (segment >> 8) & 0xff;
			buffer[11] = (segment >> 0) & 0xff;

			if (!device_->i2cAddrWrite(0x9000, 2, buffer, 12)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}

			memcpy(buffer, data + i, segment);
			if (!sendHostCmd(buffer, segment, 0x900C)) {
				success = false;
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

bool isp::Ox03J10::sendHostCmd(const uint8_t* data, uint32_t size, uint32_t addr)
{
	bool result = false, success = true;
	do
	{
		uint32_t pageAddr = 0, segment = 0;
		for (uint32_t i = 0; i < size; i += 32) {
			segment = size - i > 32 ? 32 : size - i;
			pageAddr = addr + i;
			if (!device_->i2cAddrWrite(pageAddr, 2, data + i, (uint16_t)segment)) {
				success = false;
				setLastError(device_->getLastError());
				break;
			}
		}

		if (!success) {
			break;
		}

		uint16_t addr = 0x1780;
		uint8_t value = 0xff;
		device_->i2cAddrWrite(addr, 2, &value, 1);

		addr = 0x1760; //trigger
		value = 0x01;
		device_->i2cAddrWrite(addr, 2, &value, 1);

		if (!getStatus()) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::initChip()
{
	bool result = false;
	do
	{
		uint8_t value = 0x01;
		device_->i2cAddrWrite(0x161c, 2, &value, 1);
		value = 0x00;
		device_->i2cAddrWrite(0x1902, 2, &value, 1);
		device_->i2cAddrWrite(0x0100, 2, &value, 1);
		device_->i2cAddrWrite(0xaf83, 2, &value, 1);
		value = 0xc1;
		device_->i2cAddrWrite(0x1600, 2, &value, 1);
		value = 0x11;
		device_->i2cAddrWrite(0x1603, 2, &value, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		uint16_t addr = 0x1600;
		uint8_t flag = 0;
		device_->i2cAddrRead(addr, 2, &flag, 1);
		value = flag & 0xbf;
		device_->i2cAddrWrite(addr, 2, &value, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		addr = 0x1790;
		value = 0x35;
		device_->i2cAddrWrite(addr, 2, &value, 1);
		device_->i2cAddrRead(addr, 2, &flag, 1);
		if (flag != 0x35) {
			setLastError("0x1790寄存器值不等于0x35");
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		addr = 0x1781;
		value = 0x00;
		device_->i2cAddrWrite(addr, 2, &value, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::uninitChip()
{
	bool result = false;
	do
	{
		uint8_t value = 0x01;
		device_->i2cAddrWrite(0x0107, 2, &value, 1);
		result = true;
	} while (false);
	return result;
}

bool isp::Ox03J10::getStatus()
{
	bool result = false, success = false;
	do
	{
		uint8_t data[64] = { 0 };
		uint32_t startTime = GetTickCount64();
		while (true) {
			memset(data, 0x00, sizeof(data));
			if (!device_->i2cAddrRead(0x1780, 2, data, 1)) {
				setLastError(device_->getLastError());
				break;
			}

			if (data[0] == 0x99) {
				success = true;
				break;
			}

			if (GetTickCount64() - startTime > 5000) {
				setLastError("循环读取0x1780超时");
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
