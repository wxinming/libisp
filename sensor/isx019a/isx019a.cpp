#include "ISX019A.h"

isp::Isx019A::Isx019A(std::shared_ptr<fbd::Base> device)
	:Isx019(device)
{

}

isp::Isx019A::~Isx019A()
{

}

bool isp::Isx019A::initialize()
{
	bool result = false, success = true;
	do 
	{
		if (sensor_type_ == SensorType::ISX019A_FLASH_96706G_UNIVERSAL) {
			uint8_t addr[] = { 0x0d,0x07,0x04 };
			uint8_t data[] = { 0x87,0x86,0x47 };
			if (sizeof(addr) != sizeof(data)) {
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			uint16_t slave = SLAVE_ADDR_96706;
			for (int i = 0; i < sizeof(addr); i++) {
				if (i == 2) {
					slave = SLAVE_ADDR_96701;
				}

				if (!device_->i2cAddrWrite(addr[i], 1, &data[i], 1, false, slave)) {
					success = false;
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}

			if (!success) {
				break;
			}
		}
		else if (sensor_type_ == SensorType::ISX019A_FLASH_UB934Q_UNIVERSAL) {
			uint8_t addr[] = { 0x4c,0x58,0x5c,0x5d,0x65 };
			uint8_t data[] = { 0x01,0x58,0xb0,0x34,0x34 };
			if (sizeof(addr) != sizeof(data)) {
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr); i++) {
				if (!device_->i2cAddrWrite(addr[i], 1, &data[i], 1, false, SLAVE_ADDR_934)) {
					success = false;
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}

			if (!success) {
				break;
			}
		}
		else {
			setLastError("不支持的烧录模式");
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::lock()
{
	if (!writeData(register_, { 0xf5 })) {
		setLastError("上锁失败,%s", last_error_.c_str());
		return false;
	}
	return true;
}

bool isp::Isx019A::unlock()
{
	if (!writeData(register_, { 0xf4 })) {
		setLastError("解锁失败,%s", last_error_.c_str());
		return false;
	}
	return true;
}

bool isp::Isx019A::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		std::deque<uint32_t> address = jump_address_;
		std::deque<bool> record(address.size(), true);

		if (!__super::unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		int userIndex = 0;
		for (uint32_t i = 0; i < size; i += 128) {
			*percent = getPercent(i, size);

			if (i >= 0x7e000 && i < 0x7f000) {
				memcpy(data + i, user_area_ + userIndex, 128);
				userIndex += 128;
			}
			else {
				if (address.size() && (address.front() == i)) {
					memset(data + i, 0xff, 128);
					if (record.front()) {
						address.front() += 128;
						record.front() = false;
					}
					else {
						address.pop_front();
						record.pop_front();
					}
					continue;
				}

				if (!__super::readData(i, data + i, size - i > 128 ? 128 : size - i)) {
					success = false;
					break;
				}
			}
		}

		if (!success) {
			break;
		}

		if (!lock()) {
			setLastError("上锁失败,%s", last_error_.c_str());
			break;
		}

		result = true;
	} while (false);
	return result;
	//bool result = false, success = true;
	//do
	//{
	//	if (!unlock() || !changeToFlashMode())
	//	{
	//		break;
	//	}

	//	for (int i = 0; i < size; i += 256)
	//	{
	//		*percent = getPercent(i, size);

	//		if (i >= 0x7e000 && i < 0x7f000)
	//		{
	//			memset(data + i, 0xff, 256);
	//		}
	//		else
	//		{
	//			if (!readData(i, data + i, size - i > 256 ? 256 : size - i))
	//			{
	//				break;
	//			}
	//		}
	//	}

	//	if (!success || !lock())
	//	{
	//		break;
	//	}
	//	result = true;
	//} while (false);
	//return result;
}

bool isp::Isx019A::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		jump_address_ = calculateSector(data, size, 256, 0xff);
		std::deque<uint32_t> address = jump_address_;

		if (!eraseFlash(size)) {
			break;
		}

		if (!unlock() || !changeToFlashMode()) {
			break;
		}

		int userIndex = 0;
		for (uint32_t i = 0; i < size; i += 256) {
			*percent = getPercent(i, size);

			if (i >= 0x7e000 && i < 0x7f000) {
				//跳过用户区域
				memcpy(user_area_ + userIndex, data + i, 256);
				userIndex += 256;
			}
			else {
				if (address.size() && (address.front() == i)) {
					address.pop_front();
					continue;
				}

				if (!writeData(i, data + i, size - i > 256 ? 256 : size - i)) {
					success = false;
					break;
				}
			}
		}

		if (!success || !lock()) {
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	bool result = false;
	do
	{
		if (!unlock() || !changeToFlashMode()) {
			break;
		}

		if (!readData(address, data, size)) {
			break;
		}

		if (!lock()) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	bool result = false;
	do
	{
		if (!unlock() || !changeToFlashMode()) {
			break;
		}

		if (!writeData(address, data, size)) {
			break;
		}

		if (!lock()) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	uint8_t a1 = (uint8_t)((address >> 16) & 0xff);
	uint8_t a2 = (uint8_t)((address >> 8) & 0xff);
	address = (((uint16_t)(a1 << 8)) | a2);
	if (!device_->i2cAddrRead(address, 2, data, size)) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx019A::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	uint8_t a1 = (uint8_t)((address >> 16) & 0xff);
	uint8_t a2 = (uint8_t)((address >> 8) & 0xff);
	address = (((uint16_t)(a1 << 8)) | a2);
	if (!device_->i2cAddrWrite(address, 2, data, size)) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx019A::writeData(uint32_t address, const std::initializer_list<uint8_t>& data)
{
	return writeData(address, data.begin(), data.size());
}

bool isp::Isx019A::registerAllDataWriteToFlash()
{
	bool result = false;
	do
	{	
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::eraseFlash(uint32_t address)
{
	bool result = false, success = true;
	do
	{
		//if (!__super::unlock())
		//{
		//	break;
		//}

		//for (auto i = 0; i < address; i += 4096)
		//{
		//	if (!__super::eraseFlash(i))
		//		break;
		//}
		//return false;

		if (!unlock()) {
			break;
		}

		if (!writeData(register_, { 0xf1 })) {
			setLastError("切换到擦除扇区模式失败,%s", last_error_.c_str());
			break;
		}

		for (uint32_t i = 0; i < address; i += 4096) {
			if (i >= 0x7e000 && i < 0x7f000) {
				//跳过用户区域
			}
			else {
				if (!writeData(i, {})) {
					success = false;
					setLastError("擦除扇区地址失败,%s", last_error_.c_str());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
			}
		}

		if (!success || !lock()) {
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019A::changeToFlashMode()
{
	if (!writeData(register_, { 0xf0 })) {
		setLastError("切换到Flash模式失败,%s", last_error_.c_str());
		return false;
	}
	return true;
}
