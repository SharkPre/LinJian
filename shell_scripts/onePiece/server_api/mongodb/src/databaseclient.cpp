/**********************************************************************
*
*  File Name	: DataBaseClient.cpp
*  Version  	: 1.0
*  Author		: Jack
*  Description	: Mongo数据库连接客户端类
*  History		:
*  Date         Name		Reason
*  2012/10/15   Jack		create file
*
**********************************************************************/

//#include "dbstructbase.h"
#include "databaseclient.h"

//--------------------------------------------------------------------
// 构造函数
//--------------------------------------------------------------------
DbBaseConn::DbBaseConn()
{

}

//--------------------------------------------------------------------
// 析构函数
//--------------------------------------------------------------------
DbBaseConn::~DbBaseConn( void )
{

}

DbBaseConn* DbBaseConn::Instance( void )
{
	static DbBaseConn* m_dbconn = NULL;
	if (m_dbconn == NULL)
	{
		m_dbconn = new DbBaseConn();
	}
	return m_dbconn;
}

bool DbBaseConn::ConnectMongoDB( const string& sIp, int nPort, const string& sDbName )
{
	string errMsg;
	HostAndPort hostandport( sIp, nPort );
	if (dbClientConn.connect( hostandport, errMsg ))
	{
		m_sDbName = sDbName;
		return true;
	}
	else
	{
		//GT_ERROR("ConnectMongoDB error:%s", errMsg.c_str());
		return false;
	}
}

//--------------------------------------------------------------------
// 插入数据
//--------------------------------------------------------------------
int DbBaseConn::ExecInsert( const string& sTableName, bo *bsonObj )
{
	dbClientConn.insert( m_sDbName + "." + sTableName, *bsonObj );
	return 1;
}

//--------------------------------------------------------------------
// 更新数据
//--------------------------------------------------------------------
int DbBaseConn::ExecUpdate( const string& sTableName, Query query, bo* bsonObj, bool upinsert, bool updateall )
{
	//bo o = BSON( "hello" << "world"<< "hello2" << "world2" );
	//for (int i=0; i<10; ++i)
	//{
	//	dbClientConn.insert("test.foo", o);

	//	string e = dbClientConn.getLastError();
	//	if( !e.empty() ) { 
	//		cout << "insert #1 failed: " << e << endl;
	//	}

	//	// make an index with a unique key constraint
	//	dbClientConn.ensureIndex("test.foo", BSON("hello"<<1<<"hello2"<<1), /*unique*/true);

	//	dbClientConn.insert("test.foo", o); // will cause a dup key error on "hello" field
	//}
	dbClientConn.update( m_sDbName + "." + sTableName, query, *bsonObj, upinsert, updateall );
	return 1;
}

//--------------------------------------------------------------------
// 删除数据
//--------------------------------------------------------------------
int DbBaseConn::ExecDelete( const string& sTableName, Query query, bo bsonObj )
{
	dbClientConn.remove( m_sDbName + "." + sTableName, query, false );
	return 1;
}
//--------------------------------------------------------------------
// 查询数据
//--------------------------------------------------------------------
int DbBaseConn::ExecQueryGetAllTable( const string& sTableName, BSON_VEC &bonsobjResultList, int* nListNum )
{
	unsigned long long count = dbClientConn.count( m_sDbName + "." + sTableName );
	*nListNum = (int)count;
	//int tmp_id = 1020001;
	//Query que = QUERY( "id"<<tmp_id );
	//bo res = dbClientConn.findOne( "Naruto_config.BuffEffect",que );
	//string bsons = res.toString();

	auto_ptr<DBClientCursor> cursor = dbClientConn.query( m_sDbName + "." + sTableName, Query() );
	if (!cursor.get())
	{
		return 0;
	}

	while( cursor->more() )
	{
		//cout << (cursor->next().toString()) << endl;
		bo tmpBson;
		tmpBson = cursor->next().copy();
		bonsobjResultList.push_back( tmpBson );
	}
	//system("pause");
	return 1;
}

//--------------------------------------------------------------------
// 条件查询数据
//--------------------------------------------------------------------
int DbBaseConn::ExecQueryFindOne( const string& sTableName, Query query, bo* pBsonObj )
{
	int nResult = 1;
	auto_ptr<DBClientCursor> cursor = dbClientConn.query( m_sDbName + "." + sTableName, query );
	if (!cursor.get())
	{
		return 0;
	}

	while( cursor->more() )
	{
		*pBsonObj = cursor->next().copy();
		nResult = 0;
	}

	return nResult;
}

int DbBaseConn::ExecQueryFindAll( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum )
{
	unsigned long long count = 0;
	auto_ptr<DBClientCursor> cursor = dbClientConn.query( m_sDbName + "." + sTableName, query );

	if (!cursor.get())
	{
		return 0;
	}

	while( cursor->more() )
	{
		count++;
		//cout << (cursor->next().toString()) << endl;
		bo tmpBson;
		tmpBson = cursor->next().copy();
		bonsobjResultList.push_back( tmpBson );
	}
	*nListNum = (int)count;
	return 0;
}

int DbBaseConn::Count( const string& sTableName, Query query)
{
	unsigned long long n = dbClientConn.count( m_sDbName + "." + sTableName, query.obj);
	return n;
}

void DbBaseConn::SaveStructToMongoDB(Query query, SqlStructBase* sqlstruct, bool updateall)
{
	bo boStruct;
	sqlstruct->GetUniqueKey();
	sqlstruct->StructToBson(boStruct);
	bo boObj = BSON("$set"<<boStruct);

	ExecUpdate( sqlstruct->GetTableName(), query, &boObj,true,updateall);
}

void DbBaseConn::SaveStructToMongoDB(Query query, bo* bsonObj, SqlStructBase* sqlstruct, bool updateall)
{
	ExecUpdate( sqlstruct->GetTableName(), query, bsonObj,false,updateall);
}

void DbBaseConn::DeleteStructToMongoDB(SqlStructBase* sqlstruct, Query query)
{
	bo boStruct;
	//sqlstruct->StructToBson(boStruct);
	ExecDelete( sqlstruct->GetTableName(), query, boStruct);
}

void DbBaseConn::InsertStructToMongoDB(Query query, SqlStructBase* sqlstruct)
{
	bo boStruct;
	sqlstruct->GetUniqueKey();
	sqlstruct->StructToBson(boStruct);
	ExecInsert( sqlstruct->GetTableName(), &boStruct);
}

bool DbBaseConn::GetStructFromMongoDB(Query query, SqlStructBase* sqlstruct)
{
	bo bsonUser;
	int nResult = ExecQueryFindOne( sqlstruct->GetTableName(), query, &bsonUser );
	if ( nResult == 0 )
	{
		sqlstruct->BsonToStruct(bsonUser);
		return true;
	}
	return false;
}

bool DbBaseConn::GetFullStructFromMongoDB(Query query, SqlStructBase*sqlstruct, BSON_VEC& vec_bo)
{
	BSON_VEC vbson;
	int nListNum = 0;

	ExecQueryFindAll( sqlstruct->GetTableName(), query, vbson, &nListNum );
	if (nListNum<=0)
	{
		return false;
	}
	BSON_VEC::iterator itbson = vbson.begin();
	while( itbson != vbson.end())
	{
		vec_bo.push_back((*itbson));
		itbson++;
	}
	return true;
}

int DbBaseConn::GetCountFromMongoDB(Query query, SqlStructBase*sqlstruct)
{
	BSON_VEC vbson;
	int num = Count( sqlstruct->GetTableName(), query);
	
	return num;
}

bool    DbBaseConn::GetLimitStructFromMongoDB(Query query, SqlStructBase*sqlstruct, BSON_VEC& vec_bo, int limit_cnt)
{
	BSON_VEC vbson;
	int nListNum = 0;

	ExecQueryFindLimit( sqlstruct->GetTableName(), query, vbson, &nListNum, limit_cnt );
	if (nListNum<=0)
	{
		return false;
	}
	BSON_VEC::iterator itbson = vbson.begin();
	while( itbson != vbson.end())
	{
		vec_bo.push_back((*itbson));
		itbson++;
	}
	return true;
}

int		DbBaseConn::ExecQueryFindLimit( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum, int limit_cnt )
{
	unsigned long long count = 0;
	auto_ptr<DBClientCursor> cursor = dbClientConn.query( m_sDbName + "." + sTableName, query, limit_cnt );

	if (!cursor.get())
	{
		return 0;
	}

	while( cursor->more() )
	{
		count++;
		//cout << (cursor->next().toString()) << endl;
		bo tmpBson;
		tmpBson = cursor->next().copy();
		bonsobjResultList.push_back( tmpBson );
	}
	*nListNum = (int)count;
	return 0;
}

/*
	EXAMPLE::     !!!
	
	BSON_VEC bonsobjResultList;
	int nListNum;
	Query query;
	g_dbconn->ExecQueryFindGroupBy("userbaseinfo", query, bonsobjResultList, &nListNum, GTYPE_COUNT, "nUserSex",  "nUserLevel");
*/

int DbBaseConn::ExecQueryFindGroupBy( const string& sTableName, Query query, BSON_VEC &bonsobjResultList, int* nListNum, GROUPBY_TYPE gtype, string groupbystr, string keystr)
{
	const string ns = m_sDbName+"."+sTableName;
	string tmpmapstr = "function";
	tmpmapstr = tmpmapstr + " Map(){ emit(this." + groupbystr + ", {" + keystr + ": this." + keystr + "});}";
	const char *map = tmpmapstr.c_str();
	string tmpreducestr = "function";
	tmpreducestr = tmpreducestr + " Reduce(key, values) {	var reduced = {" + keystr + ":0};  values.forEach(function(val) { reduced." + keystr;
	switch(gtype)
	{
	case GTYPE_COUNT:
		{
			tmpreducestr += " += 1;}); return reduced;}";
			break;
		}
	case GTYPE_SUM:
		{
			tmpreducestr += " += val." + keystr + ";}); return reduced;}";
			break;
		}
	default:
		break;
	}
	const char* reduce = tmpreducestr.c_str();
	const string outcoll = ns + ".out";

	BSONObj out;
	out = dbClientConn.mapreduce(ns, map, reduce, BSONObj()); // default to inline
	MONGO_PRINT(out);
	if (!out["results"].isNull())
	{
		BSONElement ele = out["results"];
		vector<BSONElement> vec_ele = ele.Array();
		for (int i=0; i<(int)vec_ele.size(); ++i)
		{
			BSONObj tmpbo = vec_ele[i].Obj();
			bob bbtmp;
			bbtmp.append(groupbystr, tmpbo["_id"].Number());
			bbtmp.append(keystr, tmpbo["value"][keystr].Number());
			bo boStruct = bbtmp.obj();
			bonsobjResultList.push_back(boStruct);
		}
	}
	return 0;
}


DBClientConnection& DbBaseConn::GetDBConn()
{
	return dbClientConn;
}

string&  DbBaseConn::GetDBName()
{
	return m_sDbName;
}