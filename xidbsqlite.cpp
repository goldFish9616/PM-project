/*******************************************************************************
* Реализация собственно
*******************************************************************************/

#pragma

//#include "stdafx.h"

#include <memory>

//using std::shared_ptr;

#include "xi/db/sqlite/xidbsqlite.h"

namespace xi {
namespace db {


//------------------------------------------------------------------------------
// Класс SQLiteConnection
//------------------------------------------------------------------------------

// consts initializing
    
const char* SQLiteConnection::ERR_SQLT_INT_ERROR      = "SQLite internal error";

// Modules description
const char* SQLiteConnection::MOD_SQLITE_CON = "SQLiteConnection";

// Special SQL's
const char* SQLiteConnection::SQL_SQLITE_SELECT_LAST_AUTOINC    = "SELECT last_insert_rowid();";
const char* SQLiteConnection::SQL_SQLITE_TRANSACT_BEGIN         = "BEGIN TRANSACTION;";
const char* SQLiteConnection::SQL_SQLITE_TRANSACT_COMMIT        = "COMMIT;";
const char* SQLiteConnection::SQL_SQLITE_TRANSACT_ROLLBACK      = "ROLLBACK;";




/**
 *  Default constructor.
 */    
SQLiteConnection::SQLiteConnection()
{
    init();
}


/**
 *  Initialize DB connection with a file name and a open flags.
 */
SQLiteConnection::SQLiteConnection(const string& fn, OpenFlag of):
    _fileName(fn),
    _of(of)
{
    init();
}

/*------------------------------------------------------------------------------
 *  Destructor     
 *----------------------------------------------------------------------------*/
SQLiteConnection::~SQLiteConnection()
{
    // if a connection is open, let's close it first
    Close();
}

/**
 *  Performs basic initializing.
 */
void SQLiteConnection::init()
{
    _db = nullptr;
    _manualTransOn = false;
    _lastNotifiedAction = sqaUndefined;

    // event callers
    _ec_TransAction = nullptr;
    _ec_UpdateAction = nullptr;
    
}


/*------------------------------------------------------------------------------
 * Sets the callback methods.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::setCallbacks()
{
    sqlite3_commit_hook(_db, &commitHooker, this);
    sqlite3_rollback_hook(_db, &rollbackHooker, this);

    //UpdHookerType uh = &SQLiteConnection::updateHooker;
    
    //UpdHookerType

    //UpdHookerType uh = &updateHooker;


    sqlite3_update_hook(_db, &SQLiteConnection::updateHooker, this);

    //void (*uh)(int, const char*, const char*, sqlite3_int64) = &updateHooker_st;


}

/*------------------------------------------------------------------------------
 * Clears the callback methods.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::clearCallbacks()
{
    sqlite3_commit_hook(_db, NULL, NULL);
    sqlite3_rollback_hook(_db, NULL, NULL);

    sqlite3_update_hook(_db, NULL, NULL);
}



/*------------------------------------------------------------------------------
 * Tries to open a DB connection. Inherits from  DBConnection.
 * virtual 
 *----------------------------------------------------------------------------*/
void SQLiteConnection::Open()
{

    // if it's already open, it's an error
    if(isOpen())
        throwException(ERR_CON_ALREADY_OPEN);

    // performs a check if the filename is presented
    if(_fileName.empty())
        throwException(ERR_FILENAME_NOT_PRESENTED);
        //throw DBException(ERR_FILENAME_NOT_PRESENTED);

    // so, do open
    sqliteOpen(); 

}

/**
 *  Closes the DB connection. Inherits from  DBConnection
 *  virtual
 */
void SQLiteConnection::Close()
{

    // NOTE: it's possible to have a number of scenarious taking into attention
    // the extended state of DB connection

    if(isOpen())
        sqliteClose();
}




/**
 *  Tries to open a sqlite connection using _fileName as a DB filename.
 *  Does not check if the filename is present!   
 *
 *  Additional resource: http://www.sqlite.org/c3ref/open.html.
 */
void SQLiteConnection::sqliteOpen()
{

    sqlite3* db;

    int ores = sqlite3_open_v2(
            _fileName.c_str(),
            &db,
            getSQLiteOpenFlags(_of),            
            0);                         // it's a 0 here for a while, but it could be useful

    // if an error occured
    if(ores != SQLITE_OK)
    {
        // first clean open resource
        sqlite3_close_v2(db);           // even if db == NULL it's ok

        throwException(ERR_OPENING_CONNECTION, ores);
    }

    // open sucessfully
    _db = db;

    // sets callbacks
    setCallbacks();
}


/**
 * Internal close procedure  
 * Does not check if the connection is established! 
 *
 *  Additional resource: http://www.sqlite.org/c3ref/close.html
 */
void SQLiteConnection::sqliteClose()
{
    // clear callbacks
    clearCallbacks();

    sqlite3_close_v2(_db);
    _db = nullptr;
    _manualTransOn = false;
}


/*------------------------------------------------------------------------------
 * Return the state of the DB connection: true if connected, false otherwise.
 * virtual 
 *----------------------------------------------------------------------------*/
bool SQLiteConnection::isOpen()
{
    return (_db != nullptr);
    //return false;           // TODO:
}

/*------------------------------------------------------------------------------
 * Returns true if a connection is ready for statements to work
 *----------------------------------------------------------------------------*/
bool SQLiteConnection::readyForStatement()
{
    return isOpen();        // now it's an equivalent 
}

/*------------------------------------------------------------------------------
 * Creates and return a new linked (to this very connection) statement
 *----------------------------------------------------------------------------*/
SQLiteConnection::Statement* SQLiteConnection::newStatement()
{
    Statement* st = new Statement(this);
    return st;
}
    
    
/*------------------------------------------------------------------------------
 * Creates and return a new linked statement with a SQL expression
 *----------------------------------------------------------------------------*/    
SQLiteConnection::Statement* SQLiteConnection::newStatement(const string& sqlExpr)
{
    Statement* st = new Statement(this, sqlExpr);
    return st;
}

/*------------------------------------------------------------------------------
 * Creates and return a new linked statement with a SQL expression
 *----------------------------------------------------------------------------*/    
SQLiteConnection::Statement* SQLiteConnection::newStatement(const char* sqlExpr)
{
    Statement* st = new Statement(this, sqlExpr);
    return st;

}

                     


/**
 *  Returns a numeric equivalent of open flags.
 *  static 
 */
TUint SQLiteConnection::getSQLiteOpenFlags(const OpenFlag of)
{
    // hardcoded now
    switch(of) {
        case ofReadOnly:
            return SQLITE_OPEN_READONLY;
        case ofReadWrite:
            return SQLITE_OPEN_READWRITE;
        case ofReadWriteCreate:
            return SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    };

    // it should not be a case, where we are in this point, by in the case...
    return 0;

}


/*------------------------------------------------------------------------------
 * Performs a special request to find out what is the last inserted autoincrement value.
 *----------------------------------------------------------------------------*/
//TInt64
int64_t SQLiteConnection::takeLastAutoIncrementValue()
{
    // т.к. может вылезти исключение, пусть указатели дохнут сами!
    std::shared_ptr<Statement> st( newStatement(SQL_SQLITE_SELECT_LAST_AUTOINC) );
    //Statement* st = newStatement(SQL_SQLITE_SELECT_LAST_AUTOINC);
    st->prepare();

    if(st->step() != Statement::rsRow)
        throwException(MOD_SQLITE_CON, ERR_CMN_EXTR_LAUTOINC);

    // trying to manipulate
    Column* laivCol = st->extractColumn(0);     // TODO: 0 -- это плохо, надо тоже константу!

    //TInt64
    int64_t res = laivCol->asInt64();

    delete laivCol;
    //delete st;

    return res;
}



/*------------------------------------------------------------------------------
 * Return true, if a DB is in autocommit mode
 *----------------------------------------------------------------------------*/
bool SQLiteConnection::isAutocommitMode()
{
    // if it isn't open still, it's an error
    if(!isOpen())
        throwException(ERR_CON_NOT_OPEN);

    return (sqlite3_get_autocommit(_db) != 0);

}


/*------------------------------------------------------------------------------
 * Starts manual transaction.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::beginTransaction(bool autoCheck)
{
    // if db not opened, error
    if(!isOpen())
        throwException(ERR_CON_NOT_OPEN);


    // если уже! транзакция ручная включена
    if(_manualTransOn)
    {
        if(autoCheck)       // проверяем: надо ли дать по шапке за недосмотр?
            return;         // просто игнорим
        else
            throwExceptionFormat(ERR_CMN_TRANS_ALR_STARTED);
    }

    // performs a special statement
//    Statement* st = newStatement(SQL_SQLITE_TRANSACT_BEGIN);

    // т.к. может вылезти исключение, пусть указатели дохнут сами!
    std::shared_ptr<Statement> st( newStatement(SQL_SQLITE_TRANSACT_BEGIN) );


    st->prepare();

    //Statement::Result rs = st->step();
    if(st->step() != Statement::rsDone)
        throwException(MOD_SQLITE_CON, ERR_CMN_TRANS_BEGIN);

    
    _manualTransOn = true;
    //return res;

}

/*------------------------------------------------------------------------------
 * Commits manual transaction.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::commitTransaction(bool autoCheck)
{
    // if db not opened, error
    if(!isOpen())
        throwException(ERR_CON_NOT_OPEN);


    // если еще не! включена ручная транзакция 
    if(!_manualTransOn)
    {
        if(autoCheck)       // проверяем: надо ли дать по шапке за недосмотр?
            return;         // просто игнорим
        else
            throwExceptionFormat(ERR_CMN_TRANS_NOT_STARTED);
    }


    // performs a special statement
    // т.к. может вылезти исключение, пусть указатели дохнут сами!
    std::shared_ptr<Statement> st( newStatement(SQL_SQLITE_TRANSACT_COMMIT) );

    st->prepare();
    if(st->step() != Statement::rsDone)
        throwException(MOD_SQLITE_CON, ERR_CMN_TRANS_COMMIT);
    
    _manualTransOn = false;
}


/*------------------------------------------------------------------------------
 * Rollbacks manual transaction.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::rollbackTransaction(bool autoCheck)
{
    // if db not opened, error
    if(!isOpen())
        throwException(ERR_CON_NOT_OPEN);

    // если еще не! включена ручная транзакция 
    if(!_manualTransOn)
    {
        if(autoCheck)       // проверяем: надо ли дать по шапке за недосмотр?
            return;         // просто игнорим
        else
            throwExceptionFormat(ERR_CMN_TRANS_NOT_STARTED);
    }

    // performs a special statement
    // т.к. может вылезти исключение, пусть указатели дохнут сами!
    std::shared_ptr<Statement> st( newStatement(SQL_SQLITE_TRANSACT_ROLLBACK) );

    st->prepare();
    if(st->step() != Statement::rsDone)
        throwException(MOD_SQLITE_CON, ERR_CMN_TRANS_ROLLBACK);
    
    _manualTransOn = false;

}


/*------------------------------------------------------------------------------
 * Callback function handler for commit transaction event.
 *  static
 *----------------------------------------------------------------------------*/
int SQLiteConnection::commitHooker(void* callee)
{
    SQLiteConnection* con = (SQLiteConnection*)(callee);

    // TODO: сюда можно подвесить обработчик!
    // с интерфейсом коллбека!
    
    bool breakCommit = false;

    if(con->_ec_TransAction)
        breakCommit = con->_ec_TransAction->OnCommit(con);


    return (breakCommit) ? 1 : 0;       // СУКАААА!!!

    //return 0;           // TODO: если не 0, то коммит превращается в роллбек!!!!!
}

/*------------------------------------------------------------------------------
 * Callback function handler for rollback transaction event.
 *  static
 *----------------------------------------------------------------------------*/
void SQLiteConnection::rollbackHooker(void* callee)
{
    SQLiteConnection* con = (SQLiteConnection*)(callee);

    if(con->_ec_TransAction)
        con->_ec_TransAction->OnRollback(con);


    // TODO: сюда можно подвесить обработчик!

}


/*------------------------------------------------------------------------------
 *  Callback function handler for update data event.
 *  static
 *----------------------------------------------------------------------------*/
void SQLiteConnection::updateHooker(void* callee, int actCode, 
        const char* dbName, const char* tableName, sqlite3_int64 rowid)
{
    SQLiteConnection* con = (SQLiteConnection*)(callee);

    // save update info for the future use
    // do it thru a special method
    con->updateHooker(actCode, dbName, tableName, rowid);

    


}


/*------------------------------------------------------------------------------
 * Complimentary member method for a \sa updateHooker.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::updateHooker(int actCode, const char* dbName, 
    const char* tableName, sqlite3_int64 rowid)
{
    _lastNotifiedAction = code2SQLAction(actCode);    
    
    _lastNotifiedDBName = dbName;
    _lastNotifiedTableName = tableName;
    _lastNotifiedRowID = rowid;
    
    // if the user's event handler presented
    if(_ec_UpdateAction)
        _ec_UpdateAction->OnUpdate(this, _lastNotifiedAction, dbName, tableName, _lastNotifiedRowID);

}

/*------------------------------------------------------------------------------
 *  Convert given SQL action code to the SQL Action
 *  static
 *----------------------------------------------------------------------------*/
DBConnection::SQLAction SQLiteConnection::code2SQLAction(int actCode)
{
    switch(actCode) {
        case SQLITE_INSERT:
            return sqaInsert;

        case SQLITE_DELETE:
            return sqaDelete;

        case SQLITE_UPDATE:
            return sqaUpdate;
    }; // switch()

    return sqaUndefined;
}




//------------------------------------------------------------------------------
// class Statement
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 * Initializing constructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Statement::Statement(SQLiteConnection* parent):
    _parent(parent)
{
    init();
}


/*------------------------------------------------------------------------------
 * Initializing constructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Statement::Statement(SQLiteConnection* parent, const string& sqlExpr):
    _parent(parent),
    _sqlExpr(sqlExpr)
{
    init();
}

/*------------------------------------------------------------------------------
 * Initializing constructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Statement::Statement(SQLiteConnection* parent, const char* sqlExpr):
    _parent(parent),
    _sqlExpr(sqlExpr)
{
    init();
}



/*------------------------------------------------------------------------------
 * Destructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Statement::~Statement()
{
    // TODO: как-то это надо правильно закончить (проверить)
    //finalize();
    if(_state != stUnprepared)
        finalize(true);
}


/*------------------------------------------------------------------------------
 * Performs basic fields initialization.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::Statement::init()
{
    _ppStmt = nullptr;                      //
    _state = stUnprepared;                  //
    _autoReset = false;                     // default no autoreset performs


    checkParentConnectionForNull();
}


/*------------------------------------------------------------------------------
 * Checks whether a parent connection is null an throws an exception if so.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::Statement::checkParentConnectionForNull()
{
    if(!_parent)    
        throwException(ERR_NULL_PARENT_CON);
}


/*------------------------------------------------------------------------------
 * Prepares a statement. If an error occured, throws an exeption.
 *----------------------------------------------------------------------------*/
void SQLiteConnection::Statement::prepare()
{
    // TODO: вообще, тут перед prepare надо смотреть на уже текущее состояние


    // checks if the parent connection is ready for statements work
    if(!_parent->readyForStatement())
        throwException(ERR_PARENT_CON_NOT_READY);

    sqlite3_stmt* ppStmt;

    int sres = sqlite3_prepare_v2(_parent->getDBHandler(),
        _sqlExpr.c_str(),
        _sqlExpr.length() + 1,          // TODO: это надо проверить-перепроверить!
        &ppStmt,
        nullptr);                       // указатель на недокомпилированную часть выражения, не используем

    // checks if an error occured
    if(sres != SQLITE_OK || ppStmt == NULL)
    {
        //throwException(ERR_STATEMENT_PREPARING, sres);

            // TODO: переделать это в throwSQLiteException + throwSQLiteExceptionFormat
        throwExceptionFormat("%s. SQLite errcode: %d, message: %s", 
            ERR_STATEMENT_PREPARING, 
            sres, 
            sqlite3_errmsg(_parent->getDBHandler()));


    }

    _ppStmt = ppStmt;
    _state = stPrepared;                // change state to "Prepared"

}


/*------------------------------------------------------------------------------
 * Makes an another step for a prepared statement.
 *----------------------------------------------------------------------------*/
/// See  http://www.sqlite.org/c3ref/step.html
///
SQLiteConnection::Statement::Result SQLiteConnection::Statement::step() throw()
{
    // checks if a preparation was complete
    if(_state == stUnprepared)
        throwException(ERR_STATEMENT_NOT_PREPARED);

    if(_state == stError)
        throwException(ERR_STATEMENT_ERROR_STATE);

    // especial state — is a fifnished one
    if(_state == stFinished)
    {
        if(_autoReset)
            reset();
        else
            throwException(ERR_STATEMENT_FINISHED);
    }

    // let's do step
    int sres = sqlite3_step(_ppStmt);

    switch(sres) {

        case SQLITE_BUSY:
            return rsBusy;              // возможно, сюда надо еще какие-то действия, т.к. не все просто затем

        case SQLITE_DONE:
            _state = stFinished;
            return rsDone;

        case SQLITE_ERROR:
            _state = stError;
            throwException(ERR_STATEMENT_WHEN_STEP);

        case SQLITE_ROW:
            return rsRow;

    }; // switch(sres)

    // all others variant should be considered more presisly
    _state = stError;
    //throwException(ERR_STATEMENT_EXC_WHEN_STEP);

    // TODO: переделать это в throwSQLiteException + throwSQLiteExceptionFormat
    throwExceptionFormat("%s. SQLite errcode: %d, message: %s", 
        ERR_STATEMENT_EXC_WHEN_STEP, 
        sres, 
        sqlite3_errmsg(_parent->getDBHandler()));
    //throw
    
    
    return rsError;     // fantom statement, never could occure after throw
}


/*------------------------------------------------------------------------------
 * Resets finished statement for a further using
 *----------------------------------------------------------------------------*/
void SQLiteConnection::Statement::reset()
{
        // checks if a preparation was complete
    if(_state == stUnprepared)
        throwException(ERR_STATEMENT_NOT_PREPARED);

    if(_state == stError)
        throwException(ERR_STATEMENT_ERROR_STATE);

    // let's think that both stPrepared and stFinished are appropriate for resetting

    int sres = sqlite3_reset(_ppStmt);

    if(sres != SQLITE_OK)
        throwException(ERR_STATEMENT_WHEN_RESET);

    // if it's all OK, then reset the state also
    _state = stPrepared;

}

/**
 *  Do nothing if the statament has ot been prepared 
 */
void SQLiteConnection::Statement::finalize()
{
    if(_state == stUnprepared)
        return;

    if(_state == stError)
        finalize(true);             // we have to force finalize it even there is an error

    finalize(false);                // in this very case an error could be when finalize is invoked
}

/**
 *  Internal version of finalization procedure.
 *  If \param ignoreError is true, does not throws an exception in the case when 
 *  invoking finalize yields an error.
 */
void SQLiteConnection::Statement::finalize(bool ignoreError)
{
    int fres = sqlite3_finalize(_ppStmt);

    if(fres != SQLITE_OK)
        if(!ignoreError)
            throwException(ERR_STATEMENT_WHEN_FINAL);
        else 
            _state = stError;

    _ppStmt = NULL;
    _state = stUnprepared;

}

/**
 *  Return a count of column or currently stepped statement  
 */
int SQLiteConnection::Statement::columnCount() throw()
{
    // checks for an appropriate state

    //if(_state == stUnprepared)
    //    throwException(MOD_COL_COUNT, ERR_STATEMENT_NOT_PREPARED);

    //if(_state == stError)
    //    throwException(MOD_COL_COUNT, ERR_STATEMENT_COMMON);

    checkForRowState(MOD_COL_COUNT);

    return sqlite3_column_count(_ppStmt);

    // есть еще int sqlite3_data_count(sqlite3_stmt *pStmt);
    // по описанию разницы вообще быть не должно! http://sqlite.org/c3ref/data_count.html  
}

/** Checks whether the statement in a row-ready state.
 *  If no, throws an appropriate exception
 */
void SQLiteConnection::Statement::checkForRowState() throw()
{
    if(_state == stUnprepared)
        throwException(ERR_STATEMENT_NOT_PREPARED);

    if(_state == stError)
        throwException(ERR_STATEMENT_COMMON);
}

/** Checks whether the statement in a row-ready state.
 *  If no, throws an appropriate exception
 */
void SQLiteConnection::Statement::checkForRowState(const char* modName) throw()
{
    if(_state == stUnprepared)
        throwException(modName, ERR_STATEMENT_NOT_PREPARED);

    if(_state == stError)
        throwException(modName, ERR_STATEMENT_COMMON);

}




/**    
 *  Tries to extract a column with a number \param colNum from stepped statement.
 *  The column object should be destroyed on the caller side.
 *  Also, after another calling step() for the parent statement, this column object
 *  stays in an unappropriate state (thereby it should be destroyed).
 */
SQLiteConnection::Column* SQLiteConnection::Statement::extractColumn(int colNum) throw()
{
    // проверяем, не загнули ли мы с номером, а заодно -- было ли выражение подготовлено
    int colMax = columnCount();

    if(colNum < 0 || colNum > colMax)
        throwException(MOD_EXTR_COLUMN, ERR_WRONG_COL_NUM);

    Column* col = new Column(this, colNum);
    
    
    return col;
}


//------------------------------------------------------------------------------
// class SQLiteConnection::Column
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 *  Constructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Column::Column(Statement* parSt, int colNum):
        _parent(parSt),
        _colNum(colNum),
        _name(nullptr),
        _dataType(nullptr)
{
    //
}

/*------------------------------------------------------------------------------
 *  Desstructor
 *----------------------------------------------------------------------------*/
SQLiteConnection::Column::~Column()
{
    if(_name != nullptr)
        delete _name;       // and put it into null may be? (also true for all other)
    // _name = nullptr;

    if(_dataType != nullptr)
        delete _dataType;    
    

}

/** Returns column name.
 *  If the name wasn't extracted previously, do it
 */
string SQLiteConnection::Column::getName()
{
    if(_name == nullptr)
    {
        _parent->checkForRowState(MOD_COLUMN_GETNAME);
        const char* colName = sqlite3_column_name(_parent->getStatementHandler(), _colNum);
                        
        if(colName == nullptr)
            throwException(MOD_COLUMN_GETNAME, ERR_SQLT_INT_ERROR);

        _name = new string(colName);
    }

    return *(_name);
}


/** Returns column data type.
 *  If the name wasn't extracted previously, do it
 */
DBConnection::ColumnDataType SQLiteConnection::Column::getDataType()
{
    if(_dataType == nullptr)
    {
        extractDataType();

        if(_dataType == nullptr)
            throwException(MOD_COLUMN_GETDT, ERR_COMMON);
    }

    return *(_dataType);

}

/** Extracts the column datatype.
 *  Special internal method.
 */
void SQLiteConnection::Column::extractDataType()
{

    _parent->checkForRowState(MOD_COLUMN_GETDT);
    
    int sdtNum = sqlite3_column_type(_parent->getStatementHandler(), _colNum);
    
    _dataType = new ColumnDataType();

    // TODO: расшифровка — возможно надо в отдельный метод или еще как-то
             
    switch(sdtNum) {

        case SQLITE_INTEGER:
            *_dataType = cdtInt;
            return;

        case SQLITE_FLOAT:
            *_dataType = cdtFloat;
            return;

        case SQLITE_TEXT:
            *_dataType = cdtText;
            return;

        case SQLITE_BLOB:
            *_dataType = cdtBlob;
            return;

        case SQLITE_NULL:
            *_dataType = cdtNull;
            return;


    }; // switch(_dataType)


}

/**
 *  Returns underlying data as BLOB.
 *  Can not be store for a long time due to memony management doing by SQLite.
 *  Must not be deallocated in any way.
 */
const void* SQLiteConnection::Column::asBlob()
{ 
    _parent->checkForRowState(MOD_COLUMN_DATAAS);

    return sqlite3_column_blob(_parent->getStatementHandler(), _colNum);
}

//------------------------------------------------------------------------------
// Returns underlying data as Integer
//------------------------------------------------------------------------------
int SQLiteConnection::Column::asInt()
{ 
    _parent->checkForRowState(MOD_COLUMN_DATAAS);

    return sqlite3_column_int(_parent->getStatementHandler(), _colNum);
}


//------------------------------------------------------------------------------
// Return the size of the result given in string or BLOB form, in bytes
//------------------------------------------------------------------------------
int SQLiteConnection::Column::bytesLen()
{ 
    _parent->checkForRowState(MOD_COLUMN_DATAAS);

    return sqlite3_column_bytes(_parent->getStatementHandler(), _colNum);
}


//------------------------------------------------------------------------------
// Returns underlying data as 64-Integer
//------------------------------------------------------------------------------
//TInt64
int64_t SQLiteConnection::Column::asInt64()
{ 
    _parent->checkForRowState(MOD_COLUMN_DATAAS);

    return sqlite3_column_int64(_parent->getStatementHandler(), _colNum);
}

//------------------------------------------------------------------------------
// Returns underlying data as double
//------------------------------------------------------------------------------
double SQLiteConnection::Column::asDouble()
{ 
    _parent->checkForRowState(MOD_COLUMN_DATAAS);

    return sqlite3_column_double(_parent->getStatementHandler(), _colNum);
}

      


}; // namespace db {
}; // namespace xi {
