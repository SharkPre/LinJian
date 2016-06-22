#ifndef _DBSTRUCTBASE_H_
#define _DBSTRUCTBASE_H_
#include <string>
#include <vector>
//#include "gt/log/log.h"
#include "dataparamtype.h"

#define ToString(STR)		#STR
#define SETTYPENAME(NAME)	NAME.SetParamName(ToString(NAME), this);
 
enum EDIRTY_DATA
{
	DIRTY_NONE= 0,
	DIRTY_UPDATE,
	DIRTY_INSERT,
	DIRTY_DELETE,
};

enum EOperate_Orient
{
	EOperate_Orient_None = 0,
	EOperate_Orient_save_useridinfo_Close,
	EOperate_Orient_save_useridinfo_Login,
	EOperate_Orient_save_useridinfo_SaveUserState,
	EOperate_Orient_save_useridinfo_ResetOnlinePlayerNewDayData,
	EOperate_Orient_Max,
};


class SqlStructBase
{
protected:
	SqlStructBase()
	{
		op_orient = EOperate_Orient_None;
		SetNone();
	}
	~SqlStructBase(){}
public:
	 virtual void  StructToBson(bo &boStruct)
	{
		bob bobStr;
		ToBobStruct(bobStr);
		boStruct = bobStr.obj();
	}

	virtual void  BsonToStruct(bo &boStruct)
	{
		BoInitParam(boStruct);
	}

	void			SetInsert();
	void			SetUpdate();
	void			SetDelete();
	void			SetNone();
	bool			NeedSave();
	virtual void	SetTableName(string tbname);
	string			GetTableName();

	//void			SetTableIndex(int index);
	//int				GetTableIndex();
	void			AddTypeToVec(DBParamType* pType);
	void			ToBobStruct(bob &bobStruct);
	void			BoInitParam(bo &boStrue);
	virtual bool	GetUniqueKey() = 0;
public:
	string						tablename;
	//int							tableindex;
	unsigned char				isdirty;
	vector<DBParamType*>		vParam;
	EOperate_Orient				op_orient;
};


#endif