#include "App.h"
#include <assert.h>

namespace // �͸� namespace. �ܺ� ���Ͽ��� namespace�� ����� �ڵ带 ������ �� ������ ��.
{
	// �����ڵ� ���� ���ο� ���� Ÿ���� �ٲ�� ���ڿ��� �ٷ� ���� �Ʒ��� ���� TEXT ��ũ�θ� ����� ���ڿ��� ����ϴ� ���� ����.
	const auto ClassName = TEXT("SampleWindowClass");    // ������ Ŭ���� �̸�.

    template<typename T>
    void SafeRelease(T*& ptr) // �Ҵ��� �ڿ��� �����ϰ� �����ϱ� ���� �Լ�.
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
/// ���ø����̼� ���� �Լ�.
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
/// ���ø����̼� �ʱ�ȭ �Լ�.
/// </summary>
bool App::InitApp()
{
    // ������ �ʱ�ȭ.
    if (!InitWnd())
    {
        return false;
    }
    // Direct3D 12 �ʱ�ȭ.
    if (!InitD3D())
    {
        return false;
    }
    // �������� ���� ���ҽ����� ����.
    if (!OnInit())
    {
        return false;
    }

    // �ʱ�ȭ�� ���������� ���� �Ǿ���.
    return true;
}

/// <summary>
/// ���ø����̼� ���� �Լ�.
/// </summary>
void App::TermApp()
{
    // �������� ���� ���� �ߴ� ���ҽ��� ����.
    OnTerm();
    // Direct3D 12 ���� ó��.
    TermD3D();
    // ������ ���� ó��.
    TermWnd();
}

/// <summary>
/// ������ �ʱ�ȭ ó�� �Լ�.
/// </summary>
bool App::InitWnd()
{
    // �ν��Ͻ� �ڵ� ��������.
    auto hInst = GetModuleHandle(nullptr);
    if (hInst == nullptr)
    {
        return false;
    }

    // ������ ����.
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

    // ������ ���.
    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    // �ν��Ͻ� �ڵ� ����.
    m_hInst = hInst;

    // ������ ũ�⸦ ����.
    RECT rc = {};
    rc.right = static_cast<LONG>(m_Width);
    rc.bottom = static_cast<LONG>(m_Height);

    // ������ ũ�⸦ ������.
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ������ ����.
    m_hWnd = CreateWindowEx(
        0,
        ClassName,
        TEXT("Window"), // ������ Ÿ��Ʋ�� ǥ�õǴ� ���ڿ�.
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

    // ������ ǥ��.
    ShowWindow(m_hWnd, SW_SHOWNORMAL);

    // ������ ����.
    UpdateWindow(m_hWnd);

    // �����쿡 ��Ŀ���� ����.
    SetFocus(m_hWnd);

    // �ʱ�ȭ ����.
    return true;
}

/// <summary>
/// ������ ���� ó�� �Լ�.
/// </summary>
void App::TermWnd()
{
    // ������ ��� ����.
    if (m_hInst != nullptr)
    {
        UnregisterClass(ClassName, m_hInst);
    }

    m_hInst = nullptr;
    m_hWnd = nullptr;
}

/// <summary>
/// ���ø����̼� ���� ���� �Լ�.
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
/// ������ ���ν��� �Լ�.
/// </summary>
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
    { PostQuitMessage(0); } // WM_QUIT �޽����� ť�� ����.
    break;

    default:
    { /* DO_NOTHING */ }
    break;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

#pragma region Rendering

/// <summary>
/// Direct3D ��ġ �ʱ�ȭ �Լ�.
/// </summary>
bool App::InitD3D()
{
    // ����� �÷���.
#if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debug;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

        // ����� ���̾� Ȱ��ȭ.
        if (SUCCEEDED(hr))
        {
            debug->EnableDebugLayer();
        }
    }
#endif

    // 1. ����̽� ����.
    auto hr = D3D12CreateDevice(
        nullptr,                       // pAdapter : ����̽� ������ ����ϴ� video adapter�� ���� ������. �⺻ ����� ���� nullptr�� �������ָ� ��.
        D3D_FEATURE_LEVEL_11_0,        // Minimum Feature Level : ����̽� ������ ���� �ʿ��� �ּ� ��� ����.
        IID_PPV_ARGS(&m_pDevice));     // riid : ����̽� �������̽��� ���� GUID�� ����. + ppDevice : ����̽� �������̽��� �ޱ� ���� �����͸� ����. void* �̹Ƿ� ĳ���� �ʿ�.
    if (FAILED(hr))
    {
        return false;
    }

    // 2. Ŀ�ǵ� ť ����. (GPU�� ������ ���� ����� ��� �� ��� ���� ����ȭ ó���� ���� �Լ����� ����)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {}; 
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;           // GPU���� ������ �� �ִ� ��� ���۸� ����.
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;                                    // GPU�� �ִ� ��� 0���� ����. ���� GPU�� ������ Ư�� GPU�� �ĺ��ϱ� ���� ���� �־���� ��.

        hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // 3. ���� ü�� ����.
    {
        // DXGI���丮 ����.
        IDXGIFactory4* pFactory = nullptr;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
        if (FAILED(hr))
        {
            return false;
        }

        // ���� ü�� ����.
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
        desc.BufferCount = FrameCount; // ���� ü�� ���� ���� = ������ ���� ������ ����.
        desc.OutputWindow = m_hWnd;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // �ۿ��� ���÷��� ��带 �ٲٱ� ���� ResizeTarget() �Լ��� ȣ���� �� �ֵ��� ��.

        // ���� ü�� ����.
        IDXGISwapChain* pSwapChain = nullptr;
        hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
        if (FAILED(hr))
        {
            SafeRelease(pFactory);
            return false;
        }

        // IDXGISwapChain3�� ������. (3���� ���� backbuffer�� �ε����� ������ �Լ��� �غ�Ǿ� ����.)
        hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        if (FAILED(hr))
        {
            SafeRelease(pFactory);
            SafeRelease(pSwapChain);
            return false;
        }

        // �� ���� �ε��� ��������.
        m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

        // �� �̻� ������� �ʱ� ������ release.
        SafeRelease(pFactory);
        SafeRelease(pSwapChain);
    }

    // 4. Ŀ�ǵ� �Ҵ��� ����. (Ŀ�ǵ� ����Ʈ�� ����Ǵ� �޸𸮸� �Ҵ� ���ִ� ������ ��.)
    {
        for (auto i = 0u; i < FrameCount; ++i)
        {
            hr = m_pDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,      // Ŀ�ǵ� ť�� ���� ��� ������ ��ɸ� �ϴ� �ۼ��� ���̹Ƿ�, DIRECT Ÿ������ ����.
                IID_PPV_ARGS(&m_pCmdAllocator[i]));
            if (FAILED(hr))
            {
                return false;
            }
        }
    }

    // 5. Ŀ�ǵ� ����Ʈ ����.
    {
        hr = m_pDevice->CreateCommandList(
            0,                                  // GPU node mask
            D3D12_COMMAND_LIST_TYPE_DIRECT,     // Ŀ�ǵ� ť�� ���� ��� ������ Ŀ�ǵ� ����Ʈ�� ����� ������ DIRECT�� ����.
            m_pCmdAllocator[m_FrameIndex].Get(),
            nullptr,                            // ���������� ������Ʈ ����. ���߿� ��������� ������ ���̹Ƿ� �ϴ� nullptr.
            IID_PPV_ARGS(&m_pCmdList));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // 6. ���� Ÿ�� �� ����.
    {
        // ��ũ���� �� ����. (��ũ���� ���� ���� ��ũ����(��)���� ������.)
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = FrameCount;              // ��ũ���� �� ������ ��ũ������ ����.
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // VISIBLE�� NONE�� �ִµ�, VISIBLE�� ���̴����� ������ �� ������ �ǹ�. CBV,SRV,UAV,Sampler�� ��쿡�� ���� ����.
        desc.NodeMask = 0;

        // ��ũ���� �� ����.
        hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
        if (FAILED(hr))
        {
            return false;
        }

        auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();   // Descriptor ���� �ּ�.
        auto incrementSize = m_pDevice                                    // Descriptor �ּ� ������.
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (auto i = 0u; i < FrameCount; ++i)
        {
            hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i])); // ���� Ÿ�� �� ������ ���� ���ҽ�.
            if (FAILED(hr))
            {
                return false;
            }

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;  // ���� Ÿ�ٿ� ��� ������ ���ΰ�.
            viewDesc.Texture2D.MipSlice = 0;                         // ����� texture�� mipmap ����.
            viewDesc.Texture2D.PlaneSlice = 0;                       // ����� texture�� plane slicer ����.

            // ���� Ÿ�� �� ����. = ���� Ÿ�� ��ũ����.
            m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

            m_HandleRTV[i] = handle;                                 // RTV�� ���� �ڵ��� ĳ��.
            handle.ptr += incrementSize;                             // ������ ��ŭ �ּ� �̵�. ���� descriptor ��ġ�� ����Ŵ.
            // Descriptor�� ���� �ּҿ��� offset ��ŭ �̵��ϸ鼭 ���� ��ũ����(��)���� �ּҸ� �����.
        }
    }

    // 7. �潺 ����. (CPU, GPU ���� ����ȭ ó���� ����.)
    {
        // �׸��� ��� �Ϸ� ���θ� �潺 ���� ���� �Ǿ������� ���� �Ǵ���.
        // �Ϸ� ���δ� �潺�� ������, ����� �Ϸ� �� �� ���� ��ٸ��� �۾��� OS�� �����ϴ� ��Ŀ������ ���� ������.

        // �潺 ī���͸� ����.
        for (auto i = 0u; i < FrameCount; ++i)
        {
            m_FenceCounter[i] = 0;
        }

        // �潺 ����.
        hr = m_pDevice->CreateFence(
            m_FenceCounter[m_FrameIndex],
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_pFence));
        if (FAILED(hr))
        {
            return false;
        }

        m_FenceCounter[m_FrameIndex]++; // ���� �ʱ⿡ m_FrameIndex�� 0(���� �� ���� �ε���)

        // �̺�Ʈ ����.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            return false;
        }
    }

    // ��ɾ� ����Ʈ�� �ݱ�.
    m_pCmdList->Close();

    return true;
}

/// <summary>
/// Direct3D ���� ó�� �Լ�.
/// </summary>
void App::TermD3D()
{
    // GPU ó�� �Ϸ� ���.
    WaitGpu();

    // �̺�Ʈ �ı�.
    if (m_FenceEvent != nullptr)
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }

    // �潺 release.
    m_pFence.Reset();

    // ���� Ÿ�� �� release.
    m_pHeapRTV.Reset();
    for (auto i = 0u; i < FrameCount; ++i)
    {
        m_pColorBuffer[i].Reset();
    }

    // Ŀ�ǵ� ����Ʈ release.
    m_pCmdList.Reset();

    // Ŀ�ǵ� �Ҵ��� release.
    for (auto i = 0u; i < FrameCount; ++i)
    {
        m_pCmdAllocator[i].Reset();
    }

    // ����ü�� release.
    m_pSwapChain.Reset();

    // Ŀ�ǵ� ť release.
    m_pQueue.Reset();

    // ����̽� release.
    m_pDevice.Reset();
}

/// <summary>
/// ������ ó�� �Լ�.
/// </summary>
void App::Render()
{
    // ��� ����� ����.
    m_pCmdAllocator[m_FrameIndex]->Reset();
    m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr); // ù��° �μ��� Ŀ�ǵ� �Ҵ���, ��� �ʱ�ȭ�� �Ҵ��ڸ� �Ѱ���. �ι�° ���ڴ� ���������� ������Ʈ.

    // ���� ���� ���� ��ɾ� ��� ���� ���°� �� ��.

    // ���ҽ� ������ ����.
    // ��Ƽ������ ȯ���� Direct3D12 ���� ���ҽ��� ������� �� ������ ���ͷ�Ʈ�� �����ϴ� ��Ŀ����.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                     // ���ҽ��� 'ǥ��' ���� '����' �뵵�� �����ϱ� ���� TRANSITION Ÿ���� ������.
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();         // ���¸� ��ȯ�� ���ҽ��� ���� ������.
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;             // ���� ���ҽ� ��� �� ����.
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;        // ���� ���ҽ� ��� �� ����.
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;  // ���¸� ��ȯ�� ���ҽ� ��ȣ�� ������. ��� ���긮�ҽ��� �� ���� �����Ϸ��� D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES�� ���.

    // ���ҽ� ������.
    m_pCmdList->ResourceBarrier(1, &barrier);

    // ���� Ÿ�� ����.
    m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

    // �ʱ�ȭ ���� ����.
    float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

    // ���� Ÿ�� �並 �ʱ�ȭ.
    m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

    // ������ ó��.
    {
        // TODO : ������ ������ �ڵ� �߰�.
    }

    // ���ҽ� ������ ����.
    // ���� Ÿ�� -> Present�� �����ؼ� ���� Ÿ�� ������ ȭ�鿡 ǥ���� �� �ֵ��� ��.
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;  
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // ���ҽ� ������.
    m_pCmdList->ResourceBarrier(1, &barrier);

    // ��� ����� ����.
    m_pCmdList->Close();

    // ��� ����. (Ŀ�ǵ� ����Ʈ ������ ��������� �ؾ� Ŀ�ǵ� ����Ʈ�� ��ɾ� ť�� ������ ��ɾ ������ �� ����.)
    ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get() };
    m_pQueue->ExecuteCommandLists(1, ppCmdLists);

    // ȭ�鿡 ǥ��.
    Present(1);
}

/// <summary>
/// ȭ�鿡 ������ ����� ǥ���ϰ�, ���� ������ �غ� �����ϴ� �Լ�.
/// </summary>
void App::Present(uint32_t interval)
{
    // ȭ�鿡 ǥ��. (ǥ�� ó�� ���� ��ɾ� ����� ���� ��������� ó���ؾ� ��.)
    m_pSwapChain->Present(interval, 0); // Present() ȣ���ϸ� ���� �� ���� �ε����� ���ŵ�.

    // Singal ó��.
    // �츮�� �ʱ⿡ �������� �� fence�� 0���� �־� ����, ��ٷ� counter�� 1�� ���� ���ױ� ������ currentValue�� 1�� ����.
    // ��, �츮�� signal �Ǿ��� �� fence ���� 1�� �Ǳ⸦ Signal() �Լ��� ���� �����ϰ�, Signal�� �Ǹ� ��μ� fence ���� 1�� �Ǿ� ����� �Ϸ���� üũ��.
    const auto currentValue = m_FenceCounter[m_FrameIndex]; 
    m_pQueue->Signal(m_pFence.Get(), currentValue); // �潺 ���� ���� �ǵ��� signal�� �Ķ� �� ���·� �������.

    // �� ���� �ε��� ������Ʈ.
    m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex(); // (0<->1 ��� ����.)

    // ���� �������� �׸� �غ� ���� �ʾ�����, �����.
    if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
    {
        // �潺 ���� ���� ���� �Ǹ� �̺�Ʈ�� �߻� ���Ѵ޶�� ��û�ϰ�, ���� ���.
        m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
    }

    // ���� �������� �潺 ī���͸� ���� ������.
    m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

/// <summary>
/// GPU�� ó�� �ϷḦ ����ϴ� �Լ�.
/// </summary>
void App::WaitGpu()
{
    assert(m_pQueue != nullptr);
    assert(m_pFence != nullptr);
    assert(m_FenceEvent != nullptr);

    // Signal ó��.
    m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

    // �Ϸ� �� �̺�Ʈ�� ������.
    m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

    // ��� ó��.
    WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

    // ī���͸� �ø�.
    m_FenceCounter[m_FrameIndex]++;
}

bool App::OnInit()
{
    // ���� ���� ����.
    {
        // ���� ������.
        Vertex vertices[] = {
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        };

        // ���� ���۸� GPU�� ������ ���� ���ҽ�(ID3D12Resource)�� ������.
        // 1. ���ҽ��� �����ϱ� ���� �� �Ӽ��� �����ϴ� �� ������Ƽ ����.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // 2. ���ҽ� �Ӽ��� �����ϴ� ���ҽ� ������ ����.
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

        // 3. �� ������Ƽ, ���ҽ� �����ڸ� ����� ���ҽ� ����.
        // Ŭ������ ��� ������ I3D312Resource Ÿ�� ComPtr�� ������ ���ҽ��� �޾���.
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

        // ������ ���ҽ��� CPU �ܿ��� ����ϱ� ���� ���� ������ ����.
        void* ptr = nullptr;
        hr = m_pVB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            return false;
        }

        // ���ε� ������ CPU �ʿ��� ������ ���� �����͸� ������ �־���.
        memcpy(ptr, vertices, sizeof(vertices));

        // �����͸� ä�� �־����Ƿ� Unmap�� ���� �ٽ� �ݾ���.
        m_pVB->Unmap(0, nullptr);

        // ���� ���� �� ����.
        m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();     // GPU�� ���� �ּҸ� ������.
        m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));  // ���� ���� ��ü ũ�⸦ ����.
        m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));  // ���� �� ������.
    }

    // ��� ���ۿ� ��ũ���� �� ����.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;      // ��� ���ۿ� ��ũ���� ���� ���� ���̱� ������ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV�� ������.
        desc.NumDescriptors = 1 * FrameCount;                    // ���� ����ȭ�� ���� FrameCount ���� ������. 
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  // ���� Ÿ���� ��� ���̴��� �� �ʿ䰡 ���� ������ NONE���� ����������, ��� ������ ��� ���̴� ������ ����ϹǷ�, ���̴��� ���̴� ���·� ������ ��. 
        desc.NodeMask = 0;

        auto hr = m_pDevice->CreateDescriptorHeap(
            &desc,
            IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // ��� ���� ���ҽ� ����.
    {
        // �� �Ӽ� ����ü.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD; // ���� ���̴����� ����ϹǷ� UPLOAD�� ����.
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // ���ҽ� ���� ����ü.
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(Transform);     // ���� ������ ũ��� Transform ����ü�� ũ��.
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
            // ���ҽ� ����.
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

            // ��� ���� ���ҽ��� GPU ���� �ּҸ� ������.
            auto address = m_pCB[i]->GetGPUVirtualAddress();
            // ��� ���� �䰡 ����� ��ũ���� ���� ���� �ڵ��� ������.
            auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
            auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();
            // ������ ��� ���۰� �� ������ ��ġ�� �������. ������ GetDescriptorHandleIncrementSize() �Լ��� ���� ��� ����.
            handleCPU.ptr += incrementSize * i;
            handleGPU.ptr += incrementSize * i;

            // ��� ���� �� ����.
            m_CBV[i].HandleCPU = handleCPU;
            m_CBV[i].HandleGPU = handleGPU;
            m_CBV[i].Desc.BufferLocation = address;
            m_CBV[i].Desc.SizeInBytes = sizeof(Transform);

            // ��� ���� �� ����. ��ũ���� ���� ������ ��ġ�� �� ��° ���ڷ� �־���.
            m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, handleCPU);

            // ���ۿ� �����͸� �о� �ֱ� ���� ����.
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

            // ��ȯ ��� ����.
            m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
            m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
            m_CBV[i].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
        }
    }

    // ��Ʈ �ñ״�ó ����.
    {
        auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // 1. ��Ʈ �Ķ���� ����.
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.Descriptor.ShaderRegister = 0;
        param.Descriptor.RegisterSpace = 0;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // 2. ��Ʈ �ñ״�ó ������ ����ü ����.
        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = 1;              // ��Ʈ �Ķ���� �迭 ��.
        desc.NumStaticSamplers = 0;          // ���� ���÷� �迭 ��.
        desc.pParameters = &param;           // ��Ʈ �Ķ���� �迭.
        desc.pStaticSamplers = nullptr;      // ���� ���÷� �迭. �ؽ��ĸ� ������� �����Ƿ� ���� ���� �ʾ���.
        desc.Flags = flag;

        ComPtr<ID3DBlob> pBlob;
        ComPtr<ID3DBlob> pErrorBlob;

        // 3. ��Ʈ �ñ״��� ����ȭ.
        auto hr = m_pDevice->CreateRootSignature(
            0,                                              // ��� ����ũ.
            pBlob->GetBufferPointer(),                      // ����ȭ�� �����Ϳ� ���� ������.
            pBlob->GetBufferSize(),                         // ����ȭ�� �������� ����Ʈ ũ��.
            IID_PPV_ARGS(m_pRootSignature.GetAddressOf())); // ��Ʈ �ñ״�ó �������̽��� GUID�� ��Ʈ �ñ״�ó�� �����ϴ� ������ ���� ������.
        if (FAILED(hr))
        {
            return false;
        }

        // 4. ��Ʈ �ñ״��� ����.
        hr = m_pDevice->CreateRootSignature(
            0,
            pBlob->GetBufferPointer(),
            pBlob->GetBufferSize(),
            IID_PPV_ARGS(m_pRootSignature.GetAddressOf()));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // �׷��Ƚ� ���������� ������Ʈ ����.
    {
        // �Է� element ���� ����ü. CPU���� ������ ������ �����Ϳ� GPU�� ���̴� �Է��� ���� �����.
        D3D12_INPUT_ELEMENT_DESC elements[2];
        elements[0].SemanticName = "POSITION";             // �ø�ƽ �̸�.
        elements[0].SemanticIndex = 0;                     // �ø�ƽ ��ȣ ����. ex) POSITION0, POSITION1 ó�� �ڿ� ���� �ε��� ��ȣ.
        elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;  // �ϳ��� ���� ������ ������ ������.
        elements[0].InputSlot = 0;                         // �Է� ���� ��ȣ. ��ȿ ������ [0,5]. �� ���ÿ����� ������ ���� ���۸� ������� �ʱ� ������ 0�� ������.
        elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // �� ��Ұ��� �������� ����Ʈ�� ����. D3D12_APPEND_ALIGNED_ELEMENT�� �����ϸ� ���� ��� ���İ� �������� ��.
        elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // ������ �Է� ����. PER_VERTEX_DATA : ������ ������, PER_INSTANCE_DATA : �ν��Ͻ� �� ������.
        elements[0].InstanceDataStepRate = 0;                                    // �ν��Ͻ� �� ������ ��ȸ Ƚ��. �Է� ��Ұ� ������ �����͸� �����ϴ� ���, 0���� ����.

        elements[1].SemanticName = "COLOR";
        elements[1].SemanticIndex = 0;
        elements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        elements[1].InputSlot = 0;
        elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elements[1].InstanceDataStepRate = 0;

        // �����Ͷ����� ������Ʈ ���� ����ü.
        D3D12_RASTERIZER_DESC descRS;
        descRS.FillMode = D3D12_FILL_MODE_SOLID; // �׸��� ���� ä��� ��带 ����. WIREFRAME : �� �״�� ������ �����ϴ� ���� �׸�. SOLID : �������� �����Ǵ� �ﰢ������ ä���׸�.
        descRS.CullMode = D3D12_CULL_MODE_NONE;  // Ư�� ������ ���ϴ� �ﰢ���� �׸��� �ʵ��� �����ϴ� ���. FRONT�� ����, BACK�� �ĸ� �ﰢ���� �׸��� �ʰ� ��.
        descRS.FrontCounterClockwise = FALSE;    // ������ �ݽð� �������� ����. FALSE�� ��� �ð� �������� ������ ������ ���� ������ ��.
        descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS; // �־��� �ȼ��� ���� ���� ���� ����. �ַ� ���� ���踦 �����ϴµ� ����ϸ�, ���⼭�� ������� �ʱ� ������ �⺻ ������ ����.
        descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP; // �ȼ� �ִ� ���� bias ���� ����. ������� �ʱ� ������ �⺻ ������ ����.
        descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS; // �ȼ��� ����(gradient)�� ���� ���� bias ���� �������ϴ� scalar ��. �׳� �⺻ ������ ����.
        descRS.DepthClipEnable = FALSE;         // �Ÿ��� �ٰ��� Ŭ������ ���� ����. ������� ������ FALSE�� ����.
        descRS.MultisampleEnable = FALSE;       // ��Ƽ���ø� Ȱ��ȭ ����. ������� ������ FALSE�� ����.
        descRS.AntialiasedLineEnable = FALSE;   // line ��Ƽ���ϸ���� Ȱ��ȭ ����. MultisampleEnable�� FALSE�̰� �� �׸��Ⱑ ����Ǵ� ��쿡�� �����.
        descRS.ForcedSampleCount = 0;           // UAV �׸��� ���� �����Ͷ����� ���� ���� ���� ������ ���� ������ �������. 0�� ���� ���� �������� ������ ��Ÿ��.
        descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // Constervative Rasterization Ȱ��ȭ ���θ� ����.

        // ���� Ÿ�� ���� ���� ���� ����ü.
        D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        // ���� ������Ʈ ���� ����ü.
        D3D12_BLEND_DESC descBS;
        descBS.AlphaToCoverageEnable = FALSE;   // AlphaToCoverageEnable ����� Ȱ��ȭ�ϴ� �÷���. �ȼ� ���̴��� ���� ������ ����ؼ� ��Ƽ ���ø� AA ó���� �����ϴ� ���.
        descBS.IndependentBlendEnable = FALSE;  // ���� Ÿ���� �������� ���� ��� ����. FALSE�� ���, RenderTaget[0]�� ����.
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            descBS.RenderTarget[i] = descRTBS;
        }

        ComPtr<ID3DBlob> pVSBlob;
        ComPtr<ID3DBlob> pPSBlob;

        // ���� ���̴� �б�.
        auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            return false;
        }

        // �ȼ� ���̴� �б�.
        hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            return false;
        }

        // ���������� ������Ʈ ���� ����ü.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.InputLayout = { elements, _countof(elements) };
        desc.pRootSignature = m_pRootSignature.Get();
        desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
        desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
        desc.RasterizerState = descRS;
        desc.BlendState = descBS;
        desc.DepthStencilState.DepthEnable = FALSE;
        desc.DepthStencilState.StencilEnable = FALSE;
        desc.SampleMask = UINT_MAX;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        // ���������� ������Ʈ ����.
        hr = m_pDevice->CreateGraphicsPipelineState(
            &desc,
            IID_PPV_ARGS(m_pPSO.GetAddressOf()));
        if (FAILED(hr))
        {
            return false;
        }
    }

    return true;
}

void App::OnTerm()
{
    // �����ϱ� ���� ��� ���ۿ� ���� ���� �� �������� ���� ������� ���ҽ����� Unmap�ϰ�, nullptr�� �����.
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