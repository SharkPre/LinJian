#ifndef _DB_REQUEST_LIST_H_
#define _DB_REQUEST_LIST_H_

#include <queue>
#include "singleton.h"
#include <boost/thread.hpp>
#include "error_code.h"
using namespace std;

enum sql_action_type
{
	sql_insert = 1,
	sql_update,
	sql_select,
	sql_delete,
};

struct request
{
	std::string buf;
	std::string tableName;
	int type;//1:insert 2:update 3:select
	request()
	{
		buf = "";
		type = 0;
	}
};

class db_request_list
{
public:

	db_request_list();
	~db_request_list();

private:
	std::queue<request> m_request_list;

	std::map<std::string, uint64_t> m_mapLastInsertID;

	boost::mutex	            m_mutex;

public:

	bool get_request_list(std::queue<request>& _vecRequest);

	void push_request_list(request& _request);

	void push_request_list(std::string buf, int type, std::string tableName);

	int get_request_size();

	bool pop_request_list(std::string& buf, int& type);

	bool pop_request_list(request& _request);

	void initInsertMap(std::string buf, uint64_t type, uint64_t auto_increment);

	void updateInsertMap(std::string buf);

	uint64_t getLastInsertID(std::string buf);
	
	bool empty(){ return m_mapLastInsertID.size() == 0; }
};

#endif