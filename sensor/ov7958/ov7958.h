#pragma once
#include "../base.h"

namespace isp {
	class LIBISP_DLL_EXPORT Ov7958 : public Base {
	public:
		Ov7958(std::shared_ptr<fbd::Base> device);
		~Ov7958();
		bool initialize() override;
		bool readFile(uint8_t* data, uint32_t size, int* percent) override;
		bool writeFile(const uint8_t* data, uint32_t size, int* percent) override;

	protected:
		bool earseAll();
		bool getStatus();
		bool setProtect(bool protect);
	};
}
