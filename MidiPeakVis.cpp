// MidiPeakVis.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MidiPeakVis.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Peakmeter.cpp -- WinMain and dialog box functions

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include "resource.h"

static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// Timer ID and period (in milliseconds)
#define ID_TIMER  1
#define TIMER_PERIOD  10

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

HMIDIOUT device;

void Debug(const WCHAR* szFormat, ...)
{
	WCHAR szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnwprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}

int noteStates[256];

void sendMidiMessage(unsigned char note, int colour)
{
	if (noteStates[note] == colour)
	{
		return;
	}
	noteStates[note] = colour;

	union { unsigned long word; unsigned char data[4]; } message;
	message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
	message.data[1] = note;    // MIDI note-on message: Key number (60 = middle C)
	message.data[2] = colour;   // MIDI note-on message: Key velocity (100 = loud)
	message.data[3] = 0;     // Unused parameter

	int flag = midiOutShortMsg(device, message.word);
	if (flag != MMSYSERR_NOERROR) {
		Debug(L"Warning: MIDI Output is not open.\n");
	}
}

void turnOnInControl()
{
	union { unsigned long word; unsigned char data[4]; } message;
	message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
	message.data[1] = 12;    // MIDI note-on message: Key number (60 = middle C)
	message.data[2] = 127;   // MIDI note-on message: Key velocity (100 = loud)
	message.data[3] = 0;     // Unused parameter

	int flag = midiOutShortMsg(device, message.word);
	if (flag != MMSYSERR_NOERROR) {
		Debug(L"Warning: MIDI Output is not open.\n");
	}
}

void openMidi()
{
// variable which is both an integer and an array of characters:
	int midiport = 2;       // select which MIDI output port to open
	int flag;           // monitor the status of returning functions
	Debug(L"Opening Output.\n");

// Open the MIDI output port
	flag = midiOutOpen(&device, midiport, 0, 0, CALLBACK_NULL);
	if (flag != MMSYSERR_NOERROR) {
		Debug(L"Error opening MIDI Output.\n");
		throw "Error opening MIDI Output.\n";
	}
	turnOnInControl();
}

int getNoteNumber(int note)
{
	switch (note) {
	case 0:
		return 32;
	case 1:
		return 48;
	case 2:
		return 49;
	case 3:
		return 50;
	case 4:
		return 51;
	case 5:
		return 35;
	case 6:
		return 19;
	case 7:
		return 3;
	default:
		return 1;
	}
}

void offFrom(int number)
{
	for (int i = 0; i < number; ++i) {
		sendMidiMessage(96 + i, getNoteNumber(i));
		sendMidiMessage(112 + i, getNoteNumber(i));
	}
	for (int i = number; i < 9; ++i) {
		sendMidiMessage(96 + i, 0);
		sendMidiMessage(112 + i, 0);
	}
}

int sendMidi(float peak) {

	turnOnInControl();
	if (peak == 0)
	{
		offFrom(0);
	}
	else if (peak < 0.1)
	{
		offFrom(1);
	}
	else if (peak < 0.3)
	{
		offFrom(2);
	}
	else if (peak < 0.45)
	{
		offFrom(3);
	}
	else if (peak < 0.6)
	{
		offFrom(4);
	}
	else if (peak < 0.7)
	{
		offFrom(5);
	}
	else if (peak < 0.8)
	{
		offFrom(6);
	}
	else if (peak < 0.9)
	{
		offFrom(7);
	}
	else if (peak < 0.95)
	{
		offFrom(8);
	}
	else
	{
		offFrom(9);
	}

	return 0;
}

//-----------------------------------------------------------
// WinMain -- Opens a dialog box that contains a peak meter.
//   The peak meter displays the peak sample value that plays
//   through the default rendering device.
//-----------------------------------------------------------
int APIENTRY WinMainSample(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance)
{
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioMeterInformation *pMeterInfo = NULL;

	if (hPrevInstance)
	{
		return 0;
	}

	CoInitialize(NULL);

	// Get enumerator for audio endpoint devices.
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
		NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator),
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		// Get peak meter for default audio-rendering device.
		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr)

		hr = pDevice->Activate(__uuidof(IAudioMeterInformation),
			CLSCTX_ALL, NULL, (void**)&pMeterInfo);
	EXIT_ON_ERROR(hr)

		DialogBoxParam(hInstance, L"PEAKMETER", NULL, (DLGPROC)DlgProc, (LPARAM)pMeterInfo);

Exit:
	if (FAILED(hr))
	{
		MessageBox(NULL, TEXT("This program requires Windows Vista."),
			TEXT("Error termination"), MB_OK);
	}
	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(pMeterInfo)
		CoUninitialize();
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	openMidi();
	WinMainSample(hInstance, hPrevInstance);

	offFrom(0);

	// turn any MIDI notes currently playing:
	midiOutReset(device);

	// Remove any data in MIDI device and close the MIDI Output port
	midiOutClose(device);
	return TRUE;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MIDIPEAKVIS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIDIPEAKVIS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIDIPEAKVIS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MIDIPEAKVIS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




//-----------------------------------------------------------
// DlgProc -- Dialog box procedure
//-----------------------------------------------------------

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static IAudioMeterInformation *pMeterInfo = NULL;
	static HWND hPeakMeter = NULL;
	static float peak = 0;
	HRESULT hr;
	static HMIDIOUT* device = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		pMeterInfo = (IAudioMeterInformation*)lParam;
		SetTimer(hDlg, ID_TIMER, TIMER_PERIOD, NULL);
		hPeakMeter = GetDlgItem(hDlg, IDC_PEAK_METER);
		return TRUE;

	case WM_COMMAND:
		switch ((int)LOWORD(wParam))
		{
		case IDCANCEL:
			KillTimer(hDlg, ID_TIMER);
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
		break;

	case WM_TIMER:
		switch ((int)wParam)
		{
		case ID_TIMER:
			// Update the peak meter in the dialog box.
			hr = pMeterInfo->GetPeakValue(&peak);
			if (FAILED(hr))
			{
				MessageBox(hDlg, TEXT("The program will exit."),
					TEXT("Fatal error"), MB_OK);
				KillTimer(hDlg, ID_TIMER);
				EndDialog(hDlg, TRUE);
				return TRUE;
			}
			sendMidi(peak);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
