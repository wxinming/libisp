#pragma once
#define LIBISP_NOT_EXPORTS
#ifdef LIBISP_NOT_EXPORTS
#define LIBISP_DLL_EXPORT
#else
//����Ϊ��̬���ӿ�
#ifdef LIBISP_EXPORTS
#define LIBISP_DLL_EXPORT __declspec(dllexport)
#pragma warning(disable:4251)
#else
#define LIBISP_DLL_EXPORT __declspec(dllimport)
#endif // LIBISP_EXPORTS
#endif // LIBISP_NOT_EXPORTS
