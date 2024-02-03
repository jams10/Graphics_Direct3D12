#pragma once

#include <Windows.h>
#include <cstdint> // uint32_t Ÿ�� ����� ����.
#include <d3d12.h>    
#include <dxgi1_4.h>  
#include <wrl/client.h>

#pragma comment( lib, "d3d12.lib" )  
#pragma comment( lib, "dxgi.lib" )  

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

/// <summary>
/// ���ø����̼� Ŭ����.
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
	HINSTANCE	m_hInst;  // �ν��Ͻ� �ڵ�.
	HWND		m_hWnd;   // ������ �ڵ�.
	uint32_t	m_Width;  // ������ �ʺ�.
	uint32_t	m_Height; // ������ ����.
#pragma endregion

#pragma region Direct3D
	static const uint32_t FrameCount = 2; // ������ ���� ����.

	ComPtr<ID3D12Device>					 m_pDevice;                     // ����̽�.
	ComPtr<ID3D12CommandQueue>				 m_pQueue;                      // Ŀ�ǵ� ť.
	ComPtr<IDXGISwapChain3>			         m_pSwapChain;                  // ���� ü��.
	ComPtr<ID3D12Resource>					 m_pColorBuffer[FrameCount];    // �÷� ����.
	ComPtr<ID3D12CommandAllocator>			 m_pCmdAllocator[FrameCount];   // Command �Ҵ���.
	ComPtr<ID3D12GraphicsCommandList>		 m_pCmdList;                    // Command list.
	ComPtr<ID3D12DescriptorHeap>			 m_pHeapRTV;                    // Descriptor Heap(���� Ÿ�� �並 ����).
	ComPtr<ID3D12Fence>				 m_pFence;                      // �潺.
	HANDLE							 m_FenceEvent;                  // �潺 �̺�Ʈ.
	uint64_t						 m_FenceCounter[FrameCount];    // �潺 ī����.
	uint32_t						 m_FrameIndex;                  // ������ ��ȣ.
	D3D12_CPU_DESCRIPTOR_HANDLE		 m_HandleRTV[FrameCount];       // CPU Descriptor(���� Ÿ�� ��).
#pragma endregion

};