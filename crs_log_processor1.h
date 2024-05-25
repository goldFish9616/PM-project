/////////////s///////////////////////////////////////////////////////////////////
/// \file
/// \brief Components for CRS logs processing
///
/// Details
////////////////////////////////////////////////////////////////////////////////

#ifndef crs_log_processor1H
#define crs_log_processor1H

#include <string>
#include <map>
#include <time.h>
#include <iosfwd>
#include <iostream>
#include <QDir>
#include <QXmlStreamReader>
#include <QStringRef>
#include <QStringView>

//#include "xiTypes.h"
#include "xi/xiTypes.h"
#include "conv1.h"

//#include "xidbsqlite\xidbsqlite.h"
#include "xi/db/sqlite/xidbsqlite.h"
#include "xi/log/filelog.h"


using std::string;
using std::map;

using namespace xi;
using namespace xi::types;

namespace CRS_processing {



/** \brief An CRS log processing specific exception.
 *
 *  
 */
class CRSLPException : public std::runtime_error {
        //public std::exception {

public:
    static const int MAX_FORMAT_STR_SIZE = 1024;

public:
    //------------<Constructors>---------------- 
    
    /// Default
    CRSLPException(): 
        std::runtime_error("") ,
        _hasErrMes(false)
    {
    }
        
    /// With Info message 
    CRSLPException(const char * const & wh):
        std::runtime_error(wh),
        _hasErrMes(false)
    {
    }

    /// With Formatting
    CRSLPException(bool, const char * _Format, ...);//: 
        //std::exception();
    //{

    //    
    //    // char * _DstBuf, size_t _SizeInBytes, const char * _Format, ...
    //    //sprintf_s()
    //}

    //virtual const char* what() const
    virtual const char* what() const noexcept
    {
        if(_hasErrMes)
            return _errMes;
        else
            return std::exception::what();
    }
protected:
    char _errMes[MAX_FORMAT_STR_SIZE];
    bool _hasErrMes;


}; // class CRSLPException




/** \brief Represents a CRS logs processor version 1.
 *
 *  This class relies on Qt framework.
 */
class CRSProcessor1 : public QObject,
    public db::DBConnection::ITransactionAction,        // Transaction action interface
    public db::SQLiteConnection::IUpdateAction          // Update action interface
{
    Q_OBJECT


public:
    //------------<Types>---------------- 
    typedef map<QString, TUint> StringIntMap;
    //typedef map<std::pair<QString, QString>, TUint> StrStrIntMap;   // <str, str> -> int


    /** \brief Класс представляет разбиение текстовой строки имени файла лога на составляющие
     *
     *  >
     */
    class LogFileNameStruct {
    public:
        //------------<Constructors>----------------
        LogFileNameStruct(const QString& fn);

    public:
        //------------<Open Methods>----------------
        QString getDomPrj() const
        {
            return subs[3] + "." + subs[4];
        }

        const QString& getPackage() const
        {
            return subs[5];
        }

    public:
        //------------<Open Fields>----------------

        // открытые поля — компоненты имени файла
        //QString datePreamble;               ///< преамбула даты
        //QString timePreamble;               ///< преамбула времени
        //QString domPrj;                     ///< домен-проект, без префикса pu.
        //QString package;                    ///< пакет — process или service только
        //QString oper;                       ///< название операции

        QStringList subs;                   ///< подстроки, на которые производится разбиение

    }; // class LogFileNameStruct


//New class for Payload information

    class PayloadNode
    {
    public:
//        std::vector<PayloadNode*> _children;

        PayloadNode (const QString& name){
            _attName = name;
        }

        PayloadNode(){ }

        void setLevel(int level){
            _lev = level;
        }
        void setValue(const QString& val){
            _value = val;
        }
        void setAttribute(const QString& t){
            _attName = t;
        }
        void setTime(time_t& t){
            _timeStamp = t;
        }

        QString getValue() const{
            return _value;
        }

        int getLevel() const{
            return _lev;
        }

        time_t getTime() const{
            return _timeStamp;
        }

        QString getAttributeName() const{
            return _attName;
        }

//        void addChild(PayloadNode* node){
//            _children.push_back(node);
//        }
    private:
        QString _value = "NULL";
        int _lev = 0;
        QString _attName;
        time_t _timeStamp;
    };

        

    /** \brief Representation for event entry  "logger" string.
     *
     *  Sections structure (0-based) where there are k sections:
     *  0, 1, 2, 3 — tracer.de.der.pu
     *  4 — domains or process
     *  5 — name of ^^
     *  6, 7, ..., (k-3) - package name 
     *  (k-2) — Interface name
     *  (k-1) — method name
     */
    class LoggerStruct {
    public:
        //------------<Types>----------------
        enum DomainOrPG  {
            dpDomains,
            dpPG
        }; // enum DomainOrPG


    public:
        //------------<Constructors>----------------
        //LoggerStruct(const QString& lg);
        LoggerStruct();
        LoggerStruct(const QStringRef& lg);

    public:
        //------------<Open Methods>----------------

        //void setLogger(const QStringRef& lg);
        void setLogger(const QStringView& lg);
        void clear();

        //QString getDomPrj() const
        //{
        //    return subs[3] + "." + subs[4];
        //}

        //const QString& getPackage() const
        //{
        //    return subs[5];
        //}

        /// Returns type of event
        DomainOrPG domainOrPG() const;

        /// Returns name of unit (PG or domain!)
        QString unitName() const; 
        //QString dpgName() const;  // old bad name, moreover never used

        /// Returns name of the package
        QString packName() const;

        /// Returns name of the Interface (Process or Service)
        QString interfaceName() const;

        /// Returns name of the method
        QString methodName() const;

        /// Returns fully qualified name for interface including domain or process token as first
        QString interfaceFullName() const;



    public:
        //------------<Open Fields>----------------

        QString _loggerStr;


        // открытые поля — компоненты логгера
        //QStringList subs;                   ///< подстроки, на которые производится разбиение

    private:
        //------------<Private Method>----------------
        void determineSectionsNum();            ///< Determines how many section does the string have.

    private:
        //------------<Private fields>----------------
        int _sectionsNum;



    }; // class LoggerStruct


    /**
     * Represents one log tracing entry.
     *  >
     */
    class CRSTracingEntry {
        friend class CRSProcessor1;

    public:
        //------------<Consts>----------------

        static const char* EV_TYPE_ABBR_REQ;
        static const char* EV_TYPE_ABBR_RES;
        static const char* EV_TYPE_ABBR_RESEXC;

    public:
        //------------<Types>----------------
        enum EventType {
            etRequest,
            etResponse,
            etResponseException,                ///< <![CDATA[Response with Exception]]>
            // Exception тоже наверное будет, для всех остальных есть мастербанк!

        }; // enum EventType


    public:
        //------------<Constructors>----------------
        CRSTracingEntry();

    public:
        //------------<Public Methods>---------------- 
        void reset();                           ///< Resets all the fields to their initial state

        //------------<Set/Get>-----------------------
        void setInvID(const QString& s);        ///< Sets inv id given as a string
        //types::TQword  GetInvID() const         //
        uint64_t  GetInvID() const         //
        {
            return _invID; 
        }

        void setlog4jTimestamp(const QString& s);   ///< Sets timestamp as string
        //void setlog4jEventType(const QStringRef& s);   ///< Sets eventtype
        void setlog4jEventType(QStringView sv);   ///< Sets eventtype


        //-- Invokation ID fields
        // Node Name        
        void setInvIDNodeName (const QString& iinn)
        {
            _invIDNodeName = iinn;             
        }

        const QString& getInvIDNodeName() const
        {
            return _invIDNodeName;
        }

        // IP Address        
        void setInvIDipAddr (const QString& iiia)
        {
            _invIDipAddr = iiia;             
        }
        
        const QString& getInvIDipAddr() const
        {
            return _invIDipAddr;
        }



        /** \brief Returns an event type 2-letters abbreviation
         *
         *  >
         */
        const char* getEventTypeAbbr() const ;


        /** \brief Returns event timestamp.
         *
         *  >
         */
        time_t getL4jevTimestamp() const
        {
            return _l4jev_timestamp;
        }

        std::vector<PayloadNode*> getAllPayload()
        {
            return _payloadVec;
        }

        void addPayload(PayloadNode*& node)
        {
            _payloadVec.push_back(node);
        }

        void addPayloadValue(const QString& val)
        {
            _cur_payload->setValue(val);
        }

        PayloadNode* getPayloadNode(int ind) const
        {
            return _payloadVec[ind];
        }

        std::vector<PayloadNode*> getPayVector() const
        {
            return _payloadVec;
        }

        PayloadNode* getPayload()
        {
            return _cur_payload;
        }

        QString getPayloadName()
        {
            return _cur_payload->getAttributeName();
        }

        QString getPayloadValue()
        {
            return _cur_payload->getValue();
        }

        int getPayloadLevel()
        {
            return _cur_payload->getLevel();

        }

        void increaseEventID()
        {
            _eventID += 1;
        }

        void increaseInterfaceID()
        {
            _interfaceID += 1;
        }



    private:
        //------------<Private fields>---------------- 

    //public: // это плохо, но лень писать оборачивалки

        // log entry fields
        //types::TQword   _invID;
//        uint64_t   _invID;
        int64_t   _invID;
        int64_t   _eventID = 0;
        int64_t   _interfaceID = 1;
        
        //QString         _l4jev_logger;      // атрибут log4j:event
        LoggerStruct    _l4jev_logger;      // атрибут log4j:event
        
        time_t          _l4jev_timestamp;   // атрибут timestamp
        EventType       _l4jev_EventType;   // значение запроса 
        TUint           _payloadSize;       // размер payload в байтах
        TUint           _payloadCRC32;      // CRC32 Boost-based payload checksum

        std::vector<PayloadNode*> _payloadVec; // vector which saves all value of payload section

        // Invokation ID fields
        QString _invIDNodeName;             // Node Name
        QString _invIDipAddr;               // IP Address
        PayloadNode* _cur_payload;


    }; // class CRSLogEntry




public:
    //------------<Consts>---------------- 
    static const char* XML_EL_ID;
    static const char16_t* XML_EL_TRACEEV;
    //static const char* XML_EL_LOG4JEVENT;
    static const char16_t* XML_EL_LOG4JEVENT;
    static const char* XML_EL_LOG4JEV_MESSAGE;
    static const char* XML_EL_PAYLOAD;
    static const char* XML_EL_INVID_NODENAME;
    static const char* XML_EL_INVID_IPADR;

    

    static //const char* //const char16_t*
        const QString XML_ATT_LOG4JEV_LOGGER;
    static const char* XML_ATT_LOG4JEV_TIMESTAMP;

    static const char16_t* XML_VAL_LOG4JEV_MES_REQ;
    static const char16_t* XML_VAL_LOG4JEV_MES_RES;
    static const char16_t* XML_VAL_LOG4JEV_MES_RESEXC;
    
    
    static const int DEF_COMMIT_EVERY_NTH_EVENT = 50;
        
    

                      
    

public:


    //------------<Constructors>---------------- 
    CRSProcessor1();                                    ///< Default constructor.
    CRSProcessor1(const QString& dbFileName);   


    //~CRSProcessor1();


public:
    //------------<Public Methods>---------------- 

    void convertLog2DB(const QString& fileName);                    ///< Main entry point for convertion procedure

    // вспомогательные

protected:
    //------------<Protected Methods>---------------- 

    /**
     * Смотрит в мапу, есть ли в таблице DomainProj такая запись, если есть
     * возвращает ИД, если нет — добавляет и возвращает
     */
    //TUint getDomPrj(const QString& domPrj);
    TUint getDomPrj(const LogFileNameStruct& lfn);
    

    // //TODO: DEBUG: delete it!
    TUint testGetDomPrj();


    /**
     *  Proceeds another XML section.
     */
    void XMLParseStartEl(QXmlStreamReader& xml);        ///< Parses beginning of the current element
    void XMLParseEndEl(QXmlStreamReader& xml);          ///< Parses end of the current element
    void XMLParseChars(QXmlStreamReader& xml);          ///< Parses the text content

    void XMLParsePayload(const QString& txt);        ///< Parses payload sector. ZJ
    void XMLParsePayloadStart(QXmlStreamReader& xml);   ///< Parses beginning of the current element in payload
    void XMLParsePayloadEnd(QXmlStreamReader& xml);     ///< Parses end of the current element in payload
    void XMLParsePayloadChars(QXmlStreamReader& xml);   ///< Parse the text content of Payload


    void saveCurEntry2DB();                             ///< Saves current extracted entry to database.


    //void scanStructure();

    // //void iterateDateDir(const string& dirName);        ///< Iterates given direcory (by its name \param dirName)
    //void processDateDir(const QString& dirName);        ///< Iterates given direcory (by its name \param dirName)
    //void processDateDir(QDir& d);
    // //void iterateDateDir(QDir dir);        ///< Iterates given direcory (by its name \param dirName)

    //void processTimeDir(QDir& d);                       ///< Iterates timed dir 
    // //void processLogItemDir(QDir& dir);                  ///< Iterates log item dir

    //static bool skipSpecialDir(const QDir& d);          ///< Return true if a given dir is a special one 

    // это все ^^^ в печку!

    //void processLogEntry(QFileInfo finfo, bool recurs = true);        ///< Processes log entry which can be both file name or dir
    //void processLogDir(QDir& d, bool recurs = true);
    //void processLogFile(const QString& fileName);


public:
    //------------<Set/Get>----------------
    /**
     *  Return the number of seconds spent for the procedure
     */
    int getSecsSpent() const 
    { 
        return _secsSpent;
    }

    //-/** \brief Sets a log file name. */
    //void setLogFileName(const char* fn) { _log.setFileName(fn); };


    //QString getBaseLogDir() const
    //{
    //    return _baseLogDir;    
    //}

    //void setBaseLogDir(const QString& ld)
    //{
    //    _baseLogDir = ld;
    //}


public:
    //------------<Callback interfaces implementations>----------------

    //--<DB::DBConnection::ITransactionAction>--
    typedef db::DBConnection::ITransactionAction ITA;
    

    //virtual void db::DBConnection::ITransactionAction::OnBegin(db::DBConnection* con) override;       // no action at all, but still have to be presented

    /*virtual*/ void /*ITA::*/OnBegin(db::DBConnection* con) override;// {} ;       // no action at all, but still have to be presented
    /*virtual */bool /*ITA::*/OnCommit(db::DBConnection* con) override;
    /*virtual */void /*ITA::*/OnRollback(db::DBConnection* con) override;

    //--<DB::SQLiteConnection::IUpdateAction>--
    typedef db::SQLiteConnection::IUpdateAction IUA;

    /*virtual */void /*IUA::*/OnUpdate(db::SQLiteConnection* con, db::DBConnection::SQLAction act,
            const char* dbName, const char* tableName, //TInt64 rowid);
                               int64_t rowid) override;



private:
    //------------<Private fields>---------------- 
    //QString _baseLogDir;                       ///< Base directory for the log files
    
    //bool _recursiveProcessing;                  ///< Flag indicates that the log extraction process is a recursive one


    //QStringList _lfFilterList;                  ///< Log files filter list

    db::SQLiteConnection _db;                   ///< Database to import log

    // DB converting helper data
    StringIntMap _domPrjId;                     // отображение DomPrj -> ID (row)

    StringIntMap _interfacesCache;              /// отображение Interface -> ID (row)



//private:
public: // временно для отладки
    //------------<Private Methods>---------------- 
    void createDBStructure();                           ///< Creates DB structure. Should be private


public:
    // отладочные методы
    void test1();       // тестовый метод
    void test2();       // тестовый метод


private slots:
    void onFoundFile1(const QString& fileName);     ///< Обработчик найденного файла для создания структуры


private:
    //------------<Private Methods>---------------- 
    void resetParseInfo();                  ///< Resets all the fields corresponding with parsing procedure
    void insertEventPayloadToDB(/*const char*& name, const char*& value,
                                    int level*/); // insert Payload Information into table "ExtractedPayload" in DB.
    TUint getInterfaceID4CurEntry();        ///< Gets interface ID for currently processed entry

    /** \brief Query DB for curEntry interface id.
     *
     *  If an interface obtained thru current processed entry is not found in cache map,
     *  then it should be asked if the interace has already been added to the DB.
     *  \return 0 if it has no been added yet, or its ID itherwise
     */
    TUint queryInterfaceNameFromDB(const char* prdm, const char* unitName, const char* packName, const char* interfaceName);


    /** \brief Inserts interface record to DB
     *
     *  Invoked where no appropriate record were found.
     */
    TUint insertInterfaceNameToDB(const char* prdm, const char* unitName,
        const char* packName, const char* interfaceName);


    
    /** \brief Inserts event entry record to DB. 
     *
     *  \param interfaceID determines an ID (PK) of a record from Interfaces table.
     *  \return ID of the inserten event record.
     */
    TUint insertEventEntryToDB(TUint interfaceID);




private:
    //------------<Private fields>----------------

    typedef std::map<QString, std::vector<PayloadNode*>> PL;



    // поля для разбора XML
    TUint           _trEventsNum;           ///< Number of currently parsed  tracing events
    CRSTracingEntry _curEntry;              ///< Current tracing entry
    QString         _curElName;             ///< Stored name of currently parsed element.

    //поля для payload
    QString _curEl;                         ///< Current observed element in payload. ZJ
//    int lastLevel;                  ///< Last opened level in payload
//    PL _payloadVec;                         ///< Saves all payload information. ZJ
    int _level;                         ///< Checks if a new level of xml-tree is reached. ZJ


    int             _commitEveryNthEvent;   ///< Determine N, which is the number of events after that 
                                            ///< a commit of current transactions does.


    // time counting
    clock_t _startCl;                       ///< Time of beginning of a procedure
    clock_t _finishCl;                      ///< Time when the procedure finished
    int _secsSpent;                         ///< Total seconds spent for the procedure

    //log::TextFileLog _log;                  ///< xi Text file log


}; // class CRSProcessor1



}; // namespace CRS_processing



#endif // crs_log_processor1H
