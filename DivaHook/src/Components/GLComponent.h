#pragma once
#include "EmulatorComponent.h"
#include "GameState.h"

namespace DivaHook::Components
{
	class GLComponent : public EmulatorComponent
	{
	public:
		GLComponent();
		~GLComponent();

		virtual const char* GetDisplayName() override;

		virtual void Initialize(ComponentsManager*) override;
		virtual void Update() override;
		virtual void UpdateInput() override;

		const int updatesPerFrame = 39;

		float* uiAspectRatio;
		float* uiWidth;
		float* uiHeight;

		int* fb1Width;
		int* fb1Height;
		int* fb2Width;
		int* fb2Height;

		double* fbAspectRatio;

		GameState currentGameState;
		GameState previousGameState;
		bool dataInitialized = false;

	private:
		
	};
}
