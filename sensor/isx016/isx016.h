#pragma once
#include "../base.h"
#define SONY_CMD_MAX_CNT 16
#define SONY_CMD_FRAME_MAX_LEN 137

namespace isp {
	struct LIBISP_DLL_EXPORT SonyCmdFrame {
		uint8_t total_byte;
		uint8_t cmd_cnt;
		struct 
		{
			uint8_t total_cmd_byte;
			uint8_t cmd_and_status_code;
			uint8_t data[128];
		} cmd[SONY_CMD_MAX_CNT];
		uint8_t check_sum;
	};

	enum SonyCmdList {
		CMD_LOCK_UNLOCK = 0x00,
		CMD_READ_REG = 0x01,
		CMD_WRITE_REG = 0x02,
		CMD_READ_EEP = 0x03,
		CMD_WRITE_EEP = 0x04,
		CMD_ALL_SAVE_REG_TO_EEP_019 = 0x05,
		CMD_WRITE_CHECK_SUM_016 = 0x08,
		CMD_ERASE_EEP_019 = 0x08,
	};

	enum SonyStatusCode {
		SC016_RATE_TOO_FAST = 0x00,
		SC016_SUCCESS = 0x01,
		SC016_ERR_EFFI_NUM = 0xF0,
		SC016_ERR_CMD_NUM_NOT_EXIST = 0xF1,
		SC016_ERR_CAT_NUM = 0xF2,
		SC016_ERR_ADDR_OFFSET = 0xF3,
		SC016_ERR_ACCESS = 0xF4,
		SC016_ERR_CMD_NUM_NOT_MATCH = 0xF5,
		SC016_ERR_CHECK_SUM = 0xF6,
		SC016_ERR_TOTAL_BYTE_NUM = 0xF7,
		SC016_ERR_EEP_ACCESS = 0xFA,
		SC016_ERR_COMMUNICATION = 0xFC,
	};

	class LIBISP_DLL_EXPORT Isx016 : public Base
	{
	public:
		Isx016(std::shared_ptr<fbd::Base> device);
		~Isx016();
		bool initialize() override;
		bool lock() override;
		bool unlock() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readEeprom(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeEeprom(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool readRegister(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeRegister(uint32_t address, const uint8_t* data, uint16_t size) override;

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;

		/*
		* @brief ���֡
		* @param[in] frame ֡�ṹ��
		* @param[out] data ���ݻ�����
		* @return bool
		*/
		bool packFrame(SonyCmdFrame frame, uint8_t* data);

		/*
		* @brief ���֡
		* @param[out] frame ֡�ṹ��
		* @param[in] data ���ݻ�����
		* @param[in] size ���ݻ�������С
		* @return bool
		*/
		bool unpackFrame(SonyCmdFrame* frame, const uint8_t* data, uint8_t size);

		/*
		* @brief ��ȡ֡
		* @param[out] frame ����֡
		* @param[in] size ��ȡ��С
		* @return bool
		*/
		bool readFrame(SonyCmdFrame* frame, uint8_t size);

		/*
		* @brief д��֡
		* @param[in] frame ����֡
		* @return bool
		*/
		bool writeFrame(SonyCmdFrame frame);

		/*
		* @brief ���Ĵ���
		* @param[in] category �Ĵ�������
		* @param[in] addrOffset ƫ�Ƶ�ַ
		* @param[out] data ��ȡ�Ļ�����
		* @param[in] dataSize ��ȡ�Ļ�������С
		* @return bool
		*/
		bool readRegister(uint8_t category, uint16_t addrOffset, uint8_t* data, uint8_t dataSize);

		/*
		* @brief д�Ĵ���
		* @param[in] category �Ĵ�������
		* @param[in] addrOffset ƫ�Ƶ�ַ
		* @param[in] data д��Ļ�����
		* @param[in] dataSize д��Ļ�������С
		* @return bool
		*/
		bool writeRegister(uint8_t category, uint16_t addrOffset, const uint8_t* data, uint8_t dataSize);

		/*
		* @brief ��ʱ��ʼ��
		* @note ���������;δ֪
		* @return bool
		*/
		bool tempInit();

		/*
		* @brief ��ʱδ��ʼ��
		* @note ���������;δ֪
		* @return bool
		*/
		bool tempUnint();

		/*
		* @brief ���ô�����Ϣ
		* @param ������
		* @return void
		*/
		void setErrorInfo(uint8_t code);

		int cmd_addr_len_ = 2;

	private:
		bool protect();
		bool unprotect();
		bool convertToAddress();
		bool changeFlashStatus();
	};
}
