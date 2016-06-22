#include "dbstructbase.h"
#include <sstream>

void SqlStructBase::SetInsert()
{
	isdirty = DIRTY_INSERT;
}

void SqlStructBase::SetUpdate()
{
	isdirty = DIRTY_UPDATE;
}

void SqlStructBase::SetDelete()
{
	isdirty = DIRTY_DELETE;
}

void SqlStructBase::SetNone()
{
	isdirty = DIRTY_NONE;
}

bool SqlStructBase::NeedSave()
{
	if (isdirty == DIRTY_NONE)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void SqlStructBase::SetTableName(string tbname)
{
	tablename = tbname;
}

string	SqlStructBase::GetTableName()
{
	return tablename;
}

//void SqlStructBase::SetTableIndex(int index)
//{
//	tableindex = index;
//}
//
//int	 SqlStructBase::GetTableIndex()
//{
//	return tableindex;
//}

void SqlStructBase::AddTypeToVec(DBParamType* pType)
{
	vParam.push_back(pType);
}

void SqlStructBase::ToBobStruct(bob &bobStruct)
{
	int vsize = vParam.size();
	for (int i=0; i<vsize; ++i)
	{
		if (vParam[i]->IsChange())
			vParam[i]->AppendToBo(bobStruct);
	}
}

void SqlStructBase::BoInitParam(bo &boStrue)
{
	int vsize = vParam.size();
	for (int i=0; i<vsize; ++i)
	{
		vParam[i]->GetFieldToVal(boStrue);
	}
}
