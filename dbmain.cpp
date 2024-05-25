/*******************************************************************************
* Реализация для xidbmain.h
*******************************************************************************/

#pragma

//#include "stdafx.h"


#include <stdarg.h>
//#include <QString>
#include <stdio.h>// для sprintf

#include "xi/db/dbmain.h"

namespace xi {
namespace db {


//------------------------------------------------------------------------------
// class DBException
//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
 * With Formatting
 * \param bool isn't used
 *----------------------------------------------------------------------------*/    
DBException::DBException(bool, const char * _Format, ...):
    //std::exception() ,
    std::runtime_error("") ,
    _hasErrMes(true)
{
    //char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, _Format);
    //vsprintf_s(_errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    //vsprintf(_errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    vsprintf(_errMes, _Format, argptr);
    va_end(argptr);
    //printf(dest);
}


/*------------------------------------------------------------------------------
 * With Formatting
 * \param bool isn't used
 *----------------------------------------------------------------------------*/    
DBException::DBException(const char * _Format, va_list argptr):
    //std::exception() ,
    std::runtime_error("") ,
    _hasErrMes(true)
{
    //vsprintf_s(_errMes, MAX_FORMAT_STR_SIZE - 1, _Format, argptr);
    vsprintf(_errMes, _Format, argptr);
}

    

//DBException::DBException(const char * _Format, va_list argptr)
//{
//}
//
//
//
//void DBConnection::throwExceptionFormat(const char * _Format, ...) throw()
//{
//
//    va_list argptr;
//    va_start(argptr, _Format);
//    DBException exc(_Format, argptr);
//    va_end(argptr);
//
//    throw exc;





//------------------------------------------------------------------------------
// class DBConnection
//------------------------------------------------------------------------------

// consts initializing
    

const char* DBConnection::ERR_COMMON                    = "An error occured";

const char* DBConnection::ERR_EXCEPTION                 = "xi DB exception";
const char* DBConnection::ERR_FILENAME_NOT_PRESENTED    = "File name is not presented";    
const char* DBConnection::ERR_OPENING_CONNECTION        = "Error occured when opening connection";    

const char* DBConnection::ERR_CON_ALREADY_OPEN          = "Connection has alredy been open";
const char* DBConnection::ERR_CON_NOT_OPEN              = "Connection is not open still";

const char* DBConnection::ERR_NULL_PARENT_CON           = "Parent connection is null";
const char* DBConnection::ERR_PARENT_CON_NOT_READY      = "Parent connection is not ready";
const char* DBConnection::ERR_STATEMENT_PREPARING       = "Error occured when SQL statement is preparing";
const char* DBConnection::ERR_STATEMENT_NOT_PREPARED    = "Statement has not been prepared accurately";
const char* DBConnection::ERR_STATEMENT_ERROR_STATE     = "Statement is in an error state";
const char* DBConnection::ERR_STATEMENT_FINISHED        = "Statement is finished already. Reset it or reprepare!";
const char* DBConnection::ERR_STATEMENT_WHEN_STEP       = "An error occured when statement steps";
const char* DBConnection::ERR_STATEMENT_EXC_WHEN_STEP   = "Got to an unusual state when statement steps";
const char* DBConnection::ERR_STATEMENT_WHEN_RESET      = "An error occured when statement resets";
const char* DBConnection::ERR_STATEMENT_WHEN_FINAL      = "An error occured when statement being finalized";
const char* DBConnection::ERR_STATEMENT_COMMON          = "An statement error occured";
const char* DBConnection::ERR_WRONG_COL_NUM             = "Wrong column number";


// common errors?
const char* DBConnection::ERR_CMN_EXTR_LAUTOINC         = "An error occured when extract last autoincrement value";
const char* DBConnection::ERR_CMN_TRANS_BEGIN           = "An error occured when begining a transaction";
const char* DBConnection::ERR_CMN_TRANS_COMMIT          = "An error occured when commiting the transaction";
const char* DBConnection::ERR_CMN_TRANS_ROLLBACK        = "An error occured when rollbacking the transaction";
const char* DBConnection::ERR_CMN_TRANS_ALR_STARTED     = "A manual transaction has already been started";
const char* DBConnection::ERR_CMN_TRANS_NOT_STARTED     = "A manual transaction has not been started yet";

// modules description
const char* DBConnection::MOD_COL_COUNT                 = "Statement::columnCount()";
const char* DBConnection::MOD_EXTR_COLUMN               = "Statement::extractColumn()";
const char* DBConnection::MOD_COLUMN_GETNAME            = "Column::getName()";
const char* DBConnection::MOD_COLUMN_GETDT              = "Column::getDataType()";
const char* DBConnection::MOD_COLUMN_DATAAS             = "Column::as...()";



                             
// methods

/*
 *  Regular exception raising. 
 *  static 
 */ 
void DBConnection::throwException() throw()
{       
    //throw DBException(ERR_EXCEPTION);
    throwException(ERR_EXCEPTION);
}

/*
 *  Строка  
 */
void DBConnection::throwException(const char* msg)// throw()
{       
    throw DBException(msg);
}

/* 
 *  Строка и код ошибки 
 */
void DBConnection::throwException(const char* msg, TUint code) throw()
{
    //static const buf_size = 300;
    char buf[300];          // buffer for a exception message
    //sprintf_s(buf, sizeof(buf) - 1, "%s: %d",  msg, code);
    sprintf(buf, "%s: %d",  msg, code);

    throwException(buf);
}


/* 
 *  Модуль и строка ошибки 
 */
void DBConnection::throwException(const char* mod, const char* msg) throw()
{

    char buf[500];          // buffer for a exception message
    //sprintf_s(buf, sizeof(buf) - 1, "%s: %s", mod,  msg);
    sprintf(buf, "%s: %s", mod,  msg);

    throwException(buf);
}

/* 
 *  Форматированный вывод
 */
void DBConnection::throwExceptionFormat(const char * _Format, ...) //throw()
{

    va_list argptr;
    va_start(argptr, _Format);
    DBException exc(_Format, argptr);
    va_end(argptr);

    throw exc;
}




} // namespace db {
} // namespace xi {
