#include "libisp.h"
#include "sensor/asx340/asx340.h"
#include "sensor/isx016/isx016.h"
#include "sensor/isx019/isx019.h"
#include "sensor/isx019a/isx019a.h"
#include "sensor/isx031/isx031.h"
#include "sensor/ov7958/ov7958.h"
#include "sensor/ox01e10/ox01e10.h"
#include "sensor/ox03j10/ox03j10.h"
#include "sensor/sc121at/sc121at.h"
#include "sensor/sc220at/sc220at.h"
#include "sensor/sc233at/sc233at.h"

std::shared_ptr<isp::Base> isp::autoReleaseNew(SensorType type, std::shared_ptr<fbd::Base> device)
{
	std::shared_ptr<isp::Base> base;
	switch (type)
	{
	case isp::SensorType::UNKNOWN:
		break;
	case isp::SensorType::ASX340_FLASH_NULL_UNIVERSAL:
	case isp::SensorType::ASX340_EEPROM_NULL_UNIVERSAL:
	case isp::SensorType::ASX340_EEPROM_UB934Q_UNIVERSAL:
		base = std::make_shared<isp::Asx340>(device);
		break;
	case isp::SensorType::ISX016_EEPROM_96706G_UNIVERSAL:
		base = std::make_shared<isp::Isx016>(device);
		break;
	case isp::SensorType::ISX019_FLASH_96706G_UNIVERSAL:
	case isp::SensorType::ISX019_FLASH_UB934Q_UNIVERSAL:
	case isp::SensorType::ISX019_FLASH_NULL_UNIVERSAL:
	case isp::SensorType::ISX019_FLASH_96717F_UNIVERSAL:
		base = std::make_shared<isp::Isx019>(device);
		break;
	case isp::SensorType::OV7958_FLASH_UB934Q_UNIVERSAL:
		base = std::make_shared<isp::Ov7958>(device);
		break;
	case isp::SensorType::OX01E10_FLASH_96706G_UNIVERSAL:
	case isp::SensorType::OX01E10_FLASH_UB934Q_UNIVERSAL:
	case isp::SensorType::OX01E10_FLASH_RLC92402_UNIVERSAL:
		base = std::make_shared<isp::Ox01E10>(device);
		break;
	case isp::SensorType::ISX019A_FLASH_96706G_UNIVERSAL:
	case isp::SensorType::ISX019A_FLASH_UB934Q_UNIVERSAL:
		base = std::make_shared<isp::Isx019A>(device);
		break;
	case isp::SensorType::ISX031_FLASH_9296A_UNIVERSAL:
		base = std::make_shared<isp::Isx031>(device);
		break;
	case isp::SensorType::SC220AT_EEPROM_9296A_UNIVERSAL:
	case isp::SensorType::SC220AT_EEPROM_NS6603_UNIVERSAL:
		base = std::make_shared<isp::Sc220At>(device);
		break;
	case isp::SensorType::SC233AT_EEPROM_9296A_UNIVERSAL:
	case isp::SensorType::SC233AT_EEPROM_NS6603_UNIVERSAL:
	case isp::SensorType::SC233AT_EEPROM_M65Q68_UNIVERSAL:
	case isp::SensorType::SC233AT_EEPROM_TI954_UNIVERSAL:
		base = std::make_shared<isp::Sc233At>(device);
		break;
	case isp::SensorType::SC121AT_EEPROM_NS6603_UNIVERSAL:
	case isp::SensorType::SC121AT_EEPROM_96706G_UNIVERSAL:
	case isp::SensorType::SC121AT_EEPROM_RLC92402_UNIVERSAL:
		base = std::make_shared<isp::Sc121At>(device);
		break;
	case isp::SensorType::OX03J10_FLASH_9296A_UNIVERSAL:
		base = std::make_shared<isp::Ox03J10>(device);
		break;
	default:
		break;
	}

	if (base) {
		base->setSensorType(type);
	}
	return base;
}
