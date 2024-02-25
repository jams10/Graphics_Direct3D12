#pragma once

#include <Windows.h>
#include <cstdint> // uint32_t 타입 사용을 위함.
#include <d3d12.h>    
#include <dxgi1_4.h>  
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h> // 벡터, 행렬 연산을 위한 라이브러리.
 
#pragma comment( lib, "d3d12.lib" )  
#pragma comment( lib, "dxgi.lib" )  
#pragma comment( lib, "d3dcompiler.lib" )

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

// 트랜스폼 구조체.
struct alignas(256) Transform
{
	DirectX::XMMATRIX   World;      // 월드 변환 행렬.
	DirectX::XMMATRIX   View;       // 뷰 변환 행렬.
	DirectX::XMMATRIX   Proj;       // 투영 변환 행렬.
};

// 상수 버퍼 구조체
template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;               // 상수 버퍼 서술자.
	D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;          // CPU 디스크립터 핸들.
	D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;          // GPU 디스크립터 핸들.
	T* pBuffer;                                         // 데이터가 들어가있는 버퍼의 시작 포인터.
};

// 정점 구조체.
struct Vertex
{
	DirectX::XMFLOAT3   Position;    // 정점 위치.
	DirectX::XMFLOAT4   Color;       // 정점 색상.
};

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
	bool OnInit();
	void OnTerm();
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
	ComPtr<ID3D12Fence>						 m_pFence;                      // 펜스.

	ComPtr<ID3D12DescriptorHeap>        m_pHeapCBV;							// 디스크립터 힙(상수 버퍼뷰, 쉐이더 리소스 뷰, 언오더드 엑세스 뷰)
	ComPtr<ID3D12Resource>              m_pVB;								// 정점 버퍼.
	ComPtr<ID3D12Resource>              m_pCB[FrameCount];					// 상수 버퍼.
	ComPtr<ID3D12RootSignature>         m_pRootSignature;					// 루트 시그니쳐.
	ComPtr<ID3D12PipelineState>         m_pPSO;								// 파이프라인 스테이트.

	HANDLE							    m_FenceEvent;                  // 펜스 이벤트.
	uint64_t						    m_FenceCounter[FrameCount];    // 펜스 카운터.
	uint32_t						    m_FrameIndex;                  // 프레임 번호.
	D3D12_CPU_DESCRIPTOR_HANDLE		    m_HandleRTV[FrameCount];       // CPU Descriptor(렌더 타겟 뷰).
	D3D12_VERTEX_BUFFER_VIEW            m_VBV;                         // 정점 버퍼 뷰.
	D3D12_VIEWPORT                      m_Viewport;                    // 뷰포트.
	D3D12_RECT                          m_Scissor;                     // 가위 직사각형.
	ConstantBufferView<Transform>       m_CBV[FrameCount];             // 상수 버퍼 뷰.
	float                               m_RotateAngle;                 // 회전 각도.
#pragma endregion

};