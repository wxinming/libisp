#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Ox03J10 : public Base
	{
	public:
		Ox03J10(std::shared_ptr<fbd::Base> device);
		~Ox03J10();
		bool initialize() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool sendHostCmd(const uint8_t* data, uint32_t size, uint32_t addr);
		bool initChip();
		bool uninitChip();
		bool getStatus();

	private:
		static const int OVX1E_DELAY = 50;
	};
}

