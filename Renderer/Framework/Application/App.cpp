#include "App.h"

namespace // 익명 namespace. 외부 파일에서 namespace에 선언된 코드를 접근할 수 없도록 함.
{
	// 유니코드 설정 여부에 따라 타입이 바뀌는 문자열을 다룰 때는 아래와 같이 TEXT 매크로를 사용해 문자열을 사용하는 것이 좋음.
	const auto ClassName = TEXT("SampleWindowClass");    // 윈도우 클래스 이름.
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

    // 초기화가 정상적으로 수행 되었음.
    return true;
}

/// <summary>
/// 애플리케이션 종료 함수.
/// </summary>
void App::TermApp()
{
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