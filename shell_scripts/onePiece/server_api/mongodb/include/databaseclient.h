/**********************************************************************
 *
 *  File Name	: databaseclient.h
 *  Version  	: 1.0
 *  Author		: Jack
 *  Description	: Mongo数据库连接客户端类
 *  History     :
 *  Date          Name		    Reason
 *  2012/10/15    Jack			create file
 *
 **********************************************************************/

#ifndef ___DATA_BASE_CLIENT_H___
#define ___DATA_BASE_CLIENT_H___

#include <iostream>
#include "dbstructbase.h"

typedef std::vector<bo> BSON_VEC;

enum GROUPBY_TYPE
{
	GTYPE_SUM,
	GTYPE_COUNT,
};

class DbBaseConn
{
public:
	/*!
    \brief 构造函数
	\param sIp     数据库IP
	\param nPort   数据库PORT
	\param sDbName 数据库名字
    */
	DbBaseConn();

	/*!
    \brief 析构函数
    */
	~DbBaseConn( void );
public:

	static DbBaseConn* Instance( void );
	/*!
    \brief 构造函数
	\param sIp     数据库IP
	\param nPort   数据库PORT
	\param sDbName 数据库名字
    */
	bool	ConnectMongoDB( const string& sIp, int nPort, const string& sDbName );

	/*!
    \brief  插入数据
	\param  sTableName 表名
	\param  bsonObj    插入数据
	\return int        是否成功
    */
	int		ExecInsert( const string& sTableName, bo *bsonObj );

	/*!
    \brief  更新数据
	\param  sTableName 表名
	\param  query	   条件
	\param  bsonObj    更新数据
	\return int        是否成功
    */
	int		ExecUpdate( const string& sTableName, Query query, bo* bsonObj, bool upinsert=true , bool updateall=false);
	/*!
    \brief  删除数据
	\param  sTableName 表名
	\param  query	   条件
	\param  bsonObj    删除数据
	\return int		   是否成功
    */
	int		ExecDelete( const string& sTableName, Query query, bo bsonObj );
	
	/*!
	\brief  查询数据
	\param  sTableName		  表名
	\param  bonsobjResultList 查询返回列表
	\param  nListNum          列表数
	\return int               是否成功
	*/
	int		ExecQueryGetAllTable( const string& sTableName, BSON_VEC &bonsobjResultList, int* nListNum );
	
	/*!
	\brief  条件查询数据
	\param  sTableName 表名
	\param  query	   条件
	\param  bsonObj    获得数据的结构
	\return int        是否成功
	*/
	int		ExecQueryFindOne( const string& sTableName, Query query, bo* pBsonObj );
	int		ExecQueryFindAll( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum );
	int		ExecQueryFindLimit( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum, int limit_cnt );
	int		ExecQueryFindGroupBy( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum, GROUPBY_TYPE gtype, string groupbystr, string keystr);

	void	SaveStructToMongoDB(Query query, SqlStructBase* sqlstruct, bool updateall=false);
	void	SaveStructToMongoDB(Query query, bo* bsonObj, SqlStructBase* sqlstruct, bool updateall=false);
	void	DeleteStructToMongoDB(SqlStructBase* sqlstruct, Query query);
	void    InsertStructToMongoDB(Query query, SqlStructBase* sqlstruct);
	bool    GetStructFromMongoDB(Query query, SqlStructBase* sqlstruct);
	bool    GetFullStructFromMongoDB(Query query, SqlStructBase*sqlstruct, BSON_VEC& vec_bo);
	bool    GetLimitStructFromMongoDB(Query query, SqlStructBase*sqlstruct, BSON_VEC& vec_bo, int limit_cnt);
	DBClientConnection& GetDBConn();
	string&  GetDBName();
	//int		RunCommand( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum );
	//统计数量，相当于sql count(*)
	int		Count( const string& sTableName, Query query);
	int		GetCountFromMongoDB(Query query, SqlStructBase*sqlstruct);
private:
	DBClientConnection	dbClientConn;
	string				m_sDbName;
};
#define g_dbconn DbBaseConn::Instance()

#endif // ___DATA_BASE_CLIENT_H___
