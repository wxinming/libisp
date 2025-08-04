#include "Base.h"
#include <stdarg.h>

isp::Base::Base(std::shared_ptr<fbd::Base> device)
	:device_(device)
{

}

isp::Base::~Base()
{

}

std::string isp::Base::getLastError() const
{
	return last_error_;
}

void isp::Base::setSensorType(SensorType sensorType)
{
	sensor_type_ = sensorType;
}

isp::SensorType isp::Base::getSensorType() const
{
	return sensor_type_;
}

void isp::Base::setFileType(FileType fileType)
{
	file_type_ = fileType;
}

isp::FileType isp::Base::getFileType() const
{
	return file_type_;
}

bool isp::Base::readFlash(uint32_t address, uint8_t* data, uint16_t size)
{
	setLastError("����δʵ�ֶ�ȡFlash");
	return false;
}

bool isp::Base::writeFlash(uint32_t address, const uint8_t* data, uint16_t size)
{
	setLastError("����δʵ��д��Flash");
	return false;
}

bool isp::Base::readEeprom(uint32_t address, uint8_t* data, uint16_t size)
{
	setLastError("����δʵ�ֶ�ȡEeprom");
	return false;
}

bool isp::Base::writeEeprom(uint32_t address, const uint8_t* data, uint16_t size)
{
	setLastError("����δʵ��д��Eeprom");
	return false;
}

bool isp::Base::readRegister(uint32_t address, uint8_t* data, uint16_t size)
{
	setLastError("����δʵ�ֶ�ȡ�Ĵ���");
	return false;
}

bool isp::Base::writeRegister(uint32_t address, const uint8_t* data, uint16_t size)
{
	setLastError("����δʵ��д��Ĵ���");
	return false;
}

bool isp::Base::lock()
{
	setLastError("����δʵ������");
	return false;
}

bool isp::Base::unlock()
{
	setLastError("����δʵ�ֽ���");
	return false;
}

bool isp::Base::enableWp()
{
	setLastError("����δʵ������д����");
	return false;
}

bool isp::Base::disableWp()
{
	setLastError("����δʵ�ֽ���д����");
	return false;
}

bool isp::Base::readData(uint32_t address, uint8_t* data, uint16_t size)
{
	setLastError("����δʵ��readData");
	return false;
}

bool isp::Base::writeData(uint32_t address, const uint8_t* data, uint16_t size)
{
	setLastError("����δʵ��writeData");
	return false;
}

void isp::Base::setLastError(const char* fmt, ...)
{
	va_list ap = nullptr;
	va_start(ap, fmt);
	auto length = _vscprintf_p(fmt, ap) + 1;
	std::unique_ptr<char[]> buffer(new char[length]);
	memset(buffer.get(), 0, length);
	_vsprintf_p(buffer.get(), length, fmt, ap);
	va_end(ap);
	last_error_ = buffer.get();
}

void isp::Base::setLastError(const std::string& msg)
{
	last_error_ = msg;
}

std::deque<uint32_t> isp::Base::calculateSector(const uint8_t* buffer, uint32_t size, uint32_t multiple, uint8_t value) const
{
	std::deque<uint32_t> result;
	int modeCount = 0;
	bool find = true;
	for (uint32_t i = 0; i < size; i += multiple)
	{
		if (i && (i % multiple == 0))
		{
			for (uint32_t j = modeCount * multiple; j < i - 1; j++)
			{
				if (buffer[j] != value)
				{
					find = false;
					break;
				}
			}

			if (find)
			{
				result.push_back(modeCount * multiple);
			}
			find = true;
			modeCount++;
		}
	}
	return result;
}

uint32_t isp::Base::findNearAddress(uint32_t address, uint32_t multiple, bool lessThan) const
{
	uint32_t result = (uint32_t)-1;
	do
	{
		//��̬�������Ч��
		std::vector<uint32_t> datas(address / multiple + 2);
		std::generate(datas.begin(), datas.end(),
			[multiple, n = 0]()mutable->uint32_t{return n += multiple; });
		bool find = false;
		for (auto& x : datas)
		{
			if (x == address)
			{
				find = true;
				break;
			}
		}

		if (find)
		{
			result = address;
			break;
		}

		datas.push_back(address);

		if (lessThan)
			std::sort(datas.begin(), datas.end(), std::less<uint32_t>());
		else
			std::sort(datas.begin(), datas.end(), std::greater<uint32_t>());

		for (auto iter = datas.begin(); iter != datas.end(); ++iter)
		{
			if (*iter == address)
			{
				result = *std::prev(iter);
				break;
			}
		}
	} while (false);
	return result;
}

void isp::Base::parseRlgRegisterText(const std::string& text, std::vector<uint8_t>& slave, 
	std::vector<uint16_t>& reg, std::vector<uint8_t>& reglen, std::vector<uint8_t>& data)
{
	std::istringstream input_stream(text);  // �����ַ����� 
	std::string line;

	// ���зָ������ַ��� 
	while (std::getline(input_stream, line)) {
		std::istringstream line_stream(line);
		std::string token;
		std::vector<std::string> tokens;

		// �ָ�ÿ�е�4���ֶ� 
		while (std::getline(line_stream, token, ',')) {
			// �Ƴ�ǰ���հ��ַ� 
			size_t start = token.find_first_not_of("  \t");
			if (start != std::string::npos) {
				token = token.substr(start);
			}
			tokens.push_back(token);
		}

		if (tokens.size() != 4) continue;  // ��Ч��У�� 

		try {
			// ʹ��lambda���ʽ�Ż�ʮ������ת�� 
			auto hex_to_int = [](const std::string& s) {
				return static_cast<uint32_t>(std::stoul(s, nullptr, 16));
			};

			// �������ֶ� 
			slave.push_back(hex_to_int(tokens[0]));
			reg.push_back(hex_to_int(tokens[1]));
			data.push_back(hex_to_int(tokens[2]));

			char temp[16] = { 0 };
			sprintf_s(temp, "%x", (hex_to_int(tokens[3]) >> 8) & 0xff);
			auto len = std::stoi(temp, nullptr, 10) / 8;
			reglen.push_back(len);
		}
		catch (...) {
			// �������ɸ�����Ҫ�����־��¼��
			continue;
		}
	}
}

static uint16_t safeStoi(const std::string& str, int base = 10, uint16_t defaultValue = 0) {
	try {
		if (str.empty()) return defaultValue;
		return std::stoi(str, nullptr, base);
	}
	catch (const std::exception&) {
		return defaultValue;
	}
}

void isp::Base::parseAviewRegisterText(const std::string& text, std::vector<uint8_t>& slave, 
	std::vector<uint16_t>& reg, std::vector<uint8_t>& data, std::vector<uint16_t>& delay)
{
	// ����������
	slave.clear();
	reg.clear();
	reg.clear();
	delay.clear();

	std::istringstream iss(text);
	std::string line;

	while (std::getline(iss, line)) {
		// ��������
		if (line.empty()) continue;

		std::istringstream lineStream(line);
		std::string token;
		std::vector<std::string> tokens;

		// �ָ������ֶ�
		while (std::getline(lineStream, token, '|')) {
			tokens.push_back(token);
		}

		// ȷ�����㹻���ֶΣ�����8����
		if (tokens.size() < 8) {
			continue;
		}

		// �����ӻ���ַ����2�У�ʮ�����ƣ�
		uint16_t slaveAddr = safeStoi(tokens[2], 16);

		// �����Ĵ�����ַ����3�У�ʮ�����ƣ�
		uint16_t regAddr = safeStoi(tokens[3], 16);

		// �����Ĵ������ݣ���6�У�ʮ�����ƣ�
		uint16_t regData = safeStoi(tokens[6], 16);

		// ������ʱ����7�У�ʮ���ƣ�
		uint16_t writeDelay = safeStoi(tokens[7], 10);

		// ��ӽ������
		slave.push_back(static_cast<uint8_t>(slaveAddr));
		reg.push_back(regAddr);  // ʹ��ʵ�ʵļĴ�����ַ����3�У�
		reg.push_back(static_cast<uint8_t>(regData));
		delay.push_back(writeDelay);
	}
}

