
#include "GLComponent.h"
#include <iostream>
#include <Windows.h>
#include "../Constants.h"

#include <stdio.h>

#include <chrono>
#include <thread>
#include "../FileSystem/ConfigFile.h"

#include "../Input/Keyboard/Keyboard.h"
#include "../Input/Xinput/Xinput.h"

#include "MinHook.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "GL/glew.h"
#include "GL/gl.h"
#include "../MainModule.h"

namespace DivaHook::Components
{
	using namespace std::chrono;
	using dsec = duration<double>;

	GLComponent::GLComponent()
	{
	}

	GLComponent::~GLComponent()
	{
	}

	const char* GLComponent::GetDisplayName()
	{
		return "gl_component";
	}
	const std::string RESOLUTION_CONFIG_FILE_NAME = "graphics.ini";

	static int moduleEquip1 = 0;
	static int moduleEquip2 = 0;
	static int btnSeEquip = 0;
	static int skinEquip = 0;
	static bool showUi = false;
	static bool showFps = false;
	static bool showUi2 = false;
	static bool showAbout = false;
	static int firstTime = 1000;
	
	static int sfxVolume = 100;
	static int bgmVolume = 100;
	static float uiTransparency = 0.8;
	static float sleep = 0;
	static float fpsDiff = 0;
	static bool morphologicalAA = 0;
	static bool morphologicalAA2 = 0;
	static bool temporalAA = 0;
	static bool temporalAA2 = 0;
	static bool copydepth = false;

	static std::chrono::time_point mBeginFrame = system_clock::now();
	static std::chrono::time_point prevTimeInSeconds = time_point_cast<seconds>(mBeginFrame);
	static unsigned frameCountPerSecond = 0;

	int last_pvid = -1;
	bool pvid_init = false;

	static bool toonShader = true;
	static bool toonShader2 = false;

	typedef PROC(*func_wglGetProcAddress_t) (LPCSTR lpszProc);
	static func_wglGetProcAddress_t _wglGetProcAddress;
	func_wglGetProcAddress_t	owglGetProcAddress;

	uint64_t hookTramp = NULL;

	typedef BOOL(__stdcall * twglSwapBuffers) (_In_ HDC hDc);

	twglSwapBuffers owglSwapBuffers;

	BOOL __stdcall hwglSwapBuffers(_In_ HDC hDc)
	{
		int* fbWidth = (int*)FB_RESOLUTION_WIDTH_ADDRESS;
		int* fbHeight = (int*)FB_RESOLUTION_HEIGHT_ADDRESS;

		//int module1 = *;
		//int module2 = *();
		//int module3 = *();

		if (copydepth)
		{
			RECT hWindow;
			GetClientRect(DivaHook::MainModule::DivaWindowHandle, &hWindow);
			long uiWidth = hWindow.right - hWindow.left;
			long uiHeight = hWindow.bottom - hWindow.top;

			glBindFramebufferEXT(GL_READ_FRAMEBUFFER, 3);
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebufferEXT(0, 0, *fbWidth, *fbHeight, 0, 0, uiWidth, uiHeight,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		ImGui::SetNextWindowBgAlpha(uiTransparency);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplOpenGL2_NewFrame();
		ImGui::NewFrame();

		RECT hWindow;
		GetClientRect(DivaHook::MainModule::DivaWindowHandle, &hWindow);
		long uiWidth = hWindow.right - hWindow.left;
		long uiHeight = hWindow.bottom - hWindow.top;

		ImGuiIO& io = ImGui::GetIO();
		auto keyboard = DivaHook::Input::Keyboard::GetInstance();
		auto xinput = DivaHook::Input::Xinput::GetInstance();
		io.MouseDown[0] = keyboard->IsDown(VK_LBUTTON);
		io.MouseDown[1] = keyboard->IsDown(VK_RBUTTON);
		io.MouseDown[2] = keyboard->IsDown(VK_MBUTTON);


		if (((keyboard->IsDown(VK_CONTROL)) && (keyboard->IsDown(VK_LSHIFT)) && (keyboard->IsTapped(VK_BACK))) || (xinput->IsTapped(XINPUT_BACK)))
		{
			if (showUi) { showUi = false; showUi2 = false; }
			else showUi = true;
		}

		int i = 48;
		while (i != 58)
		{
			if (keyboard->IsTapped(i))
			{
				io.AddInputCharacter(i);
			}
			i++;
		}

		bool p_open = false;

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		
		if (showUi) {
			ImGui::SetNextWindowBgAlpha(uiTransparency);
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("DivaHook Config", &showUi, window_flags);
			if (ImGui::CollapsingHeader("Modules and Custom Skins/Sounds"))
			{
				ImGui::Text("--- Changes only takes effect after entering a new stage. ---");
				ImGui::InputInt("Module 1 ID", (int*)0x00000001411A8A10);
				ImGui::InputInt("Module 2 ID", (int*)0x00000001411A8A14);
				ImGui::InputInt("Module 3 ID", (int*)0x00000001411A8A18);
				ImGui::InputInt("HUD Skin ID", (int*)0x00000001411A8D98);
			}
			if (ImGui::CollapsingHeader("Internal Resolution"))
			{
				ImGui::InputInt("Resolution Width", fbWidth);
				ImGui::InputInt("Resolution Height", fbHeight);
			}
			if (ImGui::CollapsingHeader("Framerate"))
			{
				ImGui::Text("--- Set the FPS cap to 0 only if you have vsync. ---");
				ImGui::InputInt("Framerate Cap", &DivaHook::MainModule::fpsLimitSet);
			}
			if (ImGui::CollapsingHeader("Graphics settings"))
			{
				ImGui::Text("--- Anti-Aliasing ---");
				ImGui::Checkbox("TAA (Temporal AA)", &temporalAA);
				ImGui::Text("--- Bug Fixes ---");
				ImGui::Checkbox("Toon Shader (When Running with: -hdtv1080/-aa)", &toonShader);
			}
			if (ImGui::CollapsingHeader("Sound Settings"))
			{
				ImGui::SliderInt("HP Volume", (int*)0x00000001411A8980, 0, 100);
				ImGui::SliderInt("ACT Volume", (int*)0x00000001411A8988, 0, 100);
				ImGui::SliderInt("SLIDER Volume", (int*)0x00000001411A898C, 0 , 100);
			}
			if (ImGui::CollapsingHeader("UI Settings"))
			{
				ImGui::SliderFloat("UI Transparency", &uiTransparency, 0, 1.0);
				ImGui::Checkbox("Framerate Overlay", &showFps);
			}
			if (ImGui::Button("Close")) { showUi = false; }; ImGui::SameLine();
			//if (ImGui::Button("Reset")) { resetGameUi = true; }; ImGui::SameLine();
			if (ImGui::Button("About")) { showAbout = true; } ImGui::SameLine();
			//if (ImGui::Button("Camera")) { cameraUi = true; } ImGui::SameLine();
			//if (ImGui::Button("Debug")) { debugUi = true; } ImGui::SameLine();
			ImGui::End();
		}

		if (showAbout)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("About DivaHook/ELAC", &showAbout, window_flags);
			ImGui::Text("Main Developer:");
			ImGui::Text("Samyuu");
			ImGui::Text("DIVAHook/ELAC Contributors:");
			ImGui::Text("Brolijah, Crash5band, Rakisaionji, Deathride58, lybxlpsv");
			ImGui::Text("DIVAHook UI by:");
			ImGui::Text("lybxlpsv");
			ImGui::Text("DIVAHook UI Contributors:");
			ImGui::Text("BesuBaru");
			if (ImGui::Button("Close")) { showAbout = false; };
			ImGui::End();
		}

		if (showFps)
		{
			ImGui::SetNextWindowBgAlpha(uiTransparency);
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoTitleBar;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::Begin("FPS", &p_open, window_flags);
			ImGui::SetWindowPos(ImVec2(hWindow.right - 100, 0));
			ImGui::SetWindowSize(ImVec2(100, 70));
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("FT: %.2fms", 1000 / ImGui::GetIO().Framerate);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		if (DivaHook::MainModule::fpsLimitSet != DivaHook::MainModule::fpsLimit)
		{
			mBeginFrame = system_clock::now();
			prevTimeInSeconds = time_point_cast<seconds>(mBeginFrame);
			frameCountPerSecond = 0;
			DivaHook::MainModule::fpsLimit = DivaHook::MainModule::fpsLimitSet;
		}

		auto invFpsLimit = round<system_clock::duration>(dsec{ 1. / DivaHook::MainModule::fpsLimit });
		auto mEndFrame = mBeginFrame + invFpsLimit;

		auto timeInSeconds = time_point_cast<seconds>(system_clock::now());
		++frameCountPerSecond;
		if (timeInSeconds > prevTimeInSeconds)
		{
			frameCountPerSecond = 0;
			prevTimeInSeconds = timeInSeconds;
		}

		// This part keeps the frame rate.
		if (DivaHook::MainModule::fpsLimit > 19)
			std::this_thread::sleep_until(mEndFrame);
		mBeginFrame = mEndFrame;
		mEndFrame = mBeginFrame + invFpsLimit;

		return owglSwapBuffers(hDc);
	}

	void GLComponent::Initialize(ComponentsManager*)
	{
		
		
		

		glewInit();
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

		DivaHook::FileSystem::ConfigFile resolutionConfig(MainModule::GetModuleDirectory(), RESOLUTION_CONFIG_FILE_NAME.c_str());
		bool success = resolutionConfig.OpenRead();
		if (!success)
		{
			printf("GLComponent: Unable to parse %s\n", RESOLUTION_CONFIG_FILE_NAME.c_str());
		}

		if (success) {
			std::string trueString = "1";
			std::string *value;
			if (resolutionConfig.TryGetValue("fpsLimit", value))
			{
				DivaHook::MainModule::fpsLimitSet = std::stoi(*value);
			}
			if (resolutionConfig.TryGetValue("MLAA", value))
			{
				morphologicalAA = std::stoi(*value);
			}
			if (resolutionConfig.TryGetValue("TAA", value))
			{
				temporalAA = std::stoi(*value);
			}
			if (resolutionConfig.TryGetValue("toonShaderWorkaround", value))
			{
				toonShader = std::stoi(*value);
			}
			if (resolutionConfig.TryGetValue("depthCopy", value))
			{
				if (*value == trueString)
					copydepth = true;
			}
			if (resolutionConfig.TryGetValue("showFps", value))
			{
				if (*value == trueString)
					showFps = true;
			}
		}

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.WantCaptureKeyboard == true;

		ImGui_ImplWin32_Init(MainModule::DivaWindowHandle);
		ImGui_ImplOpenGL2_Init();
		ImGui::StyleColorsDark();

		HMODULE hMod = GetModuleHandle(L"opengl32.dll");
		void* ptr = GetProcAddress(hMod, "wglSwapBuffers");
		MH_Initialize();
		MH_CreateHook(ptr, hwglSwapBuffers, reinterpret_cast<void**>(&owglSwapBuffers));
		MH_EnableHook(ptr);
	}

	
	void GLComponent::Update()
	{
		int* pvid = (int*)0x00000001418054C4;

		if (pvid_init == false)
		{
			DWORD oldProtect, bck;
			VirtualProtect((BYTE*)0x00000001405CBBA3, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((byte*)0x00000001405CBBA3 + 0) = 0x90;
			*((byte*)0x00000001405CBBA3 + 1) = 0x90;
			*((byte*)0x00000001405CBBA3 + 2) = 0x90;
			*((byte*)0x00000001405CBBA3 + 3) = 0x90;
			*((byte*)0x00000001405CBBA3 + 4) = 0x90;
			*((byte*)0x00000001405CBBA3 + 5) = 0x90;
			*((byte*)0x00000001405CBBA3 + 6) = 0x90;
			*((byte*)0x00000001405CBBA3 + 7) = 0x90;
			VirtualProtect((BYTE*)0x00000001405CBBA3, 8, oldProtect, &bck);
			pvid_init = true;
		}

		if (*pvid != last_pvid)
		{
			pvid_init = false;
			last_pvid = *pvid;
			DWORD oldProtect, bck;
			VirtualProtect((BYTE*)0x00000001405CBBA3, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((byte*)0x00000001405CBBA3 + 0) = 0x42;
			*((byte*)0x00000001405CBBA3 + 1) = 0x89;
			*((byte*)0x00000001405CBBA3 + 2) = 0x84;
			*((byte*)0x00000001405CBBA3 + 3) = 0xb6;
			*((byte*)0x00000001405CBBA3 + 4) = 0xc0;
			*((byte*)0x00000001405CBBA3 + 5) = 0x01;
			*((byte*)0x00000001405CBBA3 + 6) = 0x00;
			*((byte*)0x00000001405CBBA3 + 7) = 0x00;
			VirtualProtect((BYTE*)0x00000001405CBBA3, 8, oldProtect, &bck);
		}

		if (temporalAA)
		{
			DWORD oldProtect, bck;
			VirtualProtect((BYTE*)0x00000001411AB67C, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((byte*)0x00000001411AB67C + 0) = 0x01;
			VirtualProtect((BYTE*)0x00000001411AB67C, 1, oldProtect, &bck);
		}
		else {
			DWORD oldProtect, bck;
			VirtualProtect((BYTE*)0x00000001411AB67C, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((byte*)0x00000001411AB67C + 0) = 0x00;
			VirtualProtect((BYTE*)0x00000001411AB67C, 1, oldProtect, &bck);
		}

		
		if (toonShader)
		{
			if (!toonShader2)
			{
				DWORD oldProtect, bck;
				VirtualProtect((BYTE*)0x000000014050214F, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
				*((byte*)0x000000014050214F + 0) = 0x90;
				*((byte*)0x000000014050214F + 1) = 0x90;
				VirtualProtect((BYTE*)0x000000014050214F, 2, oldProtect, &bck);
				toonShader2 = true;
			}
		}
		else {
			if (toonShader2)
			{
				DWORD oldProtect, bck;
				VirtualProtect((BYTE*)0x000000014050214F, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
				*((byte*)0x000000014050214F + 0) = 0x74;
				*((byte*)0x000000014050214F + 1) = 0x18;
				VirtualProtect((BYTE*)0x000000014050214F, 2, oldProtect, &bck);
				toonShader2 = !toonShader2;
			}
		}
	}

	void GLComponent::UpdateInput()
	{
		return;
	}
}
