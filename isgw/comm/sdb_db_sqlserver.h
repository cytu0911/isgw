/*************************************************************
* 文件名：sdb_db_sqlserver.h
* 文件描述：访问sql server接口声明
* 作者：alfredwang
* 日期：2007-09-03
* 版本：OssL-SDBL v1.0.0
* 版权：深圳腾迅计算机系统有限公司
**************************************************************/
#ifndef _SDB_DATABASE_SQLSERVER_H_
#define _SDB_DATABASE_SQLSERVER_H_

#include "sdb_db_base.h"
#include "tds.h"
#include "tdsconvert.h"
#include <vector>
#include <map>
#include <errno.h>
#include <string>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

namespace sdb{

class CSqlServer : public CDataBase
{
public :
    CSqlServer();
    CSqlServer(string serverhost,string dbname , string user , string password , UINT port = 1433);
    virtual ~CSqlServer() ;
    //设置主机
    void SetHost(const CHAR* pStr) { m_serverhost = pStr ;}
    //设置数据库名
    void SetDbName(const CHAR * pStr) { m_dbname = pStr ;}
    //设置连接数据库用户名
    void SetUser(const CHAR * pStr) { m_username = pStr ;}
    //设置连接数据库用户密码
    void SetPasswd(const CHAR * pStr) { m_password = pStr ;}
    //设置数据库连接端口号
    void SetPort(UINT uiPort) { m_port = uiPort ;}
    //创建到数据库服务器的连接
    bool CreateDBConn(void) ;
    //执行sql语句
    bool ExecSQL(const string &) ;
    //获取sql 查询全部结果集
    bool GetQueryResult(RecordContainer &) ;
    //获取指定行数的查询结果集
    bool GetQueryResult(RecordContainer &, bool &, UINT = QUERY_DEFAULT_ROWS) ;
    //获取查询字段列表
    void GetQueryFieldList(vector < string > &) ;
    //获取结果集行数
    UINT GetRowNum(void) const;
    // 查询指定表是否存在
    bool IsExistTable(const string&);
    //清空sql 执行后的结果集缓冲区
    void Clear() ;
    //关闭数据库的连接
    void Close();
    //获取数据库操作最后一次失败信息
    string GetLastError(void);
    // 增加DB连接的引用计数
    void IncRef(void);
    // 减少DB连接的引用计数
    void DecRef(void) ;
    // 获取DB连接的引用计数
    UINT GetRefCount(void) ;

private :
    static int ErrHandler(const TDSCONTEXT *, TDSSOCKET *, TDSMESSAGE *);
    static int MsgHandler(const TDSCONTEXT *, TDSSOCKET *, TDSMESSAGE *);
    bool UseDatabase();
    
private:
    TDSLOGIN* m_login ; //到sql server 服务器登陆信息
    TDSCONTEXT* m_context ; //到sql server服务器的上下文环境
    TDSSOCKET* m_tds ; //到sql server服务器的链接
    string m_libname ; //使用的数据库库名
    string m_serverhost ; //sql server数据库主机
    UINT m_port ; //端口号
    string m_username ; //数据库用户名
    string m_password ; //数据库用户密码
    string m_language ; //数据库语言
    string m_appname ; //应用程序名
    string m_charset ; //字符集
    string m_dbname ; //链接的数据库名称
    static string m_strError ; //数据库操作调用失败信息，只保留了最近一次
    UINT m_linknum ; //连接次数限制
};

}

#ifdef __cplusplus
}
#endif

#endif

