#include "ISX031.h"

isp::Isx031::Isx031(std::shared_ptr<fbd::Base> device)
	:Base(device)
{

}

isp::Isx031::~Isx031()
{

}

bool isp::Isx031::initialize()
{
	bool result = false, success = true;
	do
	{
		if (sensor_type_ == SensorType::ISX031_FLASH_9296A_UNIVERSAL)
		{
			uint8_t slave[] =
			{
				0x90, 0x00, 0x80, 0x00, 0x90, 0x90,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x00,
				0x80, 0x90, 0x80, 0x00, 0x80, 0x00,
				0x80, 0x00, 0x80,
			};

			uint32_t addr[] =
			{
				0x0001, 0x0000, 0x02d6, 0x0000, 0x0330, 0x044a,
				0x0051, 0x0052, 0x0332, 0x0333, 0x0320, 0x0000,
				0x02d3, 0x0313, 0x02d3, 0x0000, 0x02be, 0x0000,
				0x02d6, 0x0000, 0x02be,
			};

			uint8_t data[] =
			{
				0x01, 0xff, 0x00, 0x0f, 0x04, 0xd0,
				0x02, 0x01, 0x30, 0x4e, 0x2c, 0xff,
				0x92, 0x02, 0x82, 0xff, 0x00, 0xff,
				0x00, 0xff, 0x99,
			};
			/*uint8_t slave[] =
			{
				0x90, 0x90, 0x90, 0x00, 0x80, 0x90, 0x90,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x00, 0x80,
				0x90, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80,
				0x00, 0x80, 0x34
			};

			uint32_t addr[] =
			{
				0x0001, 0x0010, 0x0313, 0x0000, 0x0318, 0x0330, 0x044a,
				0x0051, 0x0052, 0x0332, 0x0333, 0x0320, 0x0000, 0x02d3,
				0x0313, 0x02d3, 0x0000, 0x02d6, 0x02d6, 0x0000, 0x02be,
				0x0000, 0x02be, 0x8a01
			};

			uint8_t data[] =
			{
				0x01, 0x31, 0x00, 0xff, 0x5e, 0x04, 0xd0,
				0x02, 0x01, 0x30, 0x1b, 0x2c, 0xff, 0x92,
				0x02, 0x82, 0xff, 0x80, 0x90, 0xff, 0x00,
				0xff, 0x99, 0x80
			};*/

			if (sizeof(addr) / sizeof(*addr) != sizeof(data))
			{
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr) / sizeof(*addr); i++)
			{
				//if (i == sizeof(addr) / sizeof(*addr) - 1)
				//{
				//	continue;
				//}

				if (slave[i] != 0)
				{
					if (!device_->i2cAddrWrite(addr[i], 2, &data[i], 1, false, slave[i]))
					{
						setLastError(device_->getLastError());
						success = false;
						break;
					}
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(0xff));
				}
			}

			if (!success)
			{
				break;
			}
		}
		else
		{
			setLastError("不支持的烧录模式");
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx031::lock()
{
	if (!device_->i2cAddrWrite(0xffff, 2, { 0xf5 })) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::unlock()
{
	if (!device_->i2cAddrWrite(0xffff, 2, { 0xf4 })) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		std::deque<uint32_t> address = jump_address_;

		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		if (!flashAccess()) {
			break;
		}

		for (uint32_t i = 0; i < size; i += 256)
		{
			*percent = getPercent(i, size);

			if (address.size() && (address.front() == i))
			{
				memset(data + i, 0xff, 256);
				address.pop_front();
				continue;
			}

			if (!readData(i, data + i, size - i > 256 ? 256 : size - i))
			{
				success = false;
				break;
			}
			//if (!device_->i2cAddrWrite(0x8000, 2, { 0x01,
			//	static_cast<uint8_t>((i >> 24) & 0xff),
			//	static_cast<uint8_t>((i >> 16) & 0xff),
			//	static_cast<uint8_t>((i >> 8) & 0xff),
			//	static_cast<uint8_t>((i >> 0) & 0xff),
			//	0x5a }))
			//{
			//	success = false;
			//	break;
			//}

			//if (!device_->i2cAddrRead(0x0000, 2, data + i, size - i > 256 ? 256 : size - i))
			//{
			//	success = false;
			//	break;
			//}
		}

		if (!success)
		{
			break;
		}

		if (!lock()) {
			setLastError("上锁失败,%s", last_error_.c_str());
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx031::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		jump_address_ = calculateSector(data, size, 256, 0xff);
		std::deque<uint32_t> address = jump_address_;

		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		if (!flashAccess()) {
			break;
		}

		for (uint32_t i = 0; i < size; i += 256)
		{
			*percent = getPercent(i, size);

			if (!(i % 4096) && !eraseFlash(i))
			{
				success = false;
				break;
			}

			if (address.size() && (address.front() == i))
			{
				address.pop_front();
				continue;
			}

			if (!writeData(i, data + i, size - i > 256 ? 256 : size - i))
			{
				success = false;
				break;
			}

			//if (!device_->i2cAddrWrite(0x0000, 2, data + i, size - i > 256 ? 256 : size - i))
			//{
			//	success = false;
			//	break;
			//}

			//if (!device_->i2cAddrWrite(0x8000, 2, { 0x02,
			//	static_cast<uint8_t>((i >> 24) & 0xff),
			//	static_cast<uint8_t>((i >> 16) & 0xff),
			//	static_cast<uint8_t>((i >> 8) & 0xff),
			//	static_cast<uint8_t>((i >> 0) & 0xff),
			//	0x5a }))
			//{
			//	success = false;
			//	break;
			//}
		}

		if (!success)
		{
			break;
		}

		if (!lock()) {
			setLastError("上锁失败,%s", last_error_.c_str());
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx031::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	return unlock() && flashAccess() && readData(address, data, size) && lock();
}

bool isp::Isx031::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	return unlock() && flashAccess() && writeData(address, data, size) && lock();
}

bool isp::Isx031::readRegister(uint32_t address, uint8_t* data, uint16_t size)
{
	if (!device_->i2cAddrWrite(0xffff, 2, { 0x00 })) {
		setLastError(device_->getLastError());
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if (!device_->i2cAddrRead(address, 2, data, size)) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::writeRegister(uint32_t address, const uint8_t* data, uint16_t size)
{
	if (!device_->i2cAddrWrite(0xffff, 2, { 0x00 })) {
		setLastError(device_->getLastError());
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	if (!device_->i2cAddrWrite(address, 2, data, size)) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::saveAllToFlash()
{
	bool result = false;
	do
	{
		if (!device_->i2cAddrWrite(0xffff, 2, { 0xf4 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!device_->i2cAddrWrite(0xffff, 2, { 0xf7 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!device_->i2cAddrWrite(0xffff, 2, { 0xff })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		if (!device_->i2cAddrWrite(0xffff, 2, { 0xf5 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

bool isp::Isx031::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	if (!device_->i2cAddrWrite(0x8000, 2, { 0x01,
				static_cast<uint8_t>((address >> 24) & 0xff),
				static_cast<uint8_t>((address >> 16) & 0xff),
				static_cast<uint8_t>((address >> 8) & 0xff),
				static_cast<uint8_t>((address >> 0) & 0xff),
				0x5a }) ||
		!device_->i2cAddrRead(0x0000, 2, data, size)) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	if (!device_->i2cAddrWrite(0x0000, 2, data, size) ||
		!device_->i2cAddrWrite(0x8000, 2, { 0x02,
		static_cast<uint8_t>((address >> 24) & 0xff),
		static_cast<uint8_t>((address >> 16) & 0xff),
		static_cast<uint8_t>((address >> 8) & 0xff),
		static_cast<uint8_t>((address >> 0) & 0xff),
		0x5a })) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::flashAccess()
{
	if (!device_->i2cAddrWrite(0xffff, 2, { 0xf7 })) {
		setLastError(device_->getLastError());
		return false;
	}
	return true;
}

bool isp::Isx031::eraseFlash(uint32_t address)
{
	bool result = false;
	do
	{
		//RUN_BREAK(address % 4096 != 0, Q_SPRINTF("%x %% 4096不为0", address));

		if (!device_->i2cAddrWrite(0x8000, 2, { 0x03,
			static_cast<uint8_t>((address >> 24) & 0xff),
			static_cast<uint8_t>((address >> 16) & 0xff),
			static_cast<uint8_t>((address >> 8) & 0xff),
			static_cast<uint8_t>((address >> 0) & 0xff),
			0x5a })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		result = true;
	} while (false);
	return result;
}

bool isp::Isx031::enableWp()
{
	bool result = false;
	do
	{
		if (!device_->i2cAddrWrite(0x8ac1, 2, { 0x9c })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x8a12, 2, { 0x08 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x8a12, 2, { 0x0a })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		uint8_t data = 0;
		if (!device_->i2cAddrRead(0x60d9, 2, &data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		result = (data == 0x9c);
	} while (false);
	return result;
}

bool isp::Isx031::disableWp()
{
	bool result = false;
	do
	{
		if (!device_->i2cAddrWrite(0x8ac1, 2, { 0x00 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x8a12, 2, { 0x08 })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		if (!device_->i2cAddrWrite(0x8a12, 2, { 0x0a })) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		uint8_t data = 0;
		if (!device_->i2cAddrRead(0x60d9, 2, &data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		result = (data == 0x00);
	} while (false);
	return result;
}
