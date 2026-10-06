#ifdef _WIN32
#include "services/CoreApp.h"
#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
static coreai::CoreApp* g_app=nullptr; static HWND g_input=nullptr,g_output=nullptr,g_status=nullptr,g_send=nullptr,g_think=nullptr; static std::atomic_bool g_busy{false}; static std::atomic_int g_tick{0}; static std::mutex g_guiMutex;
static std::wstring toW(const std::string&s){int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0);std::wstring o(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),o.data(),n);return o;}
static std::string toA(const std::wstring&s){int n=WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0,nullptr,nullptr);std::string o(n,'\0');WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),o.data(),n,nullptr,nullptr);return o;}
static void setStatus(const std::wstring&s){if(g_status)SetWindowTextW(g_status,s.c_str());}
static void setBusy(bool busy){g_busy=busy;EnableWindow(g_send,!busy);EnableWindow(g_input,!busy);if(g_think)EnableWindow(g_think,!busy);}
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_CREATE){
        g_status=CreateWindowW(L"STATIC",L"Ready",WS_CHILD|WS_VISIBLE,20,20,500,25,h,(HMENU)4,GetModuleHandle(nullptr),nullptr);
        g_input=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOVSCROLL|ES_MULTILINE,20,55,650,80,h,(HMENU)1,GetModuleHandle(nullptr),nullptr);
        g_think=CreateWindowW(L"BUTTON",L"Think",WS_CHILD|WS_VISIBLE,680,55,90,38,h,(HMENU)5,GetModuleHandle(nullptr),nullptr);
        g_send=CreateWindowW(L"BUTTON",L"Send",WS_CHILD|WS_VISIBLE,780,55,90,38,h,(HMENU)2,GetModuleHandle(nullptr),nullptr);
        g_output=CreateWindowW(L"EDIT",L"CORE-AI\r\nReady.\r\n",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_VSCROLL,20,150,850,430,h,(HMENU)3,GetModuleHandle(nullptr),nullptr);
        SetTimer(h,99,120,nullptr);return 0;
    }
    if(m==WM_TIMER&&w==99){if(g_busy){int n=++g_tick%4;std::wstring dots(n,L'.');setStatus(L"CORE is working"+dots+L"  [active]");}else setStatus(L"Ready");return 0;}
    if(m==WM_COMMAND&&LOWORD(w)==2&&HIWORD(w)==BN_CLICKED){
        if(g_busy)return 0;wchar_t b[8192]{};GetWindowTextW(g_input,b,8192);auto prompt=toA(b);if(prompt.empty())return 0;setBusy(true);setStatus(L"CORE is working...\r\nNavigating obstacles...");
        std::thread([h,prompt](){auto r=g_app->chat(prompt,"chat");std::wstring out=r.ok()?toW(r.value()):toW(std::string("ERROR: ") + r.error().message);PostMessageW(h,WM_APP+1,0,(LPARAM)new std::wstring(std::move(out)));}).detach();return 0;
    }
    if(m==WM_APP+1){std::unique_ptr<std::wstring> out((std::wstring*)l);SetWindowTextW(g_output,out->c_str());setBusy(false);return 0;}
    if(m==WM_DESTROY){KillTimer(h,99);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);
}
int runNativeGui(coreai::CoreApp& app){g_app=&app;HINSTANCE hi=GetModuleHandle(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=WndProc;wc.hInstance=hi;wc.lpszClassName=L"COREAIWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);HWND h=CreateWindowW(L"COREAIWindow",L"CORE-AI — Personal Intelligence",WS_OVERLAPPEDWINDOW,100,100,930,670,nullptr,nullptr,hi,nullptr);ShowWindow(h,SW_SHOW);UpdateWindow(h);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;}
#else
int runNativeGui(coreai::CoreApp&){return 0;}
#endif
