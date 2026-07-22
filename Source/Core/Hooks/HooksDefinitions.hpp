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
	
	constexpr const char* ASSEMBLY_NAME = "GameAssembly.dll";

	constexpr const char* PATTERN_NETWORK_PLAYER_DESTROY = "";
	constexpr const char* PATTERN_NETWORK_PLAYER_START   = "";
	constexpr const char* PATTERN_WORLD_TO_SCREEN_POINT  = "48 89 5C 24 08 57 48 83 EC 40 F2 41 0F 10 00 33 C0 48 89 01 48 8B FA";
	constexpr const char* PATTERN_LOCAL_PLAYER_START      = "";

	inline uintptr_t offsetNetworkPlayerDestroy = 0x14A5620;
	inline uintptr_t offsetNetworkPlayerStart = 0x14ADBE0;
	inline uintptr_t offsetWorldToScreenPoint = 0x2DDEB50;
	inline uintptr_t offsetLocalPlayerStart = 0x10540D0;
	//

	HRESULT __stdcall HkResizeBuffers(IDXGISwapChain* pSwapChain, const UINT BufferCount, const UINT Width, const UINT Height, const DXGI_FORMAT NewFormat, const UINT SwapChainFlags);
	HRESULT __stdcall HkSetFullscreenState(IDXGISwapChain* pSwapChain, const BOOL FullScreen, IDXGIOutput* pTarget);
	HRESULT __stdcall HkSwapChainPresent(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags);
	void __fastcall HkNetworkPlayerStart(void* pNetworkPlayer, const void* methodInfo);
	LRESULT APIENTRY HkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void __fastcall HkNetworkPlayerDestroy(void* pNetworkPlayer);
	void __fastcall HkLocalPlayerStart(void* pLocalPlayer);

	const constexpr unsigned int SWAPCHAIN_FULLSCREEN_INDEX = 10u;
	const constexpr unsigned int SWAPCHAIN_PRESENT_INDEX = 8u;
	const constexpr unsigned int SWAPCHAIN_RESIZE_INDEX = 13u;

}
