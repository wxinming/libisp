#pragma once
#include "sensor/base.h"

// image sensor programming
namespace isp  {
	LIBISP_DLL_EXPORT std::shared_ptr<Base> autoReleaseNew(SensorType type, std::shared_ptr<fbd::Base> device);
}
