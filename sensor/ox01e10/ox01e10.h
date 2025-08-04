#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Ox01E10 : public Base
	{
	public:
		Ox01E10(std::shared_ptr<fbd::Base> device);
		~Ox01E10();
		bool initialize() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool sendHostCmd(const uint8_t* data, uint32_t size);
		bool initChip();
		bool uninitChip();
		bool writeInitData();
		bool getStatus();

	private:
		static const int OX01E10_DELAY = 50;
	};
}
