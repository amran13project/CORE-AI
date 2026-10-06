#ifdef _WIN32
#include "services/CoreApp.h"
#include <windows.h>
#include <string>
static coreai::CoreApp* g_app=nullptr; static HWND g_input=nullptr,g_output=nullptr,g_status=nullptr;
static std::wstring toW(const std::string&s){int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0);std::wstring o(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),o.data(),n);return o;}
static std::string toA(const std::wstring&s){int n=WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0,nullptr,nullptr);std::string o(n,'\0');WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),o.data(),n,nullptr,nullptr);return o;}
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_CREATE){g_input=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOVSCROLL|ES_MULTILINE,20,55,700,90,h,(HMENU)1,GetModuleHandle(nullptr),nullptr);CreateWindowW(L"BUTTON",L"ASK CORE",WS_CHILD|WS_VISIBLE,735,55,150,40,h,(HMENU)2,GetModuleHandle(nullptr),nullptr);g_output=CreateWindowW(L"EDIT",L"CORE-AI\r\nReady.\r\n",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_VSCROLL,20,165,865,430,h,(HMENU)3,GetModuleHandle(nullptr),nullptr);g_status=CreateWindowW(L"STATIC",L"Local-first",WS_CHILD|WS_VISIBLE,20,20,700,25,h,(HMENU)4,GetModuleHandle(nullptr),nullptr);return 0;}if(m==WM_COMMAND&&LOWORD(w)==2){wchar_t b[8192]{};GetWindowTextW(g_input,b,8192);auto r=g_app->chat(toA(b),"chat");std::wstring out=r.ok()?toW(r.value()):toW("ERROR: "+r.error().message);SetWindowTextW(g_output,out.c_str());return 0;}if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
int runNativeGui(coreai::CoreApp& app){g_app=&app;HINSTANCE hi=GetModuleHandle(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=WndProc;wc.hInstance=hi;wc.lpszClassName=L"COREAIWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);HWND h=CreateWindowW(L"COREAIWindow",L"CORE-AI — Personal Intelligence",WS_OVERLAPPEDWINDOW,100,100,930,670,nullptr,nullptr,hi,nullptr);ShowWindow(h,SW_SHOW);UpdateWindow(h);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;}
#else
int runNativeGui(coreai::CoreApp&){return 0;}
#endif
