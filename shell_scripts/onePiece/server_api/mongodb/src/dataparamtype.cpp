#include "dbstructbase.h"
#include "dataparamtype.h"

DBParamType::DBParamType():is_change(false)
{

}

DBParamType::~DBParamType()
{

}

int DBParamType::GetParamType() const
{
	return paramtype;
}

const string& DBParamType::GetParamName() const
{
	return paramname;
}

void DBParamType::SetParamType(int type)
{
	paramtype = type;
}

void DBParamType::SetParamName(const string& str, SqlStructBase* _pSqlBase)
{
	paramname = str;
	_pSqlBase->AddTypeToVec(this);
}