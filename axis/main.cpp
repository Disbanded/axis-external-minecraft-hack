//ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#pragma comment(lib, "ntdll.lib")
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800

//Others
#include "axis/axis.h"
#include "pscanner/scanner.h"
#include "modules/modules.h"

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void gui()
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, (" "), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, (" "), WS_POPUP, 100, 100, 500, 320, NULL, NULL, wc.hInstance, NULL);

	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	auto default_font = io.Fonts->AddFontDefault();
	/*
	io.Fonts->AddFontFromMemoryCompressedTTF(roboto_compressed_data, roboto_compressed_size, 18);

	io.Fonts->AddFontDefault();
	*/

	io.IniFilename = NULL;

	ImFontConfig config;
	config.MergeMode = true;
	config.PixelSnapH = true;

	static const ImWchar ranges[] =
	{
		0xf000,
		0xf976,
		NULL
	};

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static int menu_movement_x = 0;
	static int menu_movement_y = 0;

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowRounding = 0.0f;
		style->WindowBorderSize = 1.0f;

		style->Alpha = 1.0f;
		style->WindowPadding = ImVec2(0, 0);
		style->WindowRounding = 0.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 0.0f;
		style->ItemSpacing = ImVec2(5, 5);
		style->ItemInnerSpacing = ImVec2(5, 5);
		style->IndentSpacing = 10.0f;
		style->TouchExtraPadding = ImVec2(5, 5);
		style->ScrollbarSize = 13.0f;
		style->ScrollbarRounding = 15.0f;
		style->GrabMinSize = 10.0f;
		style->GrabRounding = 2.0f;
		style->ColumnsMinSpacing = 10.0f;
		style->ButtonTextAlign = ImVec2(0.5, 0.5);
		style->WindowTitleAlign = ImVec2(0.5, 0.5);

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::PushFont(default_font);
			ImGui::Begin("##main", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetWindowSize(ImVec2(500, 320));

			if (ImGui::IsMouseClicked(0))
			{
				POINT CursorPosition;
				RECT MenuPosition;

				GetCursorPos(&CursorPosition);
				GetWindowRect(hwnd, &MenuPosition);

				menu_movement_x = CursorPosition.x - MenuPosition.left;
				menu_movement_y = CursorPosition.y - MenuPosition.top;
			}

			if ((menu_movement_y >= 0 && menu_movement_y <= 27 && menu_movement_x <= 440) && ImGui::IsMouseDragging(0))
			{
				POINT cursor_position;

				GetCursorPos(&cursor_position);

				SetWindowPos(hwnd, nullptr, cursor_position.x - menu_movement_x, cursor_position.y - menu_movement_y, 0, 0, SWP_NOSIZE);
			}
			ImGui::SetCursorPos(ImVec2(10, 286));
			ImGui::Text("Axis - 1.0");
			ImGui::SetCursorPos(ImVec2(420, 282));
			if (ImGui::Button("Destruct"))
			{
				 
				exit(0);
			}

			ImGui::SetCursorPos(ImVec2(20, 20));
			ImGui::Text("AutoClicker");
			ImGui::SetCursorPos(ImVec2(20, 38));
			ImGui::SliderFloat("CPS", &aclicker::cps, 1.f, 20.f);

			ImGui::SetCursorPos(ImVec2(20, 78));
			ImGui::Text("AimAssist");
			ImGui::SetCursorPos(ImVec2(20, 95));
			ImGui::SliderFloat("Angle", &aim::max_angle, 20.f, 360.f);
			ImGui::SetCursorPos(ImVec2(20, 121));
			ImGui::SliderFloat("Speed", &aim::aim_speed, 1.f, 8.0f);
			ImGui::SetCursorPos(ImVec2(20, 147));
			ImGui::SliderFloat("Distance", &aim::aim_distance, 3.f, 6.f);

			ImGui::SetCursorPos(ImVec2(20, 192));
			ImGui::Text("Velocity");
			ImGui::SetCursorPos(ImVec2(20, 209));
			ImGui::SliderFloat("Horizontal", &velocity::velocity_h, 0.f, 100.f);
			ImGui::SetCursorPos(ImVec2(20, 235));
			ImGui::SliderFloat("Vertical", &velocity::velocity_v, 0.f, 100.f);

			ImGui::End();
			ImGui::PopFont();
		}

		// Rendering
		ImGui::Render();
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return;
}

int main() 
{
	//Making the title blank to prevent strings and shit
	SetConsoleTitleA(" ");

    if (!axis::_open_mc()) {

        for (
        printf("couldn't open minecraft process.\n"); 
        !axis::_is_pressed(0xd); Sleep(50.0));
        
		Sleep(2000);
		exit(0);
    }

    for (axis::_get_ver(); !axis::mc_version;) {

        for (
        printf("not supported minecraft version.\n");
        !axis::_is_pressed(0xd); Sleep(50.0));

		Sleep(2000);
		exit(0);
    }

    for (axis::mc_c_ptr = scanner::pattern(0x100000000, 0x1ffffffff, {

        RAND, 0x01, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, 0x00, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, 0x00, 0x01, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, RAND, RAND, RAND, RAND,
        0x28, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, 0x00, 0x00, 0x00, 0x00,
        0x28, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, 0x01, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, 0x01, 0x00, 0x00, 0x00,
        RAND, RAND, RAND, RAND, RAND, 0x00, 0x00, 0x00,
        0x01, RAND, RAND, RAND, RAND, RAND, RAND, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, RAND, RAND, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00

    }) + 0x60; axis::mc_c_ptr <= 0x100;) {

        for (
        printf("not supported minecraft version.\n");
        !axis::_is_pressed(0xd); Sleep(50.0));

		Sleep(2000);
		exit(0);
    }

    for (
        printf("succesfully loaded.\n"), 
        axis::_thread(axis::_handler);; Sleep(100.0)) {

		ShowWindow(GetConsoleWindow(), SW_HIDE);
		gui();
    }
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		if ((wParam & 0xFFF0) == SC_CLOSE)
		{
			return false;
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}