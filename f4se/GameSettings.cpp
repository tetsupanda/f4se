#include "f4se/GameSettings.h"
#include "f4se/GameTypes.h"

// 
RelocPtr <INISettingCollection *> g_iniSettings(0x03194198);
// 
RelocPtr <INIPrefSettingCollection*> g_iniPrefSettings(0x030EE6D0);
// 
RelocPtr <RegSettingCollection *> g_regSettings(0x03623AC0);
// 
RelocPtr <GameSettingCollection *> g_gameSettings(0x02E753E0);

UInt32 Setting::GetType(void) const
{
	if(!name || !name[0]) return kType_Unknown;

	switch(name[0])
	{
		case 'b':	return kType_Bool;
		case 'c':	return kType_Unknown;
		case 'h':	return kType_Unknown;
		case 'i':	return kType_Integer;
		case 'u':	return kType_Unknown;
		case 'f':	return kType_Float;
		case 'S':	return kType_String;	// dynamically allocated string
		case 's':	return kType_String;	// statically allocated string
		case 'r':	return kType_ID6;
		case 'a':	return kType_ID;
	}

	return kType_Unknown;
}

bool Setting::GetDouble(double * out) const
{
	switch(GetType())
	{
	case kType_Integer:	*out = data.s32; break;
	case kType_Float:	*out = data.f32; break;
	case kType_String:	return false;
	case kType_Bool:	*out = data.u8 ? 1 : 0; break;
	case kType_ID6:		*out = data.u32 >> 8; break;
	case kType_ID:		*out = data.u32; break;
	default:			return false;
	case kType_Unknown:	return false;
	}

	return true;
}

bool Setting::SetDouble(double value)
{
	switch(GetType())
	{
	case kType_Integer:	data.s32 = value; break;
	case kType_Float:	data.f32 = value; break;
	case kType_String:	return false;
	case kType_Bool:	data.u8 = value ? 1 : 0; break;
	case kType_ID6:		data.u32 = ((UInt32)value) << 8; break;
	case kType_ID:		data.u32 = value; break;
	default:			return false;
	case kType_Unknown:	return false;
	}

	return true;
}

char * FormHeap_Strdup(const char * src)
{
	UInt32	len = strlen(src) + 1;
	char	* result = (char *)Heap_Allocate(len);
	memcpy(result, src, len);

	return result;
}

bool Setting::SetString(const char * value)
{
	if(GetType() == kType_String)
	{
		if(name[0] == 'S')
			Heap_Free(data.s);

		data.s = FormHeap_Strdup(value);

		// mark string as dynamically allocated
		if(name[0] != 'S')
		{
			name = FormHeap_Strdup(name);
			name[0] = 'S';
		}

		return true;
	}
	else
	{
		return false;
	}
}

Setting * GetINISetting(const char * name)
{
	Setting	* setting = (*g_iniSettings)->Get(name);
	if(!setting)
		setting = (*g_iniPrefSettings)->Get(name);

	return setting;
}

Setting * GetGameSetting(const char * name)
{
	Setting * setting = nullptr;
	Setting ** setting2 = &setting;
	BSAutoFixedString searchName(name);
	(*g_gameSettings)->Get(&searchName, &setting2);
	return setting;
}

Setting * SettingCollectionList::Get(const char * name)
{
	Node * node = data;
	do
	{
		Setting * setting = node->data;
		if(setting) {
			BSAutoFixedString searchName(name);
			BSAutoFixedString settingName(setting->name);
			if(searchName == settingName) {
				return setting;
			}
		}

		node = node->next;
	} while(node);

	return nullptr;
}
