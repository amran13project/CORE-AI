#ifdef _WIN32
#include "core/CoreApp.h"
#include <windows.h>
#include <string>
namespace core::ui {
static CoreApp* g_app=nullptr;
static HWND g_edit=nullptr;
static HWND g_output=nullptr;
static HFONT g_font=nullptr;
LRESULT CALLBACK wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
    switch(msg){
    case WM_CREATE:{
        g_font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
        g_edit=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,24,24,760,42,hwnd,(HMENU)1001,GetModuleHandleW(nullptr),nullptr);
        auto* button=CreateWindowW(L"BUTTON",L"ASK CORE",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,796,24,150,42,hwnd,(HMENU)1002,GetModuleHandleW(nullptr),nullptr);
        g_output=CreateWindowW(L"EDIT",L"CORE AI\r\nReady.\r\n",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY,24,86,922,560,hwnd,(HMENU)1003,GetModuleHandleW(nullptr),nullptr);
        SendMessageW(g_edit,WM_SETFONT,(WPARAM)g_font,TRUE); SendMessageW(g_output,WM_SETFONT,(WPARAM)g_font,TRUE); SendMessageW(button,WM_SETFONT,(WPARAM)g_font,TRUE);
        return 0;}
    case WM_COMMAND:
        if(LOWORD(wParam)==1002 && HIWORD(wParam)==BN_CLICKED){
            wchar_t text[4096]{}; GetWindowTextW(g_edit,text,4096); std::wstring ws=text; std::string prompt(ws.begin(),ws.end());
            std::string response=g_app->chat(prompt); std::wstring out(response.begin(),response.end()); out += L"\r\n\r\n";
            SetWindowTextW(g_output,out.c_str()); return 0; }
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd,msg,wParam,lParam);
}
int runWin32(CoreApp& app){
    g_app=&app;
    const HINSTANCE h=GetModuleHandleW(nullptr);
    const wchar_t* cls=L"CoreAIWindow";

    WNDCLASSW wc{};
    wc.lpfnWndProc=wndProc;
    wc.hInstance=h;
    wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));
    wc.hbrBackground=CreateSolidBrush(RGB(17,18,22));
    wc.lpszClassName=cls;

    if(!RegisterClassW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){
        const DWORD err=GetLastError();
        wchar_t msg[256]{};
        wsprintfW(msg,L"CORE AI could not register its window class.\nWindows error: %lu",err);
        MessageBoxW(nullptr,msg,L"CORE AI — Startup Error",MB_OK|MB_ICONERROR);
        if(wc.hbrBackground)DeleteObject(wc.hbrBackground);
        return 10;
    }

    HWND hwnd=CreateWindowW(
        cls,
        L"CORE AI — Personal Intelligence",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,1000,710,
        nullptr,nullptr,h,nullptr);

    if(!hwnd){
        const DWORD err=GetLastError();
        wchar_t msg[256]{};
        wsprintfW(msg,L"CORE AI could not create its main window.\nWindows error: %lu",err);
        MessageBoxW(nullptr,msg,L"CORE AI — Startup Error",MB_OK|MB_ICONERROR);
        if(wc.hbrBackground)DeleteObject(wc.hbrBackground);
        return 11;
    }

    ShowWindow(hwnd,SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    while(true){
        const BOOL result=GetMessageW(&msg,nullptr,0,0);
        if(result==0)break;
        if(result==-1)break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if(g_font)DeleteObject(g_font);
    if(wc.hbrBackground)DeleteObject(wc.hbrBackground);
    return static_cast<int>(msg.wParam);
}
}
#endif
