#pragma once
#include "../isx019/isx019.h"

namespace isp {
	class LIBISP_DLL_EXPORT Isx019A : public Isx019
	{
	public:
		Isx019A(std::shared_ptr<fbd::Base> device);
		~Isx019A();
		bool initialize() override;
		bool lock() override;
		bool unlock() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const std::initializer_list<uint8_t>& data);
		bool registerAllDataWriteToFlash();
		bool eraseFlash(uint32_t address);
		bool changeToFlashMode();

	private:
		uint32_t register_ = 0xffff00;
	};
}
