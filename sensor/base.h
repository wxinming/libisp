#pragma once
#include <vector>
#include <deque>
#include <algorithm>
#include <sstream>
#include "../misc/exports.h"
#include <libfbd/libfbd.h>
#pragma comment(lib, "libfbd.lib")

#define ENABLE_NEW_SLAVE_ADDR_96706 1
#define ENABLE_NEW_SLAVE_ADDR_914 1

#if ENABLE_NEW_SLAVE_ADDR_96706
#define SLAVE_ADDR_96706 (0x9C)
#else
#define SLAVE_ADDR_96706 (0x90)
#endif

#define SLAVE_ADDR_96701 (0x80)

#define SLAVE_ADDR_934 (0x60)

#if ENABLE_NEW_SLAVE_ADDR_914
#define SLAVE_ADDR_914 (0xD0)
#else
#define SLAVE_ADDR_914 (0xC0)
#endif

#define SLAVE_ADDR_9296A (0x90)

namespace isp {
	/*
	* @brief 传感器类型枚举
	* @note 命名规则
	* 1.传感器_存储介质_解串器芯片_项目名称
	* 2.存储介质,目前包括FLASH和EEPROM
	* 3.NULL,代表不使用解串器芯片
	* 4.UNIVERSAL,代表通用,很多项目共用一个类库
	*/
	enum class SensorType {
		UNKNOWN,
		ASX340_FLASH_NULL_UNIVERSAL,
		ASX340_EEPROM_NULL_UNIVERSAL,
		ASX340_EEPROM_UB934Q_UNIVERSAL,
		ISX016_EEPROM_96706G_UNIVERSAL,
		ISX019_FLASH_96706G_UNIVERSAL,
		ISX019_FLASH_UB934Q_UNIVERSAL,
		ISX019_FLASH_NULL_UNIVERSAL,
		OV7958_FLASH_UB934Q_UNIVERSAL,
		OX01E10_FLASH_96706G_UNIVERSAL,
		OX01E10_FLASH_UB934Q_UNIVERSAL,
		ISX019A_FLASH_96706G_UNIVERSAL,
		ISX019A_FLASH_UB934Q_UNIVERSAL,
		ISX031_FLASH_9296A_UNIVERSAL,
		//思特威
		SC220AT_EEPROM_9296A_UNIVERSAL,
		SC233AT_EEPROM_9296A_UNIVERSAL,
		//美信MAXIM 96717F
		ISX019_FLASH_96717F_UNIVERSAL,
		//NS6603 瑞发科
		SC220AT_EEPROM_NS6603_UNIVERSAL,
		SC233AT_EEPROM_NS6603_UNIVERSAL,
		SC121AT_EEPROM_NS6603_UNIVERSAL,
		//锐泰微
		SC233AT_EEPROM_M65Q68_UNIVERSAL,
		SC121AT_EEPROM_96706G_UNIVERSAL,
		SC233AT_EEPROM_TI954_UNIVERSAL,
		//仁芯(R-SEMI)
		SC121AT_EEPROM_RLC92402_UNIVERSAL,
		OX01E10_FLASH_RLC92402_UNIVERSAL,
		OX03J10_FLASH_9296A_UNIVERSAL,
	};

	/*
	* @brief 文件类型
	*/
	enum class FileType {
		BIN_FILE,
		HEX_FILE,
	};

	/*
	* @brief 烧录基类
	*/
	class LIBISP_DLL_EXPORT Base
	{
	public:
		/*
		* @brief 构造
		*/
		Base(std::shared_ptr<fbd::Base> device);

		/*
		* @brief 虚析构
		*/
		virtual ~Base();

		/*
		* @brief 拷贝构造删除
		*/
		Base(const Base&) = delete;

		/*
		* @brief 赋值构造删除
		*/
		Base& operator=(const Base&) = delete;

		/*
		* @brief 获取最终错误
		* @return std::string 
		*/
		std::string getLastError() const;

		/*
		* @brief 设置传感器类型
		* @param[in] sensorType 传感器类型
		*/
		void setSensorType(SensorType sensorType);

		/*
		* @brief 获取传感器类型
		* @return 传感器类型
		*/
		SensorType getSensorType() const;

		/*
		* @brief 设置烧录文件类型
		* @param[in] fileType 文件类型
		* @return void
		*/
		void setFileType(FileType fileType);

		/*
		* @brief 获取烧录文件类型
		* @return 文件类型
		*/
		FileType getFileType() const;

	public:
		/*
		* @brief 初始化
		* @return bool
		*/
		virtual bool initialize() = 0;

		/*
		* @brief 读文件
		* @param[out] data 读取的数据缓存区
		* @param[in] size 读取大小
		* @param[out] percent 读取进度
		* @return bool
		*/
		virtual bool readFile(uint8_t* data, uint32_t size, int* percent) = 0;

		/*
		* @brief 写文件
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入的大小
		* @param[out] percent 写入进度
		* @return bool
		*/
		virtual bool writeFile(const uint8_t* data, uint32_t size, int* percent) = 0;

		/*
		* @brief 读取闪存
		* @param[in] address 起始地址
		* @param[out] data 读取的数据缓存区
		* @param[in] size 读取大小
		* @return bool
		*/
		virtual bool readFlash(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief 写入闪存
		* @param[in] address 起始地址
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入大小
		* @return bool
		*/
		virtual bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief 读取电可擦除只读存储器
		* @param[in] address 起始地址
		* @param[out] data 读取的数据缓存区
		* @param[in] size 读取大小
		* @return bool
		*/
		virtual bool readEeprom(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief 写入电可擦除只读存储器
		* @param[in] address 起始地址
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入大小
		* @return bool
		*/
		virtual bool writeEeprom(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief 读取寄存器
		* @param[in] address 起始地址
		* @param[out] data 写入的数据缓存区
		* @param[in] size 写入大小
		* @return bool
		*/
		virtual bool readRegister(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief 写入寄存器
		* @param[in] address 起始地址
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入大小
		* @return bool
		*/
		virtual bool writeRegister(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief 上锁
		* @return bool
		*/
		virtual bool lock();

		/*
		* @brief 解锁
		* @return bool
		*/
		virtual bool unlock();

		/*
		* @brief 启用写保护
		* @return bool
		*/
		virtual bool enableWp();

		/*
		* @brief 禁用写保护
		* @return bool
		*/
		virtual bool disableWp();

	protected:
		/*
		* @brief 读取数据
		* @param[in] address 起始地址
		* @param[out] data 读取的数据缓存区
		* @param[in] size 读取大小
		* @return bool
		*/
		virtual bool readData(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief 写入数据
		* @param[in] address 起始地址
		* @param[in] data 写入的数据缓存区
		* @param[in] size 写入大小
		* @return bool
		*/
		virtual bool writeData(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief 设置最终错误
		* @param[in] fmt 错误信息
		* @param[in] ... 可变参数
		* @return void
		*/
		void setLastError(const char* fmt, ...);

		/*
		* @brief 设置最终错误
		* @param[in] msg 错误消息
		* @return void
		*/
		void setLastError(const std::string& msg);

		/*
		* @brief 获取百分比
		* @param[in] current 当前
		* @param[in] total 总数
		* @return int
		*/
		inline int getPercent(uint32_t current, uint32_t total) const
		{
			return static_cast<int>(100.0f - (static_cast<float>(total - current) / total) * 100);
		}

		/*
		* @brief 计算扇区
		* @parma[in] buffer 数据缓存区
		* @param[in] size 数据缓存区大小
		* @param[in] multiple 倍数
		* @return std::vector<uint32_t>
		*/
		std::deque<uint32_t> calculateSector(const uint8_t* buffer, uint32_t size, uint32_t multiple = 256, uint8_t value = 0xff) const;

		/*
		* @brief 寻找相邻地址
		* @param[in] address 要寻找的地址
		* @param[in] multiple 倍数
		* @param[in] lessThan 是否按降序排列
		* @return 成功返回找到的地址,失败(uint32_t)-1未找到
		*/
		uint32_t findNearAddress(uint32_t address, uint32_t multiple, bool lessThan = true) const;

		/*
		* @brief 解析软龙格寄存器文本
		* @param[in] text 输入文本
		* @param[out] slave 从地址
		* @param[out] reg 寄存器
		* @param[out] reglen 寄存器长度
		* @param[out] data 数据
		* @return void
		*/
		void parseRlgRegisterText(const std::string& text, std::vector<uint8_t>& slave, 
			std::vector<uint16_t>& reg, std::vector<uint8_t>& reglen,
			std::vector<uint8_t>& data);

		/*
		* @brief 解析艾微视寄存器文本
		* @param[in] text 输入文本
		* @param[out] slave 从地址
		* @param[out] reg 寄存器
		* @param[out] data 数据
		* @param[out] delay 延时
		* @return void
		*/
		void parseAviewRegisterText(const std::string& text, std::vector<uint8_t>& slave,
			std::vector<uint16_t>& reg, std::vector<uint8_t>& data, std::vector<uint16_t>& delay);

	protected:
		//传感器类型
		SensorType sensor_type_ = SensorType::UNKNOWN;

		//文件类型
		FileType file_type_ = FileType::BIN_FILE;

		//设备基类
		std::shared_ptr<fbd::Base> device_;

		//最终错误
		std::string last_error_ = "未知错误";
	};
}