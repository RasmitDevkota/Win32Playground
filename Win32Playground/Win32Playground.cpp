#include "windows.h"
#include "Richedit.h"
#include "curl/curl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <regex>
#include "framework.h"
#include "Win32Playground.h"
#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

HDC hdc;
PAINTSTRUCT ps;

RECT cRect;

LONG width;
LONG height;

LONG vBorderA;
LONG vBorderB;
LONG vBorderC;

LONG hBorderA;
LONG hBorderB;
LONG hBorderC;
LONG hBorderD;

RECT newTab;
RECT tabsAreaRect;
RECT controlsAreaRect;
RECT extrasAreaRect;
RECT mainContentRect;

HWND window;

HWND backButton;
HWND refreshButton;
HWND urlBar;
HWND searchButton;
HWND pageContent;

CURL* curl = curl_easy_init();

int historyDepth = 0;
std::vector<std::string> pageHistory;

std::string url;

std::string pageData;

void print(std::string stringToPrint) {
	OutputDebugStringA(stringToPrint.c_str());
}

void println(std::string stringToPrint) {
	OutputDebugStringA(stringToPrint.c_str());
	OutputDebugStringA("\n");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateUrlBar(HWND hwndOwner, int x, int y, int width, int height);
HWND CreateSearchButton(HWND hwndOwner, int x, int y, int width, int height);

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

	SetWindowLong(window, GWL_STYLE, 0);

	ShowWindow(window, nCmdShow);
	
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

HWND CreateUrlBar(HWND hwndOwner, int x, int y, int width, int height) {
	LoadLibrary(TEXT("Msftedit.dll"));

	const wchar_t CLASS_NAME[] = L"OneE URL RichEdit";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	urlBar = CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		L"Type a URL",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
		x, y, width, height,
		hwndOwner, (HMENU) 102, NULL, NULL
	);

	return urlBar;
}

HWND CreateSearchButton(HWND hwndOwner, int x, int y, int width, int height) {
	const wchar_t CLASS_NAME[] = L"OneE URL Submit Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	searchButton = CreateWindow(
		L"BUTTON",
		L"GO",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 831, NULL, NULL
	);

	return searchButton;
}

HWND CreateBackButton(HWND hwndOwner, int x, int y, int width, int height) {
	const wchar_t CLASS_NAME[] = L"OneE Page Back Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	backButton = CreateWindow(
		L"BUTTON",
		L"<",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 312, NULL, NULL
	);

	return refreshButton;
}

HWND CreateRefreshButton(HWND hwndOwner, int x, int y, int width, int height) {
	const wchar_t CLASS_NAME[] = L"OneE Page Refresh Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	refreshButton = CreateWindow(
		L"BUTTON",
		L"↺",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 690561, NULL, NULL
	);

	return refreshButton;
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
		width * 0.005L, vBorderC + height * 0.025L * (historyDepth + 1L), width * 0.995L, height * 0.995L,
		hwndOwner, (HMENU) 519, NULL, NULL
	);

	return pageContent;
}

size_t requestData(char* buffer, size_t itemSize, size_t nItems, void* empty) {
	size_t nBytes = itemSize * nItems;

	((std::string*) &pageData)->append(buffer, nBytes);

	println("Buffer received");

	return nBytes;
}

CURLcode processResult(CURLcode result) {
	// println(pageData);

	if (result == CURLE_OK) {
		println("successfully did curl stuff");

		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkColor(hdc, RGB(255, 255, 255));

		HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		HFONT oldHFont = (HFONT) SelectObject(hdc, font);

		if (historyDepth == 1) {
			CreateContent(window);
		}

		SetWindowTextA(pageContent, pageData.c_str());
	} else {
		println("unsuccessfully did curl stuff");
		println(curl_easy_strerror(result));
	}

	return result;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY: {
			PostQuitMessage(0);

			curl_easy_cleanup(curl);

			return 0;
		}
		case WM_CREATE: {
			hdc = BeginPaint(hwnd, &ps);

			GetClientRect(hwnd, &cRect);

			width = cRect.right - cRect.left;
			height = cRect.bottom - cRect.top;

			vBorderA = height * 0.05L;
			vBorderB = vBorderA + height * 0.05L;
			vBorderC = vBorderB + height * 0.04L;

			hBorderA = 0.02L * width;
			hBorderB = hBorderA + 0.02L * width;
			hBorderC = hBorderB + 0.02L * width;
			hBorderD = hBorderC + 0.85L * width;

			CreateBackButton(hwnd, hBorderA, vBorderA + 0.1L * (vBorderB - vBorderA), 0.02L * width, 0.8L * (vBorderB - vBorderA));
			CreateRefreshButton(hwnd, hBorderB, vBorderA + 0.1L * (vBorderB - vBorderA), 0.02L * width, 0.8L * (vBorderB - vBorderA));
			CreateUrlBar(hwnd, hBorderC, vBorderA + 0.1L * (vBorderB - vBorderA), 0.85 * width, 0.8L * (vBorderB - vBorderA));
			CreateSearchButton(hwnd, hBorderD, vBorderA + 0.1L * (vBorderB - vBorderA), 0.05L * width, 0.8L * (vBorderB - vBorderA));

			// pageHistory.push_back('Type a URL');

			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_PAINT: {
			hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(217, 89, 140)));

			SetRect(&tabsAreaRect, 0, 0, width, vBorderA);
			FillRect(hdc, &tabsAreaRect, CreateSolidBrush(RGB(42, 42, 42)));

			SetRect(&newTab, 0.01 * width, 0, 0.08 * width, vBorderA);
			FillRect(hdc, &newTab, CreateSolidBrush(RGB(26, 26, 26)));

			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkColor(hdc, RGB(26, 26, 26));
			SetTextAlign(hdc, TA_BOTTOM);

			HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
			HFONT oldHFont = (HFONT) SelectObject(hdc, font);

			if (historyDepth == 0) {
				TextOut(hdc, 0.015 * width, 0.75 * vBorderA, L"New Tab", 7);
			}

			SetRect(&controlsAreaRect, 0, vBorderA, width, vBorderB);
			FillRect(hdc, &controlsAreaRect, CreateSolidBrush(RGB(28, 28, 28)));

			SetRect(&extrasAreaRect, 0, vBorderB, width, vBorderC);
			FillRect(hdc, &extrasAreaRect, CreateSolidBrush(RGB(31, 31, 31)));

			SetRect(&mainContentRect, 0, vBorderC, width, height);
			FillRect(hdc, &mainContentRect, CreateSolidBrush(RGB(255, 255, 255)));

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_COMMAND: {
			if (wParam == 831 || wParam == 312) {
				if (wParam == 312) {
					if (historyDepth > 0) {
						println("\n----- New Message: Go back! -----");

						historyDepth = historyDepth - 1; // Bug: Back button needs to be pressed twice to go back (is historyDepth being incremented twice somewhere?)

						int targetUrlLength = pageHistory[historyDepth].length();

						WCHAR targetUrl[2048];

						for (int i = 0; i < targetUrlLength; i++) {
							targetUrl[i] = pageHistory[historyDepth][i];
						}

						SetWindowTextA(urlBar, pageHistory[historyDepth].c_str());

						OutputDebugStringA("Going back to '");
						OutputDebugStringA(pageHistory[historyDepth].c_str());
						OutputDebugStringA("'\n");

					} else {
						println("Reached end of browsing history!");

						return 0;
					}
				} else {
					println("\n----- New Message: Search! -----");

					historyDepth = historyDepth + 1;
				}

				WCHAR tempUrl[2048];

				int urlLength = GetWindowTextW(urlBar, (LPWSTR) tempUrl, 2048) + 1;

				OutputDebugStringW(L"Url: '");
				OutputDebugStringW(tempUrl);
				OutputDebugStringW(L"'\n");

				url.clear();

				for (int i = 0; i < urlLength; i++) {
					url += tempUrl[i];
				}

				println(url);

				std::string urlArchive = url;

				pageHistory.push_back(urlArchive);

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, requestData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pageData);

				pageData.clear();

				if (processResult(curl_easy_perform(curl)) != CURLE_OK) {
					if (wParam == 312) {
						println("Failed to go back!");

						historyDepth++;
					} else {
						println("Failed to navigate!");

						historyDepth--;
					}
				}
			} else if (wParam == 690561) {
				println("Refresh the page!");

				pageData.clear();

				processResult(curl_easy_perform(curl));
			}

			return 0;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}