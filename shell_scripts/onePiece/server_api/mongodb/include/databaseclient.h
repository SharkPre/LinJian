/**********************************************************************
 *
 *  File Name	: databaseclient.h
 *  Version  	: 1.0
 *  Author		: Jack
 *  Description	: Mongo���ݿ����ӿͻ�����
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
    \brief ���캯��
	\param sIp     ���ݿ�IP
	\param nPort   ���ݿ�PORT
	\param sDbName ���ݿ�����
    */
	DbBaseConn();

	/*!
    \brief ��������
    */
	~DbBaseConn( void );
public:

	static DbBaseConn* Instance( void );
	/*!
    \brief ���캯��
	\param sIp     ���ݿ�IP
	\param nPort   ���ݿ�PORT
	\param sDbName ���ݿ�����
    */
	bool	ConnectMongoDB( const string& sIp, int nPort, const string& sDbName );

	/*!
    \brief  ��������
	\param  sTableName ����
	\param  bsonObj    ��������
	\return int        �Ƿ�ɹ�
    */
	int		ExecInsert( const string& sTableName, bo *bsonObj );

	/*!
    \brief  ��������
	\param  sTableName ����
	\param  query	   ����
	\param  bsonObj    ��������
	\return int        �Ƿ�ɹ�
    */
	int		ExecUpdate( const string& sTableName, Query query, bo* bsonObj, bool upinsert=true , bool updateall=false);
	/*!
    \brief  ɾ������
	\param  sTableName ����
	\param  query	   ����
	\param  bsonObj    ɾ������
	\return int		   �Ƿ�ɹ�
    */
	int		ExecDelete( const string& sTableName, Query query, bo bsonObj );
	
	/*!
	\brief  ��ѯ����
	\param  sTableName		  ����
	\param  bonsobjResultList ��ѯ�����б�
	\param  nListNum          �б���
	\return int               �Ƿ�ɹ�
	*/
	int		ExecQueryGetAllTable( const string& sTableName, BSON_VEC &bonsobjResultList, int* nListNum );
	
	/*!
	\brief  ������ѯ����
	\param  sTableName ����
	\param  query	   ����
	\param  bsonObj    ������ݵĽṹ
	\return int        �Ƿ�ɹ�
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
	//ͳ���������൱��sql count(*)
	int		Count( const string& sTableName, Query query);
	int		GetCountFromMongoDB(Query query, SqlStructBase*sqlstruct);
private:
	DBClientConnection	dbClientConn;
	string				m_sDbName;
};
#define g_dbconn DbBaseConn::Instance()

#endif // ___DATA_BASE_CLIENT_H___
