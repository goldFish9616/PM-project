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
*******************************************************************************/

/** \brief Custom datatypes for xi log.
 *
 *  >
 */

#ifndef xiCustomLogH
#define xiCustomLogH

#include "xi/log/filelog.h"
using namespace xi;



/** \brief A xi log event renderer for a SRA logs processor.
 *
 *  >
 */
class LogProcLERenderer : public log::DefTextFileLogEventRenderer {
//public:
//    //------------<Consts>-------------------------------------
//
//    // errors and exceptions
//    static const char* ER_NO_OWNER_LOG_SET;
public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Default constructor */
    LogProcLERenderer();

public:

    //----<ITextFileLogEventRenderer implmentations>----

    //// ITextFileLogEventRenderer::setOwner() Sets owner text file log.
    //virtual void setOwner(log::TextFileLog* fLog);

    // ITextFileLogEventRenderer::renderEvent Main method: renders a given event 
    // with using a owner text file log.
    virtual void renderEvent(const log::LogEvent* lev);
//protected:
//    log::TextFileLog* _owner;

protected:
    void renderTStEvent(const log::TimedStringLogEvent* timedLE);
}; // class DefTextFileLogEventRenderer


#endif // xiCustomLogH