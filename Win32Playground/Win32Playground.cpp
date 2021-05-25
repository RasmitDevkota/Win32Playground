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

int historyDepth = 0;

HDC hdc;
PAINTSTRUCT ps;

RECT cRect;

LONG width;
LONG height;

LONG borderA;
LONG borderB;
LONG borderC;

HWND window;

RECT newTab;
RECT tabsAreaRect;
RECT controlsAreaRect;
RECT extrasAreaRect;
RECT mainContentRect;

HWND urlBar;
HWND searchButton;

WCHAR* url[2048] = {};

std::string pageData;

HWND pageContent;

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

    window = CreateWindow(
        CLASS_NAME,
        L"OneE",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (window == NULL)
    {
        return 0;
    }

    ShowWindow(window, nCmdShow);
    
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
        L"Type a URL",
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

HWND CreateContent(HWND hwndOwner) {
    const wchar_t CLASS_NAME[] = L"OneE Page Content";

    WNDCLASS wc = { };

    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    pageContent = CreateWindow( 
        L"STATIC",
        L"Enter a URL into the address bar and hit go to begin exploring the web!",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        width * 0.005L, borderC + height * 0.025L * (historyDepth + 1L), width * 0.995L, height * 0.995L,
        hwndOwner, (HMENU) 519, NULL, NULL
    );

    return pageContent;
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
            hdc = BeginPaint(hwnd, &ps);

            GetClientRect(hwnd, &cRect);

            width = cRect.right - cRect.left;
            height = cRect.bottom - cRect.top;

            borderA = height * 0.05L;
            borderB = borderA + height * 0.05L;
            borderC = borderB + height * 0.04L;

            urlBar = CreateRichEdit(hwnd, 0.05L * width, borderA + 0.1L * (borderB - borderA), 0.85L * width, 0.8L * (borderB - borderA));
            searchButton = CreateButton(hwnd, 0.9L * width, borderA + 0.1L * (borderB - borderA), 0.05L * width, 0.8L * (borderB - borderA));

            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_PAINT: {
            hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(217, 89, 140)));

            SetRect(&tabsAreaRect, 0, 0, width, borderA);
            FillRect(hdc, &tabsAreaRect, CreateSolidBrush(RGB(42, 42, 42)));

            SetRect(&newTab, 0.01 * width, 0, 0.08 * width, borderA);
            FillRect(hdc, &newTab, CreateSolidBrush(RGB(26, 26, 26)));

            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(26, 26, 26));
            SetTextAlign(hdc, TA_BOTTOM);

            HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
            HFONT oldHFont = (HFONT) SelectObject(hdc, font);

            if (historyDepth == 0) {
                TextOut(hdc, 0.015 * width, 0.75 * borderA, L"New Tab", 7);
            }

            SetRect(&controlsAreaRect, 0, borderA, width, borderB);
            FillRect(hdc, &controlsAreaRect, CreateSolidBrush(RGB(28, 28, 28)));

            SetRect(&extrasAreaRect, 0, borderB, width, borderC);
            FillRect(hdc, &extrasAreaRect, CreateSolidBrush(RGB(31, 31, 31)));

            SetRect(&mainContentRect, 0, borderC, width, height);
            FillRect(hdc, &mainContentRect, CreateSolidBrush(RGB(255, 255, 255)));

            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_COMMAND: {
            if (wParam == 831) {
                GetWindowTextA(urlBar, (LPSTR) url, 2048);

                CURL* curl = curl_easy_init();

                curl_easy_setopt(curl, CURLOPT_URL, url);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, requestData);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pageData);

                ((std::string*) &pageData)->clear();

                CURLcode result = curl_easy_perform(curl);

                if (result == CURLE_OK) {
                    OutputDebugString(L"successfully did curl stuff\n");

                    SetTextColor(hdc, RGB(0, 0, 0));
                    SetBkColor(hdc, RGB(255, 255, 255));

                    HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
                    HFONT oldHFont = (HFONT) SelectObject(hdc, font);

                    if (historyDepth == 0) {
                        CreateContent(hwnd);
                    }

                    SetWindowTextA(pageContent, pageData.c_str());

                    historyDepth++;
                }
                else {
                    OutputDebugString(L"unsuccessfully did curl stuff\n");
                    OutputDebugStringA(curl_easy_strerror(result));
                    OutputDebugString(L"\n");
                }

                curl_easy_cleanup(curl);
            }

            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}