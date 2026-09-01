#pragma once
#include <cstdint>
#include "../Math/Vectors.hpp"
#include <d3d11.h>

namespace HooksDefinitions
{
	typedef HRESULT(__stdcall* T_SwapChainResizeBuffers)(IDXGISwapChain* pSwapChain, const UINT BufferCount, const UINT Width, const UINT Height, const DXGI_FORMAT NewFormat, const UINT SwapChainFlags);
	typedef HRESULT(__stdcall* T_SwapChainSetFullscreenState)(IDXGISwapChain* pSwapChain, const BOOL FullScreen, IDXGIOutput* pTarget);
	typedef HRESULT(__stdcall* T_SwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	typedef void(__fastcall* T_NetworkPlayerRollPerformed)(void* pNetworkPlayer, bool b1, bool b2, bool b3);
	typedef void(__fastcall* T_NetworkPlayerStart)(void* pNetworkPlayer, const void* methodInfo);
	typedef void(__fastcall* T_WorldToScreenPoint)(Vector3* output, void* camera, Vector3* input);
	typedef void(__fastcall* T_NetworkPlayerDestroy)(void* pNetworkPlayer);
	typedef void(__fastcall* T_LocalPlayerStart)(void* pLocalPlayer);
	
	inline T_SwapChainSetFullscreenState oSetFullscreenState = nullptr;
	inline T_NetworkPlayerDestroy pNetworkPlayerDestroy = nullptr;
	inline T_NetworkPlayerDestroy oNetworkPlayerDestroy = nullptr;
	inline T_NetworkPlayerStart pNetworkPlayerStart = nullptr;
	inline T_NetworkPlayerStart oNetworkPlayerStart = nullptr;
	inline T_SwapChainResizeBuffers oResizeBuffers = nullptr;
	inline T_WorldToScreenPoint WorldToScreenPoint = nullptr;
	inline T_SwapChainPresent oSwapChainPresent = nullptr;
	inline T_LocalPlayerStart pLocalPlayerStart = nullptr;
	inline T_LocalPlayerStart oLocalPlayerStart = nullptr;
	inline WNDPROC oWndProc = nullptr;

	inline BOOL(__stdcall* oGetCursorPos)(LPPOINT lpPoint) = nullptr;
	inline POINT g_realCursorPos = { 0, 0 };
	inline POINT g_aimTarget = { 0, 0 };
	inline bool g_hasAimTarget = false;
	
	constexpr const char* ASSEMBLY_NAME = "GameAssembly.dll";

	constexpr const char* PATTERN_NETWORK_PLAYER_DESTROY = "48 89 5C 24 10 57 48 83 EC 20 80 3D ? ? ? ? 00 48 8B D9 75 43";
	constexpr const char* PATTERN_NETWORK_PLAYER_START   = "48 89 5C 24 10 57 48 83 EC 40 80 3D ? ? ? ? 00 48 8B D9 75 43";
	constexpr const char* PATTERN_WORLD_TO_SCREEN_POINT  = "48 89 5C 24 08 57 48 83 EC 40 F2 41 0F 10 00 33 C0 48 89 01 48 8B FA";
	constexpr const char* PATTERN_LOCAL_PLAYER_START     = "40 57 41 55 48 83 EC 78 80 3D ? ? ? ? 00 48 8B F9 75 67";

	inline uintptr_t offsetNetworkPlayerDestroy = 0x16B6F00;
	inline uintptr_t offsetNetworkPlayerStart   = 0x16BF4C0;
	inline uintptr_t offsetWorldToScreenPoint  = 0x2EF74A0;
	inline uintptr_t offsetLocalPlayerStart    = 0x1642000;
	//

	HRESULT __stdcall HkResizeBuffers(IDXGISwapChain* pSwapChain, const UINT BufferCount, const UINT Width, const UINT Height, const DXGI_FORMAT NewFormat, const UINT SwapChainFlags);
	HRESULT __stdcall HkSetFullscreenState(IDXGISwapChain* pSwapChain, const BOOL FullScreen, IDXGIOutput* pTarget);
	HRESULT __stdcall HkSwapChainPresent(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags);
	void __fastcall HkNetworkPlayerStart(void* pNetworkPlayer, const void* methodInfo);
	LRESULT APIENTRY HkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void __fastcall HkNetworkPlayerDestroy(void* pNetworkPlayer);
	void __fastcall HkLocalPlayerStart(void* pLocalPlayer);
	BOOL __stdcall HkGetCursorPos(LPPOINT lpPoint);

	const constexpr unsigned int SWAPCHAIN_FULLSCREEN_INDEX = 10u;
	const constexpr unsigned int SWAPCHAIN_PRESENT_INDEX = 8u;
	const constexpr unsigned int SWAPCHAIN_RESIZE_INDEX = 13u;

}
