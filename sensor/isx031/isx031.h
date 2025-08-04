#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Isx031 : public Base {
	public:
		Isx031(std::shared_ptr<fbd::Base> device);
		~Isx031();
		bool initialize() override;
		bool lock() override;
		bool unlock() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;
		bool readFlash(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeFlash(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool readRegister(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeRegister(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool saveAllToFlash();

	protected:
		bool readData(uint32_t address, uint8_t* data, uint16_t size) override;
		bool writeData(uint32_t address, const uint8_t* data, uint16_t size) override;
		bool flashAccess();
		bool eraseFlash(uint32_t address);
		bool enableWp() override;
		bool disableWp() override;

	private:
		std::deque<uint32_t> jump_address_;
	};
}
