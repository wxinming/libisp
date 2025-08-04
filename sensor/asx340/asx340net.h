#pragma once

#include <string>
#include <vector>

typedef enum NetworkCmdId
{
    NET_SET_REG_16  = 0xC402,
    NET_GET_REG_16  = 0xC403,
    NET_SET_VAR_8   = 0xC500,
    NET_GET_VAR_8   = 0xC501,
    NET_SET_VAR_16  = 0xC502,
    NET_GET_VAR_16  = 0xC503,
    NET_SET_VAR_32  = 0xC504,
    NET_GET_VAR_32  = 0xC505,
    NET_SET_OVL0_8  = 0xC640,
    NET_GET_OVL0_8  = 0xC641,
    NET_SET_OVL0_16 = 0xC642,
    NET_GET_OVL0_16 = 0xC643,
    NET_SET_OVL0_32 = 0xC644,
    NET_GET_OVL0_32 = 0xC645,
    NET_SET_OVL1_8  = 0xC650,
    NET_GET_OVL1_8  = 0xC651,
    NET_SET_OVL1_16 = 0xC652,
    NET_GET_OVL1_16 = 0xC653,
    NET_SET_OVL1_32 = 0xC654,
    NET_GET_OVL1_32 = 0xC655,
    NET_SET_OVL2_8  = 0xC660,
    NET_GET_OVL2_8  = 0xC661,
    NET_SET_OVL2_16 = 0xC662,
    NET_GET_OVL2_16 = 0xC663,
    NET_SET_OVL2_32 = 0xC664,
    NET_GET_OVL2_32 = 0xC665,
    NET_SET_OVL3_8  = 0xC670,
    NET_GET_OVL3_8  = 0xC671,
    NET_SET_OVL3_16 = 0xC672,
    NET_GET_OVL3_16 = 0xC673,
    NET_SET_OVL3_32 = 0xC674,
    NET_GET_OVL3_32 = 0xC675,
    NET_SET_RAW_8   = 0xC700,
    NET_GET_RAW_8   = 0xC701,
    NET_SET_RAW_16  = 0xC702,
    NET_GET_RAW_16  = 0xC703,
    NET_SET_RAW_32  = 0xC704,
    NET_GET_RAW_32  = 0xC705,
}networkCmdId_t;

typedef enum HostCmdId
{
    SYSMGR_SET_STATE = 0x8100,
    SYSMGR_GET_STATE = 0x8101,
    FLASHMGR_GET_LOCK = 0x8500,
    FLASHMGR_LOCK_STATUS = 0x8501,
    FLASHMGR_RELEASE_LOCK = 0x8502,
    FLASHMGR_READ = 0x8504,
    FLASHMGR_WRITE = 0x8505,
    FLASHMGR_ERASE_DEVICE = 0x8507,
    FLASHMGR_STATUS = 0x8509,
    FLASHMGR_CONFIG_DEVICE = 0x850A,
}hostCmdId_t;

typedef enum SystemState
{
    SYS_STATE_ENTER_CONFIG_CHANGE = 0x28,
    SYS_STATE_ENTER_STREAMING = 0x34,
    SYS_STATE_ENTER_SUSPEND = 0x40,
    SYS_STATE_ENTER_SOFT_STANDBY = 0x50,
    SYS_STATE_LEAVE_SOFT_STANDBY  = 0x55
}systemState_t;

typedef enum RegisterAddress
{
	/*�Ĵ���оƬ�汾*/
    CHIP_VERSION_REG = 0x0000,
	/*��λ���������*/
    RESET_AND_MISC_CONTROL =  0x001A,
    /*����Ĵ���*/
	COMMAND_REGISTER = 0x0040,
    /*�Ĵ�������ַ*/
	REG_ADDRESS_MAX = 0x7FFF,
	/*�������ú�IPV4��ַ����*/
    NET_CFG_IFC0_IPV4_ADDRESS = 0x8C28,
	NET_CFG_VID_SOURCE_IP_0 = 0x8CA4,
    CMD_HANDLER_PARAMS_POOL_0 = 0xFC00,
}registerAddress_t;

typedef enum DeviceErrorCode
{
    ERROR_ENOERR = 0,           // no error
    ERROR_ENOENT = 1,           // no such entity
    ERROR_EINTR  = 2,           // operation interrupted
    ERROR_EIO    = 3,           // I/O failure
    ERROR_E2BIG  = 4,           // too big
    ERROR_EBADF  = 5,           // bad file/handle
    ERROR_EAGAIN = 6,           // would-block, try again
    ERROR_ENOMEM = 7,           // not enough memory/resource
    ERROR_EACCES = 8,           // permission denied
    ERROR_EBUSY  = 9,           // entity busy, cannot support operation
    ERROR_EEXIST = 10,          // entity exists
    ERROR_ENODEV = 11,          // device not found
    ERROR_EINVAL = 12,          // invalid argument
    ERROR_ENOSPC = 13,          // no space/resource to complete
    ERROR_ERANGE = 14,          // parameter out-of-range
    ERROR_ENOSYS = 15,          // operation not supported
    ERROR_EALREADY = 16,        // already requested/exists
}deviceErrorCode_t;

typedef enum DeviceDriver
{
    SPI_READONLY = 0,   //mounted by default
    SPI_FLASH_RW = 1,
    SPI_EEPROM_RW = 2,
}deviceDeriver_t;

#define PARAM_LENGTH 4 //��ȡ�����в�������

#define MAX_READ_SIZE 0xFF //ÿ�ζ�ȡ����ֽ�

#define MAX_WRITE_SIZE 0x10 //ÿ��д������ֽ�

class IspTool {
public:
    /*�˴���װ�ɵ���,��������ɾ��*/
    IspTool(const IspTool&) = delete;
    
    /*�˴���װ�ɵ���,���������ظ�ֵɾ��*/
    IspTool& operator=(const IspTool&) = delete;

    /*��ȡ����*/
    static IspTool* getInstance();

    /*ɾ������*/
    static void deleteInstance();

    /*��ʼ������*/
	bool initNetwork(const std::string& ipaddr, const uint16_t& port, const std::string& protocol = "UDP");
	
    /*��������*/
    bool deinitNetwork();
	
    /*���õ������*/
    void enableDebugOutput(bool result);

    /*��ȡ����*/
	const std::string& getLastError();

    /*���Ĵ���*/
	int16_t readRegister(const uint16_t& address, const uint16_t& totalBytes, std::vector<uint8_t>& regDatas);
	
    /*���Ĵ���1���ֽ�*/
    int16_t readRegister1Bytes(const uint16_t& address, uint8_t& value);
	
    /*���Ĵ���2���ֽ�*/
    int16_t readRegister2Bytes(const uint16_t& address, uint16_t& value);

    /*д�Ĵ���*/
	int16_t writeRegister(const uint16_t& address, const std::vector<uint8_t>& regDatas);
	
    /*д�Ĵ���1���ֽ�*/
    int16_t writeRegister1Bytes(const uint16_t& address, const uint8_t& value);
	
    /*д�Ĵ���2���ֽ�*/
    int16_t writeRegister2Bytes(const uint16_t& address, const uint16_t& value);

    /*�ı�����*/
	int16_t changeConfiguration();
	
    /*����ϵͳ״̬*/
    int16_t setSystemStatus(const uint16_t& status);

    /*�Ƿ���Խ���*/
	int16_t canBeCommunication();

    /*��������*/
	int16_t resetSelf();

    /*����*/
	int16_t lock();

    /*����*/
	int16_t unlock();

    /*�����״̬*/
	int16_t getLockStatus();

    /*��Flash*/
	int16_t readFlash(const uint32_t& address, const uint8_t& length, uint8_t** dataArray);
	
    /*дFlash*/
    int16_t writeFlash(const uint32_t& address, const uint8_t& length, uint8_t* dataArray);

    /*���Flash״̬*/
	int16_t getFlashStatus(uint8_t** dataArray);

    /*���Flash״̬*/
	int16_t getFlashStatus();

    /*����Flash�豸*/
	int16_t configFlashDevice(uint8_t driverId, uint8_t addressWidth, uint32_t sizeOfDevice);

    /*����Flash�豸*/
	int16_t eraseFlashDevice();
	int16_t configFlashDeviceNvm(uint8_t byteWidth = 3, uint32_t sizeOfDevice = 16 * 1024, uint8_t driverId = SPI_READONLY);
	int16_t readFlashDeviceNvm(uint8_t** dataArray, int32_t numOfBytes, uint32_t address = 0, int* progress = nullptr);
	int16_t writeFlashDeviceNvm(const uint8_t* data, int32_t numOfBytes, uint32_t byteWidth = 3, uint32_t address = 0, int* progress = nullptr);
protected:
    /*���ô���*/
	void setLastError(const std::string& err);
	
    /*�Ĵ����Ƿ�æµ*/
    int16_t registerIsBusy();
    
    /*�Ĵ�������*/
	int16_t waitRegisterLeisure(unsigned int timeout = 10, unsigned int timeoutDelay = 10);
	
    /*����һ�����к�,��������*/
    uint32_t createSn();
    
    /*���ͽ�������*/
	int16_t sendRecvMsg(uint8_t* buffer, const uint16_t& bufferLen, uint16_t& paramLen);
	
    /*���ͽ������ݼ�ǿ,��ԭ�еĻ���������ѭ��*/
    int16_t sendRecvMsgEx(uint8_t* buffer, const uint16_t& bufferLen, uint16_t& paramLen, uint16_t loop = 100);
	
    /*У���*/
    uint16_t checkSum(uint8_t* buffer, uint32_t count);
private:
	IspTool();

	~IspTool();

	std::string m_lastError = "No Error";

    std::string m_ipaddr = "127.0.0.1";

	bool m_init = false;

	int m_delay = 1;

    static IspTool* m_self;
};

