#define AIO_ABI_PREFIX <xirang/config/abi/msvc_prefix.h>
#define AIO_ABI_SUFFIX <xirang/config/abi/msvc_suffix.h>

#define AIO_FUNCTION __FUNCTION__ 

#define AIO_DLL_EXPORT __declspec( dllexport )
#define AIO_DLL_IMPORT __declspec( dllimport )

#if defined (AIO_DLL_GENERATE)
#	define AIO_API AIO_DLL_EXPORT
#elif defined (AIO_DLL_STATIC)
#	define AIO_API 
#else	//DLL import
#	define AIO_API AIO_DLL_IMPORT
#endif

#define AIO_INTERFACE __declspec(novtable)

//for c++0x explicit operator
#define EXPLICIT_OPERATOR 

