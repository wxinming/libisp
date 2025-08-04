#include "OVX1E.h"

ChipBurn::Ovx1e::Ovx1e()
{
}

ChipBurn::Ovx1e::~Ovx1e()
{

}

bool ChipBurn::Ovx1e::initialize()
{
	bool result = false, success = true;
	do 
	{
		if (m_chipType == CT_OVX1E_FLASH_96706G_UNIVERSAL)
		{
			uchar addr[] = { 0x0d,0x07,0x04 };
			uchar data[] = { 0x87,0x86,0x47 };

			if (sizeof(addr) != sizeof(data))
			{
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr); i++)
			{
				if (!i2cAddrWrite(addr[i], 1, &data[i], 1, true,
					sizeof(addr) - i != 1 ? SLAVE_ADDR_96706 : SLAVE_ADDR_96701))
				{
					success = false;
					break;
				}
				msleep(50);
			}

			if (!success)
			{
				break;
			}
		}
		else if (m_chipType == CT_OVX1E_FLASH_UB934Q_UNIVERSAL)
		{
			uchar addr[] = { 0x4c,0x58,0x5c,0x5d,0x65 };
			uchar data[] = { 0x01,0x58,0xb0,0x6c,0x6c };
			if (sizeof(addr) != sizeof(data))
			{
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr); i++)
			{
				if (!i2cAddrWrite(addr[i], 1, &data[i], 1, true, SLAVE_ADDR_934))
				{
					success = false;
					break;
				}
				msleep(50);
			}

			if (!success)
			{
				break;
			}
		}
#if 0
		else if (m_chipType == CT_OVX1E_FLASH_9296A_UNIVERSAL)
		{
			//缺少硬件支持,尚未调试成功.2024/4/26添加
			uchar slave[] =
			{
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x00,
				0x90, 0x90, 0x90, 0x00, 0x90, 0x90, 0x90, 0x90,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
				0x90, 0x90, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80,
				0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
				0x80, 0x80, 0x6c, 0x90, 0x90, 0x90, 0x90
			};

			uint addr[] =
			{
				0x0313, 0x0b06, 0x0c06, 0x0b07, 0x0c07, 0x0011, 0x0006, 0x00ff,
				0x0322, 0x0333, 0x0f00, 0x00ff, 0x040b, 0x042d, 0x040d, 0x040e,
				0x040f, 0x0410, 0x0411, 0x0412, 0x0330, 0x044a, 0x0320, 0x0325,
				0x0319, 0x0316, 0x0317, 0x031d, 0x01da, 0x01fa, 0x0b0f, 0x0c0f,
				0x0ba7, 0x0ca7, 0x0043, 0x0047, 0x00ff, 0x000f, 0x0007, 0x0047,
				0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x00ff, 0x0004, 0x0007,
				0x0067, 0x0004, 0xae00, 0x0540, 0x0541, 0x0542, 0x0313
			};

			uchar data[] =
			{
				0x00, 0x87, 0x87, 0x84, 0x84, 0x0f, 0x1f, 0xff,
				0x30, 0x4e, 0x01, 0xff, 0x07, 0x15, 0x1e, 0x1e,
				0x00, 0x00, 0x01, 0x01, 0x04, 0xd0, 0x26, 0x80,
				0x08, 0x5e, 0x0e, 0xc0, 0x18, 0x18, 0x08, 0x08,
				0x40, 0x40, 0x05, 0x2f, 0xff, 0xbf, 0x84, 0x00,
				0x02, 0x48, 0x3c, 0x6a, 0x78, 0xff, 0x47, 0x86,
				0xc7, 0x87, 0x03, 0x80, 0x00, 0x00, 0x42
			};

			if (sizeof(addr) / sizeof(*addr) != sizeof(data))
			{
				setLastError("地址列表与数据列表大小不一致");
				break;
			}

			for (int i = 0; i < sizeof(addr) / sizeof(*addr); i++)
			{
				if (i == sizeof(addr) / sizeof(*addr) - 1)
				{
					continue;
				}

				if (slave[i] != 0)
				{
					if (!i2cAddrWrite(addr[i], 2, &data[i], 1, false, slave[i] >> 1))
					{
						success = false;
						break;
					}
				}
				else
				{
					msleep(0xff);
				}
			}

			if (!success)
			{
				break;
			}
		}
#endif
		else
		{
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

bool ChipBurn::Ovx1e::readFile(uchar* data, uint size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!i2cAddrWrite(0xa10a, 2, { 0x00 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa11e, 2, { 0x7f }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa11d, 2, { 0x00 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa110, 2, { 0x80 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa10f, 2, { 0x18 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa10e, 2, { 0x4c }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa10d, 2, { 0x00 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		for (uint i = 0; i < size; i += 4096)
		{
			if (!readData(i, data + i, size - i > 4096 ? 4096 : size - i))
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
		//uninitChip();
		result = true;
	} while (false);
	return result;
}

bool ChipBurn::Ovx1e::writeFile(const uchar* data, uint size, int* percent)
{
	bool result = false, success = true;
	do
	{
		if (!initChip())
		{
			break;
		}

		for (uint i = 0; i < size; i += 4096)
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

bool ChipBurn::Ovx1e::readFlash(uint address, uchar* data, ushort size)
{
	return readData(address, data, size);
	bool result = false;
	do 
	{
		auto nearAddress = findNearAddress(address, 256);
		uchar nearData[256] = { 0 };
		if (!readData(nearAddress, nearData, sizeof(nearData))) {
			break;
		}
		auto offset = address - nearAddress;
		memcpy(data, &nearData[offset], size);
		result = true;
	} while (false);
	return result;
}

bool ChipBurn::Ovx1e::writeFlash(uint address, const uchar* data, ushort size)
{
	bool result = false;
	do 
	{
		auto nearAddress = findNearAddress(address, 256);
		uchar nearData[256] = { 0 };
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

bool ChipBurn::Ovx1e::readData(uint address, uchar* data, ushort size)
{
	bool result = false, success = true;
	do
	{
		uchar buffer[512] = { 0 };
		uint segment = 0;
		for (uint i = 0; i < size; i += 0x100)
		{
			segment = size - i > 0x100 ? 0x100 : size - i;
			buffer[0] = 0x81;
			buffer[1] = 0x00;
			buffer[2] = 0x00;
			buffer[3] = 0x06;
			buffer[4] = 0x12;
			buffer[5] = ((address + i) >> 16) & 0xff;
			buffer[6] = ((address + i) >> 8) & 0xff;
			buffer[7] = ((address + i) >> 0) & 0xff;
			buffer[8] = (segment >> 8) & 0xff;
			buffer[9] = (segment >> 0) & 0xff;

			if (!i2cAddrWrite(0x8181, 2, { 0 }))
			{
				success = false;
				break;
			}

			if (!sendHostCmd(buffer, 10))
			{
				success = false;
				break;
			}

			for (uint j = 0; j < segment; j += 32)
			{
				if (!i2cAddrRead(0xe700 + j, 2, data + i + j,
					segment - j > 32 ? 32 : segment - j))
				{
					success = false;
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

bool ChipBurn::Ovx1e::writeData(uint address, const uchar* data, ushort size)
{
	bool result = false, success = true;
	do
	{
		uchar buffer[512] = { 0 };
		buffer[0] = 0x81;
		buffer[1] = 0x00;
		buffer[2] = 0x00;
		buffer[3] = 0x04;
		buffer[4] = 0x10;
		buffer[5] = (address >> 16) & 0xff;
		buffer[6] = (address >> 8) & 0xff;
		buffer[7] = (address >> 0) & 0xff;
		buffer[8] = 0x00;
		buffer[9] = 0x00;
		if (!i2cAddrWrite(0x8181, 2, { 0 }))
		{
			break;
		}

		if (!sendHostCmd(buffer, 10))
		{
			break;
		}

		uint segment = 0;
		for (uint i = 0; i < size; i += 0x100)
		{
			segment = size - i > 0x100 ? 0x100 : size - i;

			memset(buffer, 0, sizeof(buffer));
			buffer[0] = 0x81;
			buffer[1] = 0x00;
			buffer[2] = ((segment + 7) >> 8) & 0xff;
			buffer[3] = ((segment + 7) >> 0) & 0xff;
			buffer[4] = 0x11;
			buffer[5] = 0x00;
			buffer[6] = ((address + i) >> 16) & 0xff;
			buffer[7] = ((address + i) >> 8) & 0xff;
			buffer[8] = ((address + i) >> 0) & 0xff;
			buffer[9] = (segment >> 8) & 0xff;
			buffer[10] = (segment >> 0) & 0xff;

			if (!i2cAddrWrite(0xe400, 2, buffer, 11))
			{
				success = false;
				break;
			}

			memcpy(buffer, data + i, segment);

			if (!sendHostCmd(buffer, segment))
			{
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

bool ChipBurn::Ovx1e::sendHostCmd(const uchar* data, uint size)
{
	bool result = false, success = true;
	do
	{
		uint pageAddr = 0, segment = 0;
		for (uint i = 0; i < size; i += 32)
		{
			segment = size - i > 32 ? 32 : size - i;

			if (size == 10)
				pageAddr = 0xe400 + i;
			else
				pageAddr = 0xe40b + i;

			if (!i2cAddrWrite(pageAddr, 2, data + i, (ushort)segment))
			{
				success = false;
				break;
			}
		}

		if (!success)
		{
			break;
		}

		if (!getStatus())
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

bool ChipBurn::Ovx1e::initChip()
{
	bool result = false;
	do 
	{
		uchar data[32] = { 0 };
		if (!i2cAddrRead(0x300a, 2, data, 1))
		{
			break;
		}
		RUN_BREAK(data[0] != 0x58, Q_SPRINTF("寄存器0x300a状态异常,错误代码:0x%02X", data[0]));
		msleep(OVX1E_DELAY);

		if (!i2cAddrRead(0x81a8, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if ((data[0] & 0x0f) != 0x05)
		{
			data[0] = 0x80;
			if (!i2cAddrWrite(0x8057, 2, data, 1))
			{
				break;
			}
			msleep(OVX1E_DELAY);

			if (!i2cAddrWrite(0x805F, 2, data, 1))
			{
				break;
			}
			msleep(OVX1E_DELAY);

			data[0] = 0x01;
			if (!i2cAddrWrite(0xA00D, 2, data, 1))
			{
				break;
			}
			msleep(OVX1E_DELAY);
		}

		data[0] = 0xff;
		if (!i2cAddrWrite(0xA013, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xA014, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xA015, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xA016, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x3d;
		if (!i2cAddrWrite(0x8000, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x01;
		if (!i2cAddrWrite(0x8003, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrRead(0x8000, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] |= 2;
		if (!i2cAddrWrite(0x8000, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x08;
		if (!i2cAddrWrite(0x8088, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0xa10a, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x7f;
		if (!i2cAddrWrite(0xa11e, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0xa11d, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x80;
		if (!i2cAddrWrite(0xa110, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x18;
		if (!i2cAddrWrite(0xa10f, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0xa10e, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa10d, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!writeInitData())
		{
			break;
		}

		if (!i2cAddrRead(0x8000, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0x801b, 2, { 0 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = (data[0] & (~0x3c)) | 0x3c;//0x3d
		if (!i2cAddrWrite(0x8000, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0x8088, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0xa10a, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x7f;
		if (!i2cAddrWrite(0xa11e, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0xa11d, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x80;
		if (!i2cAddrWrite(0xa110, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x18;
		if (!i2cAddrWrite(0xa10f, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x4c;
		if (!i2cAddrWrite(0xa10e, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		data[0] = 0x00;
		if (!i2cAddrWrite(0xa10d, 2, data, 1))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrRead(0x8195, 2, data, 1))
		{
			break;
		}
		RUN_BREAK(data[0] != 0x5a, Q_SPRINTF("引导模式运行失败,错误代码:0x%02X", data[0]));
		msleep(OVX1E_DELAY);

		result = true;
	} while (false);
	return result;
}

bool ChipBurn::Ovx1e::uninitChip()
{
	bool result = false;
	do
	{
		if (!i2cAddrWrite(0x8010, 2, { 0x00 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0x801c, 2, { 0x00,0x00,0x00,0x00 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		if (!i2cAddrWrite(0x8018, 2, { 0x00,0x04,0x00,0xc0 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		result = true;
	} while (false);
	return result;
}

bool ChipBurn::Ovx1e::writeInitData()
{
	const uchar* data = OVX1E_INIT_DATA;
	uint size = sizeof(OVX1E_INIT_DATA);
	bool result = false, mapping = false, success = true;
	do
	{
		uint sectorSize = 0;
		for (uint i = 0; i < size; i += 4096)
		{
			sectorSize = size - i > 4096 ? 4096 : size - i;
			uint segment = 0, pageAddr = 0;
			for (uint j = 0; j < sectorSize; j += 32)
			{
				segment = sectorSize - j > 32 ? 32 : sectorSize - j;
				pageAddr = 0xb000 + i + j;

				if (pageAddr > 0xffff)
				{
					if (!mapping)
					{
						i2cAddrWrite(0xa10a, 2, { 0x00 });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa11e, 2, { 0x7f });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa11d, 2, { 0x00 });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa110, 2, { 0x80 });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa10f, 2, { 0x18 });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa10e, 2, { 0x50 });
						msleep(OVX1E_DELAY);

						i2cAddrWrite(0xa10d, 2, { 0x00 });
						msleep(OVX1E_DELAY);

						mapping = true;
					}
					pageAddr -= 0x5000;
				}

				if (!i2cAddrWrite(pageAddr, 2, data + i + j, segment))
				{
					success = false;
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

bool ChipBurn::Ovx1e::getStatus()
{
	bool result = false, success = false;
	do
	{
		//确认连接
		if (!i2cAddrWrite(0x8160, 2, { 0x01 }))
		{
			break;
		}
		msleep(OVX1E_DELAY);

		uchar data[64] = { 0 };
		uint startTime = GetTickCount64();
		while (true)
		{
			memset(data, 0x00, sizeof(data));
			if (!i2cAddrRead(0x8180, 2, data, 1))
			{
				break;
			}

			if (data[0] == 0x99)
			{
				success = true;
				break;
			}

			if (GetTickCount64() - startTime > 5000)
			{
				setLastError("循环读取0x8180超时");
				break;
			}
			msleep(10);
		}

		if (!success)
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}
