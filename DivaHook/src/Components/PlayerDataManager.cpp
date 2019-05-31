#include "PlayerDataManager.h"
#include <string>
#include "../MainModule.h"
#include "../Constants.h"
#include "../Input/Keyboard/Keyboard.h"
#include "../FileSystem/ConfigFile.h"
#include "../Constants.h"

const std::string PLAYER_DATA_FILE_NAME = "playerdata.ini";

namespace DivaHook::Components
{
	PlayerDataManager::PlayerDataManager()
	{
	}

	PlayerDataManager::~PlayerDataManager()
	{
		if (customPlayerData != nullptr)
			delete customPlayerData;
	}

	const char* PlayerDataManager::GetDisplayName()
	{
		return "player_data_manager";
	}

	void PlayerDataManager::Initialize(ComponentsManager*)
	{
		playerData = (PlayerData*)PLAYER_DATA_ADDRESS;

		ApplyPatch();
		LoadConfig();
		ApplyCustomData();
	}

	void PlayerDataManager::Update()
	{
		ApplyCustomData();

		if (false && Input::Keyboard::GetInstance()->IsTapped(VK_F12))
		{
			printf("PlayerDataManager::Update(): Loading config...\n");
			LoadConfig();
		}
	}

	void PlayerDataManager::ApplyPatch()
	{
		DWORD oldProtect;
		VirtualProtect((void*)SET_DEFAULT_PLAYER_DATA_ADDRESS, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		{
			// prevent the PlayerData from being reset
			*(uint8_t*)(SET_DEFAULT_PLAYER_DATA_ADDRESS) = RET_OPCODE;
		}
		VirtualProtect((void*)SET_DEFAULT_PLAYER_DATA_ADDRESS, sizeof(uint8_t), oldProtect, &oldProtect);
		
		// allow player to select the module and extra item
		VirtualProtect((void*)MODSELECTOR_CHECK_FUNCTION_ERRRET_ADDRESS, sizeof(byte) * 2, PAGE_EXECUTE_READWRITE, &oldProtect);
		{
			*(byte*)(MODSELECTOR_CHECK_FUNCTION_ERRRET_ADDRESS) = 0xB0; // xor al,al -> ld al,1
			*(byte*)(MODSELECTOR_CHECK_FUNCTION_ERRRET_ADDRESS + 0x1) = 0x01;
		}
		VirtualProtect((void*)MODSELECTOR_CHECK_FUNCTION_ERRRET_ADDRESS, sizeof(byte) * 2, oldProtect, &oldProtect);

		// fix annoying behavior of closing after changing module or item (don't yet know the reason, maybe NW/Card checks)
		{
			VirtualProtect((void*)MODSELECTOR_CLOSE_AFTER_MODULE, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(uint8_t*)(MODSELECTOR_CLOSE_AFTER_MODULE) = JNE_OPCODE;
			}
			VirtualProtect((void*)MODSELECTOR_CLOSE_AFTER_MODULE, sizeof(uint8_t), oldProtect, &oldProtect);
			VirtualProtect((void*)MODSELECTOR_CLOSE_AFTER_CUSTOMIZE, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(uint8_t*)(MODSELECTOR_CLOSE_AFTER_CUSTOMIZE) = JNE_OPCODE;
			}
			VirtualProtect((void*)MODSELECTOR_CLOSE_AFTER_CUSTOMIZE, sizeof(uint8_t), oldProtect, &oldProtect);
		}
		
	}

	void PlayerDataManager::LoadConfig()
	{
		if (playerData == nullptr)
			return;

		FileSystem::ConfigFile config(MainModule::GetModuleDirectory(), PLAYER_DATA_FILE_NAME);

		if (!config.OpenRead())
			return;

		if (customPlayerData != nullptr)
			delete customPlayerData;

		customPlayerData = new CustomPlayerData();
		config.TryGetValue("player_name", &customPlayerData->PlayerName);

		customPlayerData->LevelPlateId = config.GetIntegerValue("level_plate_id");
		customPlayerData->SkinEquip = config.GetIntegerValue("skin_equip");
		customPlayerData->BtnSeEquip = config.GetIntegerValue("btn_se_equip");
		customPlayerData->SlideSeEquip = config.GetIntegerValue("slide_se_equip");
		customPlayerData->ChainslideSeEquip = config.GetIntegerValue("chainslide_se_equip");
		
		/*
		playerData->use_card = config.GetIntegerValue("use_card");

		if (playerData->use_card == 0)
		{
			DWORD oldProtect;
			//enable module selection without card
			VirtualProtect((void*)0x00000001405C5133, sizeof(byte) * 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(byte*)(0x00000001405C5133) = 0x74;
			}
			VirtualProtect((void*)0x00000001405C5133, sizeof(byte) * 1, oldProtect, &oldProtect);

			VirtualProtect((void*)0x00000001405BC8E7, sizeof(byte) * 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(byte*)(0x00000001405BC8E7) = 0x74;
			}
			VirtualProtect((void*)0x00000001405BC8E7, sizeof(byte) * 1, oldProtect, &oldProtect);
		}
		else {
			DWORD oldProtect;
			//disable module selection with card
			VirtualProtect((void*)0x00000001405C5133, sizeof(byte) * 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(byte*)(0x00000001405C5133) = 0x75;
			}
			VirtualProtect((void*)0x00000001405C5133, sizeof(byte) * 1, oldProtect, &oldProtect);

			VirtualProtect((void*)0x00000001405BC8E7, sizeof(byte) * 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			{
				*(byte*)(0x00000001405BC8E7) = 0x75;
			}
			VirtualProtect((void*)0x00000001405BC8E7, sizeof(byte) * 1, oldProtect, &oldProtect);
		}
		*/
	}

	void PlayerDataManager::ApplyCustomData()
	{
		// don't want to overwrite the default values
		auto setIfNotEqual = [](int *target, int value, int comparison)
		{
			if (value != comparison)
				*target = value;
		};

		setIfNotEqual(&playerData->level_plate_id, customPlayerData->LevelPlateId, 0);
		setIfNotEqual(&playerData->skin_equip, customPlayerData->SkinEquip, 0);
		setIfNotEqual(&playerData->btn_se_equip, customPlayerData->BtnSeEquip, -1);
		setIfNotEqual(&playerData->slide_se_equip, customPlayerData->SlideSeEquip, -1);
		setIfNotEqual(&playerData->chainslide_se_equip, customPlayerData->ChainslideSeEquip, -1);

		playerData->use_card = 1; // required to allow for module selection

		memset((void *)MODULE_TABLE_START, 0xFF, 128);
		memset((void*)ITEM_TABLE_START, 0xFF, 128);

		/*
		if (playerData->use_card == 0)
		{
			//there probably a jn/jne to patch instead but im too lazy rn
			*(int*)0x00000001411A8A28 = *(int*)0x00000001411A8A10;
			*(int*)(0x00000001411A8A28 + 4) = *(int*)(0x00000001411A8A28 + 4);
			*(int*)(0x00000001411A8A28 + 8) = *(int*)(0x00000001411A8A28 + 8);
			*(int*)(0x00000001411A8A28 + 12) = *(int*)(0x00000001411A8A28 + 12);
			*(int*)(0x00000001411A8A28 + 16) = *(int*)(0x00000001411A8A28 + 16);
			*(int*)(0x00000001411A8A28 + 18) = *(int*)(0x00000001411A8A28 + 18);
		}
		*/

		memset((void *)MODULE_TABLE_START, 0xFF, 128);

		memset((void*)ITEM_TABLE_START, 0xFF, 128);

		if (customPlayerData->PlayerName != nullptr)
		{
			playerData->field_DC = 0x10;
			playerData->player_name = (char*)customPlayerData->PlayerName->c_str();
		}
	}
}
