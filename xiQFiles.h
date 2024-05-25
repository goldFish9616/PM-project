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

/** \file 
 *  Contains some QFile extenders for various tasks.
 *  Is a candidate for a library member.
 */


#ifndef xiQFilesH
#define xiQFilesH


#include <QDir>


namespace xi {
namespace qext {

/** \brief Files Visitor pattern implementation with a Qt slots/signals
 *
 *  >
 */
class FileVisitor : public QObject {
    Q_OBJECT

public:


public:
    //------------<Constructors>---------------- 
    FileVisitor(QString nameFilter="*", bool recurs = true, bool symlinks = false);
    FileVisitor(QStringList nameFilters, bool recurs = true, bool symlinks = false);

public slots:


    /** @short processes a single directory entry
    Does not care if it is a directory or a file -
    does all the proper checking before calling the
    appropriate function, @ref processFile() on files,
    or @ref processDir() on directories.
    @param pathname location of a directory or a file
    */
    void processEntry(const QString& pathname);

signals:
    /** @short emitted whenever a file is found
        @param filename an absolute path
    */
    void foundFile(const QString& fileName);

public:
    //------------<Public Methods>---------------- 

    void processEntry(QFileInfo finfo);       
    void processDir(QDir& d);
    void processFile(const QString& fileName);




private:
    //------------<Private fields>---------------- 

    QStringList _filterList;                  ///< Log files filter list
    QDir::Filters _dirFilter;
    bool _recursive;

private:
    //------------<Private methods>---------------- 
    void initDirFilter(bool symlinks);


}; // class FileVisitor


/**
 *  Специальный класс файла-обманки, который поддает в качестве первой строки файла ложную строку, чтобы XML чувствовал себя счастливым.
 *  Также после окончания данных из файла выдает "подвал".
 */
class XmlTextFileExtender  : public QFile {

    /**
     *  Которая порция записывается
     */
    enum Stage {
      stPre,
      stRegular,
      stPost,
      stStop

    };
public:

    /** \brief Конструктор инициализирует именем файла \param fileName, и строковыми преамбулой \param preData
     *  и подвалом \param postData. 
     */
    XmlTextFileExtender(const QString& fileName, const std::string& preData, const std::string& postData);


    /** \brief Перекрытый метод, возвращает длину данных с учетом не только того, что в файле есть, но и преамбулы и подвала.
     *
     *  >
     */
    virtual qint64	bytesAvailable () const;


protected:
    

    /** \brief Копирует данные из источника \param src размером \param size в получатель \param dest.
     *
     *  Предполагается, что скопировано может быть \param maxSize байт за очередную итерацию. Всего должно быть скопировано 
     *  \param size байт, по указателю \param alreadyCopied лежит объект, указывающий, сколько в действительности байт уже 
     *  было скопировано.
     *  Осуществляет смещение источника данных относительно буфера src на alreadyCopied байт самостоятельно!
     *  Возвращает \result истину, если эта операция была последней в последовательности вызова для копирования всего
     *  объема данных (т.е. все данных выкопированы), иначе false — есть еще данные для копирования.
     *  Через обязательный объект \param pieceSize возвращает размер данных, скопированных в этот раз.
     */
    static bool copyData(void* dest, const void* src, size_t size, size_t maxSize, size_t& alreadyCopied, size_t& pieceSize);


    /** \brief Перекрытый метод чтения данных, учитывающий, что там в действительности надо выплёвывать наружу:
     *  данные из файла, из преуамбулы или подвала.
     *
     *  Перекрывая этот метод, мы можем контролировать, что там на самом деле происходит с чтением.
     *  XML читает бульками по 16 КБ, поэтому важно ему скормить сразу, что надо..
     */
    virtual qint64	readData ( char * data, qint64 maxSize );


protected:
    //bool _firstRead;
    //const std::string& _preData;        // с теперешним определением конструктора это почему-то не работает! TODO: разобраться! 
    //const std::string& _postData;

    std::string _preData;        // с теперешним определением конструктора это почему-то не работает! TODO: разобраться! 
    std::string _postData;


    size_t _preWritten;          ///< Число байт преамбулы, записанных на пред. итерации
    size_t _postWritten;         ///< Число байт хвостовика, записанных на пред. итерации

    Stage _stage;

}; // class XmlTextFileExtender

}; // namespace qext
}; // namespace xi







#endif // xiQFilesH
