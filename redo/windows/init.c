// 6 april 2015
#include "uipriv_windows.h"

HINSTANCE hInstance;
int nCmdShow;

HFONT hMessageFont;

HBRUSH hollowBrush;

struct uiInitError {
	char *msg;
	char failbuf[256];
};

#define initErrorFormat L"error %s: %s%sGetLastError() == %I32u%s"
#define initErrorArgs wmessage, sysmsg, beforele, le, afterle

static const char *loadLastError(const char *message)
{
	WCHAR *sysmsg;
	BOOL hassysmsg;
	WCHAR *beforele;
	WCHAR *afterle;
	int n;
	WCHAR *wmessage;
	WCHAR *wstr;
	const char *str;
	DWORD le;

	le = GetLastError();
	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, le, 0, (LPWSTR) (&sysmsg), 0, NULL) != 0) {
		hassysmsg = TRUE;
		beforele = L" (";
		afterle = L")";
	} else {
		hassysmsg = FALSE;
		sysmsg = L"";
		beforele = L"";
		afterle = L"";
	}
	wmessage = toUTF16(message);
	n = _scwprintf(initErrorFormat, initErrorArgs);
	wstr = (WCHAR *) uiAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	snwprintf(wstr, n + 1, initErrorFormat, initErrorArgs);
	str = toUTF8(wstr);
	uiFree(wstr);
	if (hassysmsg)
		if (LocalFree(sysmsg) != NULL)
			logLastError("error freeing system message in loadLastError()");
	uiFree(wmessage);
	return str;
}

static BOOL WINAPI consoleCtrlHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType) {
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		// the handler is run in a separate thread
		SendMessageW(utilWindow, msgConsoleEndSession, 0, 0);
		// we handled it here
		// this WON'T terminate the program because this is a DLL
		// at least, that's the best I can gather from the MSDN page on the handler function
		// it says that functions registered by a DLL replace the default handler function (which ends the process)
		// it works, anyway
		// TODO it's telling me there's one allocation that's left over with no type information
		return TRUE;
	}
	return FALSE;
}

uiInitOptions options;

#define wantedICCClasses ( \
	ICC_STANDARD_CLASSES |	/* user32.dll controls */		\
	ICC_PROGRESS_CLASS |		/* progress bars */			\
	ICC_TAB_CLASSES |			/* tabs */					\
	ICC_LISTVIEW_CLASSES |		/* table headers */			\
	ICC_UPDOWN_CLASS |		/* spinboxes */			\
	0)

const char *uiInit(uiInitOptions *o)
{
	STARTUPINFOW si;
	const char *ce;
	HICON hDefaultIcon;
	HCURSOR hDefaultCursor;
	NONCLIENTMETRICSW ncm;
	INITCOMMONCONTROLSEX icc;

	options = *o;

	if (initAlloc() == 0)
		return loadLastError("error initializing memory allocations");

	initResizes();

	nCmdShow = SW_SHOWDEFAULT;
	GetStartupInfoW(&si);
	if ((si.dwFlags & STARTF_USESHOWWINDOW) != 0)
		nCmdShow = si.wShowWindow;

	hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
	if (hDefaultIcon == NULL)
		return loadLastError("loading default icon for window classes");
	hDefaultCursor = LoadCursorW(NULL, IDC_ARROW);
	if (hDefaultCursor == NULL)
		return loadLastError("loading default cursor for window classes");

	ce = initUtilWindow(hDefaultIcon, hDefaultCursor);
	if (ce != NULL)
		return loadLastError(ce);

	if (registerWindowClass(hDefaultIcon, hDefaultCursor) == 0)
		return loadLastError("registering uiWindow window class");

	ZeroMemory(&ncm, sizeof (NONCLIENTMETRICSW));
	ncm.cbSize = sizeof (NONCLIENTMETRICSW);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICSW), &ncm, sizeof (NONCLIENTMETRICSW)) == 0)
		return loadLastError("getting default fonts");
	hMessageFont = CreateFontIndirectW(&(ncm.lfMessageFont));
	if (hMessageFont == NULL)
		return loadLastError("loading default messagebox font; this is the default UI font");

	if (initContainer(hDefaultIcon, hDefaultCursor) == 0)
		return loadLastError("initializing uiMakeContainer() window class");

	if (SetConsoleCtrlHandler(consoleCtrlHandler, TRUE) == 0)
		return loadLastError("setting up console end session handler");

	hollowBrush = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
	if (hollowBrush == NULL)
		return loadLastError("getting hollow brush");

	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = wantedICCClasses;
	if (InitCommonControlsEx(&icc) == 0)
		return loadLastError("initializing Common Controls");

	return NULL;
}

void uiUninit(void)
{
	uninitMenus();
	// TODO delete hollow brush
	if (SetConsoleCtrlHandler(consoleCtrlHandler, FALSE) == 0)
		logLastError("unregistering console end session handler");
	uninitContainer();
	if (DeleteObject(hMessageFont) == 0)
		logLastError("error deleting control font in uiUninit()");
	unregisterWindowClass();
	// TODO delete default cursor
	// TODO delete default icon
	uninitResizes();
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
	uiFree((void *) err);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInstance = hinstDLL;
	return TRUE;
}