/*************************************************************
* �ļ�����sdb_db_sqlserver.h
* �ļ�����������sql server�ӿ�����
* ���ߣ�alfredwang
* ���ڣ�2007-09-03
* �汾��OssL-SDBL v1.0.0
* ��Ȩ��������Ѹ�����ϵͳ���޹�˾
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
    //��������
    void SetHost(const CHAR* pStr) { m_serverhost = pStr ;}
    //�������ݿ���
    void SetDbName(const CHAR * pStr) { m_dbname = pStr ;}
    //�����������ݿ��û���
    void SetUser(const CHAR * pStr) { m_username = pStr ;}
    //�����������ݿ��û�����
    void SetPasswd(const CHAR * pStr) { m_password = pStr ;}
    //�������ݿ����Ӷ˿ں�
    void SetPort(UINT uiPort) { m_port = uiPort ;}
    //���������ݿ������������
    bool CreateDBConn(void) ;
    //ִ��sql���
    bool ExecSQL(const string &) ;
    //��ȡsql ��ѯȫ�������
    bool GetQueryResult(RecordContainer &) ;
    //��ȡָ�������Ĳ�ѯ�����
    bool GetQueryResult(RecordContainer &, bool &, UINT = QUERY_DEFAULT_ROWS) ;
    //��ȡ��ѯ�ֶ��б�
    void GetQueryFieldList(vector < string > &) ;
    //��ȡ���������
    UINT GetRowNum(void) const;
    // ��ѯָ�����Ƿ����
    bool IsExistTable(const string&);
    //���sql ִ�к�Ľ����������
    void Clear() ;
    //�ر����ݿ������
    void Close();
    //��ȡ���ݿ�������һ��ʧ����Ϣ
    string GetLastError(void);
    // ����DB���ӵ����ü���
    void IncRef(void);
    // ����DB���ӵ����ü���
    void DecRef(void) ;
    // ��ȡDB���ӵ����ü���
    UINT GetRefCount(void) ;

private :
    static int ErrHandler(const TDSCONTEXT *, TDSSOCKET *, TDSMESSAGE *);
    static int MsgHandler(const TDSCONTEXT *, TDSSOCKET *, TDSMESSAGE *);
    bool UseDatabase();
    
private:
    TDSLOGIN* m_login ; //��sql server ��������½��Ϣ
    TDSCONTEXT* m_context ; //��sql server�������������Ļ���
    TDSSOCKET* m_tds ; //��sql server������������
    string m_libname ; //ʹ�õ����ݿ����
    string m_serverhost ; //sql server���ݿ�����
    UINT m_port ; //�˿ں�
    string m_username ; //���ݿ��û���
    string m_password ; //���ݿ��û�����
    string m_language ; //���ݿ�����
    string m_appname ; //Ӧ�ó�����
    string m_charset ; //�ַ���
    string m_dbname ; //���ӵ����ݿ�����
    static string m_strError ; //���ݿ��������ʧ����Ϣ��ֻ���������һ��
    UINT m_linknum ; //���Ӵ�������
};

}

#ifdef __cplusplus
}
#endif

#endif

