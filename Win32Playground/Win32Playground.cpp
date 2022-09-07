#include "windows.h"
#include "Richedit.h"
#include "WindowsX.h"
#include "uxtheme.h"
#include "dwmapi.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <vector>
#include <regex>
#include "framework.h"
#include "Win32Playground.h"

#ifndef UNICODE
#define UNICODE
#endif

HDC hdc;
PAINTSTRUCT ps;

RECT cRect;

LONG width;
LONG height;

LONG vBorderControlsArea;
LONG vBorderExtrasArea;
LONG vBorderMainContent;

LONG vBorderControls;

LONG hBorderBackButton;
LONG hBorderForwardButton;
LONG hBorderRefreshButton;
LONG hBorderUrlBar;
LONG hBorderSearchButton;

LONG hBorderMinimize;
LONG hBorderMaximize;
LONG hBorderDestroyW;

bool maximized = false;

RECT newTab;
RECT tabsAreaRect;
RECT controlsAreaRect;
RECT extrasAreaRect;
RECT mainContentRect;

std::shared_ptr<HWND> window;

std::shared_ptr<HWND> minimizeButton;
std::shared_ptr<HWND> maximizeButton;
std::shared_ptr<HWND> destroyWButton;
std::shared_ptr<HWND> backButton;
std::shared_ptr<HWND> forwardButton;
std::shared_ptr<HWND> refreshButton;
std::shared_ptr<HWND> urlBar;
std::shared_ptr<HWND> searchButton;
std::shared_ptr<HWND> pageContent;

std::vector<std::shared_ptr<HWND>> windows { window };

std::vector<std::shared_ptr<HWND>> children { backButton, refreshButton, urlBar, searchButton, pageContent };

CURL* curl = curl_easy_init();

int historyDepth = 0;
std::vector<std::string> pageHistory;

std::string url;

std::string pageData;

template<typename... Args>
void println(std::string string, Args ...args)
{
	char output[2048];

	snprintf(output, 2048, string.c_str(), args...);

	OutputDebugStringA(output);
}

template<typename... Args>
void print(const char* string, Args ...args)
{
	char output[2048];

	snprintf(output, 2048, string, args...);

	OutputDebugStringA(output);
}

template<typename... Args>
void print(std::string string, Args ...args)
{
	char output[2048];

	snprintf(output, 2048, string.c_str(), args...);

	OutputDebugStringA(output);
	OutputDebugStringA("\n");
}

template<typename... Args>
void println(const char* string, Args ...args)
{
	char output[2048];

	snprintf(output, 2048, string, args...);

	OutputDebugStringA(output);
	OutputDebugStringA("\n");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::shared_ptr<HWND> CreateMinimizeButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateMaximizeButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateDestroyWButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateBackButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateRefreshButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateUrlBar(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateSearchButton(HWND hwndOwner, int x, int y, int width, int height);
std::shared_ptr<HWND> CreateContent(HWND hwndOwner);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"Light Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	window = std::make_shared<HWND>(CreateWindow(
		CLASS_NAME,
		L"Light",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL
	));

	if (window == NULL)
	{
		return 0;
	}

	SetWindowLong(*window, GWL_STYLE, 0);

	ShowWindow(*window, nCmdShow);
	
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

std::shared_ptr<HWND> CreateMinimizeButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light Minimize Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	minimizeButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"-",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_CENTER,
		x, y, width, height,
		hwndOwner, (HMENU) 69, NULL, NULL
	));

	return minimizeButton;
}

std::shared_ptr<HWND> CreateMaximizeButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light Maximize Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	maximizeButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"☐",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_CENTER,
		x, y, width, height,
		hwndOwner, (HMENU) 70, NULL, NULL
	));

	return maximizeButton;
}

std::shared_ptr<HWND> CreateDestroyWButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light DestroyW Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	destroyWButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"X",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_CENTER,
		x, y, width, height,
		hwndOwner, (HMENU) 71, NULL, NULL
	));

	return destroyWButton;
}

std::shared_ptr<HWND> CreateBackButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light Page Back Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	backButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"<",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 312, NULL, NULL
	));

	return backButton;
}

std::shared_ptr<HWND> CreateForwardButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light Page Forward Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	forwardButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L">",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 624, NULL, NULL
	));

	return forwardButton;
}

std::shared_ptr<HWND> CreateRefreshButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light Page Refresh Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	refreshButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"↺",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 1662, NULL, NULL
	));

	return refreshButton;
}

std::shared_ptr<HWND> CreateUrlBar(HWND hwndOwner, int x, int y, int width, int height)
{
	LoadLibrary(TEXT("Msftedit.dll"));

	const wchar_t CLASS_NAME[] = L"Light URL RichEdit";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	urlBar = std::make_shared<HWND> (CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		L"Type a URL",
		ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
		x, y, width, height,
		hwndOwner, (HMENU)102, NULL, NULL
	));

	return urlBar;
}

std::shared_ptr<HWND> CreateSearchButton(HWND hwndOwner, int x, int y, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"Light URL Submit Button";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	searchButton = std::make_shared<HWND> (CreateWindow(
		L"BUTTON",
		L"GO",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
		x, y, width, height,
		hwndOwner, (HMENU) 831, NULL, NULL
	));

	return searchButton;
}

std::shared_ptr<HWND> CreateContent(HWND hwndOwner)
{
	const wchar_t CLASS_NAME[] = L"Light Page Content";

	WNDCLASS wc = { };

	wc.lpszClassName = CLASS_NAME;
	wc.lpfnWndProc = WindowProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	pageContent = std::make_shared<HWND> (CreateWindow(
		L"STATIC",
		L"Enter a URL into the address bar and hit go to begin exploring the web!",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		width * 0.005L, vBorderMainContent + height * 0.025L * (historyDepth + 1L), width * 0.995L, height * 0.995L,
		hwndOwner, (HMENU) 519, NULL, NULL
	));

	return pageContent;
}

size_t RequestData(char* buffer, size_t itemSize, size_t nItems, void* empty)
{
	size_t nBytes = itemSize * nItems;

	((std::string*) &pageData)->append(buffer, nBytes);

	println("Buffer received");

	return nBytes;
}

CURLcode ProcessResult(CURLcode result)
{
	if (result == CURLE_OK)
	{
		println("successfully did curl stuff");

		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkColor(hdc, RGB(255, 255, 255));

		HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		HFONT oldHFont = (HFONT) SelectObject(hdc, font);

		if (historyDepth == 1)
		{
			CreateContent(*window);
		}

		SetWindowTextA(*pageContent, pageData.c_str());
	}
	else
	{
		println("unsuccessfully did curl stuff");
		println(curl_easy_strerror(result));
	}

	return result;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
			RECT rc;
			GetClientRect(hwnd, &rc);
			InvalidateRect(hwnd, NULL, FALSE);

			return 0;
		case WM_DESTROY:
		{
			PostQuitMessage(0);

			curl_easy_cleanup(curl);

			return 0;
		}
		case WM_CREATE:
		{
			windows.push_back(std::make_shared<HWND>(hwnd));

			hdc = BeginPaint(hwnd, &ps);

			GetWindowRect(hwnd, &cRect);
			
			SetWindowPos(
				hwnd,
				NULL,
				cRect.left, cRect.top,
				cRect.right - cRect.left, cRect.bottom - cRect.top,
				SWP_FRAMECHANGED
			);

			SetMapMode(hdc, MM_ANISOTROPIC);
			SetWindowExtEx(hdc, 100, 100, NULL);
			SetViewportExtEx(hdc, cRect.right, cRect.bottom, NULL);

			width = cRect.right - cRect.left;
			height = cRect.bottom - cRect.top;

			// @TODO - Find better way to calculate borders
			// Maybe give each a "weight" number along each axis,
			// then divide each by the total weight along that axis

			vBorderControlsArea = 0 +
				0.03L * height;
			vBorderExtrasArea = vBorderControlsArea +
				0.05L * height;
			vBorderMainContent = vBorderExtrasArea +
				0.04L * height;

			vBorderControls = vBorderControlsArea +
				0.1L * (vBorderExtrasArea - vBorderControlsArea);

			hBorderBackButton = 0 +
				0.02L * width;
			hBorderForwardButton = hBorderBackButton +
				0.02L * width;
			hBorderRefreshButton = hBorderForwardButton +
				0.02L * width;
			hBorderUrlBar = hBorderRefreshButton +
				0.02L * width;
			hBorderSearchButton = hBorderUrlBar +
				0.85L * width;

			hBorderDestroyW = width - 
				0.03L * width;
			hBorderMaximize = hBorderDestroyW -
				0.03L * width;
			hBorderMinimize = hBorderMaximize -
				0.03L * width;

			CreateMinimizeButton(hwnd, hBorderMinimize, 0L, 0.03L * width, vBorderControlsArea);
			CreateMaximizeButton(hwnd, hBorderMaximize, 0L, 0.03L * width, vBorderControlsArea);
			CreateDestroyWButton(hwnd, hBorderDestroyW, 0L, 0.03L * width, vBorderControlsArea);

			CreateBackButton(
				hwnd,
				hBorderBackButton,
				vBorderControls,
				0.02L * width,
				0.8L * (vBorderExtrasArea - vBorderControlsArea)
			);

			CreateForwardButton(
				hwnd,
				hBorderForwardButton,
				vBorderControls,
				0.02L * width,
				0.8L * (vBorderExtrasArea - vBorderControlsArea)
			);

			CreateRefreshButton(
				hwnd,
				hBorderRefreshButton,
				vBorderControls,
				0.02L * width,
				0.8L * (vBorderExtrasArea - vBorderControlsArea)
			);
			CreateUrlBar(
				hwnd,
				hBorderUrlBar,
				vBorderControls,
				0.85 * width,
				0.8L * (vBorderExtrasArea - vBorderControlsArea)
			);

			CreateSearchButton(
				hwnd,
				hBorderSearchButton,
				vBorderControls,
				0.05L * width,
				0.8L * (vBorderExtrasArea - vBorderControlsArea)
			);

			pageHistory.push_back("light://newtab");

			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(*window, &ps);

			FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(217, 89, 140)));

			SetRect(&tabsAreaRect, 0, 0, width, vBorderControlsArea);
			FillRect(hdc, &tabsAreaRect, CreateSolidBrush(RGB(42, 42, 42)));

			SetRect(&newTab, 0.01 * width, 0, 0.08 * width, vBorderControlsArea);
			FillRect(hdc, &newTab, CreateSolidBrush(RGB(26, 26, 26)));

			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkColor(hdc, RGB(26, 26, 26));
			SetTextAlign(hdc, TA_BOTTOM);

			HFONT font = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
			HFONT oldHFont = (HFONT) SelectObject(hdc, font);

			if (historyDepth == 0)
			{
				TextOut(hdc, 0.015 * width, 0.75 * vBorderControlsArea, L"New Tab", 7);
			}

			SetRect(&controlsAreaRect, 0, vBorderControlsArea, width, vBorderExtrasArea);
			FillRect(hdc, &controlsAreaRect, CreateSolidBrush(RGB(28, 28, 28)));

			SetRect(&extrasAreaRect, 0, vBorderExtrasArea, width, vBorderMainContent);
			FillRect(hdc, &extrasAreaRect, CreateSolidBrush(RGB(31, 31, 31)));

			SetRect(&mainContentRect, 0, vBorderMainContent, width, height);
			FillRect(hdc, &mainContentRect, CreateSolidBrush(RGB(255, 255, 255)));

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_COMMAND:
		{
			if (wParam == 69)
			{
				CloseWindow(hwnd);
			}
			else if (wParam == 70)
			{
				InvalidateRect(hwnd, NULL, TRUE);

				if (maximized)
				{
					ShowWindow(hwnd, SW_MINIMIZE);
				}
				else
				{
					ShowWindow(hwnd, SW_MAXIMIZE);
				}

				InvalidateRect(hwnd, NULL, TRUE);

				UpdateWindow(hwnd);

				RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

				SendMessage(hwnd, WM_PAINT, 0, 0);
				SendMessage(hwnd, WM_ERASEBKGND, 0, 0);

				// @TODO - Implement repaint on resize

				maximized = !maximized;
			}
			else if (wParam == 71)
			{
				DestroyWindow(hwnd);
			}
			else if (wParam == 312 || wParam == 831)
			{ // Pressed either the back button or the search button
				if (wParam == 312)
				{ // Pressed the back button
					if (historyDepth > 0)
					{ // Page to go back to exists
						println("\n----- New Message: Go back! -----");

						// @TODO - Back button needs to be pressed twice to go back (is historyDepth being incremented twice somewhere?)

						println("Going from %d (%s) to %d (%s)", historyDepth, pageHistory[historyDepth], historyDepth - 1, pageHistory[historyDepth - 1]);

						historyDepth = historyDepth - 1;

						int targetUrlLength = pageHistory[historyDepth].length();

						WCHAR targetUrl[2048];

						for (int i = 0; i < targetUrlLength; i++)
						{
							targetUrl[i] = pageHistory[historyDepth][i];
						}

						SetWindowTextA(*urlBar, pageHistory[historyDepth].c_str());

					}
					else
					{ // Currently at end of browsing history for current tab
						println("Reached end of browsing history!");

						return 0;
					}
				}
				else
				{ // Pressed the search button
					println("\n----- New Message: Search! -----");

					

					println("Going from %d (%s) to %d", historyDepth, pageHistory[historyDepth], historyDepth + 1);

					historyDepth = historyDepth + 1;
				}

				WCHAR tempUrl[2048];

				int urlLength = GetWindowTextW(*urlBar, (LPWSTR) tempUrl, 2048) + 1;

				OutputDebugStringW(L"Url: '");
				OutputDebugStringW(tempUrl);
				OutputDebugStringW(L"'\n");

				url.clear();

				for (int i = 0; i < urlLength; i++)
				{
					url += tempUrl[i];
				}

				println(url);

				std::string urlArchive = url;

				pageHistory.push_back(urlArchive);

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RequestData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pageData);

				pageData.clear();

				if (ProcessResult(curl_easy_perform(curl)) != CURLE_OK)
				{ // cURL Unsuccessful
					if (wParam == 312)
					{ // cURL for back button Unsuccessful
						println("Failed to go back!");

						historyDepth++;
					}
					else
					{ // cURL for search button Unsuccessful
						println("Failed to navigate!");

						historyDepth--;
					}
				}
			}
			else if (wParam == 1662)
			{
				println("Refresh the page!");

				pageData.clear();

				ProcessResult(curl_easy_perform(curl));
			}

			return 0;
		}
		case WM_NCHITTEST:
		{
			LONG x_lParam = GET_X_LPARAM(lParam);
			LONG y_lParam = GET_Y_LPARAM(lParam);

			POINT clickPoint[1];
			clickPoint[0] = tagPOINT{ x_lParam, y_lParam };

			MapWindowPoints(HWND_DESKTOP, hwnd, clickPoint, 1);

			LONG x = clickPoint->x;
			LONG y = clickPoint->y;
			
			if (y < vBorderControlsArea && x < hBorderMinimize)
			{
				return HTCAPTION;
			}

			return 0;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}