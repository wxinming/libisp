#pragma once
#include "../isx016/isx016.h"

namespace isp {
	class LIBISP_DLL_EXPORT Isx019 : public Isx016
	{
	public:
		Isx019(std::shared_ptr<fbd::Base> device);
		~Isx019();
		bool initialize() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool saveAllToFlash();

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool eraseFlash(uint32_t address);

	protected:
		std::deque<uint32_t> jump_address_;
		uint8_t user_area_[4096] = { 0 };
	};
}
