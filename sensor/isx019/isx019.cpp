#include "ISX019.h"

isp::Isx019::Isx019(std::shared_ptr<fbd::Base> device)
	:Isx016(device)
{
	cmd_addr_len_ = 4;
}

isp::Isx019::~Isx019()
{

}

bool isp::Isx019::initialize()
{
	bool result = false, success = true;
	do 
	{
		if (sensor_type_ == SensorType::ISX019_FLASH_96706G_UNIVERSAL) {
			uint8_t addr[] = { 0x0d,0x07,0x04 };
			uint8_t data[] = { 0x87,0x86,0x47 };
			if (sizeof(addr) != sizeof(data))
			{
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			uint16_t slave = SLAVE_ADDR_96706;
			for (int i = 0; i < sizeof(addr); i++) {
				if (i == 2) {
					slave = SLAVE_ADDR_96701;
				}

				if (!device_->i2cAddrWrite(addr[i], 1, &data[i], 1, true, slave)) {
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
		else if (sensor_type_ == SensorType::ISX019_FLASH_UB934Q_UNIVERSAL) {
			uint8_t addr[] = { 0x4c,0x58,0x5c,0x5d,0x65 };
			uint8_t data[] = { 0x01,0x58,0xb0,0x34,0x34 };
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

			if (!success) {
				break;
			}
		}
		else if (sensor_type_ == SensorType::ISX019_FLASH_NULL_UNIVERSAL) {

		}
		else if (sensor_type_ == SensorType::ISX019_FLASH_96717F_UNIVERSAL) {
			uint8_t slave[] = 
			{
				0x90, 0x90, 0x90, 0x00, 0x80, 0x80, 0x80,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
				0x00, 0x80, 0x90, 0x80, 0x00, 0x80, 0x80,
				0x00
			};

			uint16_t addr[] = 
			{
				0x0001, 0x0010, 0x0313, 0x0000, 0x0330, 0x0331, 0x0318,
				0x0330, 0x044a, 0x0051, 0x0052, 0x0332, 0x0333, 0x0320,
				0x0000, 0x02d3, 0x0313, 0x02d3, 0x0000, 0x02d3, 0x02d3,
				0x0000
			};

			uint8_t data[] = 
			{
				0x01, 0x31, 0x00, 0xff, 0x00, 0x30, 0x5e, 
				0x04, 0xd0, 0x02, 0x01, 0x30, 0x4e, 0x26,
				0xff, 0x92, 0x02, 0x82, 0xff, 0x80, 0x90,
				0xff
			};

			if (sizeof(addr) / sizeof(*addr) != sizeof(data)) {
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr) / sizeof(*addr); i++) {
				if (slave[i] != 0x00) {
					if (!device_->i2cAddrWrite(addr[i], 2, &data[i], 1, false, slave[i])) {
						success = false;
						setLastError(device_->getLastError());
						break;
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(0xff));
				}
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

bool isp::Isx019::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do 
	{
		std::deque<uint32_t> address = jump_address_;
		std::deque<bool> record(address.size(), true);

		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		int userIndex = 0;
		for (uint32_t i = 0; i < size; i += 128) {
			*percent = getPercent(i, size);

			//if (i >= 0x7e000 && i < 0x7f000)
			//{
			//	memcpy(data + i, m_userArea + userIndex, 128);
			//	userIndex += 128;
			//}
			//else
			{
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

				if (!readData(i, data + i, size - i > 128 ? 128 : size - i)) {
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
}

bool isp::Isx019::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		jump_address_ = calculateSector(data, size, 256, 0xff);
		std::deque<uint32_t> address = jump_address_;
		std::deque<bool> record(address.size(), true);

		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		int userIndex = 0;
		for (uint32_t i = 0; i < size; i += 128)
		{
			*percent = getPercent(i, size);

			//if (i >= 0x7e000 && i < 0x7f000)
			//{
			//	//跳过用户区域
			//	memcpy(m_userArea + userIndex, data + i, 128);
			//	userIndex += 128;
			//}
			//else
			{
				if (!(i % 4096) && !eraseFlash(i)) {
					success = false;
					break;
				}

				if (address.size() && (address.front() == i)) {
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

				if (!writeData(i, data + i, size - i > 128 ? 128 : size - i)) {
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
}

bool isp::Isx019::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	return Isx016::readEeprom(address, data, size);
}

bool isp::Isx019::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	return Isx016::writeEeprom(address, data, size);
}

bool isp::Isx019::saveAllToFlash()
{
	bool result = false;
	do
	{
		if (!unlock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		SonyCmdFrame frame = { 0 };
		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_ALL_SAVE_REG_TO_EEP_019;
		frame.cmd[0].total_cmd_byte = 2;
		if (!writeFrame(frame)) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		memset(&frame, 0, sizeof(SonyCmdFrame));
		if (!readFrame(&frame, 5)) {
			break;
		}

		if (SC016_SUCCESS != frame.cmd[0].cmd_and_status_code) {
			setErrorInfo(frame.cmd[0].cmd_and_status_code);
			break;
		}

		if (frame.total_byte != 5) {
			setLastError("总字节长度错误");
			break;
		}

		if (!lock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		result = true;
	} while (false);
	return result;
}

bool isp::Isx019::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	return Isx016::readData(address, data, size);
}

bool isp::Isx019::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	return Isx016::writeData(address, data, size);
}

bool isp::Isx019::eraseFlash(uint32_t address)
{
	bool result = false;
	do
	{
		SonyCmdFrame frame = { 0 };
		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_ERASE_EEP_019;

		if (cmd_addr_len_ == 2) {
			frame.cmd[0].data[0] = (address >> 8) & 0xFF;
			frame.cmd[0].data[1] = (address >> 0) & 0xFF;
		}
		else if (cmd_addr_len_ == 4) {
			frame.cmd[0].data[0] = (address >> 24) & 0xFF;
			frame.cmd[0].data[1] = (address >> 16) & 0xFF;
			frame.cmd[0].data[2] = (address >> 8) & 0xFF;
			frame.cmd[0].data[3] = (address >> 0) & 0xFF;
		}
		else {
			setLastError("地址长度必须为2字节或4字节");
			break;
		}

		frame.cmd[0].total_cmd_byte = 2 + cmd_addr_len_;
		if (!writeFrame(frame)) {
			break;
		}

		//此处延时不可修改,默认100
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		memset(&frame, 0, sizeof(SonyCmdFrame));
		if (!readFrame(&frame, 5)) {
			break;
		}

		if (SC016_SUCCESS != frame.cmd[0].cmd_and_status_code) {
			setErrorInfo(frame.cmd[0].cmd_and_status_code);
			break;
		}

		if (frame.total_byte != 5) {
			setLastError("总字节长度错误");
			break;
		}

		result = true;
	} while (false);
	return result;
}
