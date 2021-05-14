#if defined(SOFTRENDERLIB_EXPORTS)
#	define SOFTRENDERLIB_API extern "C" __declspec(dllexport)
#	define SOFTRENDERLIB_API_DLL __declspec(dllexport)
#elif defined (SOFTRENDERLIB_DLL_IMPORTS)
#	define SOFTRENDERLIB_API extern "C" __declspec(dllimport)
#	define SOFTRENDERLIB_API_DLL __declspec(dllimport)
#else
#	define SOFTRENDERLIB_API
#	define SOFTRENDERLIB_API_DLL
#endif
