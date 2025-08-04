#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Asx340 : public Base
	{
	public:
		Asx340(std::shared_ptr<fbd::Base> device);
		~Asx340();
		bool initialize() override;
		bool lock() override;
		bool unlock() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool setXyOffset(int x, int y);
		bool getXyOffset(int& x, int& y);

	protected:
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		uint16_t getAddress(int cmdPool = 0) const;
		bool sendHostCmd(uint16_t cmd);
		bool reset();
		bool isBusy();
		bool query();
		bool initFlash();
		bool initEeprom();
		bool configI2c();
		bool configE2p();
	};
}
