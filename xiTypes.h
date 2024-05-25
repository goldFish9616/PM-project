/*******************************************************************************
* Кроссплатформенные типы данных, призванные заменить виндусовские DWORD и т.д.
* ------------------------------------------------------------------------------
* CHANGELOG
*  v.0.2.4 // 2009-09-21
*     1) изменено название типа TLPCvoid на TPCvoid (т.к. упоминание о far все-равно нет)
*     2) добавлен тип TPvoid
*  v.0.2.3 // 2009-08-03 -- исключены полностью все упоминания о TCHAR в соответствии 
*     с новой идеологией кроссплатформенного использования механизма автопереключения 
*     широких/узких символов (см. соответствующий релиз-нот в марсокноте)
*     Все касательно TCHAR теперь в XiTchar.h
*  v.0.2.2 // 2008-12-24 -- поехали
*******************************************************************************/


#ifndef XiTypesH
#define XiTypesH


//#include <inttypes.h>
#include <cstdint>


namespace Xi {
namespace Types {
//namespace conv1{ //добавила это и ошибка с enclosure убралась....Нормально? Или не так надо было...
//}
//}
//}

// 64-битное целое: определяется оч. хитро и необходим контролировать правильность
// NOTE: может быть надо спец. директивой вообще отменять ввод этого типа
#ifdef XILINUX
typedef long long TInt64;           // определение для Linux'а
typedef unsigned long long TQword;

// тип данных char+wchar_t=TCHAR
//#ifndef TCHAR
// APPNOTE: пока-что, чтобы не плодить лишних проверок, добавляем макросы по-любому, если установлен XILINUX


// UPD: 2009-08-03: убрали в XiTchar.h
// TODO: вскорости, после адаптации всех проектов, удалить отсюда это вообще
//#ifdef XIUNICODE
//   typedef wchar_t TCHAR;
//   #define __TEXT(quote) L##quote
//#else  // XIUNICODE
//   typedef char TCHAR;
//   #define __TEXT(quote) quote
//#endif // XIUNICODE
//
//#define TEXT(quote) __TEXT(quote)

//#endif // TCHAR


#else // XILINUX
typedef int64_t TInt64;             // определение для Windows'а
typedef uint64_t TQword;
//typedef __int64 TInt64;             // определение для Windows'а
//typedef unsigned __int64 TQword;
#endif // XILINUX


typedef unsigned long         TDword;
//typedef unsigned __int64    TQword;
typedef int                   TBool;
typedef unsigned char         TByte;
typedef unsigned short        TWord;
typedef float                 TFloat;
//typedef TFLOAT               *TPFLOAT;
//typedef TBOOL near           *TPBOOL;
//typedef TBOOL far            *TLPBOOL;
//typedef TBYTE near           *TPBYTE;
//typedef TBYTE far            *TLPBYTE;
//typedef int near            *TPINT;
//typedef int far             *TLPINT;
//typedef TWORD near           *TPWORD;
//typedef TWORD far            *TLPWORD;
//typedef long far            *TLPLONG;
//typedef TDWORD near          *TPDWORD;
//typedef TDWORD far           *TLPDWORD;
//typedef void far            *TLPVOID;
typedef TDword                *TPdword;
typedef void                  *TPvoid;
typedef const void            *TPCvoid;   // переименован из TLPCvoid

typedef int                 TInt;
typedef unsigned int        TUint;
typedef unsigned int        *TPUint;


}; // namespace Types
}; // namespace Xi


#endif // XiTypesH
