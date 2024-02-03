
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif//defined(DEBUG) || defined(_DEBUG)

#include "Application/App.h"

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
    // 메모리 누수 체크
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//defined(DEBUG) || defined(_DEBUG)

    // 애플리케이션 실행.
    App app(960, 540);
    app.Run();

    return 0;
}
