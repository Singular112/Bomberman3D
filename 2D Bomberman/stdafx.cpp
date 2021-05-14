#include "stdafx.h"

namespace measures
{
#ifndef WIN32
void __stdcall OutputDebugStringA(const char* lpOutputString)
{
	std::cout << lpOutputString << std::endl;
}
#endif

g_measure_output_target_fn_t g_measure_output_target_fn
	= (measures::g_measure_output_target_fn_t)&OutputDebugStringA;
}
