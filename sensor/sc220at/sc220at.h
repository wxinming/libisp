#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Sc220At : public Base {
	public:
		Sc220At(std::shared_ptr<fbd::Base> device);
		~Sc220At();
		bool initialize() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readEeprom(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeEeprom(uint32_t address, const uint8_t* data, uint16_t size) override;

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		virtual bool openBridge();
		virtual bool closeBridge();
		bool enableWp() override;
		bool disableWp() override;

	protected:
		uint32_t read_size_ = 0;
	};
}
