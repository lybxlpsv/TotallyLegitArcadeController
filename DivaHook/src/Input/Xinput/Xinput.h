#pragma once
#include "../IInputDevice.h"
#include <xinput.h>
#include "../../Utilities/Stopwatch.h"
#include "XinputState.h"

using Stopwatch = DivaHook::Utilities::Stopwatch;

namespace DivaHook::Input
{
	class Xinput : public IInputDevice
	{
		const float DOUBLE_TAP_THRESHOLD = 200.0f;
		const float INTERVAL_TAP_DELAY_THRESHOLD = 500.0f;
		const float INTERVAL_TAP_THRESHOLD = 75.0f;

	public:
		static Xinput* GetInstance();
		
		bool PollInput() override; 
		bool IsDown(BYTE keycode);
		bool IsUp(BYTE keycode);
		bool IsTapped(BYTE keycode);
		bool IsReleased(BYTE keycode);
		bool IsDoubleTapped(BYTE keycode);
		bool IsIntervalTapped(BYTE keycode);

		bool WasDown(BYTE keycode);
		bool WasUp(BYTE keycode);

	private:
		Xinput();
		XinputState lastState;
		XinputState currentState;

		XINPUT_STATE state;

		BYTE KeyDoubleTapStates[0xFF];
		Utilities::Stopwatch KeyDoubleTapWatches[0xFF];

		Stopwatch keyIntervalWatch;

		BOOL keyIntervalInitials[0xFF];
		BYTE keyIntervalTapStates[0xFF];
		FLOAT keyIntervalTapTimes[0xFF];

		static Xinput* instance;
	};
}

