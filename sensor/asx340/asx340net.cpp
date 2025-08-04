/************************************************************************/
/* 因Windows下WinSock2.h与其他有冲突,所以在此处声明.这里水很深不要去尝试*/
/************************************************************************/
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#define mySleep(x) Sleep(x)

#include "ASX340NET.h"

static uint16_t g_port;
static SOCKET g_socket;
static sockaddr_in g_serverAddr;
IspTool* IspTool::m_self = nullptr;
void IspTool::setLastError(const std::string& err)
{
	m_lastError = err;
}
DWORD point = 0;

int16_t IspTool::registerIsBusy()
{
	uint16_t regData = 0;
	uint16_t result = readRegister2Bytes(COMMAND_REGISTER, regData);
	if (result == ERROR_ENOERR)
	{
		regData = regData & (1 << 15);
		if (regData != 0)
		{
			result = ERROR_EBUSY;
		}
		else
		{
			result = ERROR_ENOERR;
		}
	}
	return result;
}

int16_t IspTool::waitRegisterLeisure(unsigned int timeout, unsigned int timeoutDelay)
{
	uint16_t result = ERROR_EBUSY;
	do 
	{
		if (!timeout)
		{
			result = ERROR_ENOERR;
			break;
		}
		int numLoops = timeout / ((timeoutDelay > 1) ? timeoutDelay : 1);
		int loopCount = 0;
		while (loopCount < numLoops)
		{
			if (registerIsBusy() == ERROR_ENOERR)
			{
				return ERROR_ENOERR;
			}
			loopCount += 1;
			if (0 != timeoutDelay)
			{
				mySleep(timeoutDelay);
			}
		}
	} while (false);
	return result;
}

uint32_t IspTool::createSn()
{
	static uint32_t sn = 0xb01557f0;
	return sn--;
}

int16_t IspTool::sendRecvMsg(uint8_t* buffer, const uint16_t& bufferLen, uint16_t& paramLen)
{
	int16_t result = 0;
	do
	{
		uint32_t srcSn = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
		if (sendto(g_socket, (char*)buffer, bufferLen, 0, (const sockaddr*)&g_serverAddr, sizeof(g_serverAddr)) != bufferLen)
		{
			result = -1;
			setLastError("发送到" + m_ipaddr + "失败");
			break;
		}

		mySleep(m_delay);
		uint8_t recvBuffer[1024] = { 0 };
		if (recvfrom(g_socket, (char*)recvBuffer, sizeof(recvBuffer), 0, NULL, NULL) == -1)
		{
			result = -1;
			setLastError("接收来自" + m_ipaddr + "失败");
			break;
		}

		uint32_t dstSn = (recvBuffer[0] << 24) | (recvBuffer[1] << 16) | (recvBuffer[2] << 8) | recvBuffer[3];
		uint16_t resultStatusCode = (recvBuffer[4] << 8) | recvBuffer[5];
		paramLen = (recvBuffer[6] << 8) | recvBuffer[7];

		if (srcSn != dstSn)
		{
			result = ERROR_EINVAL;
			break;
		}

		switch (resultStatusCode)
		{
		case ERROR_ENOERR:
			memcpy(buffer, recvBuffer + 8, paramLen);
			result = ERROR_ENOERR;
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::sendRecvMsgEx(uint8_t* buffer, const uint16_t& bufferLen, uint16_t& paramLen, uint16_t loop)
{
	uint16_t result = ERROR_EINTR;
	do 
	{
		uint8_t payLoad[4096] = { 0 };
		memcpy(payLoad, buffer, bufferLen);
		uint16_t loopCount = 0;
		while (loopCount < loop)
		{
			uint32_t sn = createSn();
			payLoad[0] = sn >> 24;
			payLoad[1] = (sn & 0xFFFFFF) >> 16;
			payLoad[2] = (sn & 0xFFFF) >> 8;
			payLoad[3] = sn & 0xFF;
			memcpy(buffer, payLoad, bufferLen);
			result = sendRecvMsg(buffer, bufferLen, paramLen);
			if (result == ERROR_ENOERR)
			{
				break;
			}
			loopCount++;
			mySleep(10);
		}
	} while (false);
	return result;
}

uint16_t IspTool::checkSum(uint8_t* buffer, uint32_t count)
{
	uint32_t sum = 0;

	while (count > 1)
	{
		sum = sum + ((*buffer << 8) | *(buffer + 1));
		count = count - 2;
		buffer += 2;
	}
	if (count > 0)
	{
		sum = sum + *((uint8_t*)buffer);
	}

	while (sum >> 16)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return(~sum);
}

IspTool::IspTool()
{

}

IspTool::~IspTool()
{

}

IspTool* IspTool::getInstance()
{
	if (!m_self)
	{
		m_self = new(std::nothrow) IspTool;
	}
	return m_self;
}

void IspTool::deleteInstance()
{
	delete m_self;
	m_self = nullptr;
}

bool IspTool::initNetwork(const std::string& ipaddr, const uint16_t& port, const std::string& protocol)
{
	bool result = false;
	do 
	{
		if (m_init)
		{
			deinitNetwork();
		}
		WORD socketVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(socketVersion, &wsaData))
		{
			setLastError("WSAStartup初始化失败");
			break;
		}
		g_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (g_socket == INVALID_SOCKET)
		{
			setLastError("SOCKET初始化失败");
			break;
		}
		int timeout = 2000;
		if (setsockopt(g_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == -1)
		{
			setLastError("设置发送数据超时失败");
			break;
		}

		if (setsockopt(g_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1)
		{
			setLastError("设置接收数据超时失败");
			break;
		}
		memset(&g_serverAddr, 0x00, sizeof(g_serverAddr));
		g_serverAddr.sin_family = AF_INET;
		g_serverAddr.sin_port = htons(port);
		g_serverAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr.c_str());
		m_init = result = true;
	} while (false);
	return result;
}

bool IspTool::deinitNetwork()
{
	closesocket(g_socket);
	WSACleanup();
	return true;
}

void IspTool::enableDebugOutput(bool result)
{

}

const std::string& IspTool::getLastError()
{
	return m_lastError;
}

int16_t IspTool::readRegister(const uint16_t& regAddr,const uint16_t& totalBytes, std::vector<uint8_t>& regDatas)
{
	uint16_t commandId = (regAddr < REG_ADDRESS_MAX) ? NET_GET_REG_16 : NET_GET_VAR_8;
	uint16_t paramLen = PARAM_LENGTH;
	uint16_t readBytes = (regAddr < REG_ADDRESS_MAX) ? 0 : (totalBytes - 1);
	uint8_t buffer[512] = { 0 };
	uint16_t len = 4;

	buffer[len++] = commandId >> 8;
	buffer[len++] = commandId & 0xFF;
	buffer[len++] = paramLen >> 8;
	buffer[len++] = paramLen & 0xFF;
	buffer[len++] = regAddr >> 8;
	buffer[len++] = regAddr & 0xFF;
	buffer[len++] = readBytes >> 8;
	buffer[len++] = readBytes & 0xFF;

	uint16_t result = sendRecvMsgEx(buffer, len, paramLen);
	if (result == ERROR_ENOERR)
	{
		for (uint32_t i = 0; i < paramLen; i++)
		{
			regDatas.push_back(buffer[i]);
		}
	}
	else
	{
		setLastError("读寄存器失败");
	}
	return result;
}

int16_t IspTool::readRegister1Bytes(const uint16_t& regAddr, uint8_t& value)
{
	std::vector<uint8_t> values;
	uint16_t result = readRegister(regAddr, 1, values);
	if (result == ERROR_ENOERR)
	{
		value = values[0];
	}
	return result;
}

int16_t IspTool::readRegister2Bytes(const uint16_t& regAddr, uint16_t& value)
{
	std::vector<uint8_t> values;
	uint16_t result = readRegister(regAddr, 1, values);
	if (result == ERROR_ENOERR)
	{
		value = (values[0] << 8) | values[1];
	}
	return result;
}

int16_t IspTool::writeRegister(const uint16_t& regAddr, const std::vector<uint8_t>& regDatas)
{
	uint16_t commandId = (regAddr < REG_ADDRESS_MAX) ? NET_SET_REG_16 : NET_SET_VAR_8;
	uint16_t paramLen = 4 + regDatas.size();
	uint16_t writeBytes = (regAddr < REG_ADDRESS_MAX) ? 0 : (regDatas.size() - 1);
	uint8_t buffer[512];
	uint16_t len = 4;
	buffer[len++] = commandId >> 8;
	buffer[len++] = commandId & 0xFF;
	buffer[len++] = paramLen >> 8;
	buffer[len++] = paramLen & 0xFF;
	buffer[len++] = regAddr >> 8;
	buffer[len++] = regAddr & 0xFF;
	buffer[len++] = writeBytes >> 8;
	buffer[len++] = writeBytes & 0xFF;
	for (uint32_t i = 0; i < regDatas.size(); i++)
	{
		buffer[len + i] = regDatas[i];
	}
	len = len + regDatas.size();

	uint16_t result = sendRecvMsgEx(buffer, len, paramLen);
	if (result != ERROR_ENOERR)
	{
		setLastError("写寄存器失败");
	}
	return result;
}

int16_t IspTool::writeRegister1Bytes(const uint16_t& regAddr, const uint8_t& value)
{
	return writeRegister(regAddr, std::vector<uint8_t>(value));
}

int16_t IspTool::writeRegister2Bytes(const uint16_t& regAddr, const uint16_t& value)
{
	std::vector<uint8_t> values;
	values.push_back((value & 0xFF00) >> 8);
	values.push_back(value & 0xFF);
	return writeRegister(regAddr, values);
}

int16_t IspTool::changeConfiguration()
{
	int16_t result = ERROR_EINVAL;
	do
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = SYSMGR_SET_STATE;
		uint16_t paramLen = 2;
		uint8_t sysMgrState = SYS_STATE_ENTER_CONFIG_CHANGE;
		uint16_t len = 4;

		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;
		buffer[len++] = sysMgrState;
		buffer[len++] = 0;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EINVAL:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("Requested state parameter is invalid"));*/
			break;
		case ERROR_EACCES:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("State cannot be requested at present"));*/
			break;
		default:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("Appendix D: Set State Command Failure Codes"));*/
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::setSystemStatus(const uint16_t& status)
{
	int16_t result = ERROR_EINVAL;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512];
		uint16_t commandId = SYSMGR_SET_STATE;
		uint16_t paramLen = 2;
		uint16_t len = 4;

		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;
		buffer[len++] = status;
		buffer[len++] = 0;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EINVAL:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("Requested state parameter is invalid"));*/
			break;
		case ERROR_EACCES:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("State cannot be requested at present"));*/
			break;
		default:
			/*LogErrorPosition(__FUNCTION__, __LINE__);
			LogMessage(_T("Appendix D: Set State Command Failure Codes"));*/
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::canBeCommunication()
{
	uint16_t paramLen = PARAM_LENGTH;
	uint16_t numOfByte = 0;
	uint8_t buffer[512];
	uint16_t len = 4;
	buffer[len++] = (NET_GET_REG_16 & 0xFFFF) >> 8;
	buffer[len++] = NET_GET_REG_16 & 0xFF;
	buffer[len++] = (paramLen & 0xFFFF) >> 8;
	buffer[len++] = paramLen & 0xFF;
	buffer[len++] = (CHIP_VERSION_REG & 0xFFFF) >> 8;
	buffer[len++] = CHIP_VERSION_REG & 0xFF;
	buffer[len++] = (numOfByte & 0xFFFF) >> 8;
	buffer[len++] = numOfByte & 0xFF;
	return sendRecvMsgEx(buffer, len, paramLen, 1);
}

int16_t IspTool::resetSelf()
{
	uint16_t value = 0;
	readRegister2Bytes(RESET_AND_MISC_CONTROL, value);
	value |= 0x01;
	writeRegister2Bytes(RESET_AND_MISC_CONTROL, value);
	mySleep(10);
	value &= 0xFE;
	mySleep(10);
	return 0;
}

int16_t IspTool::lock()
{
	int16_t result = ERROR_EINVAL;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_GET_LOCK;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous Get Lock request has not completed"));
			break;
		case ERROR_EALREADY:
			//LogMessage(_T("Lock has already been obtained"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::unlock()
{
	int16_t result = ERROR_EAGAIN;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_RELEASE_LOCK;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous Get Lock request has not completed"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::getLockStatus()
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_LOCK_STATUS;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous Get Lock request has not completed"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::readFlash(const uint32_t& address, const uint8_t& length, uint8_t** dataArray)
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		if (length > MAX_READ_SIZE)
		{
			//LogMessage(_T("NVM Read size is too big"));
			result = -2;
			break;
		}

		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_READ;
		uint16_t paramLen = 5;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;
		buffer[len++] = address >> 24;
		buffer[len++] = (address & 0xFFFFFF) >> 16;
		buffer[len++] = (address & 0xFFFF) >> 8;
		buffer[len++] = address & 0xFF;
		buffer[len++] = length;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
		{
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			result = getFlashStatus(dataArray);
		}
		break;
		case ERROR_ENODEV:
			//LogMessage(_T("NVM device was not detected"));
			break;
		case ERROR_ERANGE:
			//LogMessage(_T("Length is out of range"));
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous operation is still in progress"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the flash manager access lock"));
			break;
		}
	} while (false);
	
	return result;
}

int16_t IspTool::writeFlash(const uint32_t& address, const uint8_t& length, uint8_t* dataArray)
{
	int16_t result = ERROR_ENOERR;
	do
	{
		if (length > MAX_WRITE_SIZE)
		{
			//LogMessage(_T("NVM write size is too big"));
			result = -2;
			break;
		}

		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		int loopCount = 0;
		int numLoops = 10;
		uint16_t timeoutDelay = 10;
		uint8_t buffer[512] = { 0 };
		while (loopCount < numLoops)
		{
			uint16_t len = 4;
			uint16_t commandId = FLASHMGR_WRITE;
			uint16_t paramLen = 5 + length;

			buffer[len++] = commandId >> 8;
			buffer[len++] = commandId & 0xFF;
			buffer[len++] = paramLen >> 8;
			buffer[len++] = paramLen & 0xFF;
			buffer[len++] = address >> 24;
			buffer[len++] = (address & 0xFFFFFF) >> 16;
			buffer[len++] = (address & 0xFFFF) >> 8;
			buffer[len++] = address & 0xFF;
			buffer[len++] = length;

			for (uint16_t i = 0; i < length; i++)
			{
				buffer[len + i] = dataArray[i];
			}
			len += length;

			result = sendRecvMsgEx(buffer, len, paramLen);
			if (result == ERROR_ENOERR)
			{
				if (waitRegisterLeisure() != ERROR_ENOERR)
				{
					//LogErrorPosition(__FUNCTION__, __LINE__);
					result = ERROR_EAGAIN;
				}
				break;
			}
			loopCount += 1;
			mySleep(timeoutDelay);
		}

		if (result == ERROR_EAGAIN)
		{
			break;
		}

		if (loopCount >= numLoops)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			break;
		}

		switch (result)
		{
		case ERROR_ENOERR:
		{
			//LogMessage(_T("FlashMgrWrite() ok"));
			uint8_t loop = 100;
			while (loop > 0)
			{
				if (getFlashStatus() == ERROR_ENOERR)
				{
					//LogMessage(_T("NVM write command finished"));
					break;
				}
				else
				{
					//LogMessage(_T("+"));
					mySleep(1); //Wait 1ms
					loop--;
				}
			}
			if (loop <= 0)
			{
				result = ERROR_EBUSY;
				//LogMessage(_T("FlashMgrWrite() is out of time"));
			}
		}
		break;
		case ERROR_ENODEV:
			//LogMessage(_T("NVM device was not detected"));
			break;
		case ERROR_ERANGE:
			//LogMessage(_T("Length is out of range"));
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous operation is still in progress"));
			break;
		case ERROR_ENOSYS:
			//LogMessage(_T("Requested operation is not supported"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the flash manager access lock"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::getFlashStatus(uint8_t** dataArray)
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_STATUS;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
		{
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			else
			{
				memcpy(*dataArray, buffer, paramLen);
			}
		}
		break;
		case ERROR_EIO:
			//LogMessage(_T("Data could not be transferred - operation aborted"));
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Operation is still in progress"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the flash manager access lock"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::getFlashStatus()
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_STATUS;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
				break;
			}
			break;
		case ERROR_EIO:
			//LogMessage(_T("Data could not be transferred - operation aborted"));
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Operation is still in progress"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the flash manager access lock"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::configFlashDevice(uint8_t driverId, uint8_t addressWidth, uint32_t sizeOfDevice)
{
	int16_t result = ERROR_ENOERR;
	do
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_CONFIG_DEVICE;
		uint16_t paramLen = 8;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;
		buffer[len++] = driverId;
		buffer[len++] = 0; //Read command: 0 - Standard (0x03 command); 1 - Fast (0x0B command).
		buffer[len++] = addressWidth;
		buffer[len++] = 0; //reserved
		buffer[len++] = sizeOfDevice >> 24;
		buffer[len++] = (sizeOfDevice & 0xFFFFFF) >> 16;
		buffer[len++] = (sizeOfDevice & 0xFFFF) >> 8;
		buffer[len++] = sizeOfDevice & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}
			break;
		case ERROR_EINVAL:
			//LogMessage(_T("Requested configuration parameter is invalid"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the Flash Manager access lock"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::eraseFlashDevice()
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		if (waitRegisterLeisure() != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			result = ERROR_EAGAIN;
			break;
		}

		uint8_t buffer[512] = { 0 };
		uint16_t commandId = FLASHMGR_ERASE_DEVICE;
		uint16_t paramLen = 0;
		uint16_t len = 4;
		buffer[len++] = commandId >> 8;
		buffer[len++] = commandId & 0xFF;
		buffer[len++] = paramLen >> 8;
		buffer[len++] = paramLen & 0xFF;

		result = sendRecvMsgEx(buffer, len, paramLen);
		switch (result)
		{
		case ERROR_ENOERR:
		{
			if (waitRegisterLeisure() != ERROR_ENOERR)
			{
				//LogErrorPosition(__FUNCTION__, __LINE__);
				result = ERROR_EAGAIN;
			}

			int8_t loop = 100;
			while (loop > 0)
			{
				if (getFlashStatus() == ERROR_ENOERR)
				{
					//LogMessage(_T("Erase NVM device finished"));
					break;
				}
				else
				{
					//LogMessage(_T("-"));
					mySleep(100); //Wait 100ms
					loop--;
				}
			}
			if (loop <= 0)
			{
				result = ERROR_EBUSY;
				//LogMessage(_T("FlashMgrEraseDevice() process is out of time"));
			}
		}
		break;
		case ERROR_ENODEV:
			//LogMessage(_T("NVM device was not detected"));
			break;
		case ERROR_EBUSY:
			//LogMessage(_T("Previous operation is still in progress"));
			break;
		case ERROR_ENOSYS:
			//LogMessage(_T("Requested operation is not supported"));
			break;
		case ERROR_EACCES:
			//LogMessage(_T("Host does not own the flash manager access lock"));
			break;
		}
	} while (false);
	return result;
}

int16_t IspTool::configFlashDeviceNvm(uint8_t byteWidth, uint32_t sizeOfDevice, uint8_t driverId)
{
	uint16_t result = ERROR_ENOERR;
	do 
	{
		unlock();
		result = lock();
		if (result == ERROR_EBUSY)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			break;
		}
		configFlashDevice(driverId, byteWidth, sizeOfDevice);
		unlock();
	} while (false);
	
	return result;
}

int16_t IspTool::readFlashDeviceNvm(uint8_t** dataArray, int32_t numOfBytes, uint32_t address, int* progress)
{
	int16_t result = ERROR_ENOERR;
	do 
	{
		uint8_t buffer[512] = { 0 };
		uint8_t* pBuf = buffer;
		uint8_t* pOutput = *dataArray;
		uint8_t len = 0;
		if (progress)
		{
			*progress = 0;
		}

		unlock();
		result = lock();
		if (result == ERROR_EBUSY)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			break;
		}
		getFlashStatus();

		int32_t lenLeft = numOfBytes;
		while (lenLeft > 0)
		{
			if (lenLeft > MAX_READ_SIZE)
			{
				len = MAX_READ_SIZE;
			}
			else
			{
				len = lenLeft;
			}
			result = readFlash(address, len, &pBuf);
			if (result != ERROR_ENOERR)
			{
				//wxLogMessage(_T("NVM read is broken"));
				break;
			}

			memcpy(pOutput, pBuf, len);
			lenLeft -= len;
			address += len;
			pOutput += len;
			if (progress)
			{
				*progress = (numOfBytes - lenLeft) * 100 / numOfBytes;
			}
		}

		if (result != ERROR_ENOERR)
		{
			break;
		}

		if (progress && result == ERROR_ENOERR)
		{
			*progress = 100;
		}
		unlock();
	} while (false);
	return result;
}

int16_t IspTool::writeFlashDeviceNvm(const uint8_t* data, int32_t numOfBytes, uint32_t byteWidth, uint32_t address, int* progress)
{
	uint8_t buffer[512] = { 0 };
	uint8_t len = 0;
	const uint8_t* pInput = data;
	uint16_t resultStatusCode = ERROR_ENOERR;
	if (progress)
	{
		*progress = 0;
	}

	unlock();
	resultStatusCode = lock();
	if (resultStatusCode == ERROR_EBUSY)
	{
		//LogErrorPosition(__FUNCTION__, __LINE__);
		return resultStatusCode;
	}
	configFlashDevice(SPI_EEPROM_RW, byteWidth, 16 * 1024);
	//eraseFlashDevice();//非闪存
	int32_t lenLeft = numOfBytes;
	int32_t printFlag = 0;
	while (lenLeft > 0)
	{
		if (lenLeft > MAX_WRITE_SIZE)
		{
			len = MAX_WRITE_SIZE;
		}
		else
		{
			len = lenLeft;
		}

		memcpy(buffer, pInput, len);
		resultStatusCode = writeFlash(address, len, buffer);
		if (resultStatusCode != ERROR_ENOERR)
		{
			//LogErrorPosition(__FUNCTION__, __LINE__);
			//wxLogMessage(_T("UDP error appears during NVM program, please reprogram without reset"));
			break;
		}
		lenLeft -= len;
		address += len;
		pInput += len;

		//if(++printFlag % 30 == 0) wxLogMessage(_T("+"));
		if (progress)
		{
			*progress = (numOfBytes - lenLeft) * 100 / numOfBytes;
		}
	}

	if (progress && resultStatusCode == ERROR_ENOERR)
	{
		*progress = 100;
	}
	configFlashDevice(SPI_READONLY, byteWidth, 16 * 1024);
	unlock();
	return resultStatusCode;
}

