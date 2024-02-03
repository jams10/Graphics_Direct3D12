#include "App.h"

namespace // �͸� namespace. �ܺ� ���Ͽ��� namespace�� ����� �ڵ带 ������ �� ������ ��.
{
	// �����ڵ� ���� ���ο� ���� Ÿ���� �ٲ�� ���ڿ��� �ٷ� ���� �Ʒ��� ���� TEXT ��ũ�θ� ����� ���ڿ��� ����ϴ� ���� ����.
	const auto ClassName = TEXT("SampleWindowClass");    // ������ Ŭ���� �̸�.
}

App::App(uint32_t width, uint32_t height)
    : m_hInst(nullptr)
    , m_hWnd(nullptr)
    , m_Width(width)
    , m_Height(height)
{}

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

    // �ʱ�ȭ�� ���������� ���� �Ǿ���.
    return true;
}

/// <summary>
/// ���ø����̼� ���� �Լ�.
/// </summary>
void App::TermApp()
{
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