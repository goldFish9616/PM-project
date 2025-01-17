///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Common types monikers.
///
/// Contatins typedefs for very common types like unsigned... 
///
///////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Кроссплатформенные типы данных, призванные заменить виндусовские DWORD и т.д.
* ------------------------------------------------------------------------------
* CHANGELOG
*  v.0.2.5 // 2013-02-21
*     1) Переводим на рельсы VS2010 + санация!
*  v.0.2.4 // 2009-09-21
*     1) изменено название типа TLPCvoid на TPCvoid (т.к. упоминание о far все-равно нет)
*     2) добавлен тип TPvoid
*  v.0.2.3 // 2009-08-03 -- исключены полностью все упоминания о TCHAR в соответствии 
*     с новой идеологией кроссплатформенного использования механизма автопереключения 
*     широких/узких символов (см. соответствующий релиз-нот в марсокноте)
*     Все касательно TCHAR теперь в XiTchar.h
*  v.0.2.2 // 2008-12-24 -- поехали
*******************************************************************************/

#pragma     // чтобы заткнулась писать про PCH с товарищами

#ifndef XILIB_XI_TYPES_MONIKERS_H_
#define XILIB_XI_TYPES_MONIKERS_H_


//#include <inttypes.h>


namespace xi {
namespace types {



//

// 64-битное целое: определяется оч. хитро и необходим контролировать правильность
// NOTE: может быть надо спец. директивой вообще отменять ввод этого типа

// Символ _MSC_VER определяет версию компилятора MSVCC. Если он определен, значит это Visual Studio!
#ifdef _MSC_VER

   typedef __int64            TInt64;             // определение для Windows'а
   typedef unsigned __int64   TQword;

#endif // _MSC_VER


// Раньше было так. Теперь для LINUX'ового компилера (GCC) надо придумывать свои фишки!
//#ifdef XILINUX
//typedef long long TInt64;           // определение для Linux'а
//typedef unsigned long long TQword;
//#else // XILINUX
//typedef __int64 TInt64;             // определение для Windows'а
//typedef unsigned __int64 TQword;
//#endif // XILINUX







typedef int                   TBool;
typedef unsigned char         TByte;
typedef unsigned short        TWord;
typedef unsigned long         TDword;

typedef float                 TFloat;

typedef TDword*               TPdword;
typedef void*                 TPvoid;
typedef const void*           TPCvoid;   // переименован из TLPCvoid

typedef int                   TInt;
typedef unsigned int          TUint;
typedef unsigned int*         TPUint;


} // namespace types
} // namespace xi


#endif // XILIB_XI_TYPES_MONIKERS_H_
