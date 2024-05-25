#include "stdafx.h"


#include "xi/log/baselog.h"

#include "stdarg.h"
#include  <stdio.h>

using namespace std;

namespace xi {
namespace log {



//==============================================================================
// class LogException
//==============================================================================


//------------------------------------------------------------------------------
// Throws an exception with sprintf formatted string.
//------------------------------------------------------------------------------    
void LogException::throwException(const char * _Format, ...)
{
    char errMes[MAX_FORMAT_STR_SIZE];

    va_list argptr;
    va_start(argptr, _Format);
//    vsprintf_s(errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    va_end(argptr);

    throw LogException(errMes);
}


//------------------------------------------------------------------------------
// Throws an exception with arguments appropriated for sprintf formatted string.
//------------------------------------------------------------------------------    
void LogException::throwException(const char * _Format, va_list argptr)
{
    char errMes[MAX_FORMAT_STR_SIZE];
//    vsprintf_s(errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);

    throw LogException(errMes);
}


//==============================================================================
// class LogEvent
//==============================================================================

//// consts
//const char* LogEvent::DEF_PATH  = "/";


//------------------------------------------------------------------------------
// Initializes by event type only
//------------------------------------------------------------------------------
//LogEvent::LogEvent(const std::string& etype)
LogEvent::LogEvent(const char* etype)
    : _type(etype)
    //, _path(DEF_PATH)
{
}


////------------------------------------------------------------------------------
//// Initializes by event type and event path
////------------------------------------------------------------------------------
////LogEvent::LogEvent(const std::string& etype, const std::string& path)
//LogEvent::LogEvent(const char* etype, const char* path)
//    : _type(etype)
//    , _path(path)
//{
//    //
//}



////------------------------------------------------------------------------------
//// Initializes by event path, event type and "value" parameters without names
////------------------------------------------------------------------------------
//LogEvent::LogEvent(const std::string& path, const std::string& etype, ...)
//{
//}


 





//==============================================================================
// class PVLogEvent
//==============================================================================


//------------------------------------------------------------------------------
// Initializes by event type only
//------------------------------------------------------------------------------
PVLogEvent::PVLogEvent(const char* etype)
    : LogEvent(etype)
{
}


////------------------------------------------------------------------------------
//// Initializes by event type and event path
////------------------------------------------------------------------------------
//PVLogEvent::PVLogEvent(const char* etype, const char* path)
//    : LogEvent(etype, path)
//{
//}


// 
//------------------------------------------------------------------------------
// LogEvent::asString() implementation: Returns string representation of a log event
// virtual
//------------------------------------------------------------------------------
std::string PVLogEvent::asString() const
{
    string res;

    // 1) формируем тип события
    res += "[";
    res += _type;
    res += "] ";

    //// 2) формируем путь без кавычек!
    //res += _path;

    // 3) каждую парочку "парамет-значение" добавляем с новой строки
    for(StrStrMMapCIt curIt = _parameters.begin();
        curIt != _parameters.end(); ++curIt)
    {
        res += '\n';
        res += curIt->first;
        res += '=';
        res += curIt->second;
    }

    return res;

}


//------------------------------------------------------------------------------
// Add a parameter.
//------------------------------------------------------------------------------
void PVLogEvent::addParameter(const char* name, const char* value)
{
    _parameters.insert(StrStrPair(name, value)); 
}


//==============================================================================
// class StringLogEvent
//==============================================================================


//------------------------------------------------------------------------------
// Initializes by event type only
//------------------------------------------------------------------------------
StringLogEvent::StringLogEvent(const char* etype)
    : LogEvent(etype)
{
}


////------------------------------------------------------------------------------
//// Initializes by event type and event path
////------------------------------------------------------------------------------
//StringLogEvent::StringLogEvent(const char* etype, const char* path)
//    : LogEvent(etype, path)
//{
//}


//------------------------------------------------------------------------------
// Initializes by event type and event path
//------------------------------------------------------------------------------
//StringLogEvent::StringLogEvent(const char* etype, const char* path, const char* data)
StringLogEvent::StringLogEvent(const char* etype, const char* data)
    : LogEvent(etype)
    , _parameter(data)
{
}


// 
//------------------------------------------------------------------------------
// LogEvent::asString() implementation: Returns string representation of a log event
// virtual
//------------------------------------------------------------------------------
std::string StringLogEvent::asString() const
{
    string res;

    // 1) формируем тип события
    res += "[";
    res += _type;
    res += "] ";

    //// 2) формируем путь без кавычек!
    //res += _path;

    //res += '\n';
    res += _parameter;;

    return res;
}

//------------------------------------------------------------------------------
// Format data string with sprintf formatted string.
//------------------------------------------------------------------------------    
void StringLogEvent::formatData(const char * _Format, ...)
{
    char buf[MAX_FORMAT_STR_SIZE];

    va_list argptr;
    va_start(argptr, _Format);
//    vsprintf_s(buf, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    va_end(argptr);

    _parameter = buf;

    //throw LogException(errMes);
}


//------------------------------------------------------------------------------
// Format data string with arguments appropriated for sprintf formatted string.
//------------------------------------------------------------------------------    
void StringLogEvent::formatData(const char * _Format, va_list argptr)
{
    char buf[MAX_FORMAT_STR_SIZE];
//    vsprintf_s(buf, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);

    _parameter = buf;
}




//==============================================================================
// class TimedStringLogEvent
//==============================================================================

// const
const char* TimedStringLogEvent::DEF_TIMEST_FORMAT_STR  = "%d.%m.%Y %H:%M:%S";



//------------------------------------------------------------------------------
// Initializes by event type only
//------------------------------------------------------------------------------
TimedStringLogEvent::TimedStringLogEvent(const char* etype)
    : StringLogEvent(etype)
    , _formatStr(DEF_TIMEST_FORMAT_STR)
    , _msNeed(msShort)
{
    setNowTime();
}

//------------------------------------------------------------------------------
// Initializes by event type and event path
//------------------------------------------------------------------------------
TimedStringLogEvent::TimedStringLogEvent(const char* etype, const char* data)
    : StringLogEvent(etype, data)
    , _formatStr(DEF_TIMEST_FORMAT_STR)
    , _msNeed(msShort)
{
    setNowTime();
}


//------------------------------------------------------------------------------
// Initializes by event type, event path,  event data and a timestamp.
//------------------------------------------------------------------------------
TimedStringLogEvent::TimedStringLogEvent(const char* etype, const char* data, 
    time_t timest)
    : StringLogEvent(etype, data)
    , _timeSt(timest)
    , _formatStr(DEF_TIMEST_FORMAT_STR)
    , _msNeed(msShort)
{
}

//------------------------------------------------------------------------------
// LogEvent::asString() implementation: Returns string representation of a log event
// virtual
//------------------------------------------------------------------------------
std::string TimedStringLogEvent::asString() const
{
    string res;
    
    // вспомог. буфер
    const int BUF_SIZE = 512;
    char dtRepr[BUF_SIZE];


    // добавить сюда метку времени
    if(!_formatStr.empty())
    {

        // представление времени
        struct tm* ntime = localtime(&_timeSt); 

        size_t shift = strftime(dtRepr, BUF_SIZE, _formatStr.c_str(), ntime);        

        res += "[";
        res += dtRepr;
        res += "]";
    }

    // допечатаем милисекунды, если надо // TODO: определить надо или не 
    if(_msNeed != msNo)
    {
        if(_msNeed == msFull)
            sprintf(dtRepr, "[%d]", _timeSt);
        else
            sprintf(dtRepr, "[%d]", _timeSt % 1000);

        res += dtRepr;
    }


    // 1) формируем тип события
    res += "[";
    res += _type;
    res += "] ";

    //// 2) формируем путь без кавычек!
    //res += _path;

    //res += '\n';
    res += _parameter;

    return res;
}

//------------------------------------------------------------------------------
// Sets the current time as a timestamp.
//------------------------------------------------------------------------------
void TimedStringLogEvent::setNowTime()
{
    _timeSt = time(NULL);
}


}; // namespace log 
}; // namespace xi



