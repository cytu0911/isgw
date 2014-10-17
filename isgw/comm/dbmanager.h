//*************************************************************
//******** file :   dbmanager.h
//******** date :   2006-06-20
//******** author: dashuiwang
//******** desc :  数据库长连接管理类
//******** history:
//*************************************************************
#ifndef _DB_MANAGER_
#define _DB_MANAGER_

#include <list>
#include <string>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include "sdb_db_base.h"
#include "sdb_db_mysql.h" 
#include "sdb_db_sqlserver.h"
#include <map>

//最多可连接数据库个数
#define MAX_NODE_SIZE 3 

//连接池最多128个数据库长连接
#define MAX_DB_CONNS  128

//数据库连接最多空闲30分钟
#define MAX_IDLE_COUNT 30

#define MAX_QUERY_COUNT 2 //最大query 时间 2min

#define MAX_CONN_COUNT 25 //最大连接时间28min

//每分钟检测一次
#define SLEEP_TIME 60

using namespace std;
using namespace sdb;
//extern UNIX_config<NDS_single_config_node>  g_stConf;

typedef void * (*LPFUNC)(void *) ;
//自动化锁
class CAutoLock
{
	private:

		pthread_mutex_t* m_ptLock;
	public:
		CAutoLock(pthread_mutex_t* ptLock)
		{
			m_ptLock = ptLock;
			pthread_mutex_lock(m_ptLock);
		}
		~CAutoLock()
		{
			pthread_mutex_unlock(m_ptLock);
		}
};

class CDBConf
{
public:
    virtual int GetDBType() = 0;
    virtual string GetHost() = 0;
    virtual int GetPort() = 0;
    virtual string GetUserName() = 0;
    virtual string GetPasswd() = 0;
    virtual string GetDBName() = 0;
};

//数据库连接标识
class CDBHandle
{
private:
    CDataBase * m_pConn;

    int    m_iCA       ;  //计数器A  记录 Connection time
    int    m_iCB	;	 //计数器  B 记录Querytime
    int    m_iArgc	;
    static pthread_mutex_t m_dbHandleMutex ;
public:
    CDBHandle() {
        m_pConn = 0;
        m_iCA = 0;
        m_iCB = 0;
        m_iArgc = 0;
    }
    ~CDBHandle()
    {
        CAutoLock autoLock(&m_dbHandleMutex);
	Free();
    }

    int   GetCA()
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        return m_iCA;
    }

    void  SetCA (const int &count)
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        m_iCA = count;
    }

    int GetCB()
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        return m_iCB;

    }
    void SetCB( const int & count)
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        m_iCB = count;
    }

    CDataBase * GetDBConn()
    {
        return m_pConn;
    }

    void Free()
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        delete m_pConn;
        m_pConn = 0;
    }

    CDataBase * CreateHandle( CDBConf * cfg)
    {
        CAutoLock autoLock(&m_dbHandleMutex);
        if(m_pConn != 0 )
        {
            delete m_pConn;
            m_pConn = 0;
        }

        m_iCA = 0;
        m_iCB = 0;
        m_iArgc = 0;

        switch( cfg->GetDBType() )
        {
            case 0:
                m_pConn = new CMySql(cfg->GetHost().c_str(), cfg->GetUserName().c_str(), 
                            cfg->GetPasswd().c_str(), cfg->GetDBName().c_str(), cfg->GetPort());
                break;
            case 1:
                m_pConn = new CSqlServer(cfg->GetHost(), cfg->GetDBName(), cfg->GetUserName(),
                            cfg->GetPasswd(), cfg->GetPort());
                break;
            default:
                break;
        } 

        if( m_pConn && m_pConn->CreateDBConn() == false )
        {
            delete m_pConn;
            m_pConn = 0;
        }
	return m_pConn;
    }
};

//长连接管理池类
class CDBManager
{
    string m_strNode;
private:
    CDBHandle  m_DBConnPool[MAX_DB_CONNS];   //数据库连接池
//    list<int> m_DBIdleIdx ;                                //保存空闲的节点ID
//    list<int> m_DBBusyIdx  ;                             //保存已经在使用的节点
//    list<int> m_DBUndoIdx  ;                             //保存尚未建立Conncection的节点 
    enum { STAT_FREE, STAT_IDLE, STAT_BUSY};

    int m_ConnStat[MAX_DB_CONNS];

    static void  stRun(void * para) ;
    pthread_t m_thCheckDBConn ;
private:
    static CDBManager * m_Instance[MAX_NODE_SIZE]; 
    static pthread_mutex_t m_dbManagerMutex ;
protected:
    CDBManager(const string & strNode) ;
public:
    void CheckLongDBConn() ;
    static CDBManager * Instance(const string & strNode);

    int ExecSql(const string & sql , vector< map<string ,string> > & vRetData, string & strErrMsg);

    ~CDBManager()
    {
        pthread_cancel(m_thCheckDBConn) ;
    }
};

std::string GetDateTimeStr(time_t *mytime, int mode=0);

//写日志的类
class CWriteLog
{
private:
    string m_logname ;  //日志文件名称
    static pthread_mutex_t m_LogMutex ;
public:
    CWriteLog()
    {}

    CWriteLog( string logname)
    {
        m_logname = logname;
    }

    ~CWriteLog()
    {}

    void WriteLog(const char* filename,int line, const char *LogStr,...);
} ;

#define LOG_FILE_NAME "./logs/CustomDBMgr.log"

extern CWriteLog g_WriteLog;

#define CWRITELOG(s...) g_WriteLog.WriteLog(__FILE__,__LINE__,##s);

#endif

