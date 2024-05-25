#include "conv1.h"
#include <QMessageBox>
#include "./ui_conv1.h"


//#include "xidbsqlite\xidbsqlite.h"
//#include "xi/db/sqlite/xidbsqlite.h"
//#include "xi/log/filelog.h"


#include <string>
#include <time.h>

#include "crs_log_processor1.h" // добавила namespace conv1 в Types и сработало...Ок?
#include "xiCustomLog.h" // и здесь

#include <boost/crc.hpp>


using namespace xi;
using CRS_processing::CRSProcessor1;

/*------------------------------------------------------------------------------
 *  
 *----------------------------------------------------------------------------*/
//conv1::conv1(QWidget *parent, Qt::WFlags flags)
conv1::conv1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::conv1Class) // добавила
//: QMainWindow(parent, flags)
{
    ui->setupUi(this);
    //this->
    QObject::connect(   ui->btnDBtest1,  SIGNAL(clicked()),
                     this,           SLOT(pushbutton1_click()) );

    QObject::connect(   ui->btnCRSLPtest1,   SIGNAL(clicked()),
                        this,               SLOT(btnCRSLPtest1_click()) );

    QObject::connect(ui->btnTest1, SIGNAL(clicked()),
                     this, SLOT(btnTest1_click()) );


    
}

/*------------------------------------------------------------------------------
 *  
 *----------------------------------------------------------------------------*/
conv1::~conv1()
{

}

/**
 *  Тестовый метод для бд
 */
void conv1::pushbutton1_click()
{

    // QMessageBox msgBox;
    // msgBox.setText("xi DB (SQLite) test");
    // msgBox.exec();


    dbTest1();





}

/*------------------------------------------------------------------------------
 * A slot for a CRS LP test 1 button click signal.
 *----------------------------------------------------------------------------*/
void conv1::btnCRSLPtest1_click()
{
    
    
    //const QString logDir = "f:\\research\\themes\\SoftwarePM\\SRA\\logs\\arch\\2013-07-25-test1";
    //const QString logDir = "f:\\research\\themes\\SoftwarePM\\SRA\\logs\\arch\\2013-07-25-test2";
    
    //const QString dbName = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\crslp_d1.xidb";
    //const QString dbName = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\crslp_d2-1.xidb";


    // новый экспорт от 12.03.2014:
    // const QString logDir = "f:\\research\\themes\\SoftwarePM\\SRA\\logs\\arch\\2013-07-25-dev";
    //const QString dbName = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\crslp_d3.xidb";
    
    // очередной новый экспорт от 11.04.2014:
    const char* xiLogFileName = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\xilog\\bigimport1.log";
    
    //const QString logDir = "f:\\research\\themes\\SoftwarePM\\SRA\\logs\\arch\\2013-07-25-test1";
//    const QString logDir = "/Users/czinuj/Desktop/Курсовая/2013-12-20"; /*пока тест, это коммент*/
//    const QString logDir ="/Users/czinuj/Desktop/testFile2.txt"; // на время теста
//    const QString logDir ="/Users/czinuj/Desktop/testPack"; // test on папку файлов upd:слишком большая папка
    const QString logDir ="/Users/czinuj/Desktop/2013-12-20.0810.pu.domain.advise.service.ReadContact.txt";
//    const QString logDir ="/Users/czinuj/Desktop/testPack33/pu.domain.extcon.service.Book";
    
    
    //const QString dbName = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\crslp_d4.xidb";
    const QString dbName = "/Users/czinuj/prg/DbahnLogConverter2/correctDB.db";
//    const QString dbName = "/Users/czinuj/prg/DbahnLogConverter2/test3.db";



    // замеряем время
    //clock_t start = clock();

    // TODO: надо попробовать все-таки доделать лог...

    //// лог живет отдельно!!
    //LogProcLERenderer* defrend = new LogProcLERenderer();
    //log::TextFileLog log(xiLogFileName, defrend, true);                  ///< xi Text file log

    //log::TimedStringLogEvent le("", "Test");
    //log.proceedLogEvent(&le);


    //return;

    // главный процессор
    CRSProcessor1 crslp(dbName);
    
    
    try {
        crslp.convertLog2DB(logDir);
        //crslp.test1();
    }
    catch(const std::exception& e)
    {
        QMessageBox msgBox(QMessageBox::Warning, 
            QString("Exception"), 
            QString(e.what()), 
            QMessageBox::Ok);
        msgBox.exec();
        throw;
    }
    
    //clock_t finish = clock();

    //int secs = ( (finish - start)/CLOCKS_PER_SEC );
    int secs = crslp.getSecsSpent();
    int a = 0;

    //crslp.test1();
    //crslp.test2();



}

/**
 *  Create a DB
 */
void conv1::dbTest1()
{
    try {
        // создаем SQLite DB
//        const char* testdb_fn = "f:\\se\\projects\\cpp\\hse\\pm\\sra\\utils\\logs\\db\\test1.sqlt";
        const char* testdb_fn = "/Users/czinuj/prg/DbahnLogConverter2/projectDatabase.db";
        //DB::SQLiteConnection sqc(testdb_fn);
        db::SQLiteConnection sqc(testdb_fn);
    
        sqc.Open();


        // create a statement for a creating a table
        std::string crSE = "CREATE TABLE T1(a, b, c);";
    
        //DB::SQLiteConnection::Statement* crSt = sqc.newStatement(crSE);
        db::SQLiteConnection::Statement* crSt = sqc.newStatement(crSE);

        try {
            crSt->prepare();
//            DB::SQLiteConnection::Statement::Result rs = crSt->step();
            db::SQLiteConnection::Statement::Result rs = crSt->step();

            int a = 0;

        }
        catch(...)
        {
            delete crSt; crSt = nullptr;
            throw;
        }


        delete crSt; crSt = nullptr;

    }
    catch(const std::exception& exc)
    //catch(const std::exception* e)
    //catch(const DB::DBException& e)
    {
        //QMessageBox::Icon::Warning
        

        QMessageBox msgBox1(QMessageBox::Warning,
            QString("Exception"), 
            QString(exc.what()),
            QMessageBox::Ok);
        //msgBox.setText("xi DB (SQLite) test");
        msgBox1.exec();
        throw;
    }
    catch(...)
    {
        QMessageBox msgBox(QMessageBox::Warning, 
            QString("Exception"), 
            QString("Unknown exception"), 
            QMessageBox::Ok);
        msgBox.exec();
        throw;
    }

}
    
/*------------------------------------------------------------------------------
 * General purpose test 1 button onclick slot
 *----------------------------------------------------------------------------*/
void conv1::btnTest1_click()
{
    // 
    boost::crc_32_type  result;
    char text[] = "Text for processing";
    result.process_bytes(text, sizeof(text) -1);
    unsigned int res = result.checksum();

    // test 2
    QString str2("Text for processing");
    QByteArray str2B = str2.toUtf8();
    boost::crc_32_type crc2;
    crc2.process_bytes(str2B.data(), str2B.size());
    unsigned int res2 = crc2.checksum();

    // исследуем UCS-2-представление
    QString str3("Text for processing");
    boost::crc_32_type crc3;   
    const QChar* data3Ptr = str3.unicode();
    int data3Len = str3.length();
    crc2.process_bytes(data3Ptr, data3Len * 2);
    res2 = crc3.checksum();





    QMessageBox msgBox(QMessageBox::Information, 
        QString("Test1"), 
        QString("Test1"), 
        QMessageBox::Ok);
    msgBox.exec();
}



