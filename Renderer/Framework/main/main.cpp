
#include "Application/App.h"

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
    // 애플리케이션 실행.
    App app(960, 540);
    app.Run();

    return 0;
}
