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
	* @brief ����������ö��
	* @note ��������
	* 1.������_�洢����_�⴮��оƬ_��Ŀ����
	* 2.�洢����,Ŀǰ����FLASH��EEPROM
	* 3.NULL,����ʹ�ý⴮��оƬ
	* 4.UNIVERSAL,����ͨ��,�ܶ���Ŀ����һ�����
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
		//˼����
		SC220AT_EEPROM_9296A_UNIVERSAL,
		SC233AT_EEPROM_9296A_UNIVERSAL,
		//����MAXIM 96717F
		ISX019_FLASH_96717F_UNIVERSAL,
		//NS6603 �𷢿�
		SC220AT_EEPROM_NS6603_UNIVERSAL,
		SC233AT_EEPROM_NS6603_UNIVERSAL,
		SC121AT_EEPROM_NS6603_UNIVERSAL,
		//��̩΢
		SC233AT_EEPROM_M65Q68_UNIVERSAL,
		SC121AT_EEPROM_96706G_UNIVERSAL,
		SC233AT_EEPROM_TI954_UNIVERSAL,
		//��о(R-SEMI)
		SC121AT_EEPROM_RLC92402_UNIVERSAL,
		OX01E10_FLASH_RLC92402_UNIVERSAL,
		OX03J10_FLASH_9296A_UNIVERSAL,
	};

	/*
	* @brief �ļ�����
	*/
	enum class FileType {
		BIN_FILE,
		HEX_FILE,
	};

	/*
	* @brief ��¼����
	*/
	class LIBISP_DLL_EXPORT Base
	{
	public:
		/*
		* @brief ����
		*/
		Base(std::shared_ptr<fbd::Base> device);

		/*
		* @brief ������
		*/
		virtual ~Base();

		/*
		* @brief ��������ɾ��
		*/
		Base(const Base&) = delete;

		/*
		* @brief ��ֵ����ɾ��
		*/
		Base& operator=(const Base&) = delete;

		/*
		* @brief ��ȡ���մ���
		* @return std::string 
		*/
		std::string getLastError() const;

		/*
		* @brief ���ô���������
		* @param[in] sensorType ����������
		*/
		void setSensorType(SensorType sensorType);

		/*
		* @brief ��ȡ����������
		* @return ����������
		*/
		SensorType getSensorType() const;

		/*
		* @brief ������¼�ļ�����
		* @param[in] fileType �ļ�����
		* @return void
		*/
		void setFileType(FileType fileType);

		/*
		* @brief ��ȡ��¼�ļ�����
		* @return �ļ�����
		*/
		FileType getFileType() const;

	public:
		/*
		* @brief ��ʼ��
		* @return bool
		*/
		virtual bool initialize() = 0;

		/*
		* @brief ���ļ�
		* @param[out] data ��ȡ�����ݻ�����
		* @param[in] size ��ȡ��С
		* @param[out] percent ��ȡ����
		* @return bool
		*/
		virtual bool readFile(uint8_t* data, uint32_t size, int* percent) = 0;

		/*
		* @brief д�ļ�
		* @param[in] data д������ݻ�����
		* @param[in] size д��Ĵ�С
		* @param[out] percent д�����
		* @return bool
		*/
		virtual bool writeFile(const uint8_t* data, uint32_t size, int* percent) = 0;

		/*
		* @brief ��ȡ����
		* @param[in] address ��ʼ��ַ
		* @param[out] data ��ȡ�����ݻ�����
		* @param[in] size ��ȡ��С
		* @return bool
		*/
		virtual bool readFlash(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief д������
		* @param[in] address ��ʼ��ַ
		* @param[in] data д������ݻ�����
		* @param[in] size д���С
		* @return bool
		*/
		virtual bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief ��ȡ��ɲ���ֻ���洢��
		* @param[in] address ��ʼ��ַ
		* @param[out] data ��ȡ�����ݻ�����
		* @param[in] size ��ȡ��С
		* @return bool
		*/
		virtual bool readEeprom(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief д���ɲ���ֻ���洢��
		* @param[in] address ��ʼ��ַ
		* @param[in] data д������ݻ�����
		* @param[in] size д���С
		* @return bool
		*/
		virtual bool writeEeprom(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief ��ȡ�Ĵ���
		* @param[in] address ��ʼ��ַ
		* @param[out] data д������ݻ�����
		* @param[in] size д���С
		* @return bool
		*/
		virtual bool readRegister(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief д��Ĵ���
		* @param[in] address ��ʼ��ַ
		* @param[in] data д������ݻ�����
		* @param[in] size д���С
		* @return bool
		*/
		virtual bool writeRegister(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief ����
		* @return bool
		*/
		virtual bool lock();

		/*
		* @brief ����
		* @return bool
		*/
		virtual bool unlock();

		/*
		* @brief ����д����
		* @return bool
		*/
		virtual bool enableWp();

		/*
		* @brief ����д����
		* @return bool
		*/
		virtual bool disableWp();

	protected:
		/*
		* @brief ��ȡ����
		* @param[in] address ��ʼ��ַ
		* @param[out] data ��ȡ�����ݻ�����
		* @param[in] size ��ȡ��С
		* @return bool
		*/
		virtual bool readData(uint32_t address, uint8_t* data, uint16_t size);

		/*
		* @brief д������
		* @param[in] address ��ʼ��ַ
		* @param[in] data д������ݻ�����
		* @param[in] size д���С
		* @return bool
		*/
		virtual bool writeData(uint32_t address, const uint8_t* data, uint16_t size);

		/*
		* @brief �������մ���
		* @param[in] fmt ������Ϣ
		* @param[in] ... �ɱ����
		* @return void
		*/
		void setLastError(const char* fmt, ...);

		/*
		* @brief �������մ���
		* @param[in] msg ������Ϣ
		* @return void
		*/
		void setLastError(const std::string& msg);

		/*
		* @brief ��ȡ�ٷֱ�
		* @param[in] current ��ǰ
		* @param[in] total ����
		* @return int
		*/
		inline int getPercent(uint32_t current, uint32_t total) const
		{
			return static_cast<int>(100.0f - (static_cast<float>(total - current) / total) * 100);
		}

		/*
		* @brief ��������
		* @parma[in] buffer ���ݻ�����
		* @param[in] size ���ݻ�������С
		* @param[in] multiple ����
		* @return std::vector<uint32_t>
		*/
		std::deque<uint32_t> calculateSector(const uint8_t* buffer, uint32_t size, uint32_t multiple = 256, uint8_t value = 0xff) const;

		/*
		* @brief Ѱ�����ڵ�ַ
		* @param[in] address ҪѰ�ҵĵ�ַ
		* @param[in] multiple ����
		* @param[in] lessThan �Ƿ񰴽�������
		* @return �ɹ������ҵ��ĵ�ַ,ʧ��(uint32_t)-1δ�ҵ�
		*/
		uint32_t findNearAddress(uint32_t address, uint32_t multiple, bool lessThan = true) const;

		/*
		* @brief ����������Ĵ����ı�
		* @param[in] text �����ı�
		* @param[out] slave �ӵ�ַ
		* @param[out] reg �Ĵ���
		* @param[out] reglen �Ĵ�������
		* @param[out] data ����
		* @return void
		*/
		void parseRlgRegisterText(const std::string& text, std::vector<uint8_t>& slave, 
			std::vector<uint16_t>& reg, std::vector<uint8_t>& reglen,
			std::vector<uint8_t>& data);

		/*
		* @brief ������΢�ӼĴ����ı�
		* @param[in] text �����ı�
		* @param[out] slave �ӵ�ַ
		* @param[out] reg �Ĵ���
		* @param[out] data ����
		* @param[out] delay ��ʱ
		* @return void
		*/
		void parseAviewRegisterText(const std::string& text, std::vector<uint8_t>& slave,
			std::vector<uint16_t>& reg, std::vector<uint8_t>& data, std::vector<uint16_t>& delay);

	protected:
		//����������
		SensorType sensor_type_ = SensorType::UNKNOWN;

		//�ļ�����
		FileType file_type_ = FileType::BIN_FILE;

		//�豸����
		std::shared_ptr<fbd::Base> device_;

		//���մ���
		std::string last_error_ = "δ֪����";
	};
}