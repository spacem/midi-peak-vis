// MidiPeakVis.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MidiPeakVis.h"
#include "MidiFlasher.cpp"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include "resource.h"

static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// Timer ID and period (in milliseconds)
#define ID_TIMER  1
#define TIMER_PERIOD  80

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

MidiFlasher* midiFlasher;

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

		DialogBoxParam(hInstance, L"MIDIPEAKVIS", NULL, (DLGPROC)DlgProc, (LPARAM)pMeterInfo);

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

	WinMainSample(hInstance, hPrevInstance);
	if (midiFlasher != NULL)
	{
		delete midiFlasher;
	}
	return TRUE;
}

//-----------------------------------------------------------
// DlgProc -- Dialog box procedure
//-----------------------------------------------------------
BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static IAudioMeterInformation *pMeterInfo = NULL;
	static float peak = 0;
	HRESULT hr;
	static HMIDIOUT* device = NULL;
	int numDevices = 0;
	int selectedIndex = 0;
	try
	{
		switch (message)
		{
		case WM_INITDIALOG:
			numDevices = midiOutGetNumDevs();
			if (numDevices == 0)
			{
				throw L"No midi output devices found";
			}

			for (int i = 0; i < numDevices; ++i) {
				MIDIOUTCAPS moc;
				if (!midiOutGetDevCaps(i, &moc, sizeof(MIDIOUTCAPS)))
				{
					SendDlgItemMessage(hDlg, IDC_OUTPUT_DEVICE, CB_ADDSTRING, 0, (LPARAM)moc.szPname);
				}
			}
			SendDlgItemMessage(hDlg, IDC_OUTPUT_DEVICE, CB_SETCURSEL, numDevices - 1, 0);

			midiFlasher = new MidiFlasher(numDevices - 1);
			pMeterInfo = (IAudioMeterInformation*)lParam;
			SetTimer(hDlg, ID_TIMER, TIMER_PERIOD, NULL);
			return TRUE;

		case WM_COMMAND:
			switch ((int)LOWORD(wParam))
			{
			case IDCANCEL:
				KillTimer(hDlg, ID_TIMER);
				EndDialog(hDlg, TRUE);
				return TRUE;

			case IDC_OUTPUT_DEVICE:
				switch (HIWORD(wParam))
				{
					case CBN_SELCHANGE:
						selectedIndex = SendDlgItemMessage(hDlg, IDC_OUTPUT_DEVICE, CB_GETCURSEL, 0, 0);;
						delete midiFlasher;
						midiFlasher = new MidiFlasher(selectedIndex);
						return TRUE;
				}
			}
			break;

		case WM_TIMER:
			switch ((int)wParam)
			{
			case ID_TIMER:
				if (midiFlasher != NULL)
				{
					// Update the peak meter in the dialog box.
					hr = pMeterInfo->GetPeakValue(&peak);
					if (FAILED(hr))
					{
						throw L"Cannot get mixer peak value";
						return TRUE;
					}

					midiFlasher->SendMidi(peak);
				}
				return TRUE;
			}
			break;
		}
		return FALSE;
	}
	catch (const LPCWSTR msg)
	{
		KillTimer(hDlg, ID_TIMER);
		EndDialog(hDlg, TRUE);
		MessageBox(hDlg, msg, L"", 0);
		return FALSE;
	}
}
