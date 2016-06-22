#ifndef _DATAPARAMTYPE_H_
#define _DATAPARAMTYPE_H_
//#include "gt/log/log.h"


#include "mongo/bson/bson.h"
#include "mongo/client/dbclient.h"

using namespace mongo;
using namespace bson;
using namespace std;

class SqlStructBase;
class DBParamType
{
public:
	DBParamType();
	~DBParamType();
public:
	virtual void	AppendToBo(bob &bobObj) = 0;
	virtual void    GetFieldToVal(bo &boObj) = 0;
public:
	inline bool		IsChange(){return is_change;}
	int				GetParamType() const;
	const string&	GetParamName() const;
	void			SetParamType(int type);
	void			SetParamName(const string& str, SqlStructBase* _pSqlBase);
private:
	string			paramname;
	int				paramtype;
protected:
	bool			is_change;
};

class DBParamInt16 : public DBParamType
{
public:
	DBParamInt16()
	{
		val = 0;
		SetParamType(mongo::NumberInt);
	}
	~DBParamInt16(){}
	DBParamInt16& operator=(int _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	operator	int() { return val; }
	int			GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::NumberInt)
		{
			val = boObj.getFieldDotted(GetParamName()).Int();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::NumberInt);
		}
	}
private:
	int				val;
};

class DBParamInt32 : public DBParamType
{
public:
	 explicit DBParamInt32()
	{
		val = 0;
		SetParamType(mongo::NumberInt);
	}
	~DBParamInt32(){}
	 DBParamInt32& operator=(int _val)
	{
		val = _val;
		is_change=true;
		return *this;
	 }
	operator	int() { return val; }
	int			GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::NumberInt)
		{
			val = boObj.getFieldDotted(GetParamName()).Int();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::NumberInt);
		}
	}
private:
	int				val;
};

class DBParamInt64 : public DBParamType
{
public:
	DBParamInt64()
	{
		val = 0;

		SetParamType(mongo::NumberLong);
	}
	~DBParamInt64(){}
	DBParamInt64& operator=(long long _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	operator	long long() { return val; }
	long long	GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::NumberLong)
		{
			val = boObj.getFieldDotted(GetParamName()).Long();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::NumberLong);
		}
	}
private:
	long long		val;
};

class DBParamString : public DBParamType
{
public:
	DBParamString()
	{
		val = "";
		SetParamType(mongo::String);
	}
	~DBParamString(){}
	DBParamString& operator=(const string& _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	string&		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::String)
		{
			val = boObj.getFieldDotted(GetParamName()).String();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::String);
		}
	}
private:
	string			val;
};

class DBParamDouble : public DBParamType
{
public:
	DBParamDouble()
	{
		val = 0;
		SetParamType(mongo::NumberDouble);
	}
	~DBParamDouble(){}
	DBParamDouble& operator=(double _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	double		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::NumberDouble)
		{
			val = boObj.getFieldDotted(GetParamName()).Double();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::NumberDouble);
		}
	}
private:
	double			val;
};

class DBParamBool : public DBParamType
{
public:
	DBParamBool()
	{
		val = false;
		SetParamType(mongo::Bool);
	}
	~DBParamBool(){}
	DBParamBool& operator=(bool _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	operator	bool() { return val; }
	bool		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::Bool)
		{
			val = boObj.getFieldDotted(GetParamName()).Bool();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::Bool);
		}
	}
private:
	bool			val;
};

class DBParamByte : public DBParamType
{
public:
	DBParamByte()
	{
		val = 0;
		SetParamType(mongo::NumberInt);
	}
	~DBParamByte(){}
	DBParamByte& operator=(int _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	operator	int() { return val; }
	int			GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::NumberInt)
		{
			val = boObj.getFieldDotted(GetParamName()).Int();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::NumberInt);
		}
	}
private:
	int				val;
};

class DBParamBSON : public DBParamType
{
public:
	DBParamBSON()
	{
		//val = "";
		SetParamType(mongo::Object);
	}
	~DBParamBSON(){}
	DBParamBSON& operator=(const bo& _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	bo&		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::Object)
		{
			val = boObj.getFieldDotted(GetParamName()).Obj();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::Object);
		}
	}
private:
	bo			val;
};

class DBParamDate : public DBParamType
{
public:
	DBParamDate()
	{
		//val = "";
		SetParamType(mongo::Date);
	}
	~DBParamDate(){}
	DBParamDate& operator=(const Date_t& _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	Date_t&		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::Date)
		{
			val = boObj.getFieldDotted(GetParamName()).Date();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::Date);
		}
	}
private:
	Date_t			val;
};

class DBParamArray : public DBParamType
{
public:
	DBParamArray()
	{
		//val = "";
		SetParamType(mongo::Array);
	}
	~DBParamArray(){}
	DBParamArray& operator=(const std::vector<BSONElement>& _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	std::vector<BSONElement>&		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::Array)
		{
			val = boObj.getFieldDotted(GetParamName()).Array();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::Array);
		}
	}
private:
	std::vector<BSONElement>			val;
};

class DBParamObjectID : public DBParamType
{
public:
	DBParamObjectID()
	{
		//val = "";
		SetParamType(mongo::jstOID);
	}
	~DBParamObjectID(){}
	DBParamObjectID& operator=(OID& _val)
	{
		val = _val;
		is_change=true;
		return *this;
	}
	OID&		GetVal(){return val;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.append(GetParamName(), GetVal());
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::jstOID)
		{
			val = boObj.getFieldDotted(GetParamName()).OID();
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::jstOID);
		}
	}
private:
	OID			val;
};

#define BIN_MAX 512
class DBParamBinary : public DBParamType
{
public:
	DBParamBinary()
	{
		memset(val, 0, sizeof(val));
		SetParamType(mongo::BinData);
	}
	~DBParamBinary(){}
	DBParamBinary& BinDataCopy(const char* _val, int _len)
	{
		if (_len >= BIN_MAX)
		{
			_len = BIN_MAX;
		}
		memcpy(val, _val, _len);
		len = _len;
		is_change=true;
		return *this;
	}
	char*		GetVal(){return val;}
	int			GetSize(){return len;}
	void		AppendToBo(bob &bobObj)
	{
		bobObj.appendBinData(GetParamName(),len, BinDataGeneral, val);
	}
	void		GetFieldToVal(bo &boObj)
	{
		int ftype = boObj.getFieldDotted(GetParamName()).type();
		if (ftype == mongo::BinData)
		{
			const char* ptmp = boObj.getFieldDotted(GetParamName()).binData(len);
			if (len >= BIN_MAX)
			{
				len = BIN_MAX;
			}
			memcpy(val, ptmp, len);
		}
		else
		{
			//GT_WARNING("field name [%s] type error %d is not %d", GetParamName().c_str(), ftype, mongo::BinData);
		}
	}
private:
	char	val[BIN_MAX];
	int		len;
};


#endif