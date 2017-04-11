#include "db_request_list.h"

db_request_list::db_request_list()
{

}

db_request_list::~db_request_list()
{

}

bool db_request_list::get_request_list(std::queue<request>& _vecRequest)
{
	boost::mutex::scoped_lock lock(m_mutex);
	_vecRequest = m_request_list;
	return true;
}

void db_request_list::push_request_list(request& _request)
{
	boost::mutex::scoped_lock lock(m_mutex);

	m_request_list.push(_request);

	if (_request.type == sql_action_type::sql_insert)
	{
		updateInsertMap(_request.tableName);
	}
}

void db_request_list::push_request_list(std::string buf, int type, std::string tableName)
{
	boost::mutex::scoped_lock lock(m_mutex);
	request req;
	req.buf = buf;
	req.type = type;
	req.tableName = tableName;
	if (type == sql_action_type::sql_insert)
	{
		updateInsertMap(tableName);
	}
	m_request_list.push(req);
}

int db_request_list::get_request_size()
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_request_list.size();
}

bool db_request_list::pop_request_list(std::string& buf, int& type)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (m_request_list.size() >= 1)
	{
		request req;
		req = m_request_list.front();
		buf = req.buf;
		type = req.type;
		m_request_list.pop();
		return true;
	}

	return false;
}

bool db_request_list::pop_request_list(request& _request)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (m_request_list.size() >= 1)
	{
		_request = m_request_list.front();
		m_request_list.pop();
		return true;
	}

	return false;
}

void db_request_list::initInsertMap(std::string buf, uint64_t value, uint64_t auto_increment)
{
	std::map<std::string, uint64_t>::iterator iterInsertID = m_mapLastInsertID.find(buf);
	if (iterInsertID == m_mapLastInsertID.end())
	{
		//value++;
		m_mapLastInsertID.insert(std::make_pair(buf, value == 0 ? auto_increment - 1 : value));
	}
}

void db_request_list::updateInsertMap(std::string buf)
{
	std::map<std::string, uint64_t>::iterator iterInsertID = m_mapLastInsertID.find(buf);
	if (iterInsertID != m_mapLastInsertID.end())
	{
		iterInsertID->second++;
	}
}

uint64_t db_request_list::getLastInsertID(std::string buf)
{
	std::map<std::string, uint64_t>::iterator iterInsertID = m_mapLastInsertID.find(buf);
	if (iterInsertID != m_mapLastInsertID.end())
	{
		return iterInsertID->second;
	}

	return 0;
}