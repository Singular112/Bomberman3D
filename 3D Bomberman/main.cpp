//
#include "stdafx.h"
#include "../game-engine/bomberman_app_c.h"

#ifdef WIN32
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
std::string g_app_path;
LONG WINAPI unhandled_exception_filter(PEXCEPTION_POINTERS exception)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

	char dumpfile_path[MAX_PATH * 2] = { 0 };

    const char* name_format = "%s\\app_crash_%02d%02d_%02d%02d.%s";

	sprintf_s(dumpfile_path,
		name_format,
        g_app_path.c_str(),
        st.wDay, st.wMonth, st.wHour, st.wMinute,
        "mdmp"
    );

	HANDLE dumpfile = CreateFileA(dumpfile_path, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (dumpfile != INVALID_HANDLE_VALUE)
    {
        __try
        {
            MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
            {
                exceptionInfo.ThreadId = GetCurrentThreadId();
                exceptionInfo.ExceptionPointers = exception;
                exceptionInfo.ClientPointers = false;
            }

			MiniDumpWriteDump
            (
                GetCurrentProcess(),
                GetCurrentProcessId(),
				dumpfile,
                MiniDumpNormal,
                &exceptionInfo,
                NULL,
                NULL
           );
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }

		CloseHandle(dumpfile);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    g_app_path = app_directory();
	SetUnhandledExceptionFilter(&unhandled_exception_filter);

    // fix highdpi bug
    {
        typedef enum PROCESS_DPI_AWARENESS {
            PROCESS_DPI_UNAWARE,
            PROCESS_SYSTEM_DPI_AWARE,
            PROCESS_PER_MONITOR_DPI_AWARE
        };

        typedef BOOL(WINAPI* SetProcessDPIAware_t)(void);
        typedef HRESULT(*SetProcessDpiAwareness_t)(PROCESS_DPI_AWARENESS value);
        //SetProcessDpiAwareness_t SetProcessDpiAwareness = nullptr;

        SetProcessDPIAware_t SetProcessDpiAware;

        SetProcessDpiAware = (decltype(SetProcessDpiAware))GetProcAddress
        (
            ::GetModuleHandle(L"user32.dll"), "SetProcessDPIAware"
        );

        if (SetProcessDpiAware)
            SetProcessDpiAware();
    }

	int return_code = 0;

	while (true)
	{
		bomberman_app_c bomberman_game;

		return_code = bomberman_game.exec();

		if (return_code != bomberman_app_c::e_code_restart)
		{
			break;
		}
	}

	return return_code;
}
#else
int main(int argc, char** argv)
{
	int return_code = 0;

	while (true)
	{
		bomberman_app_c bomberman_game;

		return_code = bomberman_game.exec();

		if (return_code != bomberman_app_c::e_code_restart)
		{
			break;
		}
	}

	return return_code;
}
#endif
