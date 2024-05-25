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

///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Some file-based logs implementations.
///
/// A more elaborate module description. 
///
///////////////////////////////////////////////////////////////////////////////

#ifndef filelogH
#define filelogH


#include "xi/log/baselog.h"


#include <stdio.h>


namespace xi {
namespace log {


class TextFileLog;

/** \brief Interface for the object which can render Log Events for a text file.
 *
 *  >
 */
class ITextFileLogEventRenderer {
public:
    /** \brief Sets owner text file log. */
    virtual void setOwner(TextFileLog* fLog) = 0;

    /** \brief Main method: renders a given event with using a owner text file log. */
    virtual void renderEvent(const LogEvent* lev) = 0;


public:
    /** \brief It is necessary for a renderers collection to delete each renderer. */
    ~ITextFileLogEventRenderer() {};

}; // class ITextFileLogEventRenderer



/** \brief Default text file log event renderer.
 *
 *  >
 */
class DefTextFileLogEventRenderer : public ITextFileLogEventRenderer {
public:
    //------------<Consts>-------------------------------------

    // errors and exceptions
    static const char* ER_NO_OWNER_LOG_SET;
public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Default constructor */
    DefTextFileLogEventRenderer();

public:

    //----<ITextFileLogEventRenderer implmentations>----

    // ITextFileLogEventRenderer::setOwner() Sets owner text file log.
    virtual void setOwner(TextFileLog* fLog);

    // ITextFileLogEventRenderer::renderEvent Main method: renders a given event 
    // with using a owner text file log.
    virtual void renderEvent(const LogEvent* lev);
protected:
    TextFileLog* _owner;

}; // class DefTextFileLogEventRenderer


/** \brief Text file log implementation with supporting of extesible event types renderers.
 *
 *  >
 */
class TextFileLog 
    : public ILogRecorder       //  implements
//    : public ILogRecorder     // implements
{
public:
    //------------<Types>--------------------------------------
    
    /** \brief Text file encoding. */
    enum Encoding {
        enDefault,
        enWide,
        enUTF8,
        enUTF16LE
    }; // enum Encoding

    /** \brief Renderer name to Renderer mapping type. */
    typedef std::map<std::string, ITextFileLogEventRenderer*> RenderersNamesMap;

public:
    //------------<Consts>-------------------------------------
    
    // common
    static const char* FILE_OPENING_FLAGS;
    static const char* FILE_ENC_WIDE;
    static const char* FILE_ENC_UTF8;
    static const char* FILE_ENC_UTF16LE;

    // errors and exceptions
    static const char* ER_NO_FILE_NAME;
    static const char* ER_CANT_OPEN_FILE;
    static const char* ER_FILE_NOT_OPEN;
    static const char* ER_FILE_IS_OPEN;
    static const char* ER_NO_NAME;
    static const char* ER_DUPL_NAME;
    static const char* ER_NO_RENDERER_FOUND;


public:
    //------------<Public Static Methods>----------------------
    
    /** \brief Returns a string representation for a giiven encoding */
    static const char* getFileEncodingStr(Encoding enc);

public:
    //------------<Constructors and destructor>----------------        
    
    /** \brief Initialize with a log file name and opens the file if need. */
    TextFileLog(const char* fileName = nullptr, ITextFileLogEventRenderer* defRenderer = nullptr, 
        bool autoOpen = false, Encoding enc = enDefault);

    /** \brief Destructor */
    ~TextFileLog();

    //------------<Public Methods>-----------------------------
    
    /** \brief Opens the log file. */
    void openFile();

    /** \brief Closes the log file. */
    void closeFile();
    
    
    /** \brief Writes a string to a file. */
    void writeStr(const char* str);

    /** \brief Flushes file if an autoFlush flag is set. */
    void flushIfNeed();

    /** \brief Flushes file. */
    void flushFile();


    //---<renderers>--
    /** \brief Returns a render by its name given as a parameter. */
    ITextFileLogEventRenderer* getRendererByName(const std::string& name, 
        bool getDef = true);


    /** \brief Adds renderer for a type */
    ITextFileLogEventRenderer* addRenderer(const std::string& name, 
        ITextFileLogEventRenderer* render);
    
    /** \brief Deletes all the renderers including a default one. */
    void clearAllRenderers();



    //----<ILogRecorder implementations>----

    // ILogRecorder::proceedLogEvent() Proceeds a new log event given thru a pointer.
    virtual void proceedLogEvent(const LogEvent* lev);


    //------------<Sets/Gets>----------------------------------
    
    /** \brief Returns true if the log file is open, false otherwise. */
    bool isOpen() const { return (_file != nullptr); }

    /** \brief Returns true, if an immediate flush after each writing is needed */
    // TODO: включить возможность управлять флагом
    bool isNeedAutoFlush() const { return true; }

    /** \brief Sets file name for a non-active log. */
    void setFileName(const char* filename);

    /** \brief Gets log file name. */
    const std::string& getFileName() const { return _fileName; }



    //---<renderers>--
    /** \brief Sets default renderer. */
    ITextFileLogEventRenderer* setDefRenderer(ITextFileLogEventRenderer* newDefRender);

protected:
    //------------<Protected fields>---------------------------


    /** \brief File descriptor. */
    FILE* _file;
    
    
    /** \brief Represents file name. */
    std::string _fileName;

    /** \brief Text file encoding. */
    Encoding _encoding;

    /** \brief A default renderer for a types that do not have their own renderers. */
    ITextFileLogEventRenderer* _defRenderer;

    /** \brief Renderers list. */
    RenderersNamesMap _renderers;

    
protected:
    //------------<Protected methods>--------------------------
    
    /** \brief Checks if a log file is open. Otherwise throws an extension. */
    void checkIfFileOpen();



}; // class TextFileLog









}; // namespace log 
}; // namespace xi



#endif // filelogH