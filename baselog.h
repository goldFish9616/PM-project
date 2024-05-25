/*******************************************************************************
* Описание ...
* Версия для Visual Studio 10. Конфигурации: Win32
*-------------------------------------------------------------------------------
* APPNOTES
*
*-------------------------------------------------------------------------------
* CHANGELOG
*  v.0.1.0 // 2013-02-24:
*     begin
*-------------------------------------------------------------------------------
* Глобальные TODO:
*   задуматься о movable-семантике
*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Base file for xiLog ver. 2 type definitions
///
/// >
///
///////////////////////////////////////////////////////////////////////////////

#ifndef baselogH
#define baselogH

// библиотечные
#include <string>
#include <map>
#include <exception>
#include <time.h>

namespace xi {
namespace log {


/** \brief An xi DB library specific exception.  */
class LogException : std::runtime_error {
        //public std::exception {

public:
    static const int MAX_FORMAT_STR_SIZE = 1024;

public:
    //------------<Public Static Methods>----------------------
    
    
    /** \brief Throws an exception with sprintf formatted string. */
    static void throwException(const char * _Format, ...); 

    /** \brief Throws an exception with arguments appropriated for sprintf formatted string. */
    static void throwException(const char * _Format, va_list argptr); 


public:
    //------------<Constructors>---------------- 
    
    /// Default
    LogException(): 
        std::runtime_error("")
            //,    _hasErrMes(false)
    {
    }
        
    /// With Info message 
    LogException(const char * const & wh):
        std::runtime_error(wh)
            //,  _hasErrMes(false)
    {
    }

}; // class DPMException



/** \brief Abstract class represents a base log event.
 *
 *  Hierarchical model is used.
 *
 *  TODO: 1) Добавить возможность иерархической вложенности ивентов друг в друга.
 *  Так, в LogEvent создаем список list<LogEvent>, но в виде указателя,
 *  чтобы по умолчанию можно было его не создавать, а использовать ивент 
 *  в "плоском" режиме. Пока обходимся без этог из-за сильного дефицита времени.
 */
class LogEvent {

   


public:

    //------------<Consts>-------------------------------------
//    static const char* DEF_PATH;
//
//public:
//    static const char DEF_LEVEL_DELIMITER = '/';

public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Initializes by event type only. */
    LogEvent(const char* etype);
    //LogEvent(const std::string& etype);

    ///** \brief Initializes by event type and event path */
    //LogEvent(const char* etype, const char* path);
    ////LogEvent(const std::string& etype, const std::string& path);


//    /** \brief Initializes by event path, event type and "value" parameters without names */
//    LogEvent(const std::string& path, const std::string& etype, ...);


public:    
    //------------<Public Methods>-----------------------------

    /** \brief Returns string representation of a log event. */
    virtual std::string asString() const = 0;


//    /** \brief Add a parameter. */
//    void addParameter(const std::string& name, const std::string& value);


public:
    //------------<sets/gets>----------------------------------

    const std::string& getType() const { return _type; }


//    /** \brief Gets string-based parameters list. */
//    StrStrMMap& getParameters()  { return _parameters; }

    

protected:
    //------------<Protected methods>--------------------------
    //-/** \brief Name of the event. */
    //std::string _name;


    ///** \brief A log event path for hierarchical data representation. 
    //    Default levels delimiter is '/'. */
    //std::string _path;    

    /** \brief A text string representing a type of log event used for maintaining
        log rendering. */
    std::string _type;

    ///** \brief String-based parameters. */
    //StrStrMMap _parameters;
    

}; // class LogEvent





/** \brief Represents param-value-based log event.
 *
 *  >
 */
class PVLogEvent : public LogEvent {
public:
    //------------<Types>--------------------------------------
    /** \brief Datatype for string-to-string multimap datatype. */
    typedef std::multimap<std::string,std::string> StrStrMMap;

    typedef StrStrMMap::iterator StrStrMMapIt;
    typedef StrStrMMap::const_iterator StrStrMMapCIt;


    /** \brief Pair of two strings datatype. */
    typedef std::pair<std::string,std::string> StrStrPair;


public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Initializes by event type only. */
    PVLogEvent(const char* etype);
    //LogEvent(const std::string& etype);

    ///** \brief Initializes by event type and event path */
    //PVLogEvent(const char* etype, const char* path);


public:    
    //------------<Public Methods>-----------------------------


    // LogEvent::asString()
    virtual std::string asString() const;



    /** \brief Add a parameter. */
    void addParameter(const char* name, const char* value);
    //void addParameter(const std::string& name, const std::string& value);


public:
    //------------<Sets/Gets>----------------------------------

    /** \brief Gets string-based parameters list. */
    StrStrMMap& getParameters()  { return _parameters; }


protected:
    //------------<Protected methods>--------------------------

    /** \brief String-based parameters. */
    StrStrMMap _parameters;

}; // class PVLogEvent
 



/** \brief Represents string-based log event.
 *
 *  >
 */
class StringLogEvent : public LogEvent {
public:
    static const int MAX_FORMAT_STR_SIZE = 2048;

public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Initializes by event type only. */
    StringLogEvent(const char* etype);
    //LogEvent(const std::string& etype);

    ///** \brief Initializes by event type and event path */
    //StringLogEvent(const char* etype, const char* path);


    /** \brief Initializes by event type, event path and event data */
    StringLogEvent(const char* etype, const char* data);
    //StringLogEvent(const char* etype, const char* path, const char* data);


public:    
    //------------<Public Methods>-----------------------------

    // LogEvent::asString()
    virtual std::string asString() const;

    //void formatData(const char * _Format, ...);
    /** \brief Format data string with sprintf formatted string. */
    void formatData(const char * _Format, ...); 

    /** \brief Format data string with arguments appropriated for sprintf formatted string. */
    void formatData(const char * _Format, va_list argptr); 


public:
    //------------<Sets/Gets>----------------------------------

    /** \brief Gets string data parameter. */
    const std::string& getData() const { return _parameter; }

    /** \brief Sets string data parameter. */
    void setData(const std::string& sdata)  { _parameter = sdata; }


    



protected:
    //------------<Protected fields>--------------------------

    /** \brief Event string parameter. */
    std::string _parameter;

    //-/** \brief Flag determines if a typename shoud */
    //bool _doOutType;

}; // class StringLogEvent



/** \brief Class extends StringLogEvent with a timestamp.
 *
 *  >
 */
class TimedStringLogEvent : public StringLogEvent {
public:
    //------------<Types>--------------------------------------
    
    /** \brief Text file encoding. */
    enum Milliseconds {
        msNo,
        msFull,
        msShort
    };

public:
    //------------<Consts>-------------------------------------
    
    // common
    static const char* DEF_TIMEST_FORMAT_STR;

public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Initializes by event type only. */
    TimedStringLogEvent(const char* etype);

    /** \brief Initializes by event type, event path and event data */
    TimedStringLogEvent(const char* etype, const char* data);

    /** \brief Initializes by event type, event path,  event data and a timestamp */
    TimedStringLogEvent(const char* etype, const char* data, time_t timest);

public:    
    //------------<Public Methods>-----------------------------

    /** \brief Sets the current time as a timestamp. */
    void setNowTime();


    


    // LogEvent::asString()
    virtual std::string asString() const;


public:
    //------------<Sets/Gets>----------------------------------
    void setFormatStr(const char* formatStr) { _formatStr = formatStr; }

    std::string getFormatStr() const { return _formatStr; }

    time_t getTimeSt() const { return _timeSt; }


    /** \brief Temporary full assigned flag settegetter */
    Milliseconds getMsNeed() const { return _msNeed; }


protected:
    //------------<Protected fields>--------------------------
    
    /** \brief Timestamp field. */
    time_t _timeSt;

    /** \brief A fromat string for formatting a time-data string representation. */
    std::string _formatStr;

    /** \brief Flag determines if there need to output a milliseconds */
    Milliseconds _msNeed;


    

}; // class TimedStringLogEvent





/** \brief Interface represents methods for event log recording
 *
 *  >
 */
class ILogRecorder {
public:


    /** \brief Proceeds a new log event given thru a pointer.
     *
     *  
     */
    virtual void proceedLogEvent(const LogEvent* lev) = 0;

////
//    /** \brief Method adds a new log event given as a pointer.
//     *
//     *  Log memory management policies imply that all the log entry objects
//     *  given by this very method should be freed by the log implemetation.
//     *  Thus it is possible to return nullptr if given object was freed
//     *  immediately after the log entry would proceed.
//     */
//    virtual LogEvent* addLogEvent(LogEvent* lev) = 0;
//
//
//    /** \brief Method adds a new log event given as an object.
//     *
//     *  Unlike the pointer-based method this method gets a log entry
//     *  to be added as an object (reference) thus processing of the entry
//     *  is doing without any necessity to manage memory.
//     */
//    virtual LogEvent& addLogEvent(LogEvent& lev) = 0;


}; // class ILogRecorder



}; // namespace log 
}; // namespace xi



    //------------<Consts>-------------------------------------
    //------------<Types>--------------------------------------
    //------------<Public Static Methods>----------------------
    
    //------------<Constructors and destructor>----------------        
    //------------<Public Methods>-----------------------------
    //------------<Sets/Gets>----------------------------------


    //------------<Protected fields>---------------------------
    //------------<Protected methods>--------------------------
    //------------<Protected static methods>-------------------

    //------------<Private fields>-----------------------------
    //------------<Private methods>----------------------------
    //------------<Private static methods>---------------------



#endif // baselogH
