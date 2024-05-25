//#include "stdafx.h"

#include "crs_log_processor1.h"
#include "xiQFiles.h"

#include <memory>
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>

#include <QMessageBox>
//#include <QXmlStreamReader>
#include <QTextStream>

#include <QRegularExpression>
#include <QString>

#include <boost/crc.hpp>

// TODO: для отладки только!
//#include <Windows.h>        // для Beep


using std::shared_ptr;

namespace CRS_processing {




//------------------------------------------------------------------------------
// class CRSLPException
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * With Formatting
 * \param bool isn't used
 *----------------------------------------------------------------------------*/    
CRSLPException::CRSLPException(bool, const char * _Format, ...):
    std::runtime_error("") ,
    _hasErrMes(true)
{
    //char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, _Format);
    //vsprintf_s(_errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    vsprintf(_errMes, _Format, argptr);
    va_end(argptr);
    //printf(dest);
}




//------------------------------------------------------------------------------
// class CRSProcessor1
//------------------------------------------------------------------------------

// Consts
const char* CRSProcessor1::XML_EL_ID = "id";
const char16_t* CRSProcessor1::XML_EL_TRACEEV = u"tracingevent";
//const char* CRSProcessor1::XML_EL_LOG4JEVENT = "event"; //  log4j: опускается
const char16_t* CRSProcessor1::XML_EL_LOG4JEVENT = u"event"; //  log4j: опускается
const char* CRSProcessor1::XML_EL_LOG4JEV_MESSAGE = "message";
const char* CRSProcessor1::XML_EL_PAYLOAD = "payload";
const char* CRSProcessor1::XML_EL_INVID_NODENAME = "nodeName";
const char* CRSProcessor1::XML_EL_INVID_IPADR = "ipAddress";


//const char*
//const char16_t* CRSProcessor1::XML_ATT_LOG4JEV_LOGGER = u"logger";
const QString CRSProcessor1::XML_ATT_LOG4JEV_LOGGER = "logger";
const char* CRSProcessor1::XML_ATT_LOG4JEV_TIMESTAMP = "timestamp";

const char16_t* CRSProcessor1::XML_VAL_LOG4JEV_MES_REQ = u"Request";
const char16_t* CRSProcessor1::XML_VAL_LOG4JEV_MES_RES = u"Response";
const char16_t* CRSProcessor1::XML_VAL_LOG4JEV_MES_RESEXC = u"Response with Exception";





/*------------------------------------------------------------------------------
 * Default constructor.
 *----------------------------------------------------------------------------*/
CRSProcessor1::CRSProcessor1():
    _commitEveryNthEvent(DEF_COMMIT_EVERY_NTH_EVENT)
{
    // фильтр имен файлов, которые будем считать логами
    //_lfFilterList.append("*.txt");
    //_recursiveProcessing = true;

}

/*------------------------------------------------------------------------------
 * Именем бд
 *----------------------------------------------------------------------------*/
CRSProcessor1::CRSProcessor1(const QString& dbFileName):
    _db(dbFileName.toLocal8Bit().data()),
    _commitEveryNthEvent(DEF_COMMIT_EVERY_NTH_EVENT)
{
    //
    //QChar c;

    // добавляем обработчики для ...
    _db.set_ec_TransAction(this);   // ... транзакций 
    _db.set_ec_UpdateAction(this);  // и update-а

}





/*------------------------------------------------------------------------------
 * Main entry point for convertion procedure
 *----------------------------------------------------------------------------*/
void CRSProcessor1::convertLog2DB(const QString& fileName)
{

    // тестируем искл. ситуацию:
    //throw CRSLPException(true, "%s: %s: %d", "Test", "Module 1", 42);



    

    // тестируем файлвизитор
    qext::FileVisitor fv("*.txt", true, true);  // с симлинками!
    QObject::connect(&fv, SIGNAL(foundFile(const QString&)), 
        this, SLOT(onFoundFile1(const QString&)) );




    // start timing    
    _startCl = clock();
    
    // сперва открываем Q
    if(!_db.isOpen())
        _db.Open();    

    _db.beginTransaction(true);         // делаем все в рамкох минимум одной транзакции
    
    fv.processEntry(fileName);
    
    _db.commitTransaction(true);
    
    _finishCl = clock();
    _secsSpent = ( (_finishCl - _startCl) / CLOCKS_PER_SEC );
    


    // вообще, если БД уже была открыта, возможно бо-бо
    //if(!_db.isOpen())
    //    _db.Open();



}


/*------------------------------------------------------------------------------
 * Обработчик найденного файла для создания структуры
 *----------------------------------------------------------------------------*/
void CRSProcessor1::onFoundFile1(const QString& fileName)
{

    //Beep(100, 20);
    if(fileName == "")
    {
        std::clog << "End!";
        return;             // TODO: вообще, это exception
    }

    //OutputDebugStringA("Log File: ");
    //OutputDebugStringA(fileName.toLocal8Bit().data());
    //OutputDebugStringA("\n");
    std::clog << "Log File: " << fileName.toLocal8Bit().data() << "\n";

    qext::XmlTextFileExtender* file = new qext::XmlTextFileExtender(fileName, "<xilog>", "</xilog>"); 
    
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw CRSLPException("Can not open the XML-file");
        
    }
    

    // пытаемся работать с ним, как с XML
    QXmlStreamReader xml(file);    
    xml.setNamespaceProcessing(false);  // чтобы не спотыкалась на log4j:event


    // цикл обработки xml
    while (!xml.atEnd() && !xml.hasError())
    {
        // получаем очередной токен
        QXmlStreamReader::TokenType token = xml.readNext();
//        std::clog << "current token" << token;

        // разбираем по существу
        switch(token) {

            case QXmlStreamReader::StartElement:
            {
                //QStringRef name = xml.name();
                //QString elementName = name.toString();
                XMLParseStartEl(xml);
            }
                break;

            case QXmlStreamReader::EndElement:
            {
                XMLParseEndEl(xml);
            }
                break;


            case QXmlStreamReader::Characters:
            {
                XMLParseChars(xml);
            }
                break;



            case QXmlStreamReader::Invalid:
            {
                QXmlStreamReader::Error er = xml.error();
                QString erStr = xml.errorString();

                //OutputDebugStringA("XML Error: ");
                //OutputDebugStringA(erStr.toLocal8Bit().data());
                //OutputDebugStringA("\n");
                std::clog << "XML Error: " << erStr.toLocal8Bit().data() << "\n";
            }
                break;

            //case QXmlStreamReader::StartDocument:
            //    break;
            //case QXmlStreamReader::EndDocument:
            //    break;


        }; // switch(token)











        ////QString tokStr = xml.tokenString();
        ////QString txt = xml.text().toString();

        //QString elementName;

        ////qint64 chOffset = xml.characterOffset();
        ////qint64 lnum = xml.lineNumber();
        ////qint64 сnum = xml.columnNumber(); 

        //          


        //// пытаемся исправить неправильное состояние (разбитые строки)
        //if(token == QXmlStreamReader::Invalid)
        //{
        //    QXmlStreamReader::Error er = xml.error();
        //    QString erStr = xml.errorString();


        //    // добавляем закрывающий тэг, чтобы все почесноку...
        //    //xml.addData("<xilog>");
        //    //xml.addData("</xilog>");

        //    token = xml.readNext();

        //    bool d = (token == QXmlStreamReader::Invalid);


        //    //bool _deb_cnl = file->canReadLine();
        //    
        //    //if(_deb_cnl)
        //    //    file->readLine();

        //    //QIODevice* iodev = xml.device();
        //    //xml.clear();
        //    //iodev = xml.device();

        //    continue;
        //}


        //
        //

        //
        //if (token == QXmlStreamReader::StartDocument)
        //{
        //    // т.к. у нас не well-formed XML, надо директивно добавить внешнюю оборачивалку, в виде элемента xilog
        //    //xml.addData("<?xml version=\"1.1\" encoding='UTF-8' ?>\n");
        //    //xml.addData("<xilog>\n");


        //    continue;
        //}
        //
        //if(token == QXmlStreamReader::EndDocument)
        //    int a = 0;
        //

        //if (token == QXmlStreamReader::StartElement)
        //{
        //    QStringRef name = xml.name();
        //    //QString 
        //    
        //    // важно: если стартовый элемент <xilog>, который мы передаем руками, надо переключаться на файл!
        //    //if(name == "xilog")
        //    //{
        //    //    xml.setDevice(file);
        //    //    QIODevice* iodev = xml.device();
        //    //    xml.setNamespaceProcessing(false);  // чтобы не спотыкалась на log4j:event
        //    //}
        //    
        //    
        //    elementName = name.toString();
        //    
        //    bool _deb_en1  = (elementName == "xilog");

        //    if (xml.name() == "tracingevent")
        //        int a  = 0;
        //}


        //if (token == QXmlStreamReader::EndElement)
        //{
        //    QStringRef name = xml.name();
        //    elementName = name.toString();
        //    
        //    bool _deb_en1  = (elementName == "xilog");

        //    if (xml.name() == "tracingevent")
        //        int a  = 0;
        //}


        //bool _deb_iscdata = xml.isCDATA();
        //
        //bool _deb_conBP = (elementName == "payload");
        //
        //int a = 0;
    } // while (!xml.atEnd() && !xml.hasError())

    std::clog << "Finish processing!";
    file->close();
    delete file;

}       

/*------------------------------------------------------------------------------
 * Proceeds the beginning of another XML section
 *----------------------------------------------------------------------------*/
void CRSProcessor1::XMLParseStartEl(QXmlStreamReader& xml)
{
    // исходя из простой структуры нашего xml проверку на принадлежность к секции НЕ ПРОИЗВОДИМ!
    // это потенциальная проблема


    //QStringRef name = xml.name();
    QStringView name = xml.name();
    //QLatin1StringView name = xml.name();
    _curElName = name.toString();       // сохраняем для будущего использования

    // сверяем подряд заинтересованные имена

    //if(name == XML_EL_ID)               // Invokation ID
    //{
    //}


    if(name == XML_EL_LOG4JEVENT)       // log4j:event
    {
        //xml.attributes().value(XML_ATT_LOG4JEV_LOGGER)
        //xml.attributes().value()
        _curEntry._l4jev_logger.setLogger(xml.attributes().value(XML_ATT_LOG4JEV_LOGGER)); //.toString();
        _curEntry.setlog4jTimestamp(xml.attributes().value(XML_ATT_LOG4JEV_TIMESTAMP).toString());
    }


}

/*------------------------------------------------------------------------------
 *  Proceeds the end of another XML section
 *----------------------------------------------------------------------------*/
void CRSProcessor1::XMLParseEndEl(QXmlStreamReader& xml)
{
    //QStringRef
    QStringView name = xml.name();
    
    // по этому событию надо осуществить запись
    if(name == XML_EL_TRACEEV) // "tracingevent")
    {
        //std::clog << "tracingevent, new blog opened" << '\n'; //удалить позже
        //TQword id = _curEntry.GetInvID();
        //int a = 0;

        ++_trEventsNum;     // one more entry 's added
        saveCurEntry2DB();
        _curEntry.reset();
        
        // проверяем, надо ли сделать очередной коммит
        if(_trEventsNum % _commitEveryNthEvent == 0)
        {
            _db.commitTransaction(true);        
            _db.beginTransaction(true);
        }


    }

    _curElName = "";        // мало, вдруг там чего недоотработалось (например, закр. тег сразу после откр.)
}


/*------------------------------------------------------------------------------
 * Parses the text content 
 *----------------------------------------------------------------------------*/
void CRSProcessor1::XMLParseChars(QXmlStreamReader& xml)
{
    


    // смотрим, внутри какой секции мы живем
    if(_curElName == XML_EL_ID)     // ID
    {
//        qDebug() << "id" << _curElName <<'\n'; //удалить позже

        ////QString tokStr = xml.tokenString();
        ////QString txt = xml.text().toString();

        _curEntry.setInvID(xml.text().toString());
        
        _curElName.clear();
    }

    // log4j:event -> log4j:message : CDATA, определяющая событие
    if(_curElName == XML_EL_LOG4JEV_MESSAGE)
    {
        //
        // bool _deb_iscdata = xml.isCDATA();       // это можно проверить
        _curEntry.setlog4jEventType(xml.text());
        _curElName.clear();                         // до след элемента!
        //QString txt = xml.text().toString();
    }

    // payload // что мне нужно доделать
    if(_curElName == XML_EL_PAYLOAD)
    {
//        qDebug() << "in payload section: " << _curElName << '\n';
        bool isCDATA = xml.isCDATA();               // это можно проверить
        //QString txt = xml.text().toString();
        
        // размер ставим только для поля CDATA, т.к. перед ним и после него идут переносы строк!
        if(isCDATA)
        {
//            qDebug() << xml.text();
//            xml.skipCurrentElement();
            QString payloadStr = xml.text().toString();
            XMLParsePayload(payloadStr); // идем в парсинг payload
            _curEntry._payloadSize =  xml.text().size();

            // добавляем расчет CRC32
            // т.к. внутреннее представление представлено двухбайтовыми символами, быстрее считать CRC
            // от данных двойного размера, чем распределять доп. буфер, копировать в него данные и считать...
            boost::crc_32_type crc;

            // UPD 17/02/2024: этот вариант был для старых QStringRef,...
            //crc.process_bytes(xml.text().unicode(), _curEntry._payloadSize * 2);

            // ... для новых QStringView надо перепроверять заново, возможно тут
            // д.б. что-то по типу xml.text().utf16()
            crc.process_bytes(xml.text().data(), _curEntry._payloadSize * 2);


            _curEntry._payloadCRC32 = crc.checksum();
            // TODO:

            _curElName.clear();                     // до след элемента!
        }
    }

    // InvocationIdentifier/nodeName
    if(_curElName == XML_EL_INVID_NODENAME)
    {
        _curEntry.setInvIDNodeName(xml.text().toString());
        _curElName.clear();                         // до след элемента!
    }

    // InvocationIdentifier/IP Address
    if(_curElName == XML_EL_INVID_IPADR)
    {
        _curEntry.setInvIDipAddr(xml.text().toString());
        _curElName.clear();                         // до след элемента!
    }



}


/*------------------------------------------------------------------------------
 * Parses payload section
 *----------------------------------------------------------------------------*/

void CRSProcessor1::XMLParsePayloadChars(QXmlStreamReader& xml){
//    if (_curEl == "ServiceRequest" || _curEl == "ServiceResponse" || _curEl == "message")
//        return;
    QString val = xml.text().toString();
    QRegularExpression re("(\\w+)");
//    qDebug() << "should be written, with + ?  " << val.contains(re);
//    qDebug() << "should be written?  " << val.contains("(\\w)");


    //if(val == "\n" || val == "\t")
    //    return;

    if (val.contains(re))
        _curEntry.getAllPayload().back()->setValue(val);
//        int i = 0;
//    else
//        val = "No Info";


//        qDebug() << "now, name is: " << lastLevel->getAttributeName() << " and value is: " << lastLevel->getValue();
//    QByteArray payloadNameC = _curEntry.getPayloadName().toLocal8Bit();
//    const char* payloadNameCC = payloadNameC.data();

//    QByteArray payloadValueC = val.toLocal8Bit();
//    const char* payloadValueCC = payloadValueC.data();

//    int level = _curEntry.getPayloadLevel();

//    insertEventPayloadToDB(payloadNameCC, payloadValueCC, level);


}

//доделано
void CRSProcessor1::XMLParsePayloadStart(QXmlStreamReader& xml)
{
    QString name = xml.name().toString();
    _curEl = name;
    if (_curEl == "ServiceRequest" || _curEl == "ServiceResponse" || _curEl == "message" || _curEl == "notificationdetails")
        return;
    PayloadNode* node = new PayloadNode(_curEl); //creates a new node
//    (_curEntry.getPayload())->setLevel(_level);
//    _curEntry.getPayload()->setAttribute(name);
//    time_t time = _curEntry.getL4jevTimestamp();
    node->setLevel(_level);
    node->setAttribute(name);
//    node->setTime(time);
    _curEntry.addPayload(node);
//    _curEntry.getAllPayload().back()->setLevel(_level);
//    _curEntry.getAllPayload().back()->setAttribute(name);

//    _curEntry.getAllPayload().back()->setTime(time);


//    qDebug() << "current element name in payload: " << node->getAttributeName() << "current payload node level" << node->getLevel();

//    if(_level == 0)
//    {
        /*if (auto search = _curEntry.getAllPayload().find(_curEntry.getInvIDNodeName()) != _curEntry.getAllPayload().end()){
            _curEntry.getAllPayload()[_curEntry.getInvIDNodeName()].push_back(node);
        }
        else{
            (*/

//    }
//    else{
//        ++lastLevel;
        ++_level;
//    }
//    qDebug() << "Now last level was: " << _level;

}

void CRSProcessor1::XMLParsePayloadEnd(QXmlStreamReader& xml)
{
    _level -= 1;
}

void CRSProcessor1::XMLParsePayload(const QString& txt) // доделать до 10.03.2024
{
    // Новый класс для хранения Payload. Дерево, класс PayloadNode
    QXmlStreamReader xml(txt);
    _level = 0;

    while (!xml.atEnd() && !xml.hasError())
    {
//        // получаем очередной токен
        QXmlStreamReader::TokenType token = xml.readNext();
//        qDebug() << "current token in payload: " << token;

//        // разбираем по существу payload
        switch(token){

        case QXmlStreamReader::StartElement:
        {
            XMLParsePayloadStart(xml);
        }
        break;

        case QXmlStreamReader::EndElement:
        {
            XMLParsePayloadEnd(xml);
        }
        break;


        case QXmlStreamReader::Characters:
        {
            XMLParsePayloadChars(xml);
        }
        break;
        }

    }
}

/*------------------------------------------------------------------------------
 * Creates necessary tables for the database.
 *----------------------------------------------------------------------------*/




/*------------------------------------------------------------------------------
 * Saves current extracted entry to database.
 *----------------------------------------------------------------------------*/
void CRSProcessor1::saveCurEntry2DB()
{
    int a = 0;
    QString ts = _curEntry._l4jev_logger.unitName();
    ts = _curEntry._l4jev_logger.packName();
    ts = _curEntry._l4jev_logger.interfaceName();
    ts = _curEntry._l4jev_logger.methodName();
    
    int dpg = _curEntry._l4jev_logger.domainOrPG();


    // здесь начинаем транзакцию БД
    //_db.beginTransaction();
    // 
    TUint iID = getInterfaceID4CurEntry();
    TUint trEvID = insertEventEntryToDB(iID);


    //_db.commitTransaction();


}



/*------------------------------------------------------------------------------
 * Resets all the fields corresponding with parsing procedure
 *----------------------------------------------------------------------------*/
void CRSProcessor1::resetParseInfo()
{
    _trEventsNum = 0;   // число событий
    _secsSpent = 0;     // число затраченных секунд
}



/*------------------------------------------------------------------------------
 * Смотрит в мапу, есть ли в таблице InterfacesCache запись о переданном интерфейсе, 
 * если есть возвращает его ИД, если нет, смотрит в БД, есть ли, есть возвращает,
 * нет — добавляет и уже точно возвращает
 * Работает с _curEntry! 
 *----------------------------------------------------------------------------*/
TUint CRSProcessor1::getInterfaceID4CurEntry()
{
    //TUint 
    //QString domPrj = lfn.getDomPrj();

    const QString& iFName = _curEntry._l4jev_logger.interfaceFullName();

    StringIntMap::iterator ii = _interfacesCache.find(iFName);
    
    if(ii != _interfacesCache.end())
        return ii->second;                // нашли
                                      
    // работаем с БД. Т.к. понадобятся одни и те же поля, вытащим их заблаговременно
    // текстовое представления для процессной группы/домена
    const char* prdm;
    if(_curEntry._l4jev_logger.domainOrPG() == CRSProcessor1::LoggerStruct::dpDomains)
        prdm = "SV";        // service
    else
        prdm = "PR";        // process

    // TODO:!!!! Achtung!!!! читаем про проблему!!!
    // http://manjeetdahiya.com/tag/c/
    // Пример, почему придется toLocal8Bit() хранить отдельно!
    //QString tests = "Bar";
    //const char* testsc = tests.toLocal8Bit().data();



    //_curEntry._l4jev_logger.dpgName



    //QString packNameS = _curEntry._l4jev_logger.packName();
    //QByteArray pn1 = packNameS.toLocal8Bit();
    
    
    QByteArray unitNameA = _curEntry._l4jev_logger.unitName().toLocal8Bit();
    const char* unitName = unitNameA.data();

    
    QByteArray packNameA = _curEntry._l4jev_logger.packName().toLocal8Bit();
    const char* packName = packNameA.data();
     
    QByteArray interfaceNameA = _curEntry._l4jev_logger.interfaceName().toLocal8Bit();
    const char* interfaceName = interfaceNameA.data();


    // иначе смотрим, есть ли что в базе
    int dbID = queryInterfaceNameFromDB(prdm, unitName, packName, interfaceName);
    if(dbID == 0)
        dbID = insertInterfaceNameToDB(prdm, unitName, packName, interfaceName);  // не мытьем, так катанием // что не так
        //dbID = testGetDomPrj();

    // добавим в мапу новый индекс

//    _interfacesCache[iFName] = dbID;
    _interfacesCache.insert({iFName, dbID});
        
    return dbID;

}


/*------------------------------------------------------------------------------
 *  Query DB for curEntry interface id.
 *----------------------------------------------------------------------------*/
TUint CRSProcessor1::queryInterfaceNameFromDB(const char* prdm,  const char* unitName,
    const char* packName, const char* interfaceName)
{
    //// текстовое представления для процессной группы/домена
    //const char* prdm;
    //if(_curEntry._l4jev_logger.domainOrPG() == CRSProcessor1::LoggerStruct::dpDomains)
    //    prdm = "SV";        // service
    //else
    //    prdm = "PR";        // process

    // подготавливаем запрос
    char stSQL[512];
    //sprintf_s(stSQL, sizeof(stSQL) - 1, "SELECT id FROM Interfaces WHERE PDType = \"%s\" AND UnitName = \"%s\" AND PackName = \"%s\" AND InterfaceName = \"%s\";",
    sprintf(stSQL,  "SELECT id FROM Interfaces WHERE PDType = \"%s\" AND UnitName = \"%s\" AND PackName = \"%s\" AND InterfaceName = \"%s\";",
        prdm,
        unitName,
        packName,
        interfaceName);
        //_curEntry._l4jev_logger.packName().toLocal8Bit().data(),
        //_curEntry._l4jev_logger.interfaceName().toLocal8Bit().data());

    // БДаним
    try {

        shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));
        
        st->prepare();

        db::SQLiteConnection::Statement::Result
            stRes = st->step();

        if(stRes == db::SQLiteConnection::Statement::rsDone)
            return 0;           // нет такой записи


        if(stRes == db::SQLiteConnection::Statement::rsRow)
        {
            // если же все ок
            // a column
            shared_ptr<db::SQLiteConnection::Column> col0(   st->extractColumn(0)    );
        
            string __deb_cn = col0->getName();
            db::DBConnection::ColumnDataType __deb_cdt = col0->getDataType();

            int num = col0->asInt();
            return num;
        }

        // иначе фигня какая-то
        throw CRSLPException("SQL query returned non-row");

    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString("Exception"), QString(e.what()), QMessageBox::Ok);
        msgBox.exec();
        throw;
    }


}


/*------------------------------------------------------------------------------
 * Inserts interface record to DB
 *----------------------------------------------------------------------------*/
TUint CRSProcessor1::insertInterfaceNameToDB(const char* prdm, const char* unitName,
    const char* packName, const char* interfaceName)
{

    // подготавливаем запрос
    char stSQL[512];
    //sprintf_s(stSQL, sizeof(stSQL) - 1,
    int ID = _curEntry._interfaceID;
    sprintf(stSQL, //sizeof(stSQL) - 1,
        "INSERT INTO Interfaces VALUES (%d, \"%s\", \"%s\", \"%s\", \"%s\");",
//        "INSERT INTO Interfaces VALUES (NULL, \"A\", \"Test1\", \"Test11\");",
        ID,
        prdm,
        unitName,
        packName,
        interfaceName);


    // DEBUG


    // БДаним
    try {

        //_db.beginTransaction();   // перенесли в вызывающий метод
        shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));
        
        st->prepare();
        db::SQLiteConnection::Statement::Result
            stRes = st->step();

        // если не удалось исполнить SQL, плохо!
        if(stRes != db::SQLiteConnection::Statement::rsDone)
            throw CRSLPException("Error when inserting a new Interface record");

        //_db.commitTransaction();

       // получение последнего вставленного автоинкремента
        //TInt64 rowId = _db.getLastNotifiedRowID();      // более дешевый аналог!
        int64_t rowId = _db.getLastNotifiedRowID();      // более дешевый аналог!

        return rowId;

        //TInt64 laiv =  _db.takeLastAutoIncrementValue();
    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString("Exception"), QString(e.what()), QMessageBox::Ok);
        msgBox.exec();
        throw;
    }


}

/*------------------------------------------------------------------------------
 * Inserts payload record for every event entry to DB.
 *----------------------------------------------------------------------------*/
void CRSProcessor1::insertEventPayloadToDB(/*const char*& payloadNameCC, const char*& payloadValueCC,
                                            int level*/)
{
    char  stSQL[5120];

    for (PayloadNode* tmp : _curEntry.getPayVector())
    {
//        PayloadNode* tmp = _curEntry.getPayloadNode(i);

//        qDebug() << "id: " << _curEntry._invID << "payloadName: " << tmp->getAttributeName()
//                 << "value: " << (tmp->getValue()).data() << "level: " << tmp->getLevel();

        QByteArray payloadNameC = tmp->getAttributeName().toLocal8Bit();
        const char* payloadNameCC = payloadNameC.data();

        if (payloadNameCC == "notificationdetails")
            return;

        QByteArray payloadValueC = tmp->getValue().toLocal8Bit();
        const char* payloadValueCC = payloadValueC.data();

        QByteArray operNameC = _curEntry._l4jev_logger.methodName().toLocal8Bit();
        const char* operNameCC = operNameC.data();

        int ID = _curEntry._invID;
        if (ID < 0)
            ID = ID * (-1);

        int eventID = _curEntry._eventID;
        sprintf(stSQL,
                "INSERT INTO ExtractedPayload VALUES (%d, \"%s\", \"%s\", %d, \"%s\", %d);",
                eventID,               //EventID
                payloadNameCC,        //PayloadName
                payloadValueCC,     // PayloadValue
                tmp->getLevel(),              //PayloadLevel
                operNameCC,                    //Under operation
                ID               // InvID
                );

        shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));

        st->prepare();
        db::SQLiteConnection::Statement::Result
            stRes = st->step();

        if(stRes != db::SQLiteConnection::Statement::rsDone)
            throw CRSLPException("Error when inserting a new Payload record");

    }

}




/*------------------------------------------------------------------------------
 * Inserts event entry record to DB.
 *----------------------------------------------------------------------------*/
TUint CRSProcessor1::insertEventEntryToDB(TUint interfaceID)
{
    insertEventPayloadToDB(); //  перед записей события запишем сначала его payloads

    //QString ts = _curEntry._l4jev_logger.dpgName();
    //ts = _curEntry._l4jev_logger.packName();
    //ts = _curEntry._l4jev_logger.interfaceName();
    //ts = _curEntry._l4jev_logger.methodName();
    //
    //int dpg = _curEntry._l4jev_logger.domainOrPG();

    //_curEntry._l4jev_EventType



    //
//отсюда
    int invID = _curEntry._invID;
    if (invID < 0)
        invID = invID * (-1);

    int eventID = _curEntry._eventID;
    _curEntry.increaseEventID();

    QByteArray operNameC = _curEntry._l4jev_logger.methodName().toLocal8Bit();
    const char* operNameCC = operNameC.data();
    // _curEntry.
        //_curEntry._l4jev_logger.interfaceName().toLocal8Bit();

    QByteArray invNodeNameC = _curEntry.getInvIDNodeName().toLocal8Bit();
    QByteArray invNodeIPaddrC = _curEntry.getInvIDipAddr().toLocal8Bit();

    const char* invNodeNameCC = invNodeNameC.data();
    const char* invNodeIPaddrCC = invNodeIPaddrC.data();

    //// подготавливаем запрос
    /*const char* */char  stSQL[2048];
//    sprintf_s(stSQL, sizeof(stSQL) - 1,
    sprintf(stSQL, /*sizeof(stSQL) - 1,*/
        //"INSERT INTO TracingEvents VALUES (NULL, %d, \"%s\", %d, NULL, NULL, NULL, NULL, \"%s\", %d, %d);",
        "INSERT INTO TracingEvents VALUES (%d, %d, \"%s\", NULL, NULL, %d, \"%s\", %d, %d);",
        eventID,                             //ID
        interfaceID,                    // Interface_ID
        operNameCC, //operNameC.data(),               // OperationName
        //"invNodeNameCC", //invNodeNameC.data(),            // InvNodeName,
        //"invNodeIPaddrCC", //invNodeIPaddrC.data(),          // InvIP
//        NULL,                           // TransContext_ID
//        NULL,                           // AppServerContext_ID
        invID,
        _curEntry.getEventTypeAbbr(),   // EventType
        (TUint)(_curEntry.getL4jevTimestamp()),  // Event Timestamp
        _curEntry._payloadSize          // PayloadSize
       );
//до сюда

//    QString sqlQuery;
//    QTextStream(&sqlQuery)
//    qDebug()
//        << "INSERT INTO TracingEvents VALUES (NULL, "   // ID
//        << interfaceID << ", \""                           // InterfaceID
//        << _curEntry._l4jev_logger.methodName() << "\", "         // OperationName
//        << _curEntry._invID << ", \""                             // InvID
//        << _curEntry.getInvIDNodeName() << "\", \""                 // InvNodeName
//        << _curEntry.getInvIDipAddr() << "\", "                  // InvIP
//        << "NULL, NULL, "
//        << "\"" <<_curEntry.getEventTypeAbbr() << "\", "
//        << (TUint)(_curEntry.getL4jevTimestamp())  << ", "
//        << _curEntry._payloadSize ;

//    //// не заработал ваще ни разу!
//    QString sqlQuery;// = QString::sprintf("asd");
//    /*sqlQuery.sprintf(*/
//    sqlQuery.sprintf("NULL, %d, \"%s\", %d, \"%s\", \"%s\", NULL, NULL, \"%s\", %d, %d",
//        interfaceID,
//        _curEntry._l4jev_logger.methodName(),
//        _curEntry._invID,
//        _curEntry.getInvIDNodeName(),
//        _curEntry.getInvIDipAddr(),
//         QString::fromLocal8Bit(_curEntry.getEventTypeAbbr()),
//        (TUint)(_curEntry.getL4jevTimestamp()),
//        _curEntry._payloadSize
//      );


    // TODO: это ужасный подход, но он работает! Надо попробовать таки биндинг сделать!

//    QString sqlQuery = QString(
//        "INSERT INTO TracingEvents VALUES (NULL, %1, \"%2\", %3, \"%4\", \"%5\", NULL, NULL, \"%6\", %7, %8, %9, 0);").arg(
//        QString::number(interfaceID),
//        _curEntry._l4jev_logger.methodName(),
//        QString::number(_curEntry._invID),
//        _curEntry.getInvIDNodeName(),
//        _curEntry.getInvIDipAddr(),
//         QString::fromLocal8Bit(_curEntry.getEventTypeAbbr()),
//        //QString::number((TQword)(_curEntry.getL4jevTimestamp())),
//        QString::number((uint64_t)(_curEntry.getL4jevTimestamp())),
//        QString::number(_curEntry._payloadSize),
//        QString::number(_curEntry._payloadCRC32)
//      );

    // TODO: попробовать здесь биндинг аргументов!

//    QByteArray  stSQL_BA = sqlQuery.toLocal8Bit();
//    stSQL = stSQL_BA.data();

//    sprintf_s(stSQL, sizeof(stSQL) - 1,
//    sprintf(stSQL,
//    snprintf(stSQL, sizeof(stSQL) - 1,
//        "INSERT INTO TracingEvents VALUES (NULL, \"%s\", \"%s\", NULL, NULL, NULL, NULL, NULL, \"%s\", \"%s\");",
//        interfaceID,
//        operNameC.data(),               // OperationName
////        NULL,                           // InvID
////        NULL,                           // InvNodeName,
////        NULL,                           // InvIP
////        NULL,                           // TransContext_ID
////        NULL,                          // AppServerContext_ID
//        _curEntry.getEventTypeAbbr(),   // ActionType
//        _curEntry._payloadSize          // PayloadSize
//       );



    //_db.beginTransaction();   // перенесли в вызывающий метод
    //и отсюда до конца
    shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));
        
    st->prepare();
    db::SQLiteConnection::Statement::Result
        stRes = st->step();

    // если не удалось исполнить SQL, плохо!
    if(stRes != db::SQLiteConnection::Statement::rsDone)
        throw CRSLPException("Error when inserting a new TracingEntry record");

    //_db.commitTransaction();

    // получение последнего вставленного автоинкремента
    //TInt64
    int64_t rowId = _db.getLastNotifiedRowID();      // более дешевый аналог!

    return rowId;

}



/*------------------------------------------------------------------------------
 * Смотрит в мапу, есть ли в таблице DomainProj такая запись, если есть
 * возвращает ИД, если нет — добавляет и возвращает
 *----------------------------------------------------------------------------*/
//TUint CRSProcessor1::getDomPrj(const QString& domPrj)
TUint CRSProcessor1::getDomPrj(const LogFileNameStruct& lfn)
{
    //TUint 
    QString domPrj = lfn.getDomPrj();

    StringIntMap::iterator dpi = _domPrjId.find(domPrj);
    if(dpi != _domPrjId.end())
        return dpi->second;                // нашли

    // иначе, надо добавлять
    
    //string updSt = "INSERT INTO DomainPrjPack VALUES (";
    //updSt += domPrj;
    //updSt += ", ";
    //updSt += lfn.getPackage();

    char stSQL[500];
    //sprintf_s(stSQL, sizeof(stSQL) - 1,
    sprintf(stSQL, //sizeof(stSQL) - 1,
        "INSERT INTO DomainPrjPack VALUES (NULL, \"%s\", \"%s\");",
        domPrj.toLocal8Bit().data(), lfn.getPackage().toLocal8Bit().data());

    //char gaiStSQL[] = "SELECT last_insert_rowid();";

    // БДаним
    try {

        bool acm = _db.isAutocommitMode();

        _db.beginTransaction();

    //DB::SQLiteConnection::Statement* st =  _db.newStatement(buf);
        shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));
        //st.reset(_db.newStatement(buf));
        
        st->prepare();
        st->step();

       // получение последнего вставленного автоинкремента
       
        //TInt64
        int64_t laiv =  _db.takeLastAutoIncrementValue();

        //TInt64
        int64_t rowId = _db.getLastNotifiedRowID();      // более дешевый аналог!
        
        _db.commitTransaction();
        //_db.rollbackTransaction();
        
        // shared_ptr<DB::SQLiteConnection::Statement> gaiSt(_db.newStatement(gaiStSQL));
       // gaiSt->prepare();
       // DB::SQLiteConnection::Statement::Result gaiStRes = gaiSt->step();


        // TODO: довести до ума commitHooker в SQLite


    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString("Exception"), QString(e.what()), QMessageBox::Ok);
        msgBox.exec();
        throw;
    }



    


    return 0;

}


/*------------------------------------------------------------------------------
 * Смотрит в мапу, есть ли в таблице DomainProj такая запись, если есть
 * возвращает ИД, если нет — добавляет и возвращает
 *----------------------------------------------------------------------------*/
//TUint CRSProcessor1::getDomPrj(const QString& domPrj)
TUint CRSProcessor1::testGetDomPrj()
{
    //TUint 
    //QString domPrj = lfn.getDomPrj();


    char stSQL[500];
    //sprintf_s(stSQL, sizeof(stSQL) - 1,
    sprintf(stSQL, //sizeof(stSQL) - 1,
        "INSERT INTO DomainPrjPack VALUES (NULL, \"%s\", \"%s\");",
        "asd", "df");

    //char gaiStSQL[] = "SELECT last_insert_rowid();";

    // БДаним
    try {

        bool acm = _db.isAutocommitMode();

        _db.beginTransaction();

    //DB::SQLiteConnection::Statement* st =  _db.newStatement(buf);
        shared_ptr<db::SQLiteConnection::Statement> st(_db.newStatement(stSQL));
        //st.reset(_db.newStatement(buf));
        
        st->prepare();
        st->step();

       // получение последнего вставленного автоинкремента
       
        //TInt64
        int64_t laiv =  _db.takeLastAutoIncrementValue();

        //TInt64
        int64_t rowId = _db.getLastNotifiedRowID();      // более дешевый аналог!
        
        _db.commitTransaction();
        //_db.rollbackTransaction();
        
        // shared_ptr<DB::SQLiteConnection::Statement> gaiSt(_db.newStatement(gaiStSQL));
       // gaiSt->prepare();
       // DB::SQLiteConnection::Statement::Result gaiStRes = gaiSt->step();


        // TODO: довести до ума commitHooker в SQLite


    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString("Exception"), QString(e.what()), QMessageBox::Ok);
        msgBox.exec();
        throw;
    }



    


    return 0;

}


//------------------------------------------------------------------------------
// тестовый метод
//------------------------------------------------------------------------------
void CRSProcessor1::test1()
{
    // открываем бд
    if(!_db.isOpen())
        _db.Open();    

    //// добавляем обработчики для ...
    //_db.set_ec_TransAction(this);   // ... транзакций 
    //_db.set_ec_UpdateAction(this);  // и update-а


    LogFileNameStruct lfn("2013-07-25.1730.pu.process.bocamo.process.ManageTir.txt");
    TUint newID  = getDomPrj(lfn);

}


//------------------------------------------------------------------------------
// тестовый метод
//------------------------------------------------------------------------------
void CRSProcessor1::test2()
{
    // открываем бд
    if(!_db.isOpen())
        _db.Open();    

    //// добавляем обработчики для ...
    //_db.set_ec_TransAction(this);   // ... транзакций 
    //_db.set_ec_UpdateAction(this);  // и update-а



    char stSQL[] = "SELECT * FROM  DomainPrjPack;";
    
    // БДаним
    try {

        // statement
        shared_ptr<db::SQLiteConnection::Statement> st(     _db.newStatement(stSQL)    );
        
        st->prepare();
        db::SQLiteConnection::Statement::Result
            gaiStRes = st->step();

        int colNum = st->columnCount();

        // a column
        shared_ptr<db::SQLiteConnection::Column> col0(   st->extractColumn(0)    );
        
        string cn = col0->getName();

        db::DBConnection::ColumnDataType cdt = col0->getDataType();

        const void* d1 = col0->asBlob();
        int d2 = col0->asInt();

        int dLen = col0->bytesLen();

    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, QString("Exception"), QString(e.what()), QMessageBox::Ok);
        msgBox.exec();
        throw;
    }


}



void CRSProcessor1::OnBegin(db::DBConnection* con)
{
}

    
/*------------------------------------------------------------------------------
 * virtual 
 *----------------------------------------------------------------------------*/    
bool CRSProcessor1::OnCommit(db::DBConnection* con)
{
    int a = 0;
    return false;
}
     

/*------------------------------------------------------------------------------
 * virtual  
 *----------------------------------------------------------------------------*/
void CRSProcessor1::OnRollback(db::DBConnection* con)
{
    int a = 0;
}


/*-----------------------------------------------------------------------------
 * virtual  
 *----------------------------------------------------------------------------*/
void CRSProcessor1::OnUpdate(db::SQLiteConnection* con,
    db::DBConnection::SQLAction act,
            const char* dbName, const char* tableName, //TInt64 rowid)
                             int64_t rowid)
{
    int a = 0;
}



/*------------------------------------------------------------------------------
 * Creates DB structure. Should be private
 *----------------------------------------------------------------------------*/
void CRSProcessor1::createDBStructure()
{

    // пока можно обойтись без автосоздания

    //
    //if(_db.getFileName().empty())
    //    throw("DB filename is empty");

    //// вообще, если БД уже была открыта, возможно бо-бо
    //if(!_db.isOpen())
    //    _db.Open();


    //const std::string crSt =
    //    ""
    //    "";

    //DB::SQLiteConnection::Statement* crSt = _db.newStatement();

}

// /*------------------------------------------------------------------------------
// * Processes log entry which can be both file name or dir
// *----------------------------------------------------------------------------*/
//void CRSProcessor1::processLogEntry(QFileInfo finfo, bool recurs)
//{   
//    if (finfo.isDir()) 
//    {
//        QString dirname = finfo.fileName();
//        
//        if ((dirname==".") || (dirname == ".."))
//            return;
//
//        QDir dir(finfo.canonicalFilePath());
//        //if (skipDir(d))
//        //    return;
//        
//        processLogDir(dir, recurs);
//    } 
//    else
//        processLogFile(finfo.canonicalFilePath());
//}
//
// /*------------------------------------------------------------------------------
// * Iterates given directory
// *----------------------------------------------------------------------------*/
//void CRSProcessor1::processLogDir(QDir& dir, bool recurs) 
//{
//    dir.setSorting(QDir::Name);
//
//    QStringList files = dir.entryList(_lfFilterList, QDir::Files | QDir::NoSymLinks);    // с симлинками еще подумать надо
//    
//    foreach(QString entry, files) {
//        processLogEntry(dir.filePath(entry));
//    }
//
//    // если надо рекурсивно поддиректории разобрать
//    if (recurs)
//    {
//        QStringList dirs = dir.entryList(QDir::Dirs);
//        foreach (QString curDir, dirs) {
//            processLogEntry(dir.filePath(curDir), recurs);  // вообще, сюда для сокращенки надо бы сразу processLogDir
//        }
//    }
//}
//
//
// /*------------------------------------------------------------------------------
// *  TBD
// *----------------------------------------------------------------------------*/
//void CRSProcessor1::processLogFile(const QString& fileName)
//{
//    //
//}



// /** 
// * Iterates given direcory (by its name \param dirName).
// * It's supposed that all the subdirectories of the given directory are the timestamped ones
// * and their names containing accurate 4 digit has a HHMM format.
// */
// //void CRSProcessor1::iterateDateDir(const string& dirName)
// //void CRSProcessor1::iterateDateDir(QDir dir)
//void CRSProcessor1::processDateDir(const QString& dirName)
//{
//    QFileInfo finfo(dirName);
//    if(!finfo.isDir())
//        throw CRSLPException("Not a dir!");
//
//    QDir d(finfo.canonicalFilePath());
//    processDateDir(d);
//
//
//}
//
// /** 
// * Iterates given direcory (by \param d).
// */
//void CRSProcessor1::processDateDir(QDir& dir)
//{
//    dir.setSorting(QDir::Name);   // сортировка по имени
//    
//    
//    QStringList dirs = dir.entryList(QDir::Dirs); // interested in directories only
//    foreach (QString curDir, dirs)
//    {
//        if(curDir == "." | curDir == "..")
//            continue;
//
//        //bool skip = skipSpecialDir(curDir);
//        //int a = 0;
//        QDir td(curDir);
//        processTimeDir(td);
//    }
//    
//    
//    //Q_FOREACH(QString entry,
//
//}
//
// /*------------------------------------------------------------------------------
// * Iterates timed dir
// *----------------------------------------------------------------------------*/
//void CRSProcessor1::processTimeDir(QDir& dir)
//{
//    dir.setSorting(QDir::Name);   // сортировка по имени
//    QStringList subDirs =  dir.entryList(QDir::Dirs);   // интересуют только каталоги с логами, 
//                                                        // никаких файлов на этом уровне
//
//    foreach (QString curDir, subDirs)
//    {
//        if(curDir == "." | curDir == "..")
//            continue;
//
//    }
//
//}







// /*------------------------------------------------------------------------------
// * Iterates log dir
// *----------------------------------------------------------------------------*/
//void CRSProcessor1::processLogDir(QDir& dir)
//{   
//    // считаем, что тут уже не должно быть подпапок
//    // 
//    dir.setSorting(QDir::Name);   // сортировка по имени
//    
//    // сперва обрабатываем собственно файлики
//
//    
//    
//    QStringList subDirs =  dir.entryList(QDir::Dirs);   // интересуют только каталоги с логами, 
//
//}




// /*------------------------------------------------------------------------------
// *  Return true if a given dir is a special one
// *----------------------------------------------------------------------------*/
//bool CRSProcessor1::skipSpecialDir(const QDir& d) 
//{
//    bool retval = false;
//    QString name = d.canonicalPath();
//    
//    //if (name.contains("CVS")) {
//    //    retval = true;
//    //}
//    
//    if (name == ".." || name == ".")
//        retval=true;
//
//    if (retval==true) {
//        //qDebug(QString("  Skipdir: %1").arg(name));
//    }
//    return retval;
//}

//------------------------------------------------------------------------------
// class CRSProcessor1::LogFileNameStruct
//------------------------------------------------------------------------------


/**
 *  Конструктор, выполняющий разбивку
 */
CRSProcessor1::LogFileNameStruct::LogFileNameStruct(const QString& fn)
{
    QString delimiterPattern(".");
    subs = fn.split(delimiterPattern);

    //QStringList subs

    //datePreamble = subs[0];
    //timePreamble = subs[1];
    // // [2] - это pu, который выкидываем просто
    //domPrj = subs[3]; 
    //domPrj = ".";
    //domPrj += subs[4];

    //package = subs[5];

}





//------------------------------------------------------------------------------
// class LoggerStruct
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
CRSProcessor1::LoggerStruct::LoggerStruct():
    _sectionsNum(0)
{

}


/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
CRSProcessor1::LoggerStruct::LoggerStruct(const QStringRef& lg)
    : _loggerStr(lg.toString())
{
    //QString delimiterPattern(".");
    //subs = lg.toString().split(delimiterPattern);
    
    determineSectionsNum();
}

/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
//void CRSProcessor1::LoggerStruct::setLogger(const QStringRef& lg)
void CRSProcessor1::LoggerStruct::setLogger(const QStringView& lg)
{
    _loggerStr = lg.toString();
    determineSectionsNum();
}

/*------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------*/
void CRSProcessor1::LoggerStruct::clear()
{
    _loggerStr.clear();
    _sectionsNum = 0;
}


/*------------------------------------------------------------------------------
 * Determines how many section doest the string have.
 *----------------------------------------------------------------------------*/
void CRSProcessor1::LoggerStruct::determineSectionsNum()
{
    _sectionsNum = 0;
    int j = 0;

    while ((j = _loggerStr.indexOf('.', j)) != -1) {
        //qDebug() << "Found <b> tag at index position" << j;
        ++j;
        ++_sectionsNum;
    }
    
    ++_sectionsNum;     // т.к. кабинонок в туалете всегда на 1 больше, чем перегородок между ними
}


/*------------------------------------------------------------------------------
 *  Returns type of event
 *----------------------------------------------------------------------------*/
CRSProcessor1::LoggerStruct::DomainOrPG CRSProcessor1::LoggerStruct::domainOrPG() const
{
    // TODO: плохой хардкод!
    QString dpg = _loggerStr.section('.', 4, 4);
    
    if(dpg == "domains")
        return CRSProcessor1::LoggerStruct::dpDomains;

    if(dpg == "process")
        return CRSProcessor1::LoggerStruct::dpPG;

    // если дошли сюда, значит какая-то странность!
    throw CRSLPException(true, "%s: %s", "Unknown domain-pg type",
                         //dpg.toLocal8Bit()); // some error
                         dpg.toStdString().c_str());

}

/*------------------------------------------------------------------------------
 *  Returns name of event (package name)
 *----------------------------------------------------------------------------*/
QString CRSProcessor1::LoggerStruct::unitName() const
{
    return _loggerStr.section('.', 5, 5);
}

/*------------------------------------------------------------------------------
 *  Returns name of the package
 *----------------------------------------------------------------------------*/
QString CRSProcessor1::LoggerStruct::packName() const
{
    return _loggerStr.section('.', 6, _sectionsNum - 3);
}


/*------------------------------------------------------------------------------
 *  Returns name of the Interface
 *----------------------------------------------------------------------------*/
QString CRSProcessor1::LoggerStruct::interfaceName() const
{
    int secNum = _sectionsNum - 2;
    return _loggerStr.section('.', secNum, secNum);
}


/*------------------------------------------------------------------------------
 *  Returns name of the method
 *----------------------------------------------------------------------------*/
QString CRSProcessor1::LoggerStruct::methodName() const
{
    int secNum = _sectionsNum - 1;
    return _loggerStr.section('.', secNum, secNum);
}


/*------------------------------------------------------------------------------
 *  Returns fully qualified name for interface including domain or process token as first
 *----------------------------------------------------------------------------*/
QString CRSProcessor1::LoggerStruct::interfaceFullName() const
{
    return _loggerStr.section('.', 4, _sectionsNum - 2);
}



//------------------------------------------------------------------------------
// class  CRSTracingEntry
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 *  consts
 *----------------------------------------------------------------------------*/
const char* CRSProcessor1::CRSTracingEntry::EV_TYPE_ABBR_REQ = "RQ";
const char* CRSProcessor1::CRSTracingEntry::EV_TYPE_ABBR_RES = "RS";
const char* CRSProcessor1::CRSTracingEntry::EV_TYPE_ABBR_RESEXC = "SE";     // <![CDATA[Response with Exception]]>


/*------------------------------------------------------------------------------
 *  Конструктор
 *----------------------------------------------------------------------------*/
CRSProcessor1::CRSTracingEntry::CRSTracingEntry()
{
    reset();
}


/*------------------------------------------------------------------------------
 *  Resets all the fields to their initial state
 *----------------------------------------------------------------------------*/    
void CRSProcessor1::CRSTracingEntry::reset()
{
    _invID = 0;
    _payloadSize = 0;
    _l4jev_timestamp = 0;

    _l4jev_logger.clear();

    _invIDNodeName.clear();
    _invIDipAddr.clear();


}


/*------------------------------------------------------------------------------
 * Sets inv id given as a string
 *----------------------------------------------------------------------------*/
void CRSProcessor1::CRSTracingEntry::setInvID(const QString& s)
{
    _invID = s.toLongLong();
}


/*------------------------------------------------------------------------------
 * Sets timestamp as string
 *----------------------------------------------------------------------------*/
void CRSProcessor1::CRSTracingEntry::setlog4jTimestamp(const QString& s)
{
    // TODO: разобраться с конвертированием
    _l4jev_timestamp = s.toULongLong();
    
}

/*------------------------------------------------------------------------------
 * Sets eventtype
 *----------------------------------------------------------------------------*/
//void CRSProcessor1::CRSTracingEntry::setlog4jEventType(const QStringRef& s)
void CRSProcessor1::CRSTracingEntry::setlog4jEventType(QStringView sv)
{
    // важный момент: тут определяем по строке тип запроса! могут новые типы появляться!
    if(sv == XML_VAL_LOG4JEV_MES_REQ)
        _l4jev_EventType = etRequest;

    else if(sv == XML_VAL_LOG4JEV_MES_RES)
        _l4jev_EventType = etResponse;

    else if(sv == XML_VAL_LOG4JEV_MES_RESEXC)
        _l4jev_EventType = etResponseException;

    else
    {
        char buf[300];
        QByteArray sb = sv.toLocal8Bit();
        //sprintf_s(buf, sizeof(buf) - 1, "%s: %s", "Unknown message type!", sb.data());
        sprintf(buf, "%s: %s", "Unknown message type!", sb.data());
        throw CRSLPException(buf);   // TODO: переделать на новый тип исключений
    }

}


/*------------------------------------------------------------------------------
 * Returns an event type 2-letters abbreviation
 *----------------------------------------------------------------------------*/
const char* CRSProcessor1::CRSTracingEntry::getEventTypeAbbr() const
{
    if(_l4jev_EventType == etRequest)
        return EV_TYPE_ABBR_REQ;

    else if(_l4jev_EventType == etResponse)
        return EV_TYPE_ABBR_RES;

    else if(_l4jev_EventType == etResponseException)
        return EV_TYPE_ABBR_RESEXC;
    else
        throw CRSLPException("Unknown message type");
                
}




}; // namespace CRS_processing
