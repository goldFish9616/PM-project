///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Contains the declaration of the main DB classes and datatypes.
///
/// aaa. 
///
///////////////////////////////////////////////////////////////////////////////




#pragma


#ifndef XIDB_XI_DB_DBMAIN_H_  // former xiDBmainH
#define XIDB_XI_DB_DBMAIN_H_    // according with Google C++ Coding Style (01/02/2015)



#pragma once



#include <exception>
#include <stdexcept>

 
//#include "xiTypes.h"
#include "xi/types/monikers.h"
#include <stdio.h>


/// Xi namespace
/** 
 *  A more elaborate namespace description.
 */
namespace xi {
namespace db {

//using std::exception;

using namespace xi::types;

/** \brief An xi DB library specific exception.
 *
 *  TODO: для каждого текстового сообщения ввести свой код! Долго, муторно, но надо!
 */
//class DBException : public std::exception {
class DBException : public std::runtime_error {

public:
    static const int MAX_FORMAT_STR_SIZE = 1024;

public:
    //------------<Constructors>---------------- 
    
    /// Default
    DBException(): 
        std::runtime_error("") ,
        _hasErrMes(false)
    {
    }
        
    /// With Info message 
    DBException(const char * const & wh):
        std::runtime_error(wh),
        _hasErrMes(false)
    {
    }

    /// With Formatting
    DBException(bool, const char * _Format, ...);//: 




    /// With Formatting
    DBException(const char * _Format, va_list argptr);//: 



    //virtual const char* what() const
    //{
    //    return std::exception::what();
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


}; // class DBException



/// DBConnection class brief description.           
/** 
 *  DBConnection class.
 */          
class DBConnection {


public:
    //------------<Consts>---------------- 
    
    // Common String Constants
    // эти константы надо раскидать со временем по классам

    

    static const char* ERR_COMMON;// = "File name is not presented";

    static const char* ERR_FILENAME_NOT_PRESENTED;// = "File name is not presented";
    static const char* ERR_EXCEPTION;
    static const char* ERR_OPENING_CONNECTION;
    static const char* ERR_CON_ALREADY_OPEN;
    static const char* ERR_CON_NOT_OPEN;
    static const char* ERR_NULL_PARENT_CON;
    static const char* ERR_PARENT_CON_NOT_READY;
    static const char* ERR_STATEMENT_PREPARING;
    static const char* ERR_STATEMENT_NOT_PREPARED;
    static const char* ERR_STATEMENT_ERROR_STATE;
    static const char* ERR_STATEMENT_FINISHED;
    static const char* ERR_STATEMENT_WHEN_STEP;
    static const char* ERR_STATEMENT_EXC_WHEN_STEP;
    static const char* ERR_STATEMENT_WHEN_RESET;
    static const char* ERR_STATEMENT_WHEN_FINAL;
    static const char* ERR_STATEMENT_COMMON;
    static const char* ERR_WRONG_COL_NUM;

    // common errors?
    static const char* ERR_CMN_EXTR_LAUTOINC;
    static const char* ERR_CMN_TRANS_BEGIN;
    static const char* ERR_CMN_TRANS_COMMIT;
    static const char* ERR_CMN_TRANS_ROLLBACK;
    static const char* ERR_CMN_TRANS_ALR_STARTED;
    static const char* ERR_CMN_TRANS_NOT_STARTED;



    // Modules description
    static const char* MOD_COL_COUNT;
    static const char* MOD_EXTR_COLUMN;
    static const char* MOD_COLUMN_GETNAME;
    static const char* MOD_COLUMN_GETDT;
    static const char* MOD_COLUMN_DATAAS;
    


public:
    //------------<Types>---------------- 

    /** \brief Enumeration for the all known column types.
     *
     *  Should be enough for the most cases.
     */
    enum ColumnDataType
    {
        cdtUnknown,                 ///< Unknown or unspecified
        cdtNull,                    ///< NULL (no data) should it be here?!?!
        cdtInt,                     ///< Integer
        cdtFloat,                   ///< Float (represented as double?)
        cdtText,                    ///< Text

        cdtBlob,                    ///< BLOB

        cdtOther                    ///< For all the other types
    }; // enum ColumnType          
    
    // TODO: надо конкретно заполнять это перечисление!


    /** \brief Represents action done thru SQL-expression.
     *
     *  >
     */
    enum SQLAction {
        sqaUndefined,               ///< Special case
        sqaInsert,
        sqaDelete,
        sqaUpdate

    }; // enum SQLAction


public:
    //------------<Callback Interfaces>---------------- 


    /** \brief Common purpose connection-specified transaction action notify interface.
     *
     *  >
     */
    class ITransactionAction {
    public:

        /**
         *  Begin transaction event handler
         */
        virtual void OnBegin(DBConnection* con) = 0;

        /** \brief Commit transaction event handler.
         *
         *  \return true, if commit should be breaked and replaced for roll back, false otherwise.
         */
        virtual bool OnCommit(DBConnection* con) = 0;


        /** \brief Rollback transaction event handler.
         *
         *  >
         */
        virtual void OnRollback(DBConnection* con) = 0;

    }; // class ITransactionAction



public:
    //------------<Public Methods>---------------- 


    /** \brief Tries to open a DB connection.
     *
     *  If a connection can not be established throws an exception.
     *  If a connection has already established throws an exception.
     */
    virtual void Open() = 0;                            
    
    /** \brief Closes the DB connection.
     *
     *  If the connection has not been established do nothing.
     */
    virtual void Close() = 0;


    /** \brief Return the state of the DB connection: true if connected, false otherwise.
     *
     *  
     */
    virtual bool isOpen() = 0;


public:
    //------------<Public Static Methods>---------------- 
    
    // TODO: разобраться с throw, почему это не везде работает!!!!!
    static void throwException() throw();                       ///< Throws a regular DB exception.
    static void throwException(const char* msg) ;//throw();        ///< Throws a DB exception with a message string.
    static void throwException(const char* msg, 
                                TUint code) throw();            ///< Throws a DB exception with a message string and a err code.

    static void throwException(const char* mod, 
                               const char* msg) throw();        ///< Module name and message string

    static void throwExceptionFormat(const char * _Format, ...); // throw(); ///< Formatted exception

}; // class DBConnection


///// SQLConnection class representates an SQL-based .
///**
// *  A more elaborate class description.
// */
//class SQLConnection  {
//
//
//};



}; // namespace db {
}; // namespace xi {

#endif // XIDB_XI_DB_DBMAIN_H_
