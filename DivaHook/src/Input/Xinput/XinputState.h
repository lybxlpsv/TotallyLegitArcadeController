#pragma once
#include <windows.h>

namespace DivaHook::Input
{
	struct XinputState
	{
		bool KeyStates[0x8F];
	
		bool IsDown(BYTE keycode);
	};
}

