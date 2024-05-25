///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Contains the declaration of the classes related to SQLite DB engine.
///
/// . 
///
///////////////////////////////////////////////////////////////////////////////
#pragma

#ifndef xisqliteH
#define xisqliteH

#include <string>
#include <cstdint>

#include "xi/db/dbmain.h"
#include "xi/db/sqlite/sqlite3.h"



//#include "xi/xiTypes.h"
//#include "xi/types/monikers.h"

namespace xi {
namespace db {

using std::string;
using namespace xi::types;

/// Represents connection to a SQLite db.
/**
 *  Additional resource:
 *      - Error Codes And Messages (http://www.sqlite.org/c3ref/errcode.html)
 *      - Result Codes (http://www.sqlite.org/c3ref/c_abort.html)
 */
class SQLiteConnection : public DBConnection {
public:
    //------------<Consts>---------------- 
    
    // SQLite based String Constants
    static const char* ERR_SQLT_INT_ERROR;

    // Modules description
    static const char* MOD_SQLITE_CON;


    // Special SQL requests
    static const char* SQL_SQLITE_SELECT_LAST_AUTOINC;

    static const char* SQL_SQLITE_TRANSACT_BEGIN;
    static const char* SQL_SQLITE_TRANSACT_COMMIT;
    static const char* SQL_SQLITE_TRANSACT_ROLLBACK;


public:
    //------------<Types>---------------- 
    
    /**
     *  Special type for callback functions need to update hooker
     */
    typedef void (*UpdHookerType)(void*, int, const char*, const char*, sqlite3_int64);

    
    /** \brief Enumeration for the flags modifying connection behaviour.
     *
     *  >
     */
    enum OpenFlag {
        ofReadOnly,                         ///< The database is opened in read-only mode. If the database does not 
                                            ///< already exist, an error is returned.
        ofReadWrite,                        ///< The database is opened for reading and writing if possible, 
                                            ///< or reading only if the file is write protected by the operating 
                                            ///< system. In either case the database must already exist, 
                                            ///< otherwise an error is returned.
        ofReadWriteCreate,                  ///< The database is opened for reading and writing, 
                                            ///< and is created if it does not already exist. 
    }; // enum OpenFlag



    class Column;

    /** \brief Special class represents a SQLite statement
     *
     *  Statement class is in aggregation with SQLiteConnection class.
     *  It should be managed for a lifetime for an each Statement by programmer
     */
    class Statement {
    public:
        //------------<Types>---------------- 
        
        /** \brief Statement prepare result
         *
         *  Each constant corresponds to a native result explaining in http://www.sqlite.org/c3ref/step.html.
         */
        enum Result {
            //rsOK,
            rsDone,             ///< SQLITE_DONE 
            rsBusy,             ///< SQLITE_BUSY 
            rsRow,              ///< SQLITE_ROW
            rsError,            ///< fanton value, never used indeed 
        }; // enum Result


        /** \brief Determines the state of a statement.
         *
         *  >
         */
        enum State {
            stUnprepared,
            stPrepared,
            stFinished,
            stError
        }; // enum State

    public:
        //------------<Constructors and destructor>---------------- 
        Statement(SQLiteConnection* parent);                            ///< Initializing constructor
        Statement(SQLiteConnection* parent, const string& sqlExpr);     ///< Initializing constructor
        Statement(SQLiteConnection* parent, const char* sqlExpr);       ///< Initializing constructor
        ~Statement();                                                   ///< Destructor
        
    public:
        //------------<Public Methods>----------------
        void prepare();                                                 ///< Prepares a statement. If an error occured, throws an exeption.
        Result step() throw();                                                  ///< Makes an another step for a prepared statement.

        void reset();                                                   ///< Resets finished statement for a further using
        void finalize();                                                ///< Finalizes an prepared statement.

        //--- Column of current rowset specific methoods
        int columnCount() throw();                                      ///< Return a count of column or currently stepped statement
        Column* extractColumn(int colNum) throw();                      ///< Tries to extract a column from stepped statement

        void checkForRowState() throw();                                ///< Checks whether the statement in a row-ready state.
        void checkForRowState(const char* modName) throw();





    public:
        //------------<Set/Get>----------------
        
        /// returns statement's parent connection
        SQLiteConnection* getParentConnection() const 
        {
            return _parent;
        }


        /**
         *  Returns SQLite statement hanler underlying.
         */
        sqlite3_stmt* getStatementHandler() const
        { 
            return _ppStmt;
        }

        /// returns the SQL-expression
        string getSQLExpr() const 
        {
            return _sqlExpr;
        }

        /// sets the SQL-expression
        void setSQLExpr(const string& sqlExpr) 
        {
            _sqlExpr = sqlExpr;
        }


    private:
        //------------<Private fields>---------------- 
        SQLiteConnection*   _parent;                    ///< Represents a parent SQLite connection object
        string              _sqlExpr;                   ///< An SQL expression to be execute
        sqlite3_stmt*       _ppStmt;                    ///< An internal statement handler

        State               _state;                     ///< Determines the current state of the statement.
        bool                _autoReset;                 ///< Flag determines if resetting of finished statement should be done automatically.

        // TODO: состояние выражения (подготовлено, выполнено и т.д.)

    private:
        //------------<Private methods>---------------- 
        void init();                                    ///< Performs basic fields initialization.
        void checkParentConnectionForNull();            ///< Checks whether a parent connection is null an throws an exception if so.
        void finalize(bool ignoreError);                ///< Finalizes an prepared statement internally with a some flags.

    }; // class Statement


    
    /** \brief Represents a column from a row of executed statement.
     *
     *  >
     */
    class Column {
        friend class Statement;
    public:
        //------------<Constructors and destructor>---------------- 
        Column(Statement* parSt, int colNum);           ///< Initialized with a parent statement
        ~Column();                 

    public:
        //------------<Public Methods>----------------
        string getName();                           ///< Returns column name.
        ColumnDataType getDataType();               ///< Returns column data type.

        // get data in various format
        // see http://sqlite.org/c3ref/column_blob.html
        const void* asBlob();                       ///< Returns underlying data as BLOB
        int         asInt();                        ///< Returns underlying data as Integer
        //TInt64      asInt64();                      ///< Returns underlying data as 64-Integer
        int64_t      asInt64();                      ///< Returns underlying data as 64-Integer
        double      asDouble();                     ///< Returns underlying data as double

        int         bytesLen();                     ///< Return the size of the result given in string or BLOB form, in bytes 

        // TODO: все методы получения сюда надо!
        

    private:
        //------------<Private fields>---------------- 
        Statement*      _parent;                    ///< Represents a parent statement
        int             _colNum;                    ///< Column number
        
        // these fields could not be assigned befor the first request
        string*         _name;                      ///< Column name. If NULL, then it wasn't extracted yet.
        ColumnDataType* _dataType;                  ///< Column data type. If NULL, then it wasn't extracted yet.

    private:
        //------------<Private methods>---------------- 
        void extractDataType();                     ///< Extracts the column datatype.


    }; // class Column

    //------------<Callback Interfaces>---------------- 
    
    
    /** \brief SQLite Connection-specified update notificator interface.
     *
     *  >
     */
    class IUpdateAction {
    public:

        /**
         *  Begin transaction event handler
         */
        virtual void OnUpdate(SQLiteConnection* con, SQLAction act, 
            const char* dbName, const char* tableName, int64_t rowid) = 0;
            //const char* dbName, const char* tableName, TInt64 rowid) = 0;
    }; // class IUpdateAction


public:
    //------------<Consts>---------------- 
    
    // /// Default SQLite open flags as a number
    // static const TUint DEFAULT_SQLITE_OFLAGS = 
    //    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    
    /// Default SQLite open flags as a typed obj
    static const OpenFlag DEFAULT_SQLITE_OF = ofReadWriteCreate;


    //static const TUint SQLITE_OFLAGS = 
    //SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;



    //------------<Constructors and destructor>---------------- 
    
    
    SQLiteConnection();                     ///< Default constructor
    SQLiteConnection(const string& fn, 
        OpenFlag of = DEFAULT_SQLITE_OF);   ///< Initializing constructor

    ~SQLiteConnection();                    ///< Destructor


    //------------<Public Methods>---------------- 
    virtual void Open();                    // Tries to open a DB connection. Inherits from  DBConnection
    virtual void Close();                   // Closes the DB connection. Inherits from  DBConnection
    virtual bool isOpen();                  // Return the state of the DB connection: true if connected, false otherwise.

    // not inherited
    bool readyForStatement();                       ///< Returns true if a connection is ready for statements to work

    Statement* newStatement();                      ///< Creates and return a new linked (to this very connection) statement
    Statement* newStatement(const string& sqlExpr); ///< Creates and return a new linked statement with a SQL expression
    Statement* newStatement(const char* sqlExpr);   ///< Creates and return a new linked statement with a SQL expression

    

    //-- Transactions

    /** \brief Starts manual transaction.
     *
     *  \param autoCheck determines whether the method should check if the current state is in manual transaction mode. 
     *  If \param autoCheck is false and manual transaction mode is on then an exception will be thrown.
     */
    void beginTransaction(bool autoCheck = false);
    
    /** \brief Commits manual transaction.
     *
     *  \param autoCheck determines whether the method should check if the current state is in manual transaction mode. 
     *  If \param autoCheck is false and manual transaction mode is off then an exception will be thrown.
     */
    void commitTransaction(bool autoCheck = false);
    
    /** \brief Rollbacks manual transaction.
     *
     *  \param autoCheck determines whether the method should check if the current state is in manual transaction mode.
     *  If \param autoCheck is false and manual transaction mode is off then an exception will be thrown.
     */
    void rollbackTransaction(bool autoCheck = false);


    //-- Special helper methods

    /** \brief Permorms a special request to find out what is the last inserted autoincrement value.
     *
     *  Work with a regular statement so no extra checks is performed.
     */
    int64_t takeLastAutoIncrementValue();
    //TInt64 takeLastAutoIncrementValue();

    /** \brief Return true, if a DB is in autocommit mode
     *
     *  If DB is not open throws an exception
     */
    bool isAutocommitMode();


    //------------<Set/Get>----------------

    /// returns an underlying SQLite DB handler
    sqlite3* getDBHandler() const
    {
        return _db;
    }

    /// Filename 
    std::string getFileName() const 
    {
        return _fileName;
    }


    /// Returns Flag indicating if a Manual transaction has been started
    bool isManualTransactionOn() const 
    {
        return _manualTransOn;            
    }

    /// Returns a last SQL action notified thru callback
    SQLAction getLastNotifiedAction() const
    { 
        return _lastNotifiedAction; 
    }
    
    /// Returns a name of last DB has been affected by the action
    const string& getLastNotifiedDBName() const
    {
        return _lastNotifiedDBName;
    }


    /// Returns a name of table of last DB has been affected by the action
    const string& getLastNotifiedTableName() const
    {
        return _lastNotifiedTableName;
    }

    
    /// Returns a rowid of a last inserted row
    //TInt64 getLastNotifiedRowID() const
    int64_t getLastNotifiedRowID() const
    {
        return _lastNotifiedRowID;
    }
        

    /// Gets Event recepient for transaction actions
    ITransactionAction* get_ec_TransAction() const
    {
        return _ec_TransAction;
    }

    /// Sets Event recepient for transaction actions
    void set_ec_TransAction(ITransactionAction* ta) 
    {
        _ec_TransAction = ta;
    }


    /// Gets Event recepient for update actions
    IUpdateAction* get_ec_UpdateAction() const
    {
        return _ec_UpdateAction;
    }

    /// Sets Event recepient for update actions
    void set_ec_UpdateAction(IUpdateAction* ua) 
    {
        _ec_UpdateAction = ua;
    }


    



public:
     //------------<Public Static Methods>---------------- 
    static TUint getSQLiteOpenFlags(const OpenFlag of);     ///< Returns a numeric equivalent of open flags
    static SQLAction code2SQLAction(int actCode);           ///< Convert given SQL action code to the SQL Action

private:
    //------------<Private fields>---------------- 

    // SQLite-oriented
    sqlite3* _db;                   ///< SQLite DB handle
    OpenFlag _of;                   ///< SQLite open flags

    // DB-oriented
    std::string _fileName;          ///< DB file name given in UTF-8!

    // transactions
    bool _manualTransOn;            ///< Flag indicates if a Manual transaction has been started

    // some info of the last update statement given from callback
    SQLAction _lastNotifiedAction;  ///< Represents a last SQL action notified thru callback
    string _lastNotifiedDBName;     ///< Represents a name of last DB has been affected by the action
    string _lastNotifiedTableName;  ///< Represents a name of table of last DB has been affected by the action
    int64_t _lastNotifiedRowID;      ///< Represents a rowid of a last inserted row
    //TInt64 _lastNotifiedRowID;      ///< Represents a rowid of a last inserted row


    //--- event callers

    /**
     *  Event recepient for transaction actions like commit and rollback (no begin for this connection type)
     */
    ITransactionAction* _ec_TransAction;
    IUpdateAction*      _ec_UpdateAction;


private:
    //------------<Private methods>---------------- 
    void init();                    ///< Performs basic fields initialization.

    // DB interconnection
    void sqliteOpen();              ///< Tries to open an sqlite connection
    void sqliteClose();             ///< Internal close procedure 

    // calbacks
    void setCallbacks();            ///< Sets the callback methods.
    void clearCallbacks();          ///< Clears the callback methods.

private:
    //------------<Private static methods>---------------- 
    
    //-- special static methods for callbacking

    /**
     *  Callback function handler for commit transaction event.
     */
    static int commitHooker(void* callee);                         

    /**
     *  Callback function handler for rollback transaction event.
     */
    static void rollbackHooker(void* callee);

    /** \brief Callback function handler for update data event.
     *  
        \param callee is a pointer to the instance of this class.
        \param actCode represents an action and could be one of SQLITE_INSERT, SQLITE_DELETE, 
        or SQLITE_UPDATE, depending on the operation that caused the callback to be invoked.
        \param dbName is a pointer to the database and \param tableName is a pointer to the 
        table name containing the affected row.
        \param rowid is the rowid of the row. In the case of an update, this is the rowid after the update takes place. 
        
        See http://sqlite.org/c3ref/update_hook.html
     */
    static void updateHooker(void* callee, int actCode, const char* dbName, const char* tableName, sqlite3_int64 rowid);

    /** \brief Complimentary member method for a \sa updateHooker.
     *
     *  >
     */
    void updateHooker(int actCode, const char* dbName, const char* tableName, sqlite3_int64 rowid);

    // 2DEL
    //static void updateHooker_st(void* , int actCode, const char* dbName, const char* tableName, sqlite3_int64 rowid){};


}; // class SQLiteConnection



}; // namespace db {
}; // namespace xi {

#endif // xisqliteH
