#include "windows.h"
#include "Richedit.h"
#include "curl/curl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "framework.h"
#include "Win32Playground.h"

#ifndef UNICODE
#define UNICODE
#endif

HWND urlBar;
HWND searchButton;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateRichEdit(HWND hwndOwner, int x, int y, int width, int height);
HWND CreateButton(HWND hwndOwner, int x, int y, int width, int height);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"OneE Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        L"OneE",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

HWND CreateRichEdit(HWND hwndOwner, int x, int y, int width, int height) {
    LoadLibrary(TEXT("Msftedit.dll"));

    const wchar_t CLASS_NAME[] = L"OneE URL RichEdit";

    WNDCLASS wc = { };

    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        MSFTEDIT_CLASS,
        TEXT("Type a URL"),
        ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
        x, y, width, height,
        hwndOwner, (HMENU) 102, NULL, NULL
    );

    return hwnd;
}

HWND CreateButton(HWND hwndOwner, int x, int y, int width, int height) {
    const wchar_t CLASS_NAME[] = L"OneE URL Submit Button";

    WNDCLASS wc = { };

    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        L"BUTTON",
        L"GO",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
        x, y, width, height,
        hwndOwner, (HMENU) 831, NULL, NULL
    );

    return hwnd;
}

size_t requestData(char* buffer, size_t itemSize, size_t nItems, void* pageData) {
    size_t nBytes = itemSize * nItems;
    
    OutputDebugStringA(buffer);
    OutputDebugString(L"\n");

    ((std::string*) pageData)->append(buffer, nBytes);

    return nBytes;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY: {
            PostQuitMessage(0);

            return 0;
        }
        case WM_CREATE: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT cRect;
            GetClientRect(hwnd, &cRect);

            LONG width = cRect.right - cRect.left;
            LONG height = cRect.bottom - cRect.top;

            LONG borderA = height * 0.05L;
            LONG borderB = borderA + height * 0.05L;
            LONG borderC = borderB + height * 0.04L;

            urlBar = CreateRichEdit(hwnd, 0.05L * width, borderA + 0.1L * (borderB - borderA), 0.85L * width, 0.8L * (borderB - borderA));
            searchButton = CreateButton(hwnd, 0.9L * width, borderA + 0.1L * (borderB - borderA), 0.05L * width, 0.8L * (borderB - borderA));

            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(217, 89, 140)));

            RECT cRect;
            GetClientRect(hwnd, &cRect);

            LONG width = cRect.right - cRect.left;
            LONG height = cRect.bottom - cRect.top;

            LONG borderA = height * 0.05L;
            LONG borderB = borderA + height * 0.05L;
            LONG borderC = borderB + height * 0.04L;

            RECT tabsAreaRect;
            SetRect(&tabsAreaRect, 0, 0, width, borderA);
            FillRect(hdc, &tabsAreaRect, CreateSolidBrush(RGB(42, 42, 42)));

            RECT newTab;
            SetRect(&newTab, 0.01 * width, 0, 0.08 * width, borderA);
            FillRect(hdc, &newTab, CreateSolidBrush(RGB(26, 26, 26)));

            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(26, 26, 26));
            SetTextAlign(hdc, TA_BOTTOM);

            HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            HFONT oldHFont = (HFONT) SelectObject(hdc, font);

            TextOut(hdc, 0.015 * width, 0.75 * borderA, L"New Tab", 7);

            RECT controlsAreaRect;
            SetRect(&controlsAreaRect, 0, borderA, width, borderB);
            FillRect(hdc, &controlsAreaRect, CreateSolidBrush(RGB(28, 28, 28)));

            RECT extrasAreaRect;
            SetRect(&extrasAreaRect, 0, borderB, width, borderC);
            FillRect(hdc, &extrasAreaRect, CreateSolidBrush(RGB(31, 31, 31)));

            RECT mainContentRect;
            SetRect(&mainContentRect, 0, borderC, width, height);
            FillRect(hdc, &mainContentRect, CreateSolidBrush(RGB(255, 255, 255)));

            std::string pageData;

            CURL* curl = curl_easy_init();

            curl_easy_setopt(curl, CURLOPT_URL, "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=AIzaSyBVT22t-x2H76119AHG8SgPU0_A0U-N1uA\0");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "email=rdatch101@gmail.com&password=jangwonyoung&returnSecureToken=true");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, requestData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pageData);

            CURLcode result = curl_easy_perform(curl);

            if (result == CURLE_OK) {
                OutputDebugString(L"successfully did curl stuff\n");

                SetTextColor(hdc, RGB(0, 0, 0));
                SetBkColor(hdc, RGB(255, 255, 255));

                HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
                HFONT oldHFont = (HFONT) SelectObject(hdc, font);

                // TextOutA(hdc, width * 0.005L, borderC + height * 0.025L, pageData.c_str(), pageData.size());

                // DrawTextExA(hdc, (LPSTR) pageData.c_str(), pageData.size(), &contentRect, DT_CALCRECT, NULL);

                RECT contentArea;
                SetRect(&contentArea, width * 0.025L, width * 0.975L, borderC + height * 0.025L, height * 0.975L);
                ExtTextOutA(hdc, width * 0.025L, borderC + height * 0.025L, ETO_CLIPPED, &contentArea, (LPSTR) pageData.c_str(), pageData.size(), NULL);

                OutputDebugString(L"drew curl stuff\n");

                EndPaint(hwnd, &ps);
            }
            else {
                OutputDebugString(L"unsuccessfully did curl stuff\n");
                OutputDebugStringA(curl_easy_strerror(result));
                OutputDebugString(L"\n");
            }

            curl_easy_cleanup(curl);

            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_COMMAND: {
            if (wParam == 831) {
                OutputDebugStringA("Button pressed");

                WCHAR* url[2048];

                GetWindowTextA(urlBar, (LPSTR) url, 2048);

                OutputDebugStringA((LPSTR) url);
            }
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}