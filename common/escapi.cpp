#include <windows.h>
#include "escapi.h"
#include <strsafe.h>

countCaptureDevicesProc countCaptureDevices;
initCaptureProc initCapture;
deinitCaptureProc deinitCapture;
doCaptureProc doCapture;
isCaptureDoneProc isCaptureDone;
getCaptureDeviceNameProc getCaptureDeviceName;
getCaptureDeviceNameUniqueProc getCaptureDeviceNameUnique;
ESCAPIVersionProc ESCAPIVersion;
getCapturePropertyValueProc getCapturePropertyValue;
getCapturePropertyAutoProc getCapturePropertyAuto;
setCapturePropertyProc setCaptureProperty;
getCaptureErrorLineProc getCaptureErrorLine;
getCaptureErrorCodeProc getCaptureErrorCode;
initCaptureWithOptionsProc initCaptureWithOptions;


/* Internal: initialize COM */
typedef void (*initCOMProc)();
initCOMProc initCOM;

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

int setupESCAPI(bool enableMessage)
{
	/* Load DLL dynamically */
	HMODULE capdll = LoadLibraryA("escapi.dll");
	if (capdll == NULL)
	{
		if (enableMessage)
		{
			ErrorExit(TEXT("LoadEscapiDll"));
		}
		return ERR_LIBRARY_SETUP;
	}
	/* Fetch function entry points */
	countCaptureDevices = (countCaptureDevicesProc)GetProcAddress(capdll, "countCaptureDevices");
	initCapture = (initCaptureProc)GetProcAddress(capdll, "initCapture");
	deinitCapture = (deinitCaptureProc)GetProcAddress(capdll, "deinitCapture");
	doCapture = (doCaptureProc)GetProcAddress(capdll, "doCapture");
	isCaptureDone = (isCaptureDoneProc)GetProcAddress(capdll, "isCaptureDone");
	initCOM = (initCOMProc)GetProcAddress(capdll, "initCOM");
	getCaptureDeviceName = (getCaptureDeviceNameProc)GetProcAddress(capdll, "getCaptureDeviceName");
	getCaptureDeviceNameUnique = (getCaptureDeviceNameUniqueProc)GetProcAddress(capdll, "getCaptureDeviceNameUnique");
	ESCAPIVersion = (ESCAPIVersionProc)GetProcAddress(capdll, "ESCAPIVersion");
	getCapturePropertyValue = (getCapturePropertyValueProc)GetProcAddress(capdll, "getCapturePropertyValue");
	getCapturePropertyAuto = (getCapturePropertyAutoProc)GetProcAddress(capdll, "getCapturePropertyAuto");
	setCaptureProperty = (setCapturePropertyProc)GetProcAddress(capdll, "setCaptureProperty");
	getCaptureErrorLine = (getCaptureErrorLineProc)GetProcAddress(capdll, "getCaptureErrorLine");
	getCaptureErrorCode = (getCaptureErrorCodeProc)GetProcAddress(capdll, "getCaptureErrorCode");
	initCaptureWithOptions = (initCaptureWithOptionsProc)GetProcAddress(capdll, "initCaptureWithOptions");


	/* Check that we got all the entry points */
	if (initCOM == NULL ||
		ESCAPIVersion == NULL ||
		getCaptureDeviceName == NULL ||
		countCaptureDevices == NULL ||
		initCapture == NULL ||
		deinitCapture == NULL ||
		doCapture == NULL ||
		isCaptureDone == NULL ||
		getCapturePropertyValue == NULL ||
		getCapturePropertyAuto == NULL ||
		setCaptureProperty == NULL ||
		getCaptureErrorLine == NULL ||
		getCaptureErrorCode == NULL ||
		initCaptureWithOptions == NULL)
	{
		return ERR_ENTRY_POINT;
	}

	/* Verify DLL version is at least what we want */
	int version = ESCAPIVersion();

	if (version < 0x300)
	{
		return ERR_OLD_VERSION;
	}

	/* Initialize COM.. */
	initCOM();

	/* and return the number of capture devices found. */
	return countCaptureDevices();
}

