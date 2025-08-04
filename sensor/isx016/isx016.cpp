#include "ISX016.h"

isp::Isx016::Isx016(std::shared_ptr<fbd::Base> device)
	:Base(device)
{
	cmd_addr_len_ = 2;
}

isp::Isx016::~Isx016()
{

}

bool isp::Isx016::initialize()
{
	bool result = false, success = true;
	do
	{
		if (sensor_type_ == SensorType::ISX016_EEPROM_96706G_UNIVERSAL) {
			uint8_t addr[] = { 0x0d,0x07,0x04,0x0e };
			uint8_t data[] = { 0x87,0x86,0x47,0x02 };
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
		else {
			setLastError("不支持的烧录模式");
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::lock()
{
	bool result = false, success = true;
	do
	{
		uint8_t wdata[9] = { 0x09, 0x01, 0x06, 0x00, 0x57, 0x52, 0x44, 0x53, 0x50 };
		uint8_t rdata[5] = { 0x05, 0x01, 0x02, 0x01, 0x09 };
		uint8_t req[5] = { 0 };

		if (9 != device_->i2cWrite(wdata, 9)) {
			setLastError(device_->getLastError());
			break;
		}

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::unlock()
{
	bool result = false, success = true;
	do
	{
		uint8_t wdata[9] = { 0x09, 0x01, 0x06, 0x00, 0x57, 0x52, 0x45, 0x4e, 0x4c };
		uint8_t rdata[5] = { 0x05, 0x01, 0x02, 0x01, 0x09 };
		uint8_t req[5] = { 0 };

		if (9 != device_->i2cWrite(wdata, 9)) {
			setLastError(device_->getLastError());
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::readFile(uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		for (uint32_t i = 0; i < size; i += 8) {
			if (!readData(i, data + i, size - i > 8 ? 8 : size - i)) {
				success = false;
				break;
			}
			*percent = getPercent(i, size);
		}

		if (!success) {
			break;
		}

		if (!protect()) {
			setLastError("写保护失败,%s", last_error_.c_str());
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

bool isp::Isx016::writeFile(const uint8_t* data, uint32_t size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!unlock()) {
			setLastError("解锁失败,%s", last_error_.c_str());
			break;
		}

		for (uint32_t i = 0; i < size; i += 8) {
			if (!writeData(i, data + i, size - i > 8 ? 8 : size - i)) {
				success = false;
				break;
			}
			*percent = getPercent(i, size);
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

bool isp::Isx016::readEeprom(uint32_t address, uint8_t* data, uint16_t size)
{
	return unlock() && readData(address, data, size) && lock();
}

bool isp::Isx016::writeEeprom(uint32_t address, const uint8_t* data, uint16_t size)
{
	return unlock() && writeData(address, data, size) && lock();
}

bool isp::Isx016::readRegister(uint32_t address, uint8_t* data, uint16_t size)
{
	auto category = (address >> 16) & 0xff;
	address &= 0xffff;
	return readRegister(category, address, data, size);
}

bool isp::Isx016::writeRegister(uint32_t address, const uint8_t* data, uint16_t size)
{
	auto category = (address >> 16) & 0xff;
	address &= 0xffff;
	return writeRegister(category, address, data, size);
}

bool isp::Isx016::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	bool result = false;
	do
	{
		SonyCmdFrame frame = { 0 };

		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_READ_EEP;

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

		frame.cmd[0].data[cmd_addr_len_] = (uint8_t)size;
		frame.cmd[0].total_cmd_byte = 3 + cmd_addr_len_;
		if (!writeFrame(frame)) {
			break;
		}

		memset(&frame, 0, sizeof(SonyCmdFrame));
		if (!readFrame(&frame, 5 + size)) {
			break;
		}

		if (SC016_SUCCESS != frame.cmd[0].cmd_and_status_code) {
			setErrorInfo(frame.cmd[0].cmd_and_status_code);
			break;
		}

		if (frame.total_byte != size + 5) {
			setLastError("总字节长度错误");
			break;
		}
		memcpy(data, frame.cmd[0].data, size);

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	bool result = false;
	do
	{
		SonyCmdFrame frame = { 0 };
		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_WRITE_EEP;

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

		memcpy(&frame.cmd[0].data[cmd_addr_len_], data, size);
		frame.cmd[0].total_cmd_byte = 2 + size + cmd_addr_len_;
		if (!writeFrame(frame)) {
			break;
		}

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

bool isp::Isx016::packFrame(SonyCmdFrame i2cCmd, uint8_t* data)
{
	bool result = false;
	do
	{
		if (i2cCmd.cmd_cnt > SONY_CMD_MAX_CNT) {
			setLastError("命令行总数为%d大于上限%d", i2cCmd.cmd_cnt, SONY_CMD_MAX_CNT);
			break;
		}

		data[1] = i2cCmd.cmd_cnt;
		uint8_t* pcmd = &data[2];
		for (int i = 0; i < i2cCmd.cmd_cnt; i++) {
			memcpy(&pcmd[0], &i2cCmd.cmd[i].total_cmd_byte, 1);
			memcpy(&pcmd[1], &i2cCmd.cmd[i].cmd_and_status_code, 1);
			memcpy(&pcmd[2], i2cCmd.cmd[i].data, i2cCmd.cmd[i].total_cmd_byte - 2);
			pcmd += i2cCmd.cmd[i].total_cmd_byte;
			i2cCmd.total_byte += i2cCmd.cmd[i].total_cmd_byte;
		}
		i2cCmd.total_byte += 3;
		data[0] = i2cCmd.total_byte;
		pcmd[0] = 0;
		for (int i = 0; i < i2cCmd.total_byte - 1; i++) {
			pcmd[0] += data[i];
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::unpackFrame(SonyCmdFrame* frame, const uint8_t* data, uint8_t size)
{
	bool result = false, success = true;
	do
	{
		if (data[0] != size) {
			for (int i = 1; i < size; i++) {
				if (data[i] != 0) {
					success = false;
					break;
				}
			}

			if (success) {
				frame->cmd[0].cmd_and_status_code = data[0];
				return true;
			}
			size = data[0];
		}

		if (data[1] <= 0) {
			setLastError("命令行总数不能小于等于0");
			break;
		}

		uint8_t checkSum = 0;
		for (int i = 0; i < size - 1; i++) {
			checkSum += data[i];
		}

		if (checkSum != data[size - 1]) {
			setLastError("校验和错误");
			break;
		}

		frame->total_byte = size;
		frame->cmd_cnt = data[1];
		frame->check_sum = data[size - 1];
		const uint8_t* pcmd = &data[2];
		for (int i = 0; i < frame->cmd_cnt; i++) {
			frame->cmd[i].total_cmd_byte = pcmd[0];
			frame->cmd[i].cmd_and_status_code = pcmd[1];
			memcpy(frame->cmd[i].data, &pcmd[2], frame->cmd[i].total_cmd_byte - 2);
			pcmd += frame->cmd[i].total_cmd_byte;
		}
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::readFrame(SonyCmdFrame* frame, uint8_t size)
{
	bool result = false;
	do
	{
		uint8_t data[SONY_CMD_FRAME_MAX_LEN * 2] = { 0 };
		int count = device_->i2cRead(data, size);
		if (count <= 0) {
			setLastError(device_->getLastError());
			break;
		}

		if (!unpackFrame(frame, data, count)) {
			break;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(1);
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::writeFrame(SonyCmdFrame frame)
{
	bool result = false;
	do
	{
		if (frame.cmd_cnt <= 0) {
			setLastError("命令行写入长度小于等于0");
			break;
		}

		uint8_t data[SONY_CMD_FRAME_MAX_LEN * 2] = { 0 };
		if (!packFrame(frame, data)) {
			break;
		}

		int count = device_->i2cWrite(data, data[0]);
		if (count != data[0]) {
			setLastError(device_->getLastError());
			break;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(2);
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::readRegister(uint8_t category, uint16_t addrOffset, uint8_t* data, uint8_t dataSize)
{
	bool result = false;
	do
	{
		SonyCmdFrame frame = { 0 };
		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_READ_REG;
		frame.cmd[0].data[0] = category;
		frame.cmd[0].data[1] = (addrOffset & 0xFF00) >> 8;
		frame.cmd[0].data[2] = addrOffset & 0x00FF;
		frame.cmd[0].data[3] = dataSize;
		frame.cmd[0].total_cmd_byte = 6;
		if (!writeFrame(frame)) {
			break;
		}

		memset(&frame, 0, sizeof(SonyCmdFrame));
		if (!readFrame(&frame, 5 + dataSize)) {
			break;
		}

		if (SC016_SUCCESS != frame.cmd[0].cmd_and_status_code) {
			setErrorInfo(frame.cmd[0].cmd_and_status_code);
			break;
		}

		if (frame.total_byte != dataSize + 5) {
			setLastError("总字节长度错误");
			break;
		}
		memcpy(data, frame.cmd[0].data, dataSize);

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::writeRegister(uint8_t category, uint16_t addrOffset, const uint8_t* data, uint8_t dataSize)
{
	bool result = false;
	do
	{
		SonyCmdFrame frame = { 0 };

		frame.cmd_cnt = 1;
		frame.cmd[0].cmd_and_status_code = CMD_WRITE_REG;
		frame.cmd[0].data[0] = category;
		frame.cmd[0].data[1] = (addrOffset & 0xFF00) >> 8;
		frame.cmd[0].data[2] = addrOffset & 0x00FF;
		memcpy(&frame.cmd[0].data[3], data, dataSize);
		frame.cmd[0].total_cmd_byte = 5 + dataSize;
		if (!writeFrame(frame)) {
			break;
		}

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

bool isp::Isx016::tempInit()
{
	bool result = false, success = true;
	do
	{
		uint8_t wdata1[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x60,0x8c,0x0a };
		uint8_t wdata2[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x64,0x01,0x83 };
		uint8_t wdata3[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x6c,0x01,0x8b };
		uint8_t wdata4[9] = { 0x09,0x01,0x06,0x01,0x0c,0x00,0x68,0x01,0x86 };
		uint8_t rdata[5] = { 0x05,0x01,0x02,0x01,0x09 };
		uint8_t rdata1[6] = { 0x06,0x01,0x03,0x01,0x8c,0x97 };
		uint8_t req[5] = { 0 }, req1[6] = { 0 };

		if (9 != device_->i2cWrite(wdata1, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata2, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata3, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata4, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (6 != device_->i2cRead(req1, 6)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 6; i++) {
			if (rdata1[i] != req1[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::tempUnint()
{
	bool result = false, success = true;
	do
	{
		uint8_t wdata1[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x60,0x80,0xfe };
		uint8_t wdata2[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x64,0x01,0x83 };
		uint8_t wdata3[9] = { 0x09,0x01,0x06,0x02,0x0c,0x00,0x6c,0x01,0x8b };
		uint8_t wdata4[9] = { 0x09,0x01,0x06,0x01,0x0c,0x00,0x68,0x01,0x86 };
		uint8_t rdata[5] = { 0x05,0x01,0x02,0x01,0x09 };
		uint8_t rdata1[6] = { 0x06,0x01,0x03,0x01,0x80,0x8b };
		uint8_t req[5] = { 0 }, req1[6] = { 0 };

		if (9 != device_->i2cWrite(wdata1, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata2, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata3, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (5 != device_->i2cRead(req, 5)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 5; i++) {
			if (rdata[i] != req[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (9 != device_->i2cWrite(wdata4, 9)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (6 != device_->i2cRead(req1, 6)) {
			setLastError(device_->getLastError());
			break;
		}

		for (int i = 0; i < 6; i++) {
			if (rdata1[i] != req1[i]) {
				success = false;
				break;
			}
		}

		if (!success) {
			setErrorInfo(req[3]);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

void isp::Isx016::setErrorInfo(uint8_t code)
{
	switch (code)
	{
	case SC016_RATE_TOO_FAST:
		setLastError("设备速率太快,请降低速率");
		break;

	case SC016_SUCCESS:
		setLastError("成功");
		break;

	case SC016_ERR_EFFI_NUM:
		setLastError("效率通信字节数错误");
		break;

	case SC016_ERR_CMD_NUM_NOT_EXIST:
		setLastError("命令编号错误(命令行不存在)");
		break;

	case SC016_ERR_CAT_NUM:
		setLastError("类别号错误");
		break;

	case SC016_ERR_ADDR_OFFSET:
		setLastError("字节偏移量错误");
		break;

	case SC016_ERR_ACCESS:
		setLastError("访问错误");
		break;

	case SC016_ERR_CMD_NUM_NOT_MATCH:
		setLastError("命令编号错误(命令不匹配)");
		break;

	case SC016_ERR_CHECK_SUM:
		setLastError("校验和错误");
		break;

	case SC016_ERR_TOTAL_BYTE_NUM:
		setLastError("总字节数错误");
		break;

	case SC016_ERR_EEP_ACCESS:
		setLastError("EEPROM访问错误");
		break;

	case SC016_ERR_COMMUNICATION:
		setLastError("通讯错误");
		break;

	default:
		setLastError("校验错误,未知代码%d", code);
		break;
	}
	return;
}

bool isp::Isx016::protect()
{
	bool result = false;
	do
	{
		uint8_t data[256] = { 0 };
		data[0] = 0x02;
		if (!device_->i2cAddrWrite(0x0E, 1, data, 1, true, SLAVE_ADDR_96701)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		data[0] = 0x8c;
		if (!writeRegister(0x0c, 0x0060, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		data[0] = 0x01;
		if (!writeRegister(0x0c, 0x0064, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		data[0] = 0x01;
		if (!writeRegister(0x0c, 0x006c, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!readRegister(0x0c, 0x0068, data, 1)) {
			break;
		}

		if (data[0] != 0x8c) {
			setLastError("0x0c寄存器不等于0x8c");
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!unlock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!readData(0x0126, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		uint8_t value = data[0] + 1;
		if (!writeData(0x0126, &value, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		value = data[0] + 2;
		if (!readData(0x0126, &value, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (value == data[0] + 1) {
			if (!writeData(0x0126, data, 1)) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			if (!readData(0x0126, &value, 1)) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			if (value != data[0]) {
				setLastError("地址0x0126对比失败");
				break;
			}
			break;
		}

		if (!lock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::unprotect()
{
	bool result = false;
	do
	{
		uint8_t data[256] = { 0 };
		data[0] = 0x02;
		if (!device_->i2cAddrWrite(0x0E, 1, data, 1, true, SLAVE_ADDR_96701)) {
			setLastError(device_->getLastError());
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		data[0] = 0x80;
		if (!writeRegister(0x0c, 0x0060, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		data[0] = 0x01;
		if (!writeRegister(0x0c, 0x0064, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1100));

		data[0] = 0x01;
		if (!writeRegister(0x0c, 0x006c, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		if (!readRegister(0x0c, 0x0068, data, 1)) {
			break;
		}

		if (data[0] != 0x80) {
			setLastError("0x0c寄存器不等于0x80");
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!unlock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (!readData(0x0126, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		uint8_t value = data[0] + 1;
		if (!writeData(0x0126, &value, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		value = data[0] + 2;
		if (!readData(0x0126, &value, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (value != data[0] + 1) {
			setLastError("地址0x0126对比1失败");
			break;
		}

		if (!writeData(0x0126, data, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (!readData(0x0126, &value, 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (value != data[0]) {
			setLastError("地址0x0126对比2失败");
			break;
		}

		if (!lock()) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		result = true;
	} while (false);
	return result;
}

bool isp::Isx016::convertToAddress()
{
	uint8_t data[1] = { 0 };
	return writeData(0x0154, data, 1);
}

bool isp::Isx016::changeFlashStatus()
{
	/*
	* @索尼016"FLASH"
	* @notice,FLASH已弃用,
	* 现在为EEPROM.
	* 地址0xffff
	* 写0xf1,改变为读状态
	* 写0xf0,改编为写状态
	*/
	return device_->i2cAddrWrite(0xFFFF, 2, { 0xf1 });
}


