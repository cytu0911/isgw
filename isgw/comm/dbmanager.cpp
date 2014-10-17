//*************************************************************
//******** file :   dbmanager.cpp
//******** date :   2006-06-20
//******** author: dashuiwang
//******** desc :  数据库长连接管理类
//******** history:
//*************************************************************
#include "dbmanager.h"
#include <fstream>
#include "sdb_cfg_unixStyle.h"
#include "isgw_comm.h"

using namespace std;

CDBManager * CDBManager::m_Instance[MAX_NODE_SIZE] = {0};

pthread_mutex_t  CDBManager::m_dbManagerMutex = PTHREAD_MUTEX_INITIALIZER ;

pthread_mutex_t  CDBHandle::m_dbHandleMutex = PTHREAD_MUTEX_INITIALIZER ;

pthread_mutex_t  CWriteLog::m_LogMutex = PTHREAD_MUTEX_INITIALIZER ;

//写日志的类
CWriteLog g_WriteLog(LOG_FILE_NAME) ;
void CWriteLog::WriteLog(const char* filename,int line, const char *LogStr,...)
{
	CAutoLock autoLock(&m_LogMutex);

	FILE  *pstFile =0;
	pstFile=fopen(m_logname.c_str(), "a+" );

	time_t iCurTime;
	time(&iCurTime);
	fprintf(pstFile, "[%s][%s][%d] ", GetDateTimeStr(&iCurTime).c_str(),filename,line);

	va_list ap;
	va_start(ap, LogStr);
	vfprintf(pstFile, LogStr, ap);
	va_end(ap);
	fclose(pstFile);
}


//取当前时间
string GetDateTimeStr(time_t *mytime, int mode)
{
	char s[50];
	struct tm curr;
	curr = *localtime(mytime);
	if (curr.tm_year > 50)
	{
		if (mode == 0)
		{
			sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
		}
		else if(mode == 1)
		{
			sprintf(s, "%04d%02d%02d%02d%02d%02d",
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
		}
	}
	else
	{
		if (mode == 0)
		{
			sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
		}
		else if (mode == 1)
		{
			sprintf(s, "%04d%02d%02d%02d%02d%02d",
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
		}
	}
	return string(s);
}


CDBManager * CDBManager::Instance(const string & strNode)
{
	CAutoLock  autoLock(&m_dbManagerMutex); //加锁

	CDBManager * p = 0;
	int iFirstiEmpty = -1;
	for( int i = 0; i < MAX_NODE_SIZE; i++)
	{
		if( m_Instance[i] == 0 )
		{
			iFirstiEmpty = i;
		}
		else
		{
			if( m_Instance[i]->m_strNode == strNode )
			{
				p = m_Instance[i];
			}
		}
	}

	if( p== 0 ) 
	{
		if( iFirstiEmpty != -1 )
		{
			p = m_Instance[iFirstiEmpty] = new CDBManager(strNode);
		}
	}
	return p;
}

//构造函数
CDBManager::CDBManager(const string & strNode) : m_strNode(strNode)
{
	for(int idx=0;idx<MAX_DB_CONNS;idx++)
	{
		m_ConnStat[idx] = STAT_FREE;
	}

	//创建数据库连接检测线程
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize (&attr,10*1024*1024); //每个堆栈大小为10M

	pthread_create(&m_thCheckDBConn,&attr,(LPFUNC)stRun,this);
}

void CDBManager::stRun(void * para)
{
	CDBManager* instance = ( CDBManager* )para ;
	return  instance->CheckLongDBConn() ;
}


//定时检测所有的数据库长连接 CA,CB
//如果连接长时间不被使用则回收
void CDBManager::CheckLongDBConn()
{
	while(1)
	{
		pthread_mutex_lock(&m_dbManagerMutex);  //保证DB链接的原子性
		int iFreeBusy = 0, iFreeIdle = 0, iBusy = 0, iIdle = 0;
		for( int idx = 0; idx < MAX_DB_CONNS; idx++)
		{ 
			if(  m_ConnStat[idx] == STAT_BUSY )
			{
				m_DBConnPool[idx].SetCA(m_DBConnPool[idx].GetCA()+1); //设置时间

				int idlecount = m_DBConnPool[idx].GetCB();
				if( (++idlecount) >= MAX_QUERY_COUNT )
				{
					iFreeBusy ++;
					m_DBConnPool[idx].Free();
					m_ConnStat[idx] = STAT_FREE;
				}	
				else
				{
					iBusy ++;
					m_DBConnPool[idx].SetCB(idlecount);
				}
			}
			else if (  m_ConnStat[idx] == STAT_IDLE )
			{
				int idlecount = m_DBConnPool[idx].GetCA();
				if( (++idlecount) >= MAX_CONN_COUNT) //空闲超时
				{
					iFreeIdle++;
					m_DBConnPool[idx].Free();
					m_ConnStat[idx] = STAT_FREE;
				}
				else
				{
					iIdle ++;
					m_DBConnPool[idx].SetCA(idlecount); //重新设置空闲时间
				}

			}
		}
		pthread_mutex_unlock(&m_dbManagerMutex);  //保证DB链接的原子性
		ofstream outFile("./CheckLongDBConn.log", ios::app);
		char szLog[256] = {0};
		time_t t = time(0);
		sprintf(szLog, "[%s] Free Busy %d, Free Idle %d, Busy %d, Idle %d, Unuse %d", 
					GetDateTimeStr(&t, 0).c_str(), iFreeBusy, iFreeIdle, iBusy, iIdle, MAX_DB_CONNS-iBusy-iIdle);
		outFile << szLog << endl; 
		//每分钟检测一次
		sleep(SLEEP_TIME);
	}
}

class CUnixDBConf : public CDBConf, private CUnixCnf
{
	string m_strNode;
public:
	CUnixDBConf(const string & strNode) : m_strNode(strNode)
	{
	};
	~CUnixDBConf() {};

	virtual int GetDBType()
	{
		int iType = -1;
		SysConf::instance()->get_conf_int(m_strNode.c_str(), "type", &iType);
		return iType;
	}

	virtual string GetHost()
	{
		char szHost[32] = {0} ;
		SysConf::instance()->get_conf_str(m_strNode.c_str(), "host", szHost, 30);
		return szHost;
	}

	virtual int GetPort()
	{
		int iPort = -1;
		SysConf::instance()->get_conf_int(m_strNode.c_str(), "port", &iPort);
		return iPort;
	}
	virtual string GetUserName()
	{
		char szUser[64] = {0};
		SysConf::instance()->get_conf_str(m_strNode.c_str(), "user", szUser, 60);
		return szUser;
	}
	virtual string GetPasswd()
	{
		char szPasswd[64] = {0};
		SysConf::instance()->get_conf_str(m_strNode.c_str(), "passwd", szPasswd, 60);
		return szPasswd;
	}
	virtual string GetDBName()
	{
		char szDBName[64] = {0};
		SysConf::instance()->get_conf_str(m_strNode.c_str(), "dbname", szDBName, 63);
		return szDBName;
	} 
};

int CDBManager::ExecSql(const string & sql , vector< map<string ,string> > & vRetData, string & strErrMsg)
{
	int idx;

	CDataBase * pDB = 0;
	pthread_mutex_lock(&m_dbManagerMutex);  //Lock

	//Choose connection
	for( idx = 0;  idx < MAX_CONN_COUNT; idx++)
	{
		if( m_ConnStat[idx] != STAT_BUSY )
		{
			break;
		}
	}

	if( idx == MAX_CONN_COUNT )
	{
		pthread_mutex_unlock(&m_dbManagerMutex);
		strErrMsg = "No more free connections";
		return -1;
	}

	int oldStat = m_ConnStat[idx];
	m_ConnStat[idx] = STAT_BUSY;
	m_DBConnPool[idx].SetCB(0);
	pthread_mutex_unlock(&m_dbManagerMutex);

	//Create connection
	if( oldStat == STAT_FREE )
	{
		CUnixDBConf cfg(m_strNode);
		m_DBConnPool[idx].CreateHandle(&cfg);
	}	

	pDB = m_DBConnPool[idx].GetDBConn();
	if( pDB == 0 )
	{
		pthread_mutex_lock(&m_dbManagerMutex);  //Lock
		m_ConnStat[idx] = STAT_FREE;
		pthread_mutex_unlock(&m_dbManagerMutex);
		strErrMsg = "GetDBConn failed.";
		return -1;
	}

	int ret = -2;
	strErrMsg="";
	vRetData.clear();
	try 
	{
		if ( pDB->ExecSQL(sql) &&  pDB->GetQueryResult(vRetData) )
		{
			ret = 0;
		}
		else
		{
			strErrMsg = pDB->GetLastError();
		}
	}
	catch (...)
	{
		pthread_mutex_lock(&m_dbManagerMutex);
		m_DBConnPool[idx].Free();
		m_ConnStat[idx] = STAT_FREE;
		pthread_mutex_unlock(&m_dbManagerMutex);
		return -2;
	}

	pthread_mutex_lock(&m_dbManagerMutex);
	m_ConnStat[idx] = STAT_IDLE;
	pthread_mutex_unlock(&m_dbManagerMutex);

	return ret;
}


