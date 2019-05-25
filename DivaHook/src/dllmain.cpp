#include <iostream>
#include <vector>
#include <Windows.h>
#include <Dbt.h>
#include "Constants.h"
#include "MainModule.h"
#include "Input/Mouse/Mouse.h"
#include "Input/Xinput/Xinput.h"
#include "Input/Keyboard/Keyboard.h"
#include "Input/DirectInput/DirectInput.h"
#include "Input/DirectInput/Ds4/DualShock4.h"
#include "Components/ComponentsManager.h"
#include "FileSystem/ConfigFile.h"

LRESULT CALLBACK MessageWindowProcessCallback(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI WindowMessageDispatcher(LPVOID);
VOID RegisterMessageWindowClass();

struct
{
	DWORD ID = NULL;
	HANDLE Handle = NULL;
} MessageThread;

const wchar_t *MessageWindowClassName = TEXT("MessageWindowClass");
const wchar_t *MessageWindowName = TEXT("MessageWindowTitle");

namespace DivaHook
{
	const std::string RESOLUTION_CONFIG_FILE_NAME = "graphics.ini";
	Components::ComponentsManager ComponentsManager;
	bool DeviceConnected = true;
	bool FirstUpdateTick = true;
	bool HasWindowFocus, HadWindowFocus;

	void InstallCustomResolution()
	{
		
	}

	void* InstallHook(void* source, void* destination, int length)
	{
		const DWORD minLen = 0xE;

		if (length < minLen)
			return NULL;

		BYTE stub[] =
		{
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ?ptr [$+6]
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // ptr???
		};

		void* trampoline = VirtualAlloc(0, length + sizeof(stub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		DWORD oldProtect;
		VirtualProtect(source, length, PAGE_EXECUTE_READWRITE, &oldProtect);

		DWORD64 returnAddress = (DWORD64)source + length;

		// trampoline
		memcpy(stub + 6, &returnAddress, 8);

		memcpy((void*)((DWORD_PTR)trampoline), source, length);
		memcpy((void*)((DWORD_PTR)trampoline + length), stub, sizeof(stub));

		// orig
		memcpy(stub + 6, &destination, 8);
		memcpy(source, stub, sizeof(stub));

		for (int i = minLen; i < length; i++)
			*(BYTE*)((DWORD_PTR)source + i) = NOP_OPCODE;

		VirtualProtect(source, length, oldProtect, &oldProtect);

		return (void*)((DWORD_PTR)trampoline);
	}

	void InitializeTick()
	{
		RegisterMessageWindowClass();
		if ((MessageThread.Handle = CreateThread(0, 0, WindowMessageDispatcher, 0, 0, 0)) == NULL)
			printf("InitializeTick(): CreateThread() Error: %d\n", GetLastError());

		MainModule::DivaWindowHandle = FindWindow(0, MainModule::DivaWindowName);
		if (MainModule::DivaWindowHandle == NULL)
			MainModule::DivaWindowHandle = FindWindow(0, MainModule::GlutDefaultName);

		HRESULT diInitResult = Input::InitializeDirectInput(MainModule::Module);
		if (FAILED(diInitResult))
			printf("InitializeTick(): Failed to initialize DirectInput. Error: 0x%08X\n", diInitResult);

		ComponentsManager.Initialize();
	}

	void UpdateTick()
	{
		if (FirstUpdateTick)
		{
			FirstUpdateTick = false;
			InitializeTick();
		}

		if (DeviceConnected)
		{
			DeviceConnected = false;

			if (!Input::DualShock4::InstanceInitialized())
			{
				if (Input::DualShock4::TryInitializeInstance())
					printf("UpdateTick(): DualShock4 connected and initialized\n");
			}
		}

		ComponentsManager.Update();

		HadWindowFocus = HasWindowFocus;
		HasWindowFocus = MainModule::DivaWindowHandle == NULL || GetForegroundWindow() == MainModule::DivaWindowHandle;

		if ((HasWindowFocus) && (!MainModule::inputDisable))
		{
			Input::Keyboard::GetInstance()->PollInput();
			Input::Mouse::GetInstance()->PollInput();
			Input::Xinput::GetInstance()->PollInput();

			if (Input::DualShock4::GetInstance() != nullptr)
			{
				if (!Input::DualShock4::GetInstance()->PollInput())
				{
					Input::DualShock4::DeleteInstance();
					printf("UpdateTick(): DualShock4 connection lost\n");
				}
			}
			ComponentsManager.UpdateInput();
		}

		if ((MainModule::inputDisable))
		{
			Input::Keyboard::GetInstance()->PollInput();
			Input::Mouse::GetInstance()->PollInput();
			Input::Xinput::GetInstance()->PollInput();

			if (Input::DualShock4::GetInstance() != nullptr)
			{
				if (!Input::DualShock4::GetInstance()->PollInput())
				{
					Input::DualShock4::DeleteInstance();
					printf("UpdateTick(): DualShock4 connection lost\n");
				}
			}
		}

		if (HasWindowFocus && !HadWindowFocus)
			ComponentsManager.OnFocusGain();

		if (!HasWindowFocus && HadWindowFocus)
			ComponentsManager.OnFocusLost();
	}

	void InstallHooks()
	{
		InstallHook((void*)ENGINE_UPDATE_HOOK_TARGET_ADDRESS, (void*)UpdateTick, 0xE);

		DivaHook::FileSystem::ConfigFile resolutionConfig(MainModule::GetModuleDirectory(), RESOLUTION_CONFIG_FILE_NAME);
		bool success = resolutionConfig.OpenRead();
		if (!success)
		{
			printf("Resolution: Unable to parse %s\n", RESOLUTION_CONFIG_FILE_NAME.c_str());
		}

		if (success) {
			int maxWidth = 2560;
			int maxHeight = 1440;
			std::string *value;
			std::string trueValue = "true";
			bool isEnabled = false;
			if (resolutionConfig.TryGetValue("customRes", value))
			{
				if (*value == trueValue)
					isEnabled = true;
			}

			if (isEnabled == true)
			{
				printf("\n");
				if (resolutionConfig.TryGetValue("maxWidth", value))
				{
					maxWidth = std::stoi(*value);
					printf(value->c_str());
				}
				printf("x");
				if (resolutionConfig.TryGetValue("maxHeight", value))
				{
					maxHeight = std::stoi(*value);
					printf(value->c_str());
				}
				{
					DWORD oldProtect, bck;
					VirtualProtect((BYTE*)0x00000001409B8B68, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((int*)0x00000001409B8B68) = maxWidth;
					VirtualProtect((BYTE*)0x00000001409B8B68, 6, oldProtect, &bck);
				}
				{
					DWORD oldProtect, bck;
					VirtualProtect((BYTE*)0x00000001409B8B6C, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((int*)0x00000001409B8B6C) = maxHeight;
					VirtualProtect((BYTE*)0x00000001409B8B6C, 6, oldProtect, &bck);
				}
				//*((int*)0x00000001409B8B6C) = maxHeight;
				//*((int*)0x00000001409B8B14) = maxWidth;
				//*((int*)0x00000001409B8B18) = maxHeight;
			}
		}

	}

	void Dispose()
	{
		ComponentsManager.Dispose();

		delete Input::Keyboard::GetInstance();
		delete Input::Mouse::GetInstance();
		delete Input::DualShock4::GetInstance();

		Input::DisposeDirectInput();

		PostThreadMessage(MessageThread.ID, WM_QUIT, 0, 0);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("DllMain(): Installing hooks...\n");
		DivaHook::InstallHooks();
		DivaHook::MainModule::Module = hModule;
		break;

	case DLL_PROCESS_DETACH:
		DivaHook::Dispose();
		break;
	}

	return TRUE;
}

DWORD WINAPI WindowMessageDispatcher(LPVOID lpParam)
{
	HWND windowHandle = CreateWindowW(
		MessageWindowClassName,
		MessageWindowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL,
		DivaHook::MainModule::Module,
		NULL);

	if (!windowHandle)
	{
		printf("WindowMessageDispatcher(): CreateWindowW() Error: %d\n", GetLastError());
		return 1;
	}

	MessageThread.ID = GetCurrentThreadId();

	MSG message;
	DWORD returnValue;

	printf("WindowMessageDispatcher(): Entering message loop...\n");

	while (1)
	{
		returnValue = GetMessage(&message, NULL, 0, 0);
		if (returnValue != -1)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			printf("WindowMessageDispatcher(): GetMessage() Error: %d\n", returnValue);
		}
	}

	DestroyWindow(windowHandle);
	return 0;
}

BOOL RegisterDeviceInterface(HWND hWnd, HDEVNOTIFY *hDeviceNotify)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {};

	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	*hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

	return *hDeviceNotify != NULL;
}

LRESULT CALLBACK MessageWindowProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		HDEVNOTIFY hDevNotify = NULL;

		if (!RegisterDeviceInterface(hWnd, &hDevNotify))
			printf("MessageWindowProcessCallback(): RegisterDeviceInterface() Error: %d\n", GetLastError());

		break;
	}

	case WM_DEVICECHANGE:
	{
		switch (wParam)
		{
		case DBT_DEVICEARRIVAL:
			DivaHook::DeviceConnected = true;
			break;

		default:
			break;
		}
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

VOID RegisterMessageWindowClass()
{
	WNDCLASS windowClass = { };

	windowClass.lpfnWndProc = MessageWindowProcessCallback;
	windowClass.hInstance = DivaHook::MainModule::Module;
	windowClass.lpszClassName = MessageWindowClassName;

	RegisterClass(&windowClass);
}
