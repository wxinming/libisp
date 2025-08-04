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
		* @brief 打包帧
		* @param[in] frame 帧结构体
		* @param[out] data 数据缓存区
		* @return bool
		*/
		bool packFrame(SonyCmdFrame frame, uint8_t* data);

		/*
		* @brief 解包帧
		* @param[out] frame 帧结构体
		* @param[in] data 数据缓存区
		* @param[in] size 数据缓存区大小
		* @return bool
		*/
		bool unpackFrame(SonyCmdFrame* frame, const uint8_t* data, uint8_t size);

		/*
		* @brief 读取帧
		* @param[out] frame 命令帧
		* @param[in] size 读取大小
		* @return bool
		*/
		bool readFrame(SonyCmdFrame* frame, uint8_t size);

		/*
		* @brief 写入帧
		* @param[in] frame 命令帧
		* @return bool
		*/
		bool writeFrame(SonyCmdFrame frame);

		/*
		* @brief 读寄存器
		* @param[in] category 寄存器种类
		* @param[in] addrOffset 偏移地址
		* @param[out] data 读取的缓存区
		* @param[in] dataSize 读取的缓存区大小
		* @return bool
		*/
		bool readRegister(uint8_t category, uint16_t addrOffset, uint8_t* data, uint8_t dataSize);

		/*
		* @brief 写寄存器
		* @param[in] category 寄存器种类
		* @param[in] addrOffset 偏移地址
		* @param[in] data 写入的缓存区
		* @param[in] dataSize 写入的缓存区大小
		* @return bool
		*/
		bool writeRegister(uint8_t category, uint16_t addrOffset, const uint8_t* data, uint8_t dataSize);

		/*
		* @brief 临时初始化
		* @note 这个函数用途未知
		* @return bool
		*/
		bool tempInit();

		/*
		* @brief 临时未初始化
		* @note 这个函数用途未知
		* @return bool
		*/
		bool tempUnint();

		/*
		* @brief 设置错误信息
		* @param 错误码
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
