// #include "stdafx.h"

#include "xiQFiles.h"

//#include "xi/xiTypes.h"
#include "xi/types/monikers.h"

namespace xi {
namespace qext {



//------------------------------------------------------------------------------
// class FileVisitor
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
FileVisitor::FileVisitor(QString nameFilter, bool recurs, bool symlinks):
    _recursive(recurs)
{
    _filterList << nameFilter;
    
    initDirFilter(symlinks);
}


/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
FileVisitor::FileVisitor(QStringList nameFilters, bool recurs, bool symlinks):
    _filterList(nameFilters),
    _recursive(recurs)
{
    initDirFilter(symlinks);
}


/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
void FileVisitor::initDirFilter(bool symlinks)
{
    _dirFilter = QDir::Files;
    if (symlinks == false) 
        _dirFilter = _dirFilter | QDir::NoSymLinks;

}

/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
void FileVisitor::processEntry(const QString& current) 
{
    //current = expandTilde(current);       // TODO: реализовать, если надо ~ как хомдир (см. исходный)
    QFileInfo finfo(current);
    processEntry(finfo);
}

//void processEntry(QFileInfo finfo, bool recurs = true);       
//void processDir(QDir& d, bool recurs = true);
//void processFile(const QString& fileName);


/*------------------------------------------------------------------------------
 * Processes entry which can be both file name or dir
 *----------------------------------------------------------------------------*/
void FileVisitor::processEntry(QFileInfo finfo)
{   
    if (finfo.isDir()) 
    {
        QString dirname = finfo.fileName();
        
        if ((dirname==".") || (dirname == ".."))
            return;

        QDir dir(finfo.canonicalFilePath());
        //if (skipDir(d))                       // TODO: реализовать, если нужны доп. фичи
        //    return;
        
        processDir(dir);
    } 
    else
        processFile(finfo.canonicalFilePath());
}

/*------------------------------------------------------------------------------
 * Iterates given directory
 *----------------------------------------------------------------------------*/
void FileVisitor::processDir(QDir& dir) 
{
    // по аналогии с файлами можно что-то по типу
    // emit foundDir(dirName);


    dir.setSorting(QDir::Name);

    QStringList files = dir.entryList(_filterList, _dirFilter);    // с симлинками еще подумать надо
    
    foreach(QString entry, files) {
        processEntry(dir.filePath(entry));
    }

    // если надо рекурсивно поддиректории разобрать
    if (_recursive)
    {
        QStringList dirs = dir.entryList(QDir::Dirs);
        foreach (QString curDir, dirs) {
            processEntry(dir.filePath(curDir));  // вообще, сюда для сокращенки надо бы сразу processLogDir
        }
    }
}


/*------------------------------------------------------------------------------
 *  TBD
 *----------------------------------------------------------------------------*/
void FileVisitor::processFile(const QString& fileName)
{
    emit foundFile(fileName);
}




//------------------------------------------------------------------------------
// class XmlTextFileExtender
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * Конструктор
 *----------------------------------------------------------------------------*/
XmlTextFileExtender::XmlTextFileExtender(const QString& fileName, const std::string& preData, const std::string& postData):
    QFile(fileName),
    _preData(preData),
    _postData(postData)

{

    if(preData.length() != 0)
        _stage = stPre;
    else
        _stage = stRegular;

    //_firstRead = true;
    _preWritten = 0;
    _postWritten = 0;

}

/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
qint64	XmlTextFileExtender::bytesAvailable () const
{

    qint64 r = QFile::bytesAvailable();

    // в зависимости от стадии вывода, размер данных может включать (а может и не включать) размер преамбулы и подвала
    switch(_stage)
    {
        case stPre:
            return  r + (_preData.length() - _preWritten) + _postData.length();
            
        case stRegular:
            return r + _postData.length();

        case stPost:
            return _postData.length() - _postWritten;

    };  // switch(_stage)


    // // если дошли до конца нормального файла, делаем ход конем
    //if(r == 0)
    //{
    //    _stage = stLast;
    //    r = 
    //}

    // во всех остальных случаях, если такие возможны
    return 0;

}

/*------------------------------------------------------------------------------
 * Чтение данных
 *----------------------------------------------------------------------------*/
qint64	XmlTextFileExtender::readData ( char * data, qint64 maxSize )
{
    //    // все ост. вызовы — по-нормальному, из файлика
    //    return QFile::readData(data, maxSize);
  
    //memcpy(

    size_t wasCopied;       // сколько было скопировано

    switch(_stage) {

        case stPre:
            if(copyData(data, _preData.c_str(), _preData.length(), maxSize, _preWritten, wasCopied))      // если все скопировали, идем дальше
                _stage = stRegular;
            return wasCopied;

        case stRegular:

            // здесь надо проверить, есть ли данные для записи из оригинального файла
            if(QFile::bytesAvailable() != 0)
                return QFile::readData(data, maxSize);

            // дошли сюда, значит в самом файле ничего больше нет!
            if(_postData.length() != 0)
            {
                _stage = stPost;                    // переключим режим
                return readData(data, maxSize);     // да, самовызовемся
            }
             
            // а уже ежли сюда дошли, значит и в основном файле ничего нет, и в хвосте нет, все, пока!
            _stage = stStop;
            return -1;                              // по спецификации больше быть не может
                
        case stPost:
            if(copyData(data, _postData.c_str(), _postData.length(), maxSize, _postWritten, wasCopied))      // если все скопировали, идем дальше
                _stage = stStop;
            return wasCopied;

    };  // switch

    // сюда можно прийти только в режиме stStop, что значит, что данных больше не будет!
    return -1;       // по спецификации больше быть не может


}


/*------------------------------------------------------------------------------
 * Копирует данные из источника \param src в получатель \param dest и отмечает, 
 * сколько еще осталось недокопированного.
 * static
 *----------------------------------------------------------------------------*/
bool XmlTextFileExtender::copyData(void* dest, const void* src, size_t size, 
    size_t maxSize, size_t& alreadyCopied, size_t& pieceSize)
{   
    // сколько осталось докопировать до полногсчастья
    qint64 bytesToCopyLeft = size - alreadyCopied;
    

    // сколько можно скопировать за одну итерацию: либо все, либо по максимуму
    //qint64 bytesToCopy = (bytesToCopyLeft > maxSize) ? maxSize : bytesToCopyLeft;
    pieceSize = (bytesToCopyLeft > maxSize) ? maxSize : bytesToCopyLeft;

    // копируем с заданной позиции с учетом УЖЕ скопированного (делаем здесь сдвиг)
    memcpy(dest, ((types::TByte*)src + (alreadyCopied)), pieceSize);
    //memcpy(dest, src, bytesToCopy);

    // вернем инфу о том, сколько было скопировано
    //pieceSize

    // записываем, сколько уже было сопировано (добавочка)
    (alreadyCopied) += pieceSize;


    // смотрим: все ли, что просили, сделали
    if((alreadyCopied) == size)
        return true;                // завершили этим кусочком

    return false;                   // осталось еще, что покопировать
}


}; // namespace qext
}; // namespace xi



