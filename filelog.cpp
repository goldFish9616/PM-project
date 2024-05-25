#include "stdafx.h"

#include "xi/log/filelog.h"

#include <string>

using namespace std;

namespace xi {
namespace log {



//==============================================================================
// class DefTextFileLogEventRenderer
//==============================================================================

// const
const char* DefTextFileLogEventRenderer::ER_NO_OWNER_LOG_SET    = "No owner log set";

//------------------------------------------------------------------------------
// Default constructor 
//------------------------------------------------------------------------------
DefTextFileLogEventRenderer::DefTextFileLogEventRenderer()
    : _owner(nullptr)
{
    //
}


//------------------------------------------------------------------------------
// ITextFileLogEventRenderer::setOwner() Sets owner text file log.
// virtual 
//------------------------------------------------------------------------------
void DefTextFileLogEventRenderer::setOwner(TextFileLog* fLog)
{
    _owner = fLog;
}


 
//------------------------------------------------------------------------------
// ITextFileLogEventRenderer::renderEvent Main method: renders a given event 
// with using a owner text file log.
// virtual 
//------------------------------------------------------------------------------
void DefTextFileLogEventRenderer::renderEvent(const LogEvent* lev)
{
    if(_owner == nullptr)
        throw LogException(ER_NO_OWNER_LOG_SET);

    // не задумыва€сь (сильно) о насто€щем типе событи€, просто выведем в файл
    // его текстовое представление

    string s = lev->asString();

    _owner->writeStr(s.c_str());
    _owner->writeStr("\n");
    

}


//==============================================================================
// class TextFileLog
//==============================================================================

// consts

    
const char* TextFileLog::FILE_OPENING_FLAGS     = "a+t";
const char* TextFileLog::FILE_ENC_WIDE          = "ccs=UNICODE";
const char* TextFileLog::FILE_ENC_UTF8          = "ccs=UTF-8";
const char* TextFileLog::FILE_ENC_UTF16LE       = "ccs=UTF-16LE";



const char* TextFileLog::ER_NO_FILE_NAME        = "No log file name presented";
const char* TextFileLog::ER_CANT_OPEN_FILE      = "Can't open file <%s>. Error #%d";
const char* TextFileLog::ER_FILE_NOT_OPEN       = "Log file not open";
const char* TextFileLog::ER_FILE_IS_OPEN        = "Can not proceed the operation when file open";
const char* TextFileLog::ER_NO_NAME             = "No name for the type presented";
const char* TextFileLog::ER_DUPL_NAME           = "Render for the given type name <%s> has already registered";
const char* TextFileLog::ER_NO_RENDERER_FOUND   = "No renderer for a type <%s> found";


/** \brief  */
//------------------------------------------------------------------------------
// Returns a string representation for a giiven encoding.
// static
//------------------------------------------------------------------------------
const char* TextFileLog::getFileEncodingStr(Encoding enc)
{
    switch(enc) {
    case enWide:
        return FILE_ENC_WIDE;
    case enUTF8:
        return FILE_ENC_UTF8;
    case enUTF16LE:
        return FILE_ENC_UTF16LE;
    }; // switch(enc)

    // по умолчанию, ничего вообще
    return nullptr;
}



//------------------------------------------------------------------------------
// Initialize with log file name.
//------------------------------------------------------------------------------
//TextFileLog::TextFileLog(const char* fileName, Encoding enc, bool autoOpen, 
//    ITextFileLogEventRenderer* defRenderer)
TextFileLog::TextFileLog(const char* fileName, ITextFileLogEventRenderer* defRenderer, 
        bool autoOpen, Encoding enc)
    : _file(nullptr)
    , _encoding(enc)
    , _fileName(fileName)
    , _defRenderer(defRenderer)
{
    if(autoOpen)
        openFile();

    //if(defRenderer != nullptr)
    //    defRenderer->setOwner(this);
    setDefRenderer(defRenderer);

    // TODO: создать рендерер по умолчанию, если нет дефолтного!.. а надо ли?!
}


//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
TextFileLog::~TextFileLog()
{
    // всегда пытаетс€ закрыть файл
    closeFile();

    // удал€ем все рендереры
    clearAllRenderers();

}

//------------------------------------------------------------------------------
// Opens the log file.
//------------------------------------------------------------------------------
void TextFileLog::openFile()
{
    if(isOpen())
        return;                 // уже открыт

    // если не задано им€
    if(_fileName.empty())
        throw LogException(ER_NO_FILE_NAME);

    // задаем гибкий режим открыти€ файла
    string fileOpenMode = FILE_OPENING_FLAGS;
    if(_encoding != enDefault)
    {
        fileOpenMode += ", ";
        fileOpenMode += getFileEncodingStr(_encoding);
    }
    

    // пытаемс€ открыть
    _file = fopen(_fileName.c_str() , fileOpenMode.c_str());

    // если не открылась, значит ошибка
    if(!_file)
    {
        LogException::throwException(ER_CANT_OPEN_FILE, 
            _fileName.c_str() ,
            errno);
    }

   
    // если открылась, возможно что-то надо сделать

}
   

//------------------------------------------------------------------------------
// Closes the log file.
//------------------------------------------------------------------------------
void TextFileLog::closeFile()
{
    if(!isOpen())
        return;                 // не открыт

    fclose(_file);              // закрываем
    _file = nullptr;

}




//------------------------------------------------------------------------------
// Writes a string to a file.
//------------------------------------------------------------------------------
void TextFileLog::writeStr(const char* str)
{
    checkIfFileOpen();

    // важно! этот метод fprintf  нельз€ использовать дл€ unicode-кодировок!

    fprintf(_file, str); 
    flushIfNeed();                   // сбрасывает буфер, если необходимо        
}


//------------------------------------------------------------------------------
// Flushes file
//------------------------------------------------------------------------------
void TextFileLog::flushFile()
{
    checkIfFileOpen();
    fflush(_file);

}

//------------------------------------------------------------------------------
// ≈сли необходим немедленный сброс буфера на жесткий диск, вызывает виртуальный
// метод, наследники класса должны его реализовать в плане сброса буфера
//------------------------------------------------------------------------------
void TextFileLog::flushIfNeed()
{
   if(isNeedAutoFlush())
      flushFile();                  // д/б перекрыта в наследнике-классе
}


    
//------------------------------------------------------------------------------
// Checks if a log file is open. Otherwise throws an extension.
//------------------------------------------------------------------------------
void TextFileLog::checkIfFileOpen()
{
    if(!isOpen())
        throw LogException(ER_FILE_NOT_OPEN);
}

//------------------------------------------------------------------------------
// ”дал€ет все рендеры, в т.ч. и дефолтный
//------------------------------------------------------------------------------
void TextFileLog::clearAllRenderers()
{
    if(_defRenderer)
        delete _defRenderer;

    _defRenderer = nullptr;


    for(RenderersNamesMap::iterator curRenderIt = _renderers.begin();
        curRenderIt != _renderers.end();   ++curRenderIt)
    {
        ITextFileLogEventRenderer* ren = curRenderIt->second;
        delete ren;                 // убираем рендер...
    }

    _renderers.clear();               // очищаем мапу
}

//------------------------------------------------------------------------------
// ¬озвращает рендер по имени типа блока
//------------------------------------------------------------------------------
ITextFileLogEventRenderer* TextFileLog::getRendererByName(const string& name, 
    bool getDef)
{
    RenderersNamesMap::iterator renderIt = _renderers.find(name);

    if(renderIt == _renderers.end())
    {
        // если в качестве последней попытки прос€т вернуть хот€ бы дефолтный
        if(getDef)
            return _defRenderer;

        return nullptr;                     // ну, на нет и суда нет
    }
        

    return renderIt->second;                    // найдено
}


//------------------------------------------------------------------------------
// ƒобавл€ет парочку "им€ рендера -- рендер"
//------------------------------------------------------------------------------
ITextFileLogEventRenderer* TextFileLog::addRenderer(const string& name, ITextFileLogEventRenderer* render)
{

    // нельз€ добавл€ть рендеры без прив€зки к имени типа блока
    if(name.empty())
        throw LogException(ER_NO_NAME);
        //LogException::throwException(ER_NO_RENDERER_FOUND, name.c_str());
        //throw DPMException(ER_NO_NAME);

    // дл€ одного типа  Ч один рендер. хотим больше Ч используйте другой BlocksRenderers
    if(getRendererByName(name) != nullptr)
        LogException::throwException(ER_DUPL_NAME, name.c_str());        

    // кладем в мапу новую св€зочку "им€ типа блока Ч рендер"    
    _renderers[name] = render;

    // прив€зываем рендерер к этому логу
    render->setOwner(this);

    return render;
}

//------------------------------------------------------------------------------
// ”станавливает рендер по умолчанию.
//------------------------------------------------------------------------------
ITextFileLogEventRenderer* TextFileLog::setDefRenderer(ITextFileLogEventRenderer* newDefRender)
{
    ITextFileLogEventRenderer* prevRender = _defRenderer;

    if(prevRender != nullptr)
        prevRender->setOwner(nullptr);      // все, нет больше хоз€ина!

    _defRenderer = newDefRender;

    if(_defRenderer != nullptr)
        _defRenderer->setOwner(this);      // хоз€ин дл€ нового рендера

    return prevRender;
}


//------------------------------------------------------------------------------
// ILogRecorder::proceedLogEvent() Proceeds a new log event given thru a pointer.
// virtual
//------------------------------------------------------------------------------
void TextFileLog::proceedLogEvent(const LogEvent* lev)
{
    // ищем подход€щий рендерер
    ITextFileLogEventRenderer* renderer = nullptr;

    // сперва в мапе отображени€
    RenderersNamesMap::const_iterator rendIt = _renderers.find(lev->getType());
    if(rendIt != _renderers.end())
        renderer = rendIt->second;
    else
        renderer = _defRenderer;

    // если ни в мапе, ни дефолтного Ч ничего нет...  все, рендерить нельз€!
    if(renderer == nullptr)
        LogException::throwException(ER_NO_RENDERER_FOUND, lev->getType().c_str()); 

    // дошли сюда, значит есть рендерер, и Ч упирод!
    renderer->renderEvent(lev);

    //return lev;

}



//------------------------------------------------------------------------------
//  Sets file name for a non-active log.
//------------------------------------------------------------------------------
void TextFileLog::setFileName(const char* filename)
{
    // дл€ открытого лога нельз€
    if(isOpen())
        throw LogException(ER_FILE_IS_OPEN);

    _fileName = filename;
}


}; // namespace log 
}; // namespace xi