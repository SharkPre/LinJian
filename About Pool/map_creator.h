/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//地图生成器

#pragma once

#include "map.h"
#include "mutex.h"
#include <vector>

#include "role_mgr.h"
#include "role.h"
#include "..\_common\_server_frame\class_template_factory.h"

//#define USE_HEBING_MAP_THREAD			//开启是否使用合并线程

struct tag_map_info;
class Map;
class Team;
class map_instance;
class MapMgr;
class MapRestrict;
class map_threadmgr;

//---------------------------------------------------------------------------------
// 切换地图的结构
//---------------------------------------------------------------------------------
struct tag_change_map_info
{
	DWORD		dw_role_id;			// 角色ID
	DWORD		dw_dest_map_id;		// 目标地图ID
	Vector3		v_position;				// 目标地图坐标
	Vector3		v_face;				// 朝向
	DWORD		dw_misc;				// 如果针对于随机副本，则代表副本难度
	UINT16		lineId;				//线路Id，自动分配则为0，

	tag_change_map_info(DWORD)
	{
		dw_role_id = INVALID_VALUE;
	}
	tag_change_map_info()
	{
		dw_role_id = INVALID_VALUE;
	}
};

//-----------------------------------------------------------------------------------
// 副本删除结构
//-----------------------------------------------------------------------------------
struct tag_instance_destroy_info
{
	DWORD		dw_map_id;			// 地图ID
	DWORD		dw_instance_id;		// 副本ID

	tag_instance_destroy_info(DWORD)
	{
		dw_map_id = INVALID_VALUE;
		dw_instance_id = INVALID_VALUE;
	}
	tag_instance_destroy_info(DWORD _dwMapID, DWORD _dwInstanceID)
	{
		dw_map_id = _dwMapID;
		dw_instance_id = _dwInstanceID;
	}
	tag_instance_destroy_info()
	{
		dw_map_id = INVALID_VALUE;
		dw_instance_id = INVALID_VALUE;
	}
};

//-------------------------------------------------------------------------------------------------
// 地图管理器
//-------------------------------------------------------------------------------------------------
class map_creator
{
public:
	typedef		package_map<DWORD, MapMgr*>		MapMgrMap;
	typedef		package_map<DWORD, tag_map_info*>	MapInfoMap;

public:
	map_creator();
	~map_creator();
	static bool s_use_threads;

	//---------------------------------------------------------------------------------------------
	// 初始化，更新及销毁
	//---------------------------------------------------------------------------------------------
	BOOL				init();
	VOID				update();
	VOID				destroy();

	//--------------------------------------------------------------------------------------------
	// 工厂相关
	//--------------------------------------------------------------------------------------------
	Map*				create_factory_map(const tag_map_info* p_info_);
	map_instance*		create_factory_map_instance(const tag_map_info* p_info_);
	MapRestrict*		create_factory_map_restrict(const tag_map_info* p_info_);

	VOID				destroy_factory_map(Map* p_map_);
	VOID				destroy_factory_map_restrict(MapRestrict* p_restrict_);

	//--------------------------------------------------------------------------------------------
	// 线程管理
	//--------------------------------------------------------------------------------------------
	VOID				start_all_map_manager();								// 本Tick开启所有地图线程运行
	VOID				wait_all_map_manager_end();								// 等待本Tick所有地图线程结束
	VOID				update_all_delayed_stuff();						// 每一Tick更新所有地图线程暂停后的操作
	VOID				stop_all_map_manager();								// 永久停止所有地图管理器线程
	Event&				get_all_map_start_event(INT n);						// 得到本Tick开启所有地图线程运行的事件
	VOID				one_map_manager_end();									// 一个地图线程本Tick执行完毕

	//--------------------------------------------------------------------------------------------
	// 各种Get
	//--------------------------------------------------------------------------------------------
	MapMgr*				get_map_manager(DWORD dw_map_id_)			{ return m_map_map_manager.find(dw_map_id_); }
	const tag_map_info*	get_map_info(DWORD dw_map_id_)			{ return m_map_map_info.find(dw_map_id_); }
	DWORD				get_born_map_id()						{ return m_dw_born_map_id; }
	BOOL				is_map_exist(DWORD dw_map_id_)			{ return m_map_map_info.is_exist(dw_map_id_); }
	BOOL				is_sbk_map(DWORD dw_map_id);

	DWORD				get_first_map_id()						{ return m_dw_first_map_id; } //第一次进入地图ID
	Vector3				get_first_position()					//{ return m_pv_first_position[0]; } //第一次进入地图坐标
	{
		INT n_rand = get_tool()->tool_rand() % m_n_born_position_num;
		return m_pv_first_position[n_rand];
	}

	Vector3				rand_get_one_born_position(INT& n_index_);
	FLOAT				get_born_yaw(INT n_index_);
	Map*				get_map(DWORD dw_map_id_, DWORD dw_instance_);
	Map*				get_line_map(DWORD dw_map_id_, UINT16 nLine);
	DWORD				get_prison_map_id()			{ return m_dw_prison_map_id; }
	const Vector3		get_putin_prison_point()		{ return m_putin_prison_point; }
	const Vector3		get_putout_prison_point()		{ return m_putout_prison_point; }
	const Vector3		get_reborn_point(DWORD dw_reborn_map_id_);

	DWORD				get_yamun_map_id( ) const { return m_dw_yamun_map_id; }
	const Vector3&		get_putin_yamun_point( ) const { return m_putin_yamun_point; }
	
	UINT16					get_stable_instance_num()		{ return m_n_max_stable_instance_num; }
	//UINT16					get_stable_instance_rolenum()	{ return m_n_max_stable_instance_rolenum; }
	//UINT16					get_main_instance_rolenum()	{ return m_n_max_main_instance_rolenum; }
	UINT16					get_map_instance_del_time()		{ return m_n_map_instance_del_time; }
	UINT16					get_select_map_state()			{ return m_n_select_map_state; }
	UINT16					get_good_map_state()			{ return m_n_good_map_state; }
	UINT16					get_busy_map_state()			{ return m_n_busy_map_state; }
	

	INT					get_map_role_num(DWORD dw_map_id);
	INT					get_map_leave_role_num(DWORD dw_map_id);
	INT					get_instance_role_num(DWORD dw_map_id);
	INT					get_instance_leave_role_num(DWORD dw_map_id);

	DWORD				get_update_time(DWORD	dw_map_id);

	VOID				onclock(INT nClock);

	//---------------------------------------------------------------------------------------------
	// 角色管理函数
	//---------------------------------------------------------------------------------------------
	VOID				role_logout(DWORD dw_role_id)			{ m_list_logout_role.push_back(dw_role_id); }
	VOID				role_return_character(DWORD dw_role_id)	{ m_list_return_character_role.push_back(dw_role_id); }
	VOID				role_logout_leave_role(DWORD	dw_role_id) { m_list_logout_leave_role.push_back(dw_role_id); }
	VOID				role_change_map(DWORD dw_role_id, DWORD dw_dest_map_id_, const Vector3& v_pos_, const Vector3& v_face_, DWORD dw_misc_,UINT16 lineId = 0);
	VOID				role_will_kick(DWORD dw_role_id, INT nTime) { m_map_kick_role.add(dw_role_id, nTime); }

	BOOL				take_over_role_when_online(Role* p_role_);

	//角色返回出生点
	BOOL				role_return_reborn_map(Role* p_role);
	
	//---------------------------------------------------------------------------------------------
	// 副本管理
	//---------------------------------------------------------------------------------------------
	DWORD				create_new_instance_id()	{ Interlocked_Exchange_Add((LPLONG)&m_dw_instance_id_gen, 1); return m_dw_instance_id_gen; }
	VOID				instance_destroyed(DWORD dw_map_id_, DWORD dw_instance_id_);

	VOID				set_instance_delete(DWORD dw_map_id_, DWORD dw_instance_id_);

	BOOL				can_create_instance(const tag_map_info* p_info_);
	VOID				add_instance(const tag_map_info* p_info_);
	VOID				remove_instance(const tag_map_info* p_info_);
	INT					get_instance_num(INT n_index_)		{ return m_n_instance_cur_num[n_index_]; }
	INT					get_instance_coef_num()			{ return m_n_instance_coef_cur_num; }
	INT					get_instance_coef_num_limit()		{ return m_n_instance_coef_num_limit; }

	package_map<DWORD, DWORD>&	get_guild_tripod_map() { return map_guild_tripod; }

	const std::vector<tstring>& GetMapNames( ) const{ return mMapNames; }
	const std::vector<tstring>& GetInstanceNames( ) const { return mInstanceNames; }

	map_threadmgr*		add_map_threadmgr(MapMgr* pMapMgr);
private:
	//---------------------------------------------------------------------------------------------
	// 初始化
	//---------------------------------------------------------------------------------------------
	BOOL				loag_map_info(LPCTSTR sz_file_map_name_, file_container* p_logic_);				// 载入地图的基本信息，载入成功时，会加入到m_map_map_info
	BOOL				load_map_way_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// 载入地图中的所有导航点
	//BOOL				load_map_trigger(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);			// 载入地图中的所有触发器
	BOOL				load_map_area(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);			// 载入地图中的所有区域
	//BOOL				load_map_creature(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// 载入地图中的所有场景生物
	
	//gx add 2013.4.17
	BOOL				load_map_trigger_extra(LPCTSTR sz_file_map_name_, tag_map_info* p_info_);			
	//end
	BOOL				load_map_spawn_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// 载入地图中的所有刷怪点
	BOOL				load_map_trigger_effect(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);	// 载入地图中的所有特效
	BOOL                load_map_door(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);
	BOOL				load_map_block(DWORD handle, tag_map_info* p_info_);

	BOOL				load_map_creature(XmlHandle& xmlHandle, tag_map_info* p_info_);//载入地图中所有的刷怪点
	BOOL				load_map_block(XmlHandle& xmlHandle, tag_map_info* p_info_);
	BOOL				load_map_trigger(XmlHandle& xmlHandle, tag_map_info* p_info_);			// 载入地图中的所有触发器
	BOOL				load_map_way_point(XmlHandle& xmlHandle, tag_map_info* p_info_);		// 载入地图中的所有导航点
	BOOL				load_map_area(XmlHandle& xmlHandle, tag_map_info* p_info_);//载入地图区域
	//----------------------------------------------------------------------------------------------
	// 工厂相关
	//----------------------------------------------------------------------------------------------
	VOID				register_factory_map();
	VOID				register_factory_map_restrict();
	VOID				un_register_all();

	//----------------------------------------------------------------------------------------------
	// 线程管理
	//----------------------------------------------------------------------------------------------
	MapMgr*				create_map_manager(tag_map_info* p_info_);

	//----------------------------------------------------------------------------------------------
	// 各种清理函数
	//----------------------------------------------------------------------------------------------
	VOID				logout_all_remove_role_pertick();
	VOID				deal_all_change_map_role();
	VOID				deal_all_return_character_role();
	VOID				deal_all_destroied_instance();
	VOID				logout_all_leave_role();
	VOID				logout_one_leave_role(DWORD	dw_role_id_);
	VOID				deal_all_kick_role();

public:
	//获得list队列
	void get_logout_role_list(std::list<DWORD>& list_out){ m_list_logout_role.copy_list(list_out);};
	package_safe_list<DWORD> get_logout_role_list(){return m_list_logout_role;};

	void get_change_map_role_list(std::list<tag_change_map_info>& list_out){ m_list_change_map_role.copy_list(list_out);};
	//package_safe_list<tag_change_map_info> get_change_map_role_list(){return m_list_change_map_role;};

	void get_return_character_role_list(std::list<DWORD>& list_out){m_list_return_character_role.copy_list(list_out);};
	//package_safe_list<DWORD> get_return_character_role_list(){return m_list_return_character_role;};

	void get_logout_leave_role_list(std::list<DWORD>& list_out){ m_list_logout_leave_role.copy_list(list_out);};
	//package_safe_list<DWORD> get_logout_leave_role_list(){return m_list_logout_leave_role;};

	//package_safe_map<DWORD, INT> get_map_kick_role_list(){return m_map_kick_role;};
	

private:
	
	std::vector<tstring>						mMapNames;
	std::vector<tstring>						mInstanceNames;

	class_template_factory<MapRestrict>			factory_restrict;					// 地图策略类工厂（地图类型——策略派生类）
	class_template_factory<Map>					factory_map;						// 地图类工厂（地图类型——地图派生类）

	MapMgrMap							m_map_map_manager;					// 所有的MapMgr列表（这个是否要加锁？）
	MapInfoMap							m_map_map_info;					// 所有地图的逻辑属性信息
	volatile DWORD						m_dw_instance_id_gen;				// 副本ID生成器

	INT									m_n_instance_cur_num[EMIG_END];	// 不同规格副本的当前创建数量
	INT									m_n_instance_coef_cur_num;			// 当前所有副本的加权总和
	INT									m_n_instance_coef_num_limit;		// 所有副本的加权总和上限

	DWORD								m_dw_born_map_id;					// 玩家出生地图的ID
	INT									m_n_born_position_num;					// 出生点个数
	Vector3*							m_pv_born_position;					// 出生点坐标数组
	FLOAT*								m_p_yaw;							// 出生点人物朝向

	DWORD								m_dw_first_map_id;					// 第一次玩家上线地图ID
	Vector3*							m_pv_first_position;				// 第一次玩家上线地图坐标

	Event								m_map_start_event[2];				// 事件——开启所有地图线程执行（两个事件，所有地图线程的Update线程交替监听这两个事件）
	INT									m_n_which_event;					// 当前使用的是哪个事件
	Event								m_map_end_event;					// 事件——所有地图Update完毕
	volatile INT						m_n_num_map_manager;					// MapMgr的数量，每个Tick重置为MapMgr的总数量，每个MapMgr运行完一次后减少该值
	volatile INT						m_n_num_thread_manager;				//管理线程

	package_safe_list<DWORD>					m_list_logout_role;				// 每个Tick登出的玩家
	package_safe_list<tag_change_map_info>			m_list_change_map_role;			// 每个Tick要切换地图的玩家
	package_safe_list<DWORD>					m_list_return_character_role;		// 每个Tick要回到选人界面的玩家
	package_safe_list<DWORD>					m_list_logout_leave_role;		// 每个tick登出的离线修炼玩家
	package_safe_map<DWORD, INT>				m_map_kick_role;				// 每个tic要踢出的玩家
	package_safe_list<tag_instance_destroy_info>	m_list_destroy_instance;			// 每个Tick要删除的副本

	// 牢狱信息
	DWORD								m_dw_prison_map_id;				// 牢狱地图id
	Vector3								m_putin_prison_point;				// 进牢狱区域point
	Vector3								m_putout_prison_point;			// 出牢狱区域point

	//衙门
	DWORD								m_dw_yamun_map_id;					//衙门地图ID
	Vector3								m_putin_yamun_point;				//进衙门区域point

	// 新手村
	UINT16								m_n_max_stable_instance_num;		//新手村副本数量
	//UINT16								m_n_max_stable_instance_rolenum;	//新手村副本最大角色数量
	//UINT16								m_n_max_main_instance_rolenum;		//主城最大角色数量
	UINT16								m_n_map_instance_del_time;			//副本删除倒计时
	UINT16								m_n_select_map_state;				//优先选择地图的百分比
	UINT16								m_n_good_map_state;					//空闲地图的百分比
	UINT16								m_n_busy_map_state;					//繁忙地图的百分比
	UINT16								m_n_merge_map_thread;				//几个地图共用一个线程

	//-------------------------------------------------------------------------------------------
	// 怪物场景帮会鼎列表
	//-------------------------------------------------------------------------------------------
	package_map<DWORD, DWORD>		map_guild_tripod;


	few_connect_client	trans_ip;

	std::vector<map_threadmgr*>			m_map_map_threadmgr;
};

//--------------------------------------------------------------------------------------------
// 开启所有地图线程
//--------------------------------------------------------------------------------------------
inline VOID map_creator::start_all_map_manager()
{
	m_n_which_event = ((m_n_which_event == 0) ? 1 : 0);
#ifdef USE_HEBING_MAP_THREAD
	Interlocked_Exchange((LPLONG)&m_n_num_thread_manager, m_map_map_threadmgr.size());
#else
	Interlocked_Exchange((LPLONG)&m_n_num_map_manager, m_map_map_manager.size());
#endif
	
	m_map_start_event[m_n_which_event].Set();
}

//--------------------------------------------------------------------------------------------
// 等待所有地图线程本tick完毕
//--------------------------------------------------------------------------------------------
inline VOID	map_creator::wait_all_map_manager_end()
{
	m_map_end_event.Wait();
	m_map_end_event.ReSet();
	m_map_start_event[m_n_which_event].ReSet();
}

//---------------------------------------------------------------------------------------------
// 随机得到一个出生坐标点
//---------------------------------------------------------------------------------------------
inline Vector3 map_creator::rand_get_one_born_position(INT& n_index_)
{
	INT n_rand = get_tool()->tool_rand() % m_n_born_position_num;
	n_index_ = n_rand;
	return m_pv_born_position[n_rand];
}

//---------------------------------------------------------------------------------------------
// 得到出生点朝向
//---------------------------------------------------------------------------------------------
inline FLOAT map_creator::get_born_yaw(INT n_index_)
{
	return m_p_yaw[n_index_];
}

//---------------------------------------------------------------------------------------------
// 得到启动地图线程运行的事件
//---------------------------------------------------------------------------------------------
inline Event& map_creator::get_all_map_start_event(INT n)
{
	return m_map_start_event[n];
}

//---------------------------------------------------------------------------------------------
// 某个地图线程本Tick执行完毕
//---------------------------------------------------------------------------------------------
inline VOID map_creator::one_map_manager_end()
{
	// 当所有地图线程执行完毕后，设置事件通知
#ifdef USE_HEBING_MAP_THREAD
	if( 0 == Interlocked_Decrement((LPLONG)&m_n_num_thread_manager ) ) 
#else
	if( 0 == Interlocked_Decrement((LPLONG)&m_n_num_map_manager ) ) 
#endif
	{
		m_map_end_event.Set();
	}
}

//----------------------------------------------------------------------------------------------
// 玩家切换地图
//----------------------------------------------------------------------------------------------
inline VOID map_creator::role_change_map(DWORD dw_role_id, DWORD dw_dest_map_id_, const Vector3& v_pos_, const Vector3& v_face_, DWORD dw_misc_,UINT16 lineId /*= 0*/ )
{
	MapMgr* p_map_manager = get_map_manager(dw_dest_map_id_);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(p_role) )
		return;

	if( p_role->IsInChangeIngMap() )
		return;

	//设置正在切换地图状态
	p_role->setInChangeIngMap();

	tag_change_map_info info;
	info.dw_role_id		=	dw_role_id;
	info.dw_dest_map_id	=	dw_dest_map_id_;
	info.v_position		=	v_pos_;
	info.v_face			=   v_face_;
	info.dw_misc		=	dw_misc_;
	info.lineId			=	lineId;
	m_list_change_map_role.push_back(info);
}

//------------------------------------------------------------------------------------------------
// 副本请求删除
//------------------------------------------------------------------------------------------------
inline VOID map_creator::instance_destroyed(DWORD dw_map_id_, DWORD dw_instance_id_)
{
	MapMgr* p_map_manager = get_map_manager(dw_map_id_);
	if( !VALID_POINT(p_map_manager) ) return;

	tag_instance_destroy_info info(dw_map_id_, dw_instance_id_);

	m_list_destroy_instance.push_back(info);
}

extern map_creator g_mapCreator;


//地图线程管理器		//合并地图线程 LIUBO 20150915
class map_threadmgr
{
public:
	map_threadmgr(const tstring& name);
	~map_threadmgr();
	UINT thread_update();
	void add_map_mgr(DWORD id, MapMgr* mapmgr);
	int get_map_mgr_num() { return m_map_manager.size();}
	static VOID			StopThread()					{ Interlocked_Exchange((LPLONG)&m_bTerminate, TRUE); }
	tstring&			GetThreadName()					{ return m_strThreadName; }
	static UINT WINAPI static_thread_update(LPVOID p_data);
private:
	tstring						m_strThreadName;					// 线程名称
	static volatile BOOL		m_bTerminate;						// 线程停止标志
	std::vector<MapMgr*>		m_map_manager;						// MapMgr列表
};