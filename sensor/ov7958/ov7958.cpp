#include "OV7958.h"

isp::Ov7958::Ov7958(std::shared_ptr<fbd::Base> device)
	:Base(device)
{

}

isp::Ov7958::~Ov7958()
{
}

bool isp::Ov7958::initialize()
{
	bool result = false, success = true;
	do
	{
		if (sensor_type_ == SensorType::OV7958_FLASH_UB934Q_UNIVERSAL)
		{
			uint8_t addr[] = { 0x4c,0x58,0x5c,0x5d,0x65 };
			uint8_t data[] = { 0x01,0x58,0xb4,0x80,0x80 };
			if (sizeof(addr) != sizeof(data)) {
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr); i++) {
				if (!device_->i2cAddrWrite(addr[i], 1, &data[i], 1, true, SLAVE_ADDR_934)) {
					setLastError(device_->getLastError());
					success = false;
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

bool isp::Ov7958::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false;
	do {

		uint8_t data[1] = { 0x00 };
		data[0] = 0x3b; 
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(14));
		data[0] = 0xfb;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x06;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(14));
		data[0] = 0xfb;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x0b;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		uint32_t count = 0, value_i = 0;
		for (value_i = 0; value_i <= size; value_i++)
		{
			if (((value_i % 257) == 0) && (value_i / 257 > 0))
			{
				data[0] = 0x3b; 
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(52));

				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0xfb;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x06;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(14));
				data[0] = 0xfb;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				data[0] = 0x0b;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x00;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				data[0] = uint8_t(value_i / 257);
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				data[0] = 0x00;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				data[0] = 0x00;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
			}

			if (!device_->i2cAddrRead(0x6231, 2, data, 1)) {
				setLastError(device_->getLastError());
				break;
			}

			if (value_i % 257)
			{
				data[count++] = data[0];
				(*percent) = (count * 100) / size;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		if (size != value_i - 1)
		{
			break;
		}

		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ov7958::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false;
	do
	{
		if (!setProtect(false)) {
			setLastError("解保护失败,%s", last_error_.c_str());
			break;
		}

		if (!earseAll()) {
			break;
		}

		for (uint32_t i = 0; i < 10; i++) {
			(*percent)++;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		uint8_t data[] = { 0x00 };
		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(14));

		data[0] = 0xfb;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x06;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(14));

		data[0] = 0xfb;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x06;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(14));

		data[0] = 0xfb;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x02;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		data[0] = 0x00;
		if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		uint32_t val_i = 0;
		for (val_i = 0; val_i < size; val_i++) {
			if (((val_i % 256) == 0) && (val_i / 256 > 0)) {
				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(14));

				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0xfb;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x06;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(14));

				data[0] = 0xfb;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x06;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x3b;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(14));

				data[0] = 0xfb;
				if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x02;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x00;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = uint8_t(val_i / 256); 
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}

				data[0] = 0x00;
				if (!device_->i2cAddrWrite(0x6217, 2, data, 1)) {
					setLastError(device_->getLastError());
					break;
				}
			}
			*percent = (val_i * 80 / size) + 10;
			if (!device_->i2cAddrWrite(0x6217, 2, &data[val_i], 1)) {
				setLastError(device_->getLastError());
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (val_i != size) {
			break;
		}

		data[0] = 0x3b;
		if (!device_->i2cAddrWrite(0x6202, 2, data, 1)) {
			setLastError(device_->getLastError());
			break;
		}

		for (uint32_t i = 0; i < 10; i++) {
			(*percent)++;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (!setProtect(true)) {
			setLastError("写保护失败,%s", last_error_.c_str());
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ov7958::earseAll()
{
	bool result = false, success = true;
	do
	{
		uint16_t addr[] =
		{
			0x6202,0x6202,0x6217,0x6202,0x6202,
			0x6202,0x6202,0x6202,0x6202,0x6202,
			0x6202,0x6202,0x6217,0x6202,0x6202,
			0x6202,0x6202,0x6202,0x6202,0x6202,
			0x6202,0x6202
		};

		uint8_t data[] =
		{
			0x3b,0xfb,0x06,0x3b,0x3b,
			0x3b,0x3b,0x3b,0x3b,0x3b,
			0x3b,0xfb,0xc7,0x3b,0x3b,
			0x3b,0x3b,0x3b,0x3b,0x3b,
			0x3b,0x3b
		};

		for (int i = 0; i < sizeof(addr); i++)
		{
			if (!device_->i2cAddrWrite(addr[i], 2, &data[i], 1, true))
			{
				setLastError(device_->getLastError());
				success = false;
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

bool isp::Ov7958::getStatus()
{
	bool result = false;
	do
	{
		uint8_t data[1] = { 0 };
		if (!device_->i2cAddrRead(0x3003, 2, data, 1))
		{
			setLastError(device_->getLastError());
			break;
		}
		/*0x1C硬件通讯成功,FLASH是空的*/

		/*0x13硬件通讯成功,FLASH有数据*/

		/*0x00硬件通讯失败*/
		if (!*data)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Ov7958::setProtect(bool protect)
{
	bool result = false, success = true;
	do
	{
		uint16_t addr[] = 
		{
			0x6202,0x6202,0x6217,0x6202,0x6202,
			0x6202,0x6202,0x6202,0x6202,0x6202,
			0x6202,0x6217,0x6217,0x6217,0x6202,
			0x6202,0x6202,0x6202,0x6202,0x6202,
			0x6202 
		};

		uint8_t value = protect ? 0xfc : 0x00;
		uint8_t data[] = 
		{
			0x3b,0xfb,0x06,0x3b,0x3b,0x3b,0x3b,
			0x3b,0x3b,0x3b,0xfb,0x01,value,0x00,
			0x3b,0x3b,0x3b,0x3b,0x3b,0x3b,0x3b
		};

		if (sizeof(addr) / sizeof(uint16_t) != sizeof(data)) {
			setLastError("地址列表与数据列表大小不一致");
			break;
		}

		for (int i = 0; i < sizeof(addr); i++) {
			if (!device_->i2cAddrWrite(addr[i], 2, &data[i], 1, true)) {
				setLastError(device_->getLastError());
				success = false;
				break;
			}
		}

		if (!success) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		result = true;
	} while (false);
	return result;
}
