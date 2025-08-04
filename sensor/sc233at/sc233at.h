#pragma once
#include "../sc220at/sc220at.h"

namespace isp {
	class LIBISP_DLL_EXPORT Sc233At : public Sc220At {
	public:
		Sc233At(std::shared_ptr<fbd::Base> device);
		~Sc233At();

	protected:
		bool openBridge() override;
		bool closeBridge() override;
		bool enableWp() override;
		bool disableWp() override;
	};
}
