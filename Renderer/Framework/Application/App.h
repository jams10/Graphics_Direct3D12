#pragma once

#include <Windows.h>
#include <cstdint> // uint32_t 타입 사용을 위함.
#include <d3d12.h>    
#include <dxgi1_4.h>  
#include <wrl/client.h>

#pragma comment( lib, "d3d12.lib" )  
#pragma comment( lib, "dxgi.lib" )  

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

/// <summary>
/// 애플리케이션 클래스.
/// </summary>
class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

#pragma region Rendering
	bool InitD3D();  
	void TermD3D();  
	void Render();   
	void WaitGpu();  
	void Present(uint32_t interval);
#pragma endregion

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

private:

#pragma region Window
	HINSTANCE	m_hInst;  // 인스턴스 핸들.
	HWND		m_hWnd;   // 윈도우 핸들.
	uint32_t	m_Width;  // 윈도우 너비.
	uint32_t	m_Height; // 윈도우 높이.
#pragma endregion

#pragma region Direct3D
	static const uint32_t FrameCount = 2; // 프레임 버퍼 개수.

	ComPtr<ID3D12Device>					 m_pDevice;                     // 디바이스.
	ComPtr<ID3D12CommandQueue>				 m_pQueue;                      // 커맨드 큐.
	ComPtr<IDXGISwapChain3>			         m_pSwapChain;                  // 스왑 체인.
	ComPtr<ID3D12Resource>					 m_pColorBuffer[FrameCount];    // 컬러 버퍼.
	ComPtr<ID3D12CommandAllocator>			 m_pCmdAllocator[FrameCount];   // Command 할당자.
	ComPtr<ID3D12GraphicsCommandList>		 m_pCmdList;                    // Command list.
	ComPtr<ID3D12DescriptorHeap>			 m_pHeapRTV;                    // Descriptor Heap(렌더 타겟 뷰를 위한).
	ComPtr<ID3D12Fence>				 m_pFence;                      // 펜스.
	HANDLE							 m_FenceEvent;                  // 펜스 이벤트.
	uint64_t						 m_FenceCounter[FrameCount];    // 펜스 카운터.
	uint32_t						 m_FrameIndex;                  // 프레임 번호.
	D3D12_CPU_DESCRIPTOR_HANDLE		 m_HandleRTV[FrameCount];       // CPU Descriptor(렌더 타겟 뷰).
#pragma endregion

};