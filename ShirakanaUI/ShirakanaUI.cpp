/*
* file: ShiralanaTTSUI.cpp
* info: ShiralanaTTS的UI界面实现 应用程序主类
* 
* Author: Maplespe(mapleshr@icloud.com)
* 
* date: 2022-9-19 Create
*/
#include "framework.h"
#include "ShirakanaUI.h"
#include "Helper/Helper.h"
//Gdiplus
#include <comdef.h>
#include <gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")

ULONG_PTR g_gdiplusToken = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    //初始化Gdiplus
    Gdiplus::GdiplusStartupInput StartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &StartupInput, NULL);
    
    //初始化MiaoUI界面库
    std::wstring _error;
    if (!AiUI::MiaoUI_Initialize(_error))
    {
        MessageBoxW(nullptr, _error.c_str(), L"MiaoUI 初始化失败", MB_ICONERROR);
        return -1;
    }

#ifdef MOESSLOG

    const std::wstring prefix = GetCurrentFolder() + L"\\logs";
    if (_waccess(prefix.c_str(), 0) == -1)
        if (_wmkdir(prefix.c_str()))
            fprintf_s(stdout, "[Info] Created logs Dir");

    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    const std::wstring LogFile = GetCurrentFolder()+ L"\\logs\\" + std::to_wstring(sys_time.wYear) + L'-' +
        std::to_wstring(sys_time.wMonth) + L'-' +
        std::to_wstring(sys_time.wDay) + L' ' +
        std::to_wstring(sys_time.wHour) + L'-' +
        std::to_wstring(sys_time.wMinute) + L'-' +
        std::to_wstring(sys_time.wSecond) + L" MoeSS.log";

    FILE* log_file = nullptr;
    FILE* err_file = nullptr;

    _wfreopen_s(&log_file, (LogFile + L"_stdout.log").c_str(), L"w+", stdout);
    _wfreopen_s(&err_file, (LogFile + L"_stderr.log").c_str(), L"w+", stderr);

    const std::ofstream logf(LogFile);
    std::streambuf* logBuf = logf.rdbuf();
    std::cout.rdbuf(logBuf);

#endif

    //创建主窗口和消息循环
    AiUI::MainWindow_CreateLoop();

    //反初始化界面库
    AiUI::MiaoUI_Uninitialize();

#ifdef MOESSLOG

    fclose(err_file);
    fclose(log_file);

#endif

    return 0;
}