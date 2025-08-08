#define _WIN32_WINNT 0x0500
#include <windows.h>
#include "resource.h"

HINSTANCE hInst;
HWND hMain, hConvert;
char szFile[MAX_PATH];
BITMAPFILEHEADER* pbminfhead;
BITMAPINFO* pbmpinf;
BYTE* pbitsbmp = 0;
long cxDib, cyDib;
HANDLE hHeap;
char *szAppName = "GrayScale";
char *szInfo = "L'image ne peut pas être convertie!\n"
               "Ce programme n'accepte que\ndes fichiers bitmap 24 bits (2^24 couleurs).";

void DrawGrayScale()
{
	int i;
	BYTE r, g, b, y;
	for(i = 0; i < cxDib * cyDib; i++) {
		b = pbitsbmp[i * 3]; // Blue
		g = pbitsbmp[i * 3 + 1]; // Green
		r = pbitsbmp[i * 3 + 2]; // Red
		y = (306 * r + 601 * g + 117 * b) >> 10; // 0.299 * r + 0.587 * g + 0.114 * b
		pbitsbmp[i * 3] = y;
		pbitsbmp[i * 3 + 1] = y;
		pbitsbmp[i * 3 + 2] = y;		
	}
}

void LoadBmp()
{
	HANDLE hFile;
	DWORD  dwFileSize, dwRead;
	hFile = CreateFile(szFile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE) return;
	dwFileSize = GetFileSize(hFile, 0);
  pbminfhead = (BITMAPFILEHEADER *) HeapAlloc(hHeap, 0, dwFileSize);
  if(pbminfhead == 0) goto flClose;
	dwRead = 0; ReadFile(hFile, pbminfhead, dwFileSize, &dwRead, 0);
  if((dwRead != dwFileSize) || (pbminfhead->bfType != 0x4D42) // 0x4D42 = 'BM'
     || (pbminfhead->bfSize != dwFileSize)) {
    HeapFree(hHeap, 0, pbminfhead);
    pbminfhead = 0; goto flClose;
  }
	pbmpinf = (BITMAPINFO*) (pbminfhead + 1);
	pbitsbmp = (BYTE*) pbminfhead + pbminfhead->bfOffBits;
  if(pbmpinf->bmiHeader.biSize == sizeof(BITMAPCOREHEADER)) {
    cxDib = ((BITMAPCOREHEADER*) pbmpinf)->bcWidth;
    cyDib = ((BITMAPCOREHEADER*) pbmpinf)->bcHeight;
  }
  else {
    cxDib = pbmpinf->bmiHeader.biWidth;
    cyDib = pbmpinf->bmiHeader.biHeight;
  }
	if(pbmpinf->bmiHeader.biBitCount != 24) {
		EnableWindow(hConvert, 0);
		MessageBox(hMain, szInfo, szAppName, MB_ICONEXCLAMATION);
	}
	else EnableWindow(hConvert, 1);
flClose: CloseHandle(hFile);
}

int dlgSelectBitmap(HWND hOwner)
{
  OPENFILENAME ofn;
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hInstance = 0; ofn.hwndOwner = hOwner;
  ofn.lpstrFilter = "BITMAP(*.bmp)\0*.bmp\0\0";
	ofn.lpstrFile = szFile;
	ofn.lpstrCustomFilter = ofn.lpstrFileTitle = 0;
  ofn.nFileExtension = ofn.nFileOffset = 0;
  ofn.lCustData = ofn.dwReserved = 0;
  ofn.lpTemplateName = ofn.lpstrInitialDir = ofn.lpstrDefExt = 0;
  ofn.lpfnHook = 0; ofn.pvReserved = 0;
  ofn.nMaxCustFilter = ofn.nMaxFileTitle = 0;
  ofn.FlagsEx = 0;
  ofn.nFilterIndex = 1; ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = "Ouvrir un bitmap";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
  szFile[0] = 0;
  return GetOpenFileName(&ofn);
}

void onPaint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	hDC = BeginPaint(hWnd, &ps);
	if(pbminfhead)
		StretchDIBits(hDC, 8, 46, cxDib, cyDib, 0, 0, cxDib, cyDib, pbitsbmp,
							pbmpinf, DIB_RGB_COLORS, SRCCOPY);
	EndPaint(hWnd , &ps);
}

LRESULT CALLBACK AppWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		case WM_CREATE:	
			hConvert = CreateWindow("BUTTON", "Convertir", WS_CHILD | WS_VISIBLE, 
								     8, 8, 100, 30, hWnd, (HMENU)IDM_CONVERT, hInst, 0);
			EnableWindow(hConvert, 0);
			return 0;
		case WM_PAINT:
			onPaint(hWnd);
			return 0;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDM_OPEN:
					if(!dlgSelectBitmap(hWnd)) return 0;
					InvalidateRect(hWnd, 0, 1);
					if(pbminfhead) {HeapFree(hHeap, 0, pbminfhead); pbminfhead = 0;}
					LoadBmp();
					return 0;
				case IDM_CONVERT:
					EnableWindow(hConvert, 0);
					DrawGrayScale();
					InvalidateRect(hWnd, 0, 0);
					return 0;
				case IDM_CLOSE:
					PostMessage(hWnd, WM_DESTROY, 0, 0);
			}
			break;
		case WM_DESTROY:
			if(pbminfhead) HeapFree(hHeap, 0, pbminfhead);
			PostQuitMessage(0); return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int InitInstance()
{
  WNDCLASSEX wclsx;
  wclsx.cbSize = sizeof(WNDCLASSEX);
  wclsx.style = 0;
  wclsx.lpfnWndProc = AppWndProc;
  wclsx.cbClsExtra = wclsx.cbWndExtra = 0;
	wclsx.hInstance = hInst;
  wclsx.hIcon = LoadIcon(0, IDI_APPLICATION);
  wclsx.hCursor = LoadCursor(0, IDC_ARROW);
  wclsx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wclsx.lpszMenuName = (LPCTSTR) IDMNU_APP;
  wclsx.lpszClassName = szAppName;
	wclsx.hIconSm = 0; 
  if(!RegisterClassEx(&wclsx)) return 0;
  hMain = CreateWindowEx(0, szAppName, szAppName, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
												CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
												0, 0, hInst, 0);
	return (hMain != 0);
}

#pragma comment(linker, "/entry:myWinMain")
int __stdcall myWinMain()
{
	MSG msg;
	HACCEL hAccel;
	hInst = GetModuleHandle(0);
	hHeap = GetProcessHeap();
	if(!InitInstance()) return 0;
	hAccel = LoadAccelerators(hInst, (LPCTSTR)IDACC_APP);
	ShowWindow(hMain, SW_SHOW);
  while(GetMessage(&msg, NULL, 0, 0)) {
    if(!TranslateAccelerator(hMain, hAccel, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
	ExitProcess(0);
}
