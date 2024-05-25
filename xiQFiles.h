/*******************************************************************************
* �������� ...
* ������ ��� Visual Studio 10. ������������: Win32
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
 *  ����������� ����� �����-�������, ������� ������� � �������� ������ ������ ����� ������ ������, ����� XML ���������� ���� ����������.
 *  ����� ����� ��������� ������ �� ����� ������ "������".
 */
class XmlTextFileExtender  : public QFile {

    /**
     *  ������� ������ ������������
     */
    enum Stage {
      stPre,
      stRegular,
      stPost,
      stStop

    };
public:

    /** \brief ����������� �������������� ������ ����� \param fileName, � ���������� ���������� \param preData
     *  � �������� \param postData. 
     */
    XmlTextFileExtender(const QString& fileName, const std::string& preData, const std::string& postData);


    /** \brief ���������� �����, ���������� ����� ������ � ������ �� ������ ����, ��� � ����� ����, �� � ��������� � �������.
     *
     *  >
     */
    virtual qint64	bytesAvailable () const;


protected:
    

    /** \brief �������� ������ �� ��������� \param src �������� \param size � ���������� \param dest.
     *
     *  ��������������, ��� ����������� ����� ���� \param maxSize ���� �� ��������� ��������. ����� ������ ���� ����������� 
     *  \param size ����, �� ��������� \param alreadyCopied ����� ������, �����������, ������� � ���������������� ���� ��� 
     *  ���� �����������.
     *  ������������ �������� ��������� ������ ������������ ������ src �� alreadyCopied ���� ��������������!
     *  ���������� \result ������, ���� ��� �������� ���� ��������� � ������������������ ������ ��� ����������� �����
     *  ������ ������ (�.�. ��� ������ ������������), ����� false � ���� ��� ������ ��� �����������.
     *  ����� ������������ ������ \param pieceSize ���������� ������ ������, ������������� � ���� ���.
     */
    static bool copyData(void* dest, const void* src, size_t size, size_t maxSize, size_t& alreadyCopied, size_t& pieceSize);


    /** \brief ���������� ����� ������ ������, �����������, ��� ��� � ���������������� ���� ���������� ������:
     *  ������ �� �����, �� ���������� ��� �������.
     *
     *  ���������� ���� �����, �� ����� ��������������, ��� ��� �� ����� ���� ���������� � �������.
     *  XML ������ �������� �� 16 ��, ������� ����� ��� �������� �����, ��� ����..
     */
    virtual qint64	readData ( char * data, qint64 maxSize );


protected:
    //bool _firstRead;
    //const std::string& _preData;        // � ���������� ������������ ������������ ��� ������-�� �� ��������! TODO: �����������! 
    //const std::string& _postData;

    std::string _preData;        // � ���������� ������������ ������������ ��� ������-�� �� ��������! TODO: �����������! 
    std::string _postData;


    size_t _preWritten;          ///< ����� ���� ���������, ���������� �� ����. ��������
    size_t _postWritten;         ///< ����� ���� ����������, ���������� �� ����. ��������

    Stage _stage;

}; // class XmlTextFileExtender

}; // namespace qext
}; // namespace xi







#endif // xiQFilesH
