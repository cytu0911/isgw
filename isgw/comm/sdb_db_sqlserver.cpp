/*************************************************************
* 文件名：sdb_db_sqlserver.cpp
* 文件描述：访问sql server接口实现
* 作者：alfredwang
* 日期：2007-09-03
* 版本：OssL-SDBL v1.0.0
* 版权：深圳腾迅计算机系统有限公司
**************************************************************/

#include "sdb_db_sqlserver.h"
using namespace std;
using namespace sdb;

string CSqlServer::m_strError = "" ;

sdb::CSqlServer::CSqlServer()
{
    m_login = NULL ;
    m_context = NULL ;
    m_tds = NULL ;
    m_libname = "TDS-Library" ;
    m_language = "us_english" ;
    m_appname = program_invocation_short_name ; //设置成运行程序名
    m_charset = "ISO-8859-1";
    m_linknum = 0 ; //连接次数初始化为0
}

sdb::CSqlServer::CSqlServer(string serverhost,string dbname , string user , string password , UINT port )
{
    m_login = NULL ;
    m_context = NULL ;
    m_tds = NULL ;
    m_libname = "TDS-Library" ;
    m_serverhost = serverhost ;
    m_port = port ;
    m_username = user ;
    m_password = password ;
    m_language = "us_english" ;
    m_appname = program_invocation_short_name ; //设置成运行程序名
    m_charset = "ISO-8859-1";
    m_dbname = dbname ;
    m_linknum = 0 ; //连接次数初始化为0
}

sdb::CSqlServer:: ~CSqlServer()
{
    if(m_login)  tds_free_login(m_login) ;
    if(m_context) tds_free_context(m_context);
    if(m_tds) tds_free_socket(m_tds);
}

bool sdb::CSqlServer::CreateDBConn(void)
{
    if(m_linknum == 0)
    {
    //申请分配TDSLOGIN内存块
    m_login = tds_alloc_login();
    if( !m_login )
    	{
    	    CSqlServer::m_strError = "Allocate Login Information of TDS  Failed" ;
	    return false ;
    	}

    //申请分配TDSCONTEXT内存块及相关域
    m_context = tds_alloc_context(NULL);
    if( !m_context )
    	{
    	    CSqlServer::m_strError = "Allocate Context Information of TDS Failed" ;
	    return false ;
    	}
    if(m_context->locale)
    	{
    	    m_context->locale->date_fmt = strdup("%Y-%m-%d %H:%M:%S");
    	}
    m_context->msg_handler = MsgHandler ;
    m_context->err_handler = ErrHandler ;

    //初始化设置m_login
    tds_set_user(m_login,m_username.c_str());
    tds_set_app(m_login ,m_appname.c_str()) ;
    tds_set_library(m_login , m_libname.c_str()) ;
    tds_set_server(m_login , m_serverhost.c_str());
    tds_set_port(m_login , m_port);
    tds_set_client_charset(m_login , m_charset.c_str());
    tds_set_language(m_login , m_language.c_str());
    tds_set_passwd(m_login , m_password.c_str());

    //申请分配TDSSOCKET内存块
    m_tds = tds_alloc_socket(m_context , 512);
    if( !m_tds)
    	{
    	    CSqlServer::m_strError = "Allocate socket Information of TDS Failed" ;
	    return false ;
    	}
    tds_set_parent(m_tds,NULL);

    //申请分配TDSCONNECT内存块
    TDSCONNECTION* connect = tds_read_config_info(m_tds ,m_login , m_context->locale) ;
     if( !connect )
     	{
     	    CSqlServer::m_strError = "Allocate TDS Connection Failed" ;
	    return false ;
     	}
     //设置数据库
     //tds_dstr_copy(&(connect->database),m_dbname.c_str());

    //建立与数据库主机的连接
    if( tds_connect(m_tds , connect) == TDS_FAIL)
    	{
    	    CSqlServer::m_strError = "Create Connection To Database Failed" ;
	    tds_free_connection(connect) ;
	    return false ;
    	}
    tds_free_connection(connect) ;
    UseDatabase();
   IncRef();
   
   return true ;
   }
   else
   {
       return true ;
   }
	
}

bool sdb::CSqlServer::UseDatabase()
{
    //连接到数据库
   string sql = "use " + m_dbname ;
   int result = tds_submit_query(m_tds , sql.c_str());
   if( result != TDS_SUCCEED)
   	{
   	    CSqlServer::m_strError = "Submit Sql Query Error";
	    return false ;
   	}
   TDS_INT result_type = 0 ;
   result = tds_process_tokens(m_tds ,&result_type,NULL,TDS_TOKEN_RESULTS);
   if( result != TDS_SUCCEED)
   	{
   	    CSqlServer::m_strError = "Use Database Error" ;
	    return false ;
   	}

   return true;
}

 bool sdb::CSqlServer::ExecSQL(const string & sql)
 {
     if(!m_tds)
     	{
     	    CSqlServer::m_strError = "Connect Invalid" ;
	    return false ;
     	}
	 else
	 {
	     if(TDS_SUCCEED != tds_submit_query(m_tds , sql.c_str()))
	     	{
	     	    CSqlServer::m_strError = "Query Failed" ;
		    return false ;
	     	}
		 else
		 {
		     return true ;
		 }
	 }
 }

 bool sdb::CSqlServer::GetQueryResult(RecordContainer & results) 
 {
     int rc ;
     TDSCOLUMN *col;
     int ctype;
     CONV_RESULT dres;
     unsigned char *src;
     TDS_INT srclen;
     TDS_INT resulttype;
     while((rc = tds_process_tokens(m_tds,&resulttype,NULL,TDS_TOKEN_RESULTS)) 
	 	== TDS_SUCCEED)
     	{
     	    switch(resulttype)
     	    	{
     	    	    case TDS_ROWFMT_RESULT :
				break ;
		    case TDS_COMPUTE_RESULT :
		    case TDS_ROW_RESULT :
				while((rc = tds_process_tokens(m_tds,&resulttype,NULL,
					TDS_STOPAT_ROWFMT|TDS_RETURN_DONE|
					TDS_RETURN_ROW|TDS_RETURN_COMPUTE)) ==TDS_SUCCEED)
					{
					    if(resulttype != TDS_ROW_RESULT && resulttype != 
						TDS_COMPUTE_RESULT)
					    	{
					    	    break ;
					    	}
					    if ( !m_tds->current_results)
							continue ;
					    map<string,string> row_value ;
					    for(int i = 0 ; i < m_tds->current_results->num_cols ; i++)
					    	{
					    	    col = m_tds->current_results->columns[i] ;
						    string rname = col->column_name ;
						    if(col->column_cur_size < 0)
						    	{
						    	    row_value.insert(pair<string,string>(rname,""));
							    continue ;
						    	}
						    else
						    	{
						    	    ctype = tds_get_conversion_type(col->column_type,
										col->column_size);
							    src = &(m_tds->current_results->current_row[col->column_offset]);
							    if(is_blob_type(col->column_type))
							    	{
							    	    src = (unsigned char*)((TDSBLOB*)src)->textvalue;
							    	}
							    srclen = col->column_cur_size;
							    if(tds_convert(m_tds->tds_ctx,ctype,(TDS_CHAR*)src,srclen,
									SYBVARCHAR ,&dres) < 0)
							    	{
							    	    row_value.insert(pair<string,string>(rname,""));
							    	}
								else
								{
								   row_value.insert(pair<string,string>(rname,dres.c)) ;
								   free(dres.c);
								}
						    	}
					    	}
					    results.push_back(row_value) ;
					}
				break ;
			case TDS_STATUS_RESULT :
			default : 
				break ;
     	    	}
     	}

	return true ;
}

bool sdb::CSqlServer::GetQueryResult(RecordContainer & results, bool & ret, UINT num)
{
    return true ;
}

 void sdb::CSqlServer::GetQueryFieldList(vector < string > & fieldlist) 
 {
    int rc ;
    TDS_INT resulttype;
    while((rc = tds_process_tokens(m_tds,&resulttype,NULL,TDS_TOKEN_RESULTS)) 
		== TDS_SUCCEED)
    	{
    	    switch(resulttype)
    	    	{
    	    	    case TDS_ROWFMT_RESULT :
				if(m_tds->current_results)
				{
				    for(int i =0 ; i < m_tds->current_results->num_cols ; i++)
				    	{
				    	   fieldlist.push_back(m_tds->current_results->columns[i]->column_name);
				    	}
				}
				break ;
		    case TDS_COMPUTE_RESULT:
		    case TDS_ROW_RESULT:
				while((rc = tds_process_tokens(m_tds,&resulttype,NULL,
					TDS_STOPAT_ROWFMT|TDS_RETURN_DONE|
					TDS_RETURN_ROW|TDS_RETURN_COMPUTE)) ==TDS_SUCCEED)
					{
					    if(resulttype != TDS_ROW_RESULT && resulttype != TDS_COMPUTE_RESULT)
							break ;
					}
		    case TDS_STATUS_RESULT:
		    default :
				break ;
    	    	}
    	}
 }

 UINT sdb::CSqlServer::GetRowNum(void) const
 {
     unsigned int rows = 0;
     int rc ;
     TDS_INT resulttype;
      while((rc = tds_process_tokens(m_tds,&resulttype,NULL,TDS_TOKEN_RESULTS)) 
		== TDS_SUCCEED)
      	{
      	    switch(resulttype)
      	    	{
      	    	    case TDS_ROWFMT_RESULT:
				break ;
		    case TDS_COMPUTE_RESULT :
		    case TDS_ROW_RESULT :
				while((rc = tds_process_tokens(m_tds,&resulttype,NULL,
					TDS_STOPAT_ROWFMT|TDS_RETURN_DONE|
					TDS_RETURN_ROW|TDS_RETURN_COMPUTE)) ==TDS_SUCCEED)
					{
					    if(resulttype != TDS_ROW_RESULT && resulttype != TDS_COMPUTE_RESULT)
							break ;
					    if ( !m_tds->current_results) continue ;
					    rows++ ;
					}
      	    	}
      	}

	return rows ;
 }

 bool sdb::CSqlServer::IsExistTable(const string& tablebname)
 {
     return true;
 }

 void sdb::CSqlServer::Clear()
 {
     if(m_tds) 
     	{
     	    tds_free_socket(m_tds);
	    DecRef();
     	}
 }

 void sdb::CSqlServer::Close()
 {
    if(m_login)  tds_free_login(m_login) ;
    if(m_context) tds_free_context(m_context);
    if(m_tds) tds_free_socket(m_tds);
 }

 string sdb::CSqlServer::GetLastError(void)
 {
     return CSqlServer::m_strError ;
 }

 int sdb::CSqlServer::ErrHandler(const TDSCONTEXT * cnt, TDSSOCKET * sck, TDSMESSAGE * msg)
 {
     ostringstream out ;
     if( msg->msgno == 0)
     	{
     	    out<<"Error Message : "<<msg->message<<endl;
	    CSqlServer::m_strError = out.str();
	    return 1 ;
     	}
     return 0;
 }

 int sdb::CSqlServer::MsgHandler(const TDSCONTEXT * cnt, TDSSOCKET * sck, TDSMESSAGE * msg)
 {
     ostringstream out ;
     if( msg->msgno == 0)
     	{
     	    out<<"Error Message : "<<msg->message<<endl;
	    CSqlServer::m_strError = out.str();
	    return 1 ;
     	}
     return 0;
 }

 void sdb::CSqlServer::IncRef(void)
 {
     m_linknum++;
 }

 void sdb::CSqlServer::DecRef(void) 
 {
     m_linknum--;
 }

 UINT sdb::CSqlServer::GetRefCount(void) 
 {
     return m_linknum;
 }
 
