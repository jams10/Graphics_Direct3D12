#include "App.h"
#include <assert.h>

namespace // 익명 namespace. 외부 파일에서 namespace에 선언된 코드를 접근할 수 없도록 함.
{
	// 유니코드 설정 여부에 따라 타입이 바뀌는 문자열을 다룰 때는 아래와 같이 TEXT 매크로를 사용해 문자열을 사용하는 것이 좋음.
	const auto ClassName = TEXT("SampleWindowClass");    // 윈도우 클래스 이름.

    template<typename T>
    void SafeRelease(T*& ptr) // 할당한 자원을 안전하게 해제하기 위한 함수.
    {
        if (ptr != nullptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }
}

App::App(uint32_t width, uint32_t height)
    : m_hInst(nullptr)
    , m_hWnd(nullptr)
    , m_Width(width)
    , m_Height(height)
    , m_pDevice(nullptr)
    , m_pQueue(nullptr)
    , m_pSwapChain(nullptr)
    , m_pCmdList(nullptr)
    , m_pHeapRTV(nullptr)
    , m_pFence(nullptr)
    , m_FrameIndex(0)
{
    for (auto i = 0u; i < FrameCount; ++i)
    {
        m_pColorBuffer[i] = nullptr;
        m_pCmdAllocator[i] = nullptr;
        m_FenceCounter[i] = 0;
    }
}

App::~App()
{}

/// <summary>
/// 애플리케이션 실행 함수.
/// </summary>
void App::Run()
{
    if (InitApp())
    {
        MainLoop();
    }

    TermApp();
}

/// <summary>
/// 애플리케이션 초기화 함수.
/// </summary>
bool App::InitApp()
{
    // 윈도우 초기화.
    if (!InitWnd())
    {
        return false;
    }
    // Direct3D 12 초기화.
    if (!InitD3D())
    {
        return false;
    }
    // 렌더링을 위한 리소스들을 생성.
    if (!OnInit())
    {
        return false;
    }

    // 초기화가 정상적으로 수행 되었음.
    return true;
}

/// <summary>
/// 애플리케이션 종료 함수.
/// </summary>
void App::TermApp()
{
    // 렌더링을 위해 생성 했던 리소스들 제거.
    OnTerm();
    // Direct3D 12 종료 처리.
    TermD3D();
    // 윈도우 종료 처리.
    TermWnd();
}

/// <summary>
/// 윈도우 초기화 처리 함수.
/// </summary>
bool App::InitWnd()
{
    // 인스턴스 핸들 가져오기.
    auto hInst = GetModuleHandle(nullptr);
    if (hInst == nullptr)
    {
        return false;
    }

    // 윈도우 설정.
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInst, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = ClassName;
    wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

    // 윈도우 등록.
    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    // 인스턴스 핸들 설정.
    m_hInst = hInst;

    // 윈도우 크기를 설정.
    RECT rc = {};
    rc.right = static_cast<LONG>(m_Width);
    rc.bottom = static_cast<LONG>(m_Height);

    // 윈도우 크기를 조정함.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // 윈도우 생성.
    m_hWnd = CreateWindowEx(
        0,
        ClassName,
        TEXT("Window"), // 윈도우 타이틀에 표시되는 문자열.
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr);

    if (m_hWnd == nullptr)
    {
        return false;
    }

    // 윈도우 표시.
    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    // 윈도우 갱신.
    UpdateWindow(m_hWnd);

    // 윈도우에 포커스를 설정.
    SetFocus(m_hWnd);

    // 초기화 성공.
    return true;
}

/// <summary>
/// 윈도우 종료 처리 함수.
/// </summary>
void App::TermWnd()
{
    // 윈도우 등록 해제.
    if (m_hInst != nullptr)
    {
        UnregisterClass(ClassName, m_hInst);
    }

    m_hInst = nullptr;
    m_hWnd = nullptr;
}

/// <summary>
/// 애플리케이션 메인 루프 함수.
/// </summary>
void App::MainLoop()
{
    MSG msg = {};

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

}

/// <summary>
/// 윈도우 프로시져 함수.
/// </summary>
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
    { PostQuitMessage(0); } // WM_QUIT 메시지를 큐로 보냄.
    break;

    default:
    { /* DO_NOTHING */ }
    break;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

#pragma region Rendering

/// <summary>
/// Direct3D 장치 초기화 함수.
/// </summary>
bool App::InitD3D()
{
    // 디버깅 플래그.
#if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debug;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

        // 디버깅 레이어 활성화.
        if (SUCCEEDED(hr))
        {
            debug->EnableDebugLayer();
        }
    }
#endif

    // 1. 디바이스 생성.
    auto hr = D3D12CreateDevice(
        nullptr,                       // pAdapter : 디바이스 생성시 사용하는 video adapter에 대한 포인터. 기본 어댑터 사용시 nullptr를 지정해주면 됨.
        D3D_FEATURE_LEVEL_11_0,        // Minimum Feature Level : 디바이스 생성을 위해 필요한 최소 기능 수준.
        IID_PPV_ARGS(&m_pDevice));     // riid : 디바이스 인터페이스를 위한 GUID를 지정. + ppDevice : 디바이스 인터페이스를 받기 위한 포인터를 설정. void* 이므로 캐스팅 필요.
    if (FAILED(hr))
    {
        return false;
    }

    // 2. 커맨드 큐 생성. (GPU로 보내기 위한 드로잉 명령 및 명령 실행 동기화 처리를 위한 함수들을 제공)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {}; 
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;           // GPU에서 실행할 수 있는 명령 버퍼를 지정.
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;                                    // GPU가 있는 경우 0으로 설정. 여러 GPU가 있으면 특정 GPU를 식별하기 위한 값을 넣어줘야 함.

        hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // 3. 스왑 체인 생성.
    {
        // DXGI팩토리 생성.
        IDXGIFactory4* pFactory = nullptr;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
        if (FAILED(hr))
        {
            return false;
        }

        // 스왑 체인 설정.
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferDesc.Width = m_Width;
        desc.BufferDesc.Height = m_Height;
        desc.BufferDesc.RefreshRate.Numerator = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = FrameCount; // 스왑 체인 버퍼 개수 = 프레임 버퍼 개수로 설정.
        desc.OutputWindow = m_hWnd;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 앱에서 디스플레이 모드를 바꾸기 위해 ResizeTarget() 함수를 호출할 수 있도록 함.

        // 스왑 체인 생성.
        IDXGISwapChain* pSwapChain = nullptr;
        hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
        if (FAILED(hr))
        {
            SafeRelease(pFactory);
            return false;
        }

        // IDXGISwapChain3를 얻어오기. (3에는 현재 backbuffer의 인덱스를 얻어오는 함수가 준비되어 있음.)
        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        if (FAILED(hr))
        {
            SafeRelease(pFactory);
            SafeRelease(pSwapChain);
            return false;
        }

        // 백 버퍼 인덱스 가져오기.
        m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

        // 더 이상 사용하지 않기 때문에 release.
        SafeRelease(pFactory);
        SafeRelease(pSwapChain);
    }

    // 4. 커맨드 할당자 생성. (커맨드 리스트가 저장되는 메모리를 할당 해주는 역할을 함.)
    {
        for (auto i = 0u; i < FrameCount; ++i)
        {
            hr = m_pDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,      // 커맨드 큐에 직접 등록 가능한 명령만 일단 작성할 것이므로, DIRECT 타입으로 지정.
                IID_PPV_ARGS(&m_pCmdAllocator[i]));
            if (FAILED(hr))
            {
                return false;
            }
        }
    }

    // 5. 커맨드 리스트 생성.
    {
        hr = m_pDevice->CreateCommandList(
            0,                                  // GPU node mask
            D3D12_COMMAND_LIST_TYPE_DIRECT,     // 커맨드 큐에 직접 등록 가능한 커맨드 리스트를 만들기 때문에 DIRECT로 지정.
            m_pCmdAllocator[m_FrameIndex].Get(),
            nullptr,                            // 파이프라인 스테이트 지정. 나중에 명시적으로 설정할 것이므로 일단 nullptr.
            IID_PPV_ARGS(&m_pCmdList));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // 6. 렌더 타겟 뷰 생성.
    {
        // 디스크립터 힙 설정. (디스크립터 힙에 여러 디스크립터(뷰)들이 존재함.)
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = FrameCount;              // 디스크립터 힙 내부의 디스크립터의 개수.
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // VISIBLE과 NONE이 있는데, VISIBLE은 쉐이더에서 참조될 수 있음을 의미. CBV,SRV,UAV,Sampler일 경우에만 적용 가능.
        desc.NodeMask = 0;

        // 디스크립터 힙 생성.
        hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
        if (FAILED(hr))
        {
            return false;
        }

        auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();   // Descriptor 시작 주소.
        auto incrementSize = m_pDevice                                    // Descriptor 주소 오프셋.
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (auto i = 0u; i < FrameCount; ++i)
        {
            hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i])); // 렌더 타겟 뷰 생성을 위한 리소스.
            if (FAILED(hr))
            {
                return false;
            }

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;  // 렌더 타겟에 어떻게 접근할 것인가.
            viewDesc.Texture2D.MipSlice = 0;                         // 사용할 texture의 mipmap 레벨.
            viewDesc.Texture2D.PlaneSlice = 0;                       // 사용할 texture의 plane slicer 개수.

            // 렌더 타겟 뷰 생성. = 렌더 타겟 디스크립터.
            m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

            m_HandleRTV[i] = handle;                                 // RTV에 대한 핸들을 캐싱.
            handle.ptr += incrementSize;                             // 오프셋 만큼 주소 이동. 다음 descriptor 위치를 가리킴.
            // Descriptor의 시작 주소에서 offset 만큼 이동하면서 여러 디스크립터(뷰)들의 주소를 계산함.
        }
    }

    // 7. 펜스 생성. (CPU, GPU 간의 동기화 처리를 위함.)
    {
        // 그리기 명령 완료 여부를 펜스 값이 증가 되었는지를 통해 판단함.
        // 완료 여부는 펜스의 증가로, 명령이 완료 될 때 까지 기다리는 작업은 OS가 제공하는 매커니즘을 통해 수행함.

        // 펜스 카운터를 리셋.
        for (auto i = 0u; i < FrameCount; ++i)
        {
            m_FenceCounter[i] = 0;
        }

        // 펜스 생성.
        hr = m_pDevice->CreateFence(
            m_FenceCounter[m_FrameIndex],
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_pFence));
        if (FAILED(hr))
        {
            return false;
        }

        m_FenceCounter[m_FrameIndex]++; // 제일 초기에 m_FrameIndex는 0(현재 백 버퍼 인덱스)

        // 이벤트 생성.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            return false;
        }
    }

    // 명령어 리스트를 닫기.
    m_pCmdList->Close();

    return true;
}

/// <summary>
/// Direct3D 종료 처리 함수.
/// </summary>
void App::TermD3D()
{
    // GPU 처리 완료 대기.
    WaitGpu();

    // 이벤트 파기.
    if (m_FenceEvent != nullptr)
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }

    // 펜스 release.
    m_pFence.Reset();

    // 렌더 타겟 뷰 release.
    m_pHeapRTV.Reset();
    for (auto i = 0u; i < FrameCount; ++i)
    {
        m_pColorBuffer[i].Reset();
    }

    // 커맨드 리스트 release.
    m_pCmdList.Reset();

    // 커맨드 할당자 release.
    for (auto i = 0u; i < FrameCount; ++i)
    {
        m_pCmdAllocator[i].Reset();
    }

    // 스왑체인 release.
    m_pSwapChain.Reset();

    // 커맨드 큐 release.
    m_pQueue.Reset();

    // 디바이스 release.
    m_pDevice.Reset();
}

/// <summary>
/// 렌더링 처리 함수.
/// </summary>
void App::Render()
{
    // 명령 기록을 시작.
    m_pCmdAllocator[m_FrameIndex]->Reset();
    m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr); // 첫번째 인수는 커맨드 할당자, 방금 초기화한 할당자를 넘겨줌. 두번째 인자는 파이프라인 스테이트.

    // 여기 까지 오면 명령어 기록 시작 상태가 된 것.

    // 리소스 베리어 설정.
    // 멀티스레드 환경의 Direct3D12 에서 리소스를 사용중일 때 끼어드는 인터럽트를 방지하는 매커니즘.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                     // 리소스를 '표시' 에서 '쓰기' 용도로 변경하기 위해 TRANSITION 타입을 지정함.
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();         // 상태를 전환할 리소스에 대한 포인터.
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;             // 서브 리소스 사용 전 상태.
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;        // 서브 리소스 사용 후 상태.
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  // 상태를 전환할 리소스 번호를 지정함. 모든 서브리소스를 한 번에 변경하려면 D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES를 사용.

    // 리소스 베리어.
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 렌더 타겟 설정.
    m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

    // 초기화 색상 설정.
    float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

    // 렌더 타겟 뷰를 초기화.
    m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

    // 렌더링 처리.
    {
        // TODO : 폴리곤 렌더링 코드 추가.
    }

    // 리소스 베리어 설정.
    // 렌더 타겟 -> Present로 설정해서 렌더 타겟 내용을 화면에 표시할 수 있도록 함.
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;  
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // 리소스 베리어.
    m_pCmdList->ResourceBarrier(1, &barrier);

    // 명령 기록을 종료.
    m_pCmdList->Close();

    // 명령 실행. (커맨드 리스트 실행은 명시적으로 해야 커맨드 리스트를 명령어 큐에 전달해 명령어를 실행할 수 있음.)
    ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get() };
    m_pQueue->ExecuteCommandLists(1, ppCmdLists);

    // 화면에 표시.
    Present(1);
}

/// <summary>
/// 화면에 렌더링 결과를 표시하고, 다음 프레임 준비를 실행하는 함수.
/// </summary>
void App::Present(uint32_t interval)
{
    // 화면에 표시. (표시 처리 또한 명령어 실행과 같이 명시적으로 처리해야 함.)
    m_pSwapChain->Present(interval, 0); // Present() 호출하면 현재 백 버퍼 인덱스가 갱신됨.

    // Singal 처리.
    // 우리가 초기에 설정해줄 때 fence에 0값을 넣어 놓고, 곧바로 counter를 1로 증가 시켰기 때문에 currentValue는 1인 상태.
    // 즉, 우리는 signal 되었을 때 fence 값이 1이 되기를 Signal() 함수를 통해 설정하고, Signal이 되면 비로소 fence 값이 1이 되어 명령이 완료됨을 체크함.
    const auto currentValue = m_FenceCounter[m_FrameIndex]; 
    m_pQueue->Signal(m_pFence.Get(), currentValue); // 펜스 값이 변경 되도록 signal을 파란 불 상태로 만들어줌.

    // 백 버퍼 인덱스 업데이트.
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex(); // (0<->1 계속 스왑.)

    // 다음 프레임을 그릴 준비가 되지 않았으면, 대기함.
    if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
    {
        // 펜스 값이 지정 값이 되면 이벤트를 발생 시켜달라고 요청하고, 무한 대기.
        m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
    }

    // 다음 프레임의 펜스 카운터를 증가 시켜줌.
    m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

/// <summary>
/// GPU의 처리 완료를 대기하는 함수.
/// </summary>
void App::WaitGpu()
{
    assert(m_pQueue != nullptr);
    assert(m_pFence != nullptr);
    assert(m_FenceEvent != nullptr);

    // Signal 처리.
    m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

    // 완료 시 이벤트를 설정함.
    m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

    // 대기 처리.
    WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

    // 카운터를 늘림.
    m_FenceCounter[m_FrameIndex]++;
}

bool App::OnInit()
{
    // 정점 버퍼 생성.
    {
        // 정점 데이터.
        Vertex vertices[] = {
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        };

        // 정점 버퍼를 GPU로 보내기 위한 리소스(ID3D12Resource)를 생성함.
        // 1. 리소스를 생성하기 위한 힙 속성을 정의하는 힙 프로퍼티 생성.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // 2. 리소스 속성을 정의하는 리소스 서술자 생성.
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(vertices);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // 3. 힙 프로퍼티, 리소스 서술자를 사용해 리소스 생성.
        // 클래스의 멤버 변수인 I3D312Resource 타입 ComPtr에 생성한 리소스를 받아줌.
        auto hr = m_pDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_pVB.GetAddressOf()));
        if (FAILED(hr))
        {
            return false;
        }

        // 생성한 리소스를 CPU 단에서 사용하기 위해 로컬 변수에 매핑.
        void* ptr = nullptr;
        hr = m_pVB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            return false;
        }

        // 매핑된 변수에 CPU 쪽에서 생성한 정점 데이터를 복사해 넣어줌.
        memcpy(ptr, vertices, sizeof(vertices));

        // 데이터를 채워 주었으므로 Unmap을 통해 다시 닫아줌.
        m_pVB->Unmap(0, nullptr);

        // 정점 버퍼 뷰 설정.
        m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();     // GPU의 가상 주소를 설정함.
        m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));  // 정점 버퍼 전체 크기를 설정.
        m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));  // 정점 당 사이즈.
    }

    // 상수 버퍼용 디스크립터 힙 생성.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;      // 상수 버퍼용 디스크립터 힙을 만들 것이기 때문에 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV를 지정함.
        desc.NumDescriptors = 1 * FrameCount;                    // 더블 버퍼화를 위해 FrameCount 값을 곱해줌. 
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  // 렌더 타겟의 경우 쉐이더를 볼 필요가 없기 때문에 NONE으로 설정했으나, 상수 버퍼의 경우 쉐이더 내에서 사용하므로, 쉐이더에 보이는 상태로 만들어야 함. 
        desc.NodeMask = 0;

        auto hr = m_pDevice->CreateDescriptorHeap(
            &desc,
            IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // 상수 버퍼 리소스 생성.
    {
        // 힙 속성 구조체.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD; // 정점 쉐이더에서 사용하므로 UPLOAD로 설정.
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // 리소스 설정 구조체.
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(Transform);     // 정점 버퍼의 크기는 Transform 구조체의 크기.
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (auto i = 0; i < FrameCount; ++i)
        {
            // 리소스 생성.
            auto hr = m_pDevice->CreateCommittedResource(
                &prop,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_pCB[i].GetAddressOf()));
            if (FAILED(hr))
            {
                return false;
            }

            // 방금 만든 리소스의 GPU 가상 주소를 가져옴.
            auto address = m_pCB[i]->GetGPUVirtualAddress();
            // 상수 버퍼 뷰가 저장될 디스크립터 힙의 시작 핸들을 가져옴.
            auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
            auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();
            // 생성한 상수 버퍼가 들어갈 오프셋 위치를 계산해줌. 위에서 GetDescriptorHandleIncrementSize() 함수를 통해 계산 했음.
            handleCPU.ptr += incrementSize * i;
            handleGPU.ptr += incrementSize * i;

            // 상수 버퍼 뷰 설정.
            m_CBV[i].HandleCPU = handleCPU;
            m_CBV[i].HandleGPU = handleGPU;
            m_CBV[i].Desc.BufferLocation = address;
            m_CBV[i].Desc.SizeInBytes = sizeof(Transform);

            // 상수 버퍼 뷰 생성. 디스크립터 힙의 오프셋 위치를 두 번째 인자로 넣어줌.
            m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, handleCPU);

            // 버퍼에 데이터를 밀어 넣기 위해 매핑.
            hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
            if (FAILED(hr))
            {
                return false;
            }

            auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
            auto targetPos = DirectX::XMVectorZero();
            auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            auto fovY = DirectX::XMConvertToRadians(37.5f);
            auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

            // 변환 행렬 설정.
            m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
            m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
            m_CBV[i].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
        }
    }

    return true;
}

void App::OnTerm()
{
    // 종료하기 전에 상수 버퍼와 정점 버퍼 등 렌더링을 위해 만들어준 리소스들을 Unmap하고, nullptr로 비워줌.
    for (auto i = 0; i < FrameCount; ++i)
    {
        if (m_pCB[i].Get() != nullptr)
        {
            m_pCB[i]->Unmap(0, nullptr);
            memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
        }
        m_pCB[i].Reset();
    }

    m_pVB.Reset();
    m_pPSO.Reset();
}

#pragma endregion