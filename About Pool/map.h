/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
*	@file		map.h
*	@author		lc
*	@date		2011/03/14	initial
*	@version	0.0.1.0
*	@brief		地图
*/


#ifndef MAP
#define MAP

#include "..\_common\_server_define\_server_world_define\time.h"
#include "..\_common\_server_define\_server_world_define\map_protocol.h"
#include "..\_common\_server_define\_server_world_define\combat_protocol.h"
#include "..\_common\_server_define\_server_world_define\ItemDefine.h"
#include "..\_common\_server_define\_server_world_define\group_define.h"
#include "..\_common\_server_define\_server_world_define\WeatherProtocol.h"
#include "group_mgr.h"
#include "script_mgr.h"

class PlayerSession;
class Unit;
class Role;
class Creature;
class Pet;
class Map;
class Shop;
class MapScript;
class NPCTeamMgr;
class guild_chamber;
class guild;
class c_sparse_graph;


struct tagItem;
struct tagEquip;
struct pathNode;
struct s_act_info;
enum   ECreatureSpawnedType;


//-------------------------------------------------------------------------------
//! 角色站立位置的最大，最小高度
//-------------------------------------------------------------------------------
const FLOAT GET_MAX_HEIGHT_Y = 10000000.0f;
const FLOAT GET_MIN_HEIGHT_Y = -10000000.0f;

const INT GROUND_ITEM_PICK_UP_TIME = 40;  //地上物保护时间40秒 
const INT GROUND_ITEM_DISAPPEAR_TIME = 90; //地上物销毁时间90秒

//--------------------------------------------------------------------------------
//! 副本级别
//--------------------------------------------------------------------------------
enum E_map_instance_grade
{
	EMIG_NOLIMIT	=	0,				
	EMIG_512		=	1,				
	EMIG_1024		=	2,				
	EMIG_2048		=	3,			
	EMIG_3072		=	4,				

	EMIG_END		=	5,
};

const INT INSTANCE_POWER[EMIG_END] = {0, 1, 3, 6, 9};	//! 副本权重

//--------------------------------------------------------------------------------
//！ 地图导航点数据
//--------------------------------------------------------------------------------
struct tag_way_point_info
{
	Vector3		v_pos;
	Vector3		v_range;
	DWORD		dw_time;			// 导航点停留时间 
	FLOAT		f_yaw;				// 朝向, 单位是度.

public:
	tag_way_point_info(DWORD)
	{
		v_pos.x = v_pos.y = v_pos.z = -1.0;dw_time = 0;f_yaw = 0;
	}
	tag_way_point_info()
	{
		v_pos.x = v_pos.y = v_pos.z = -1.0;dw_time = 0;f_yaw = 0;
	}
};

struct tag_map_way_point_info_list
{
	DWORD							dw_id;					// 导航点ID
	mutable package_list<tag_way_point_info>	list;		// 导航点列表

	tag_map_way_point_info_list() { dw_id = INVALID_VALUE; }
};


//--------------------------------------------------------------------------------
//! 地图触发器数据
//--------------------------------------------------------------------------------
struct tag_map_trigger_info
{
	DWORD 				dw_att_id;					//! 地图编辑器id
	EMapTrigger			e_type;						//！地图类型
	Vector3				boxPos;						//! 范围点
	Vector3				boxSize;					//！范围大小
	DWORD				dw_map_id;					//！触发器的目标地图ID
	DWORD				dw_way_point_id;			//！触发器的目标地图导航点ID
	DWORD				dw_param;					//！参数
	DWORD				dw_quest_id;				//！任务的编号

	tag_map_trigger_info()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMT_Null;
		dw_map_id = INVALID_VALUE;
		dw_way_point_id = INVALID_VALUE;
		dw_param = INVALID_VALUE;
		dw_quest_id = INVALID_VALUE;
	}
	~tag_map_trigger_info()
	{
	}
};
//--------------------------------------------------------------------------------
//! 地图触发器数据
// gx add 2013.4.17
//--------------------------------------------------------------------------------
struct tag_map_trigger_info_extra
{
	DWORD 				dw_att_id;					//! 地图编辑器id
	EMapTrigger			e_type;						//！地图类型
	FLOAT				f_height;					//！高度
	DWORD				dw_map_id;					//！触发器的目标地图ID
	DWORD				dw_way_point_id;			//！触发器的目标地图导航点ID
	DWORD				dw_param;					//！参数
	DWORD				dw_quest_id;				//！任务的编号

	tag_map_trigger_info_extra()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMT_Null;
		f_height = 0.0f;
		dw_map_id = INVALID_VALUE;
		dw_way_point_id = INVALID_VALUE;
		dw_param = INVALID_VALUE;
		dw_quest_id = INVALID_VALUE;
	}
	~tag_map_trigger_info_extra()
	{
	}
};
//end 
//--------------------------------------------------------------------------------
//! 地图区域数据
//--------------------------------------------------------------------------------
struct tag_map_area_info
{
	DWORD 				dw_att_id;			//! 地图编辑器id
	EMapArea			e_type;				//! 区域类型
	//HRGN				h_polygon;			//! 多边形句柄
	//AABBox				box;				//! 区域包围盒
	//FLOAT				f_height;			//! 高度
	Vector3				boxPos;						//! 范围点
	Vector3				boxSize;					//！范围大小

	tag_map_area_info()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMA_Null;
		//f_height = 0.0f;
	}
	~tag_map_area_info()
	{
		//DeleteObject(h_polygon);
	}
};

//--------------------------------------------------------------------------------
//! 地图怪物，NPC
//--------------------------------------------------------------------------------
struct tagMapCreatureInfo
{
	DWORD 					dw_att_id;			//! 地图编辑器id
	DWORD 					dw_type_id;			//! 类型id
	Vector3					v_pos;				//！坐标
	FLOAT 					f_yaw;				//! 朝向
	tag_map_way_point_info_list*	list_patrol;			//! 巡逻导航点列表
	BOOL					b_collide;			//! 是否碰撞

	tagMapCreatureInfo() { ZeroMemory(this, sizeof(*this)); }
};
//--------------------------------------------------------------------------------
//! 地图刷怪点信息
//- gx add 2013.4.16
//--------------------------------------------------------------------------------
struct tagMapMonsterInfo
{
	DWORD 					dw_type_id;			//! 刷怪点id
	Vector3					v_pos;				//！坐标
	FLOAT 					f_yaw;				//! 朝向
	tagMapMonsterInfo() { ZeroMemory(this, sizeof(*this)); }
};
//gx end

//--------------------------------------------------------------------------------
//! 地图刷怪点数据
//--------------------------------------------------------------------------------
struct tag_map_spawn_point_info
{
	DWORD				dw_att_id;				//! 地图编辑器id
	DWORD				dw_spawn_point_id;		//! 刷怪点id
	DWORD				dw_group_id;			//! 所属组id
	INT					n_level_increase;		//! 副本刷怪等级增量
	BOOL				b_collide;				//！是否碰撞
	Vector3				v_pos;					//! 坐标
	FLOAT				f_angle;				//! 方向
	//BOOL				b_hasUnit;				//! 是否刷了怪
	tag_map_way_point_info_list*	list_patrol;//! 巡逻导航点列表

	tag_map_spawn_point_info() { ZeroMemory(this, sizeof(*this)); }
};

// 刷怪点组列表
struct tag_map_spawn_point_list
{
	DWORD							dw_id;						//组id
	mutable package_map<DWORD,	tag_map_spawn_point_info*> list;//刷怪点
};


//--------------------------------------------------------------------------------
//! 地图门数据 
//--------------------------------------------------------------------------------
struct tag_map_door_info
{
	DWORD 					dw_att_id;			//! 地图编辑器id
	DWORD 					dw_type_id;			//! 门类型id
	Vector3					v_pos;				//! 坐标
	FLOAT 					f_yaw;				//! 朝向
	BOOL                    b_init_open_state;     //! 门初始状态：TRUE打开；FALSE关闭		

	tag_map_door_info() { ZeroMemory(this, sizeof(*this)); }
};


//--------------------------------------------------------------------------------
//! 地图静态数据
//--------------------------------------------------------------------------------
struct tag_map_info
{
	typedef package_map<DWORD, tag_map_way_point_info_list*>	MAP_WAY_POINT_LIST;		// 场景导航点
	typedef package_map<DWORD, tag_map_trigger_info*>			MAP_TRIGGER;			// 场景触发器
	typedef package_map<DWORD, tag_map_area_info*>				MAP_AREA;				// 场景区域
	typedef package_map<DWORD, tagMapCreatureInfo*>				MAP_CREATURE_INFO;		// 场景生物
	typedef package_map<DWORD, tag_map_spawn_point_info*>		MAP_SPAWN_POINT;		// 场景刷怪点
	typedef package_map<DWORD, tagMapTriggerEffect*>			MAP_TRIGGER_EFFECT;		// 场景特效
	typedef package_map<DWORD, tag_map_door_info*>					MAP_DOOR_INFO;		    // 场景门
	typedef package_map<DWORD, tag_map_spawn_point_list*>		MAP_SPAWN_POINT_LIST;	// 场景刷怪点组
	//gx add 2013.4.16
	typedef package_map<DWORD, tagMapMonsterInfo*>				MAP_MONSTER_INFO;		// 场景生物 用于记录刷怪点
	//end
	//gx add 2013.4.17
	typedef package_map<DWORD, tag_map_trigger_info_extra*>		MAP_TRIGGER_EXTRA;			// 场景触发器
	//end
	DWORD					dw_id;							// 地图ID
	TCHAR					sz_map_name[X_SHORT_NAME];	// 地图文件名称

	EMapType				e_type;							// 地图类型
	ENormalMapType			e_normal_map_type;				// 普通地图的类型
	E_map_instance_grade	e_instance_grade;					// 副本级别
	
	INT						n_width;							// 长度
	INT						n_height;						// 宽度
	INT						n_visit_distance;						// 可视距离
	DWORD					dw_weather_id;					// 对应的天气属性ID
	Vector3					v_reborn_positon;						// 复活点
	DWORD					dw_out_map_id;						// 从场景中传送出的地图ID
	Vector3					v_out_map_position;						// 从场景中传送出的地图坐标
	INT						n_act_id;							// 活动ID 
	BOOL					b_raid;		
	BOOL					b_AddCooldownTime;
	INT						n_weather_change_tick;
	INT						n_maxBroadCastNum;					//广播同步人数上限
	INT						n_maxPeopleNum;						//地图人数上限
	INT						n_mapLootPer;						//地图掉落
	INT16					n16_limitLv;						//地图等级限制
    INT32                   nIsMapRaid;                         // 为 1 时 ，检测是否有马牌 [2016-4-10 by willing]
	DWORD					dwActivityId;						//活动ID，该活动在进行中才能进入地图


	mutable std::vector<DWORD>		vec_weahter_ids;		// 场景天气
	mutable MAP_WAY_POINT_LIST		map_way_point_list;		// 场景导航点
	mutable MAP_TRIGGER				map_trigger;			// 场景触发器
	//gx add 2013.4.17
	mutable MAP_TRIGGER_EXTRA		map_trigger_extra;		//场景触发器
	//end
	mutable MAP_AREA				map_safe_area;			// 安全区
	mutable MAP_AREA				map_pvp_area;			// PVP区
	mutable MAP_AREA				map_stall_area;			// 摆摊区
	mutable MAP_AREA				map_prison_area;		// 牢狱区
	mutable MAP_AREA				map_script_area;		// 脚本区域
	mutable MAP_AREA				map_common_area;		// 普通区
	mutable MAP_AREA				map_real_safe_area;		// 绝对安全区
	mutable MAP_AREA				map_guild_battle;		// 帮会战区
	mutable MAP_AREA				map_no_punish_area;		// 无惩罚区
	mutable MAP_AREA				map_hang_area;			// 挂机区
	mutable MAP_AREA				map_KongFu_area;		// 练武区
	mutable MAP_AREA				map_Comprehend_area;	// 神悟区
	mutable MAP_AREA				map_Dancing_area;		// 跳舞区
	mutable MAP_CREATURE_INFO		map_creature_info;		// 场景怪物
	mutable MAP_MONSTER_INFO		map_monster_info;		// 场景怪物 gx add 2013.4.16
	mutable MAP_SPAWN_POINT			map_spawn_point;		// 创景刷怪点
	mutable MAP_TRIGGER_EFFECT		map_trigger_effect;		// 场景特效
	mutable MAP_DOOR_INFO		    map_door_info;			// 场景门
	mutable MAP_SPAWN_POINT_LIST	map_spawn_point_list;	// 场景刷怪点组
	mutable std::vector<int>		map_block_data;			// 碰撞信息
	mutable std::vector<int>		map_dynamic_block;		// [bing] 动态阻挡


	DWORD GetMemorySize()
	{
		DWORD dwMemory = 0;
		dwMemory += sizeof(dw_id);
		dwMemory += sizeof(sz_map_name);
		dwMemory += sizeof(e_type);
		dwMemory += sizeof(e_normal_map_type);
		dwMemory += sizeof(e_instance_grade);
		dwMemory += sizeof(n_width);
		dwMemory += sizeof(n_height);
		dwMemory += sizeof(n_visit_distance);
		dwMemory += sizeof(dw_weather_id);
		dwMemory += sizeof(v_reborn_positon);
		dwMemory += sizeof(dw_out_map_id);
		dwMemory += sizeof(v_out_map_position);
		dwMemory += sizeof(n_act_id);
		dwMemory += sizeof(b_raid);
		dwMemory += sizeof(b_AddCooldownTime);
		dwMemory += sizeof(n_weather_change_tick);
		dwMemory += sizeof(n_maxBroadCastNum);
		dwMemory += sizeof(n_maxPeopleNum);
		dwMemory += sizeof(n_mapLootPer);	

		dwMemory += (map_way_point_list.size() * sizeof(tag_map_way_point_info_list));
		dwMemory += (map_trigger.size() * sizeof(tag_map_trigger_info));
		dwMemory += (map_trigger_extra.size() * sizeof(tag_map_trigger_info_extra));//gx add 2013.4.17

		dwMemory += (map_safe_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_pvp_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_stall_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_prison_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_script_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_common_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_real_safe_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_guild_battle.size() * sizeof(tag_map_area_info));
		dwMemory += (map_no_punish_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_hang_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_creature_info.size() * sizeof(tagMapCreatureInfo));//gx modify
		dwMemory += (map_monster_info.size() * sizeof(tagMapMonsterInfo));//gx add 2013.4.16
		dwMemory += (map_spawn_point.size() * sizeof(tag_map_spawn_point_info));
		dwMemory += (map_trigger_effect.size() * sizeof(tagMapTriggerEffect));
		dwMemory += (map_door_info.size() * sizeof(tag_map_door_info));
		return dwMemory;
	}

};

//------------------------------------------------------------------------
//! 地图地物id生成器
//------------------------------------------------------------------------
class ground_item_id_builder
{
public:
	ground_item_id_builder():n64_last_valid(1){}
	INT64 builder_valid_id()
	{
		return n64_last_valid++;
	}
private:
	INT64	n64_last_valid;
};

enum E_ground_item_update
{
	EGIU_null				= -1,
	EGIU_remove				= 0,	// 移除
	EGIU_synchronize		= 1,	// 同步
};

//------------------------------------------------------------------------
//! 掉落物品骰子数据
//------------------------------------------------------------------------
struct tagSiceData
{
	BYTE byNeedSice;
	BYTE byNumber;
	DWORD dw_role_id[MAX_TEAM_NUM];
	BYTE byGreedSice[MAX_TEAM_NUM];
	BYTE byRequireSice[MAX_TEAM_NUM];
public:	
	tagSiceData( )
	{ 
		ZeroMemory(this,sizeof(*this));
	}

	VOID SetSiceTotal( BYTE byTotal ){ byNeedSice = byTotal;}
	VOID AddGreedSice( DWORD dwRole, BYTE byScore )
	{
		INT index = GetIndex( dwRole );
		if( index < 0 && byNumber >= MAX_TEAM_NUM) return; 
		if( index < 0 ) index = byNumber++;
		dw_role_id[index] = dwRole;
		byGreedSice[index] =  byScore;
	}

	VOID AddRequireSice( DWORD dwRole, BYTE byScore )
	{
		INT index = GetIndex( dwRole );
		if( index < 0 && byNumber >= MAX_TEAM_NUM) return; 
		if( index < 0 ) index = byNumber++;
		dw_role_id[index] = dwRole;
		byRequireSice[index] =  byScore;
	}


	INT GetIndex( DWORD dwRole )
	{
		INT index = -1;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] == dwRole )
			{
				index = n; break;
			}
		}
		return index;
	}

	DWORD GetGreedMax( ) 
	{
		INT index = -1; BYTE byScore = 0;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] && byGreedSice[n] > byScore ) 
			{
				byScore = byGreedSice[n];
				index = n;
			}
		}

		return index >= 0 ? dw_role_id[index] : INVALID_VALUE;	
	}

	DWORD GetRequireMax( BYTE& byMaxRequireSice) 
	{
		INT index = -1; BYTE byScore = 0;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] && byRequireSice[n] > byScore )
			{
				byScore = byRequireSice[n];
				index = n;
			}
		}
		byMaxRequireSice = byScore;
		return index >= 0 ? dw_role_id[index] : INVALID_VALUE;	
	}

	BYTE GetScore( DWORD dwRole, BYTE& byType) const
	{
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] == dwRole )
			{
				if( byRequireSice[n] )
				{
					byType = EPSM_Require;
					return  byRequireSice[n];
				}
				if( byGreedSice[n] )
				{
					byType = EPSM_Greed;
					return  byGreedSice[n];
				}
			}
		}
		byType = EPSM_Null;
		return 0;
	}


	BOOL IsSiceFinishi( ) const {return byNumber == byNeedSice;}

	VOID SetForceSiceFinishi( ){ byNeedSice = byNumber; }
};

struct tagEnmity;
//------------------------------------------------------------------------
// 掉到地面上的物品结构
//------------------------------------------------------------------------
struct tag_ground_item
{
	INT64					n64_id;				// 地物ID
	DWORD					dw_type_id;			// 类型ID
	INT						n_num;				// 数量或金钱

	tagItem*				p_item;				// 物品指针
	DWORD					dw_put_down_unit_id;	// 掉落物品的UnitID
	DWORD					dw_owner_id;			// 归属角色
	DWORD					dw_team_id;			// 归属队伍
	DWORD					dw_group_id;			// 归属团队ID
	Vector3					v_pos;				// 物品坐标

	EPickMode				e_pick_mode;
	EGTAssignState			e_assign_state;
	INT						n_assign_tick_down;
	tagSiceData				st_sice_data;
	DWORD					dw_boss_id;
	bool					b_exclusive;		// 是否专属
	tagDWORDTime			dwDropTime;		  //创建时间
	tagDWORDTime			dwDieTime;		 //消亡时间
	tagDWORDTime			dwPickUpTime;	 //保护时间
	tagDWORDTime			dwShowTime;		//展示时间
	std::set<DWORD>			mRoleList;		//可拾取玩家列表
	std::set<DWORD>			mTeamList;		//可拾取队伍列表

	tag_ground_item(	INT64 n64_id_, DWORD dw_type_id_, INT n_num_, tagItem* p_item_, 
					Vector3 v_pos_, DWORD dw_owner_id_, DWORD dw_team_id_, DWORD dw_group_id_, DWORD dw_put_down_unit_id_,tagDWORDTime dwDropTime_,
					EPickMode e_pick_mode_ = EPUM_Free, EGTAssignState e_assign_state_ = EGTAS_Null, INT n_tick_down_ = 0, DWORD dw_boss_id_ = 0, bool bExclusive = false)
	{
		n64_id			=	n64_id_;
		dw_type_id		=	dw_type_id_;
		n_num			=	n_num_;

		p_item			=	p_item_;
		v_pos			=	v_pos_;
		dw_owner_id		=	dw_owner_id_;
		dw_team_id		=	dw_team_id_;
		dw_group_id		=	dw_group_id_;
		dw_put_down_unit_id	=	dw_put_down_unit_id_;

		e_pick_mode = e_pick_mode_;
		e_assign_state = e_assign_state_;
		n_assign_tick_down = n_tick_down_;

		dw_boss_id = dw_boss_id_;
		dwDropTime 	 = dwDropTime_;
		dwDieTime    = IncreaseTime(dwDropTime,GROUND_ITEM_DISAPPEAR_TIME);
		dwPickUpTime = IncreaseTime(dwDropTime,GROUND_ITEM_PICK_UP_TIME);
		dwShowTime	 = dwDropTime;
		b_exclusive		= bExclusive;
	}

	void setPickUpTime(int _time)
	{
		dwPickUpTime = IncreaseTime(dwDropTime,_time);
	}

	void setShowTime(int _time)
	{
		dwShowTime = IncreaseTime(dwDropTime,_time);
	}

	// 地物更新，返回TRUE时删除地物
	BOOL update()
	{
		E_ground_item_update e_update = EGIU_null;

		if( n_assign_tick_down > 0 && e_assign_state != EGTAS_WaitForUmpirage )
		{//需要分配
			--n_assign_tick_down;
			if( !n_assign_tick_down && e_assign_state != EGTAS_AssignFinish && e_assign_state != EGTAS_Null)
			{//到点了还没分配完
				if( e_pick_mode == EPUM_Leader )
				{ //队长没有分配,变成自由拾取
					e_pick_mode = EPUM_Free; 
					e_assign_state = EGTAS_Null;
					dw_team_id	= INVALID_VALUE;
					dw_owner_id	= INVALID_VALUE;
				}

				else if( e_pick_mode == EPUM_Sice )
				{ 
					e_assign_state =  EGTAS_AssignFinish; 
					st_sice_data.SetForceSiceFinishi( ); 

					tagAssignFinish send;
					send.dwTeamID = dw_team_id;
					send.n64GroundID = n64_id;
					g_groupMgr.AddEvent(0, EVT_TeamSiceFinish, sizeof(tagAssignFinish), &send);
				}
			}
		}

		//40秒的保护时间,专属道具一直受保护
		if(EGIU_synchronize != e_update && 0 == CalcTimeDiff(dwPickUpTime,GetCurrentDWORDTime()) && !b_exclusive)
		{
			dw_team_id	= INVALID_VALUE;
			dw_owner_id	= INVALID_VALUE;
			e_update = EGIU_synchronize;
			mRoleList.clear();
			mTeamList.clear();
		}

		// 90秒消失
		if (EGIU_remove != e_update && 0 == CalcTimeDiff(dwDieTime,GetCurrentDWORDTime()))
		{
			e_update = EGIU_remove;
		}

		return e_update;
	}

	// 清理
	VOID destroy_item()
	{
		if (VALID_POINT(p_item))
		{
			::Destroy(p_item);
		}
	}

	// 是否有效
	BOOL is_valid()
	{
		if (dw_type_id == TYPE_ID_MONEY && !VALID_POINT(p_item))
		{
			return TRUE;
		}
		else if (dw_type_id != TYPE_ID_MONEY && VALID_POINT(p_item))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	BOOL is_notice( ) const
	{
		if(e_pick_mode == EPUM_Free /*||  e_pick_mode == EPUM_Order */)
			return FALSE;
		
		return e_assign_state == EGTAS_WaitForUmpirage;
	}

	BOOL is_wait_for_assign( ) const 
	{
		if( e_assign_state == EGTAS_WaitForSice ) return TRUE;
		if( e_assign_state == EGTAS_WaitForLeaderAssign ) return TRUE;
		return FALSE;
	}

	VOID finist_umpirage( )
	{
		if( e_pick_mode == EPUM_Leader )
			e_assign_state = EGTAS_WaitForLeaderAssign;

		if( e_pick_mode == EPUM_Sice )
			e_assign_state = EGTAS_WaitForSice;
	}

	BYTE get_score( DWORD dw_role_id_, BYTE& by_type_ ) const
	{
		return get_sice_data( ).GetScore( dw_role_id_, by_type_ );
	}

	DWORD get_boss_id() const {return dw_boss_id;}
	const tagSiceData& get_sice_data( ) const { return st_sice_data;}
	tagSiceData& get_sice_data( ) { return st_sice_data;}
	//VOID SetDelNextTick( ){ nTickCountDown = GROUND_ITEM_DISAPPEAR_TIME; }
	void addPickMap(std::vector<tagEnmity*> enmityList, int pickType);
	bool isRoleDrop()
	{
		if (IS_PLAYER(dw_put_down_unit_id) )
		{
			return true;
		}
		return false;
	}
};

//----------------------------------------------------------------------------------
// 可视地砖
//----------------------------------------------------------------------------------

// 定义中间加上八方向
enum E_unit_direction
{
	EUD_center = 0,
	EUD_top = 1,
	EUD_bottom = 2,
	EUD_left = 3,
	EUD_right = 4,
	EUD_left_top = 5,
	EUD_left_bottom = 6,
	EUD_right_top = 7,
	EUD_right_bottom = 8,
	EUD_end = 9,
};

struct tagVisTile
{
	package_map<DWORD, Role*>				map_role;
	package_map<DWORD, Role*>				map_leave_role;
	package_map<DWORD, Creature*>			map_creature;
	package_map<INT64, tag_ground_item*>	map_ground_item;
	package_map<DWORD, Unit*>				map_invisible_unit;
	INT										n_max_num;

	tagVisTile()
	{
		n_max_num = 300;
	}
};

//---------------------------------------------------------------------------------
// 生物ID生成器
//---------------------------------------------------------------------------------
class creature_id_builder
{
public:
	VOID init(const tag_map_info* p_info_)
	{
		// 考虑到巢穴刷怪以及实时刷出的怪物，在这个基础之上再增加100个ID
		//INT nCreatureNum = p_info_->map_creature_info.size() + p_info_->map_spawn_point.size() + 100;
		INT nCreatureNum = p_info_->map_monster_info.size() + p_info_->map_spawn_point.size() + 100;
		DWORD dw_min_id = (DWORD)MIN_CREATURE_ID;
		for(INT n = 0; n < nCreatureNum; n++)
		{
			list_free_creature_id.push_back(dw_min_id+(DWORD)n);
		}
		dw_max_creature_id = dw_min_id + nCreatureNum;
	}

	DWORD get_new_creature_id()
	{
		DWORD dw_creature_id = list_free_creature_id.pop_front();

		if( !VALID_POINT(dw_creature_id) )
		{
			dw_creature_id = dw_max_creature_id;
			// list中已经没有多余的id了，则再生成100个
			for(INT n = 1; n < 100; ++n)
			{
				++dw_max_creature_id;
				//DWORD dwNewID = dw_max_creature_id+(DWORD)n;
				if (dw_max_creature_id > MAX_CREATURE_ID)
				{
					dw_max_creature_id = MID_CREATURE_ID;
				}
				list_free_creature_id.push_back(dw_max_creature_id);
			}
			
			++dw_max_creature_id;
		}

		return dw_creature_id;
	}

	DWORD return_creature_id(DWORD dw_creature_id_)
	{
		ASSERT( IS_CREATURE(dw_creature_id_) );
		list_free_creature_id.push_front(dw_creature_id_);
	}

private:
	package_list<DWORD>		list_free_creature_id;
	DWORD					dw_max_creature_id;
};

//---------------------------------------------------------------------------------------
// 地图类
//---------------------------------------------------------------------------------------
class Map : public ScriptData<ESD_Map>
{
public:
	typedef	package_map<DWORD, PlayerSession*>		SESSION_MAP;
	typedef package_map<DWORD, Role*>				ROLE_MAP;
	typedef package_map<DWORD, Creature*>			CREATURE_MAP;
	typedef package_map<INT64, tag_ground_item*>	GROUND_ITEM_MAP;
	typedef package_map<DWORD, Shop*>				SHOP_MAP;
	typedef package_map<DWORD, guild_chamber*>		CHAMBER_MAP;
	//typedef std::list<DWORD>						DEAD_BOSS_LIST;

public:
	Map();
	virtual ~Map();

	//-------------------------------------------------------------------------------------
	// 创建，更新和销毁
	//-------------------------------------------------------------------------------------
	BOOL				init(const tag_map_info* pInfo);

	void	TestCallFunc(DWORD dwTick, void* pData)
	{
		if(1)
		{
			int a = 1;
		}
	}


	virtual VOID		update();
	virtual VOID		destroy();
	virtual std::set<DWORD>	*get_role_mapSBKtower(){return NULL;}
	virtual VOID		del_role_mapSBKtower(DWORD roleId){}
	virtual VOID		add_role_mapSBKtower(DWORD roleId){}
	virtual BOOL		can_enter_this_map(Role *pRole){return TRUE;}
	virtual DWORD		getAllRoleSameGuildID(){return -1;}
	VOID				init_creature_script();

	VOID				update_weather();
	
	//bool				initSparseGraph(const tag_map_info* pInfo);
	//-------------------------------------------------------------------------------------
	// 各种Get
	//-------------------------------------------------------------------------------------
	DWORD				get_map_id()			{ return dw_id; }
	DWORD				get_instance_id()		{ return dw_instance_id; }
	UINT16				getCurLine()			{ return m_sCurLine;}
	void				setCurLine(UINT16 index) { m_sCurLine = index;}
	EMapType			get_map_type()		{ return p_map_info->e_type; }
	ENormalMapType		get_normal_map_type()	{ return p_map_info->e_normal_map_type; }
	const tag_map_info*	get_map_info()		{ return p_map_info; }
	INT					get_role_num()		{ return map_role.size(); }
	INT					get_max_role_num_limit() {return p_map_info->n_maxPeopleNum;}
	INT					get_leave_role_num() { return map_leave_role.size(); }
	NPCTeamMgr*			get_team_manager()		{ return p_npc_team_mgr; }
	ROLE_MAP&			get_role_map()		{ return map_role; }
	ROLE_MAP&			get_leave_role_map() { return map_leave_role; }
	DWORD				get_update_time()	{ return dw_update_time; }
	//c_sparse_graph*		get_path_finder()	{ return m_pSparseGraph; }

	//------------------------------------------------------------------------------------------
	// 双倍
	//------------------------------------------------------------------------------------------
	FLOAT				get_exp_rate() ;
	FLOAT				get_drop_rate();
	FLOAT				get_practice_rate();

	//--------------------------------------------------------------------------------------
	// 可视地砖相关
	//--------------------------------------------------------------------------------------
	VOID				get_visible_tile(INT n_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile(const Vector3& v_pos_, FLOAT f_range_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_add(INT n_ole_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_add_and_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_add_[EUD_end], tagVisTile* p_visible_tile_dec_[EUD_end]);

	//--------------------------------------------------------------------------------------
	// 玩家，生物和掉落物品的添加和删除
	//--------------------------------------------------------------------------------------
	virtual VOID		add_role(Role* p_role_, Map* pSrcMap = NULL);
	virtual INT			can_enter(Role *p_role_)			{ return TRUE; }
	virtual BOOL		changePkValue()	{ return TRUE; }
	virtual DWORD		get_param() { return INVALID_VALUE; }
	VOID				role_logout(DWORD dw_id_);
	VOID				role_leave_logout(DWORD	dw_id_);
	VOID				role_leave_map_practice(DWORD dw_id_);
	virtual	VOID		role_leave_map(DWORD dw_id_, BOOL b_leave_online = FALSE);
	VOID				add_creature_on_load(Creature* p_creature_);
	VOID				add_creature(Creature* p_creature_);
	VOID				remove_creature(Creature* p_creature_);
	VOID				remove_npc( Creature* p_creature_ );
	VOID				remove_pet(Pet* p_pet_, BOOL bSendMsg = TRUE);
	VOID				remove_flow_creature(Creature* p_creature);
	VOID				add_ground_item(tag_ground_item *p_ground_item_);
	VOID				remove_ground_item(tag_ground_item* p_ground_item_);
	virtual VOID 		spawn_creature_replace(Creature* p_dead_creature_);
	VOID				spawn_list_creature_replace(Creature* p_dead_creature_);
	Creature*			create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, DWORD dw_spawner_id_=INVALID_VALUE, BOOL b_collide_=FALSE, LPCTSTR p_patrol_list_name_=NULL,Role* pRole = NULL,BOOL isInScriptCreate = FALSE, BOOL AddMap = TRUE);
	Creature*			create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, ECreatureSpawnedType e_spawned_type_, 
										DWORD dw_guild_id_ = INVALID_VALUE, DWORD dw_spawner_id_=INVALID_VALUE, BOOL b_collide_=FALSE, 
										TCHAR* p_patrol_list_name_=NULL);
	VOID				delete_creature(DWORD dw_id_);
	VOID				send_npc_loading(Role* p_role_);
	VOID				add_pet_delete_map(Creature* pPet);
	//---------------------------------------------------------------------------------------
	// 游戏内玩家和生物的查询
	//---------------------------------------------------------------------------------------
	Role*				find_role(DWORD dw_id_)			{ return map_role.find(dw_id_); }		// 一般在地图线程之内使用
	Role*				find_leave_role(DWORD dw_id_)	{ return map_leave_role.find(dw_id_); }
	Creature*			find_creature(DWORD dw_id_)		{ return map_creature.find(dw_id_); }	// 一般在地图线程之内使用
	Creature*			find_creature_for_typeId(DWORD dw_id_);	// 一般在地图线程之内使用
	Pet*				find_pet(DWORD dw_id_);				// 一般在地图线程之内使用;	
	Unit*				find_unit(DWORD dw_id_);
	tag_ground_item*	get_ground_item(INT64 n64_serial)	{ return map_ground_item.find(n64_serial); }
	GROUND_ITEM_MAP		get_map_ground_item()	{ return map_ground_item; }
	Shop*				get_shop(DWORD dw_npc_id_)			{ return map_shop.find(dw_npc_id_); }
	guild_chamber*		get_chamber(DWORD dw_npc_id_)			{ return map_chamber.find(dw_npc_id_); }
	Creature*			find_npc(DWORD dw_id_)			{ return map_allnpc.find(dw_id_); }
	bool				find_is_npc(DWORD dw_id_);

	//--------------------------------------------------------------------------------------
	// 基于地图九宫格的各种同步方法
	//--------------------------------------------------------------------------------------
	VOID				synchronize_movement_to_big_visible_tile(Unit* p_self_);
	VOID				synchronize_remove_unit_to_big_visible_tile(Unit* p_self_);
	VOID				synchronize_change_visible_tile(Unit* p_self_, INT n_old_visible_index_, INT n_new_visible_index_);
	VOID				synchronize_role_leave_map(Role* p_self_, BOOL b_leave_online = FALSE);
	VOID				send_big_visible_tile_message(Unit* p_self_, const LPVOID p_message_, const DWORD dw_size_);
	
	VOID				send_big_visible_tile_message(tagVisTile *p_visible_tile_[EUD_end], const LPVOID p_message_, const DWORD dw_size_, Unit* p_self_ = NULL);
	VOID				send_big_visible_tile_messageMysteryShop(tagVisTile *p_visible_tile_[EUD_end], Unit* p_self_ = NULL);
	
	VOID				send_friend_tile_message(Unit* p_self_, const LPVOID p_message_, const DWORD dw_size_);
	VOID				send_map_message(const LPVOID p_message_, const DWORD dw_size_);
	VOID				synchronize_remove_units(Unit* p_self_, tagVisTile* p_visible_tile_dec_[EUD_end]);
	VOID				synchronize_add_units(Unit* p_self_, tagVisTile* p_visible_tile_add_[EUD_end]);
	VOID				synchronize_add_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);
	VOID				synchronize_ground_item_state(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);
	VOID				synchronize_remove_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);

	//向玩家发送此角色ID的具体信息
	VOID				syn_roleid_info_to_role(Role* pRole, unsigned long roleid);
	//向地砖内玩家发送此角色ID的具体信息
	VOID				send_big_visible_tile_roleinfo(tagVisTile *p_visible_tile_[EUD_end], unsigned long roleid, const LPVOID p_message_, const DWORD dw_size_);

	//--------------------------------------------------------------------------------------
	// 地图信息的同步
	//--------------------------------------------------------------------------------------
	VOID				send_scene_effect(Role* p_self_);
	VOID				play_scene_effect(ESceneEffectType e_type_, DWORD dw_obj_id_, Vector3 v_pos_, DWORD dw_effect_id_);
	VOID				stop_scene_effect(DWORD dw_obj_id_);

	//----------------------------------------------------------------------------------------
	// 地图区域相关
	//----------------------------------------------------------------------------------------
	INT64				check_area(Role* p_role_);							// 检测人物所在的各种区域（返回区域标志位）

	bool				check_area_is_in_NoPunish(Role* p_role_);			// 检测人物是否在无惩罚区（专为沙巴克皇宫区域判断）

	bool				check_canride(Role* p_role_);										//检查是否可以骑马

	//----------------------------------------------------------------------------------------
	// 坐标检测及修正
	//----------------------------------------------------------------------------------------
	BOOL				is_position_valid(const Vector3& v_vector_);
	VOID				fix_position(Vector3& v_vector_);

	//----------------------------------------------------------------------------------------
	// 地图场景触发器相关
	//----------------------------------------------------------------------------------------
	BOOL				is_in_trigger(Role* p_role_, DWORD dw_map_trigger_id_);	// 是否在触发器内
	DWORD				get_trigger_quest_serial(DWORD dw_map_trigger_id_);	// 得到某个trigger对应的任务序列号
	
	//----------------------------------------------------------------------------------------
	// 掉落相关的一些函数
	//----------------------------------------------------------------------------------------
	BOOL				can_putdown(Unit* p_unit, INT n_index_, Vector3& v_position_);
	VOID				putdown_item(Creature* p_creature_, tagItem* p_loot_item_, DWORD dw_owner_id_, DWORD dw_team_id_, 
									 Vector3& v_position_, INT dropProtect, INT showTime, std::vector<tagEnmity*> enmityList, 
									 EPickMode e_pick_mode_ = EPUM_Free, EGTAssignState e_assign_sate_ = EGTAS_Null, INT n_tick_down_ = 0, bool bExclusive = false);
	VOID				putdown_money(Creature* p_creature_, INT n_money_, DWORD dwTeamID, DWORD dwOwnerID, Vector3 v_positon_, 
									INT dropProtect, INT showTime, std::vector<tagEnmity*> enmityList);
	BOOL				get_can_go_position_from_index(INT32 n_index_, INT32 n_index_x_, INT32 n_index_z_, INT32 n_, Vector3& v_position_);
	INT64				get_ground_item_id()		{ return builder_ground_item_id.builder_valid_id(); }

	//----------------------------------------------------------------------------------------
	// 地图战斗系统限制相关，主要应用于副本
	//----------------------------------------------------------------------------------------
	BOOL				can_use_item(DWORD dw_data_id, INT64 n64_Item_Serial);
	BOOL				can_use_skill(DWORD dw_item_id_);
	VOID				onclock(INT nClock);

	//----------------------------------------------------------------------------------------
	// 敌我关系判断
	//----------------------------------------------------------------------------------------
	DWORD				get_friend_enemy_flag(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_);

	//------------------------------------------------------------------------------------------
	// 地图区域相关
	//------------------------------------------------------------------------------------------
	tag_map_area_info*		is_in_area(tag_map_info::MAP_AREA& map_area_, float x, float z);

	//----------------------------------------------------------------------------------------
	// 一些辅助函数
	//----------------------------------------------------------------------------------------
	VOID				send_goto_new_map_to_player(Role* p_self_);
	BOOL				in_same_big_visible_tile(Unit* p_self_, Unit* p_remote_);
	INT					world_position_to_visible_index(const Vector3& v_pos_);
	INT					world_pos_to_tile_index(const Vector3& v_pos_);
	BOOL				if_can_go(FLOAT f_x_, FLOAT f_z_);
	BOOL				if_can_dynamic_go(FLOAT f_x_, FLOAT f_z_);
	BOOL				if_can_dynamic_go(int nVisIndex);
	BOOL				if_can_direct_go(FLOAT f_srcx_, FLOAT f_src_y_, FLOAT f_dest_x_, FLOAT f_dest_z_, pathNode& n_near_pos_);

	template<typename Operation>
	VOID				foreach_unit_in_big_visible_tile(Unit* p_unit_, Operation oper_);

	template<typename RoleOperation>
	VOID				for_each_role_in_map( RoleOperation oper_);

	template<typename Operation, typename Predicate>
	VOID				foreach_unit_in_map(Operation oper_, Predicate pred_);
	
	BOOL				is_point_has_uint(DWORD id);

	BOOL				RandSpwanPoint(tag_map_spawn_point_list* spawnList, tag_map_spawn_point_info*& pMapSpawnInfo);
	
	//BOOL				is_have_unit(const Vector3& point);
	//VOID				set_have_unit(const Vector3& point, bool bset);
	//-----------------------------------------------------------------------------------------
	// 地图事件
	//-----------------------------------------------------------------------------------------
	virtual	VOID		on_role_die(Role* p_role_, Unit* p_killer_);
    virtual VOID        on_role_att_change(Role* p_role_, const bool bRecalFlag[ERA_End]) {};
	virtual VOID		on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
								  INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_);
	virtual VOID		on_creature_die(Creature* p_creature_, Unit* p_killer_);
	VOID				on_creature_disappear(Creature* p_creature_);
	VOID				on_enter_trigger(Role* p_role_, tag_map_trigger_info* p_info_);
	VOID				on_enter_area(Role* p_role_, tag_map_area_info* p_info_);
	INT		            can_invite_join_team();
    INT                 can_apply_join_team();
    INT                 can_be_invited_join_team();
	INT 				can_leave_team();
	INT					can_change_leader();
	INT					can_kick_member();
	BOOL				can_set_safe_guard();
	//使用技能回调
	virtual	VOID		on_use_skill(Unit* p_role_, Skill* pSkill) {};

	//------------------------------------------------------------------------------------------
	// 潜行
	//------------------------------------------------------------------------------------------
	VOID				lurk(Unit *p_unit_);
	VOID				unlurk(Unit *p_unit_);
	VOID				update_lurk_to_big_visible_tile_role(Unit *p_unit_);
	VOID				update_big_visible_tile_lurk_unit_to_role(Role* p_role_);

	Role*				stop_leave_prictice(DWORD	dw_id_);
	VOID				remove_leave_pricitce_visible_tile(Unit* p_unit_);


	void SetWeather(DWORD id, BOOL bSnd = TRUE)
	{
		if(id != dw_weather_id_)
		{
			dw_weather_id_ = id;

			if(bSnd){
				NET_SIS_change_Weather send;
				send.dwMapID = get_map_id();
				send.dwInstanceID = get_instance_id( );
				send.dwWeather = id;
				send_map_message(&send, send.dw_size);
			}
		}
	}
	DWORD GetWeather( ) const{return dw_weather_id_; }

	BOOL IsHaveUnit(const Vector3 piont);

	DWORD getNewCreatureID() { return builder_creature_id.get_new_creature_id(); }

	// 获取地图内角色帮会id,有不同的返回inval
	//DWORD getAllRoleSameGuildID();

	VOID				on_init_finish();						// [bing] 初始化完成时

	CTimerEvent&		GetTimerEvent() { return m_TimerEvent; }

	//--------------------------------------------------------------------------------
	// 技能特殊效果
	//--------------------------------------------------------------------------------
	VOID				CollideCallFunc( DWORD dwTick, void* pData ); //冲撞技能的回调函数  by: XSea

	//[bing] 挑战类副本扫荡回调
	VOID				FubenSaoDangCallFunc( DWORD dwTick, void* pData );

	//[bing] 处理动态阻挡
	void				SetDynamicBlock(int nTileIndex, BOOL bSet);

	VOID				send_big_team_tile_message(Unit* p_self_, const LPVOID p_message_, const DWORD dw_size_);

	VOID				repet_login_enter_map(Role* pRole);
	VOID				addRoleHuoYue(const s_act_info* actInfo);
protected:
	//-----------------------------------------------------------------------------------------
	// 初始化
	//-----------------------------------------------------------------------------------------
	BOOL				init_logical_info(DWORD dw_guild_id_ = INVALID_VALUE);	// 根据地图属性信息初始化所有逻辑信息
	BOOL				init_all_map_creature(DWORD dw_guild_id_ = INVALID_VALUE);					// 生成地图里的初始怪物
	virtual BOOL		init_all_fixed_creature(DWORD dw_guild_id_ = INVALID_VALUE);					// 生成地图里的所有摆放怪物
	virtual BOOL		init_all_fixed_creature_ex(DWORD dw_guild_id_ = INVALID_VALUE);//gx add 2013.4.16
	virtual BOOL		init_all_spawn_point_creature(DWORD dw_guild_id_ = INVALID_VALUE);			// 生成所有刷新点怪物
	virtual BOOL		init_all_spawn_list_creature(DWORD dw_guild_id_ = INVALID_VALUE);	// 生成所有刷怪点组怪物
	VOID				init_nest_creature(Creature* p_creature_);	// 初始化巢穴怪物
	VOID				init_team_creature(Creature* p_creature_);  // 初始化小队怪物
	VOID				init_team_creature(Creature* p_creature_, guild* p_guild_);	//初始化小队怪物
	BOOL				add_shop(DWORD dw_npc_id_, DWORD dw_shop_id_);	// 初始化商店
	BOOL				add_chamber(DWORD dw_npc_id_, DWORD dw_chamber_id_);	// 初始化商会

	//------------------------------------------------------------------------------------------
	// 地图事件
	//------------------------------------------------------------------------------------------
	VOID				on_init();								// 初始化时

	//------------------------------------------------------------------------------------------
	// 更新
	//------------------------------------------------------------------------------------------
	VOID				update_session();			// 处理该地图所管理的玩家的所有消息
	VOID				update_all_objects();			// 更新该地图内所有对象
	VOID				update_all_shops();			// 更新该地图内所有对象
	VOID				update_all_chamber();			// 更新该地图内所有对象
	VOID				update_num();

	//------------------------------------------------------------------------------------------
	// 九宫格相关
	//------------------------------------------------------------------------------------------
	VOID				add_to_visible_tile(Unit* p_unit_, INT n_visible_index_);
	VOID				remove_from_visible_tile(Unit* p_unit_, BOOL b_leave_online = FALSE);
	VOID				add_ground_item_to_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_);
	VOID				remove_ground_item_from_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_);

	//------------------------------------------------------------------------------------------
	// 数据同步
	//------------------------------------------------------------------------------------------
	VOID				syn_big_visible_tile_invisible_unit_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end]);
	VOID				syn_big_visible_tile_visible_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end]);

	VOID				syn_invisible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
													const LPVOID p_message, const DWORD dw_size);
	VOID				syn_visible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
													const LPVOID p_message, const DWORD dw_size);

	VOID				syn_remove_big_visible_tile_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTileDec[EUD_end]);

	//------------------------------------------------------------------------------------------
	// 辅助函数
	//------------------------------------------------------------------------------------------
	DWORD				calculate_movement_message(Unit* pSelf, LPBYTE p_message, DWORD dw_size);

	bool				FilterBroadCast(Role* pRemoteRole, Role* pRole, bool isCreate = false);			//广播同步裁剪 LIUBO 20150914
protected:
	

	DWORD					dw_id;					// 地图ID
	DWORD					dw_instance_id;			// 副本ID（普通地图此ID为INVALID_VALUE）
	UINT16					m_sCurLine;				// 当前线路(非分线地图为INVALID_VALUE)

	//-------------------------------------------------------------------------------------------
	// 对象容器
	//-------------------------------------------------------------------------------------------
	SESSION_MAP					map_session;				// 地图里面的session列表
	ROLE_MAP					map_role;					// 地图里面的所有玩家
	ROLE_MAP					map_leave_role;				// 地图里的所有挂机玩家
	CREATURE_MAP				map_creature;				// 地图里面的所有生物
	CREATURE_MAP				map_respawn_creature;		// 死亡后等待刷新的生物
	CREATURE_MAP				map_door;					// 地图上的门对象列表
	CREATURE_MAP				map_npc;					// 从地图上移出的被护送的NPC
	GROUND_ITEM_MAP				map_ground_item;			// 地面掉落物品
	SHOP_MAP					map_shop;					// 地图里面的所有商店<dwNpcID, pShop>
	CHAMBER_MAP					map_chamber;				// 帮派跑商商会
	CREATURE_MAP				map_pet_delete;				// 地图里面的所有要释放内存的宠物
	CREATURE_MAP				map_allnpc;					// 地图所有NPC
	
	//DEAD_BOSS_LIST				list_dead_boos;				//所有死亡boos的ID
	//-------------------------------------------------------------------------------------------
	// 九宫格
	//-------------------------------------------------------------------------------------------
	INT						n_visible_tile_array_width;	// 最大可视地砖宽度
	INT						n_visible_tile_array_height;	// 最大可视地砖长度
	tagVisTile*				p_visible_tile;				// 可视地砖，动态生成的数组

	//-------------------------------------------------------------------------------------------
	// 地图属性
	//-------------------------------------------------------------------------------------------
	const tag_map_info*		p_map_info;				// 地图属性信息（外部静态属性，不能删除）
	const MapScript*		p_script;				// 地图脚本
	typedef std::map<DWORD, BOOL> 	MAP_POINT_HAS_LIST;			// 刷怪点是否刷了怪
	MAP_POINT_HAS_LIST		map_point_has_list;
	//c_sparse_graph*				m_pSparseGraph;		// 寻路
	//MAP_POINT_HAS_LIST		map_point_has_unit;		// 某个点是否有人了

	//-------------------------------------------------------------------------------------------
	// 已触发场景特效列表
	//-------------------------------------------------------------------------------------------
	package_list<DWORD>			list_scene_effect;		// 已触发场景特效ObjID列表
	Mutex					h_mutex;				// 特效列表锁

	//-------------------------------------------------------------------------------------------
	// 怪物ID生成器
	//-------------------------------------------------------------------------------------------
	creature_id_builder			builder_creature_id;		// 生物ID生成器

	//-------------------------------------------------------------------------------------------
	// 地物ID生成器
	//-------------------------------------------------------------------------------------------
	ground_item_id_builder			builder_ground_item_id;		// 地物ID生成器

	//-------------------------------------------------------------------------------------------
	// 怪物小队管理器
	//-------------------------------------------------------------------------------------------
	NPCTeamMgr*				p_npc_team_mgr;

	DWORD					dw_update_num_time;

	package_list<tagNPCLoading*>		list_NPCLoading;

	DWORD					dw_update_time;
	DWORD				dw_weather_id_;
	INT				n_chang_weahter_tick_;


	CTimerEvent					m_TimerEvent;						// 时间轴
	UINT						n_last_dead_boos;//最后死亡的boss的类型ID
};

//-----------------------------------------------------------------------------------------------
// 副本基类
//-----------------------------------------------------------------------------------------------
class map_instance : public Map
{
public:
	map_instance() : b_delete(FALSE), b_end(FALSE),b_complete(FALSE),b_send_complete(FALSE),m_pInstance(NULL){}
	virtual ~map_instance() {}

	virtual BOOL	init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE) = 0;
	virtual VOID	update() = 0;
	virtual VOID	destroy() = 0;

	virtual VOID	add_role(Role* pRole, Map* pSrcMap) = 0;
	virtual	VOID	role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE) = 0;
	virtual INT		can_enter(Role *pRole, DWORD dwParam = 0)		
	{ 
		if( is_delete() || is_complete() ) 
			return E_Instance_End_Delete; 
		else 
			return E_Success; 
	}
	virtual BOOL	can_destroy()				{ return is_delete() && map_role.empty(); }

	virtual VOID	on_destroy() = 0;
	virtual VOID	on_delete() = 0;						// 副本设置删除标志时的处理，注意，并不是在副本析构时调用
	//---------------------------------------------------------------------------------------------
	// 副本结束和删除标志位
	//---------------------------------------------------------------------------------------------
	VOID			set_delete()					
	{ 
		if( is_delete() ) 
			return; Interlocked_Exchange((LPLONG)&b_delete, TRUE); on_delete(); }
	VOID			set_end()					{ if( is_end() ) return; Interlocked_Exchange((LPLONG)&b_end, TRUE); }
	VOID			set_complete()				{ if(is_complete()) return; Interlocked_Exchange((LPLONG)&b_complete, TRUE); }
	BOOL			is_end()						{ return b_end; }
	BOOL			is_delete()					{ return b_delete; }
	BOOL			is_complete()				{ return b_complete; }
	VOID			sendMapRoleInstanceComplete();
	virtual VOID	complete_estimate(bool scriptComplete = false) {};
	const tagInstance*	get_instance_proto()		{ return m_pInstance; }

	virtual VOID	set_boss(Creature* p_boss_) {}
	virtual VOID	on_team_create(const Team* p_team_){};
	virtual VOID	on_team_delete(const Team* p_team_){};
	virtual VOID	on_role_leave_team(DWORD dw_role_id_, const Team* p_team_){};
	virtual VOID	on_role_enter_team(DWORD dw_role_id_, const Team* p_team_){};
protected:
	virtual BOOL	init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE) = 0;	// 生成所有刷新点怪物
	virtual BOOL	is_time_limit()		{ return m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTargetLimit != INVALID_VALUE; }

protected:
	volatile BOOL			b_delete;					// 副本删除标志位
	volatile BOOL			b_end;						// 副本结束标志位
	volatile BOOL			b_complete;					// 副本进度是否完成
	volatile BOOL			b_send_complete;			// 是否发送了副本关闭倒计时
	const tagInstance*		m_pInstance;				// 副本静态属性
};

//------------------------------------------------------------------------------------------------
// 某个点是否是否是可行走点
//------------------------------------------------------------------------------------------------
inline BOOL Map::if_can_go(FLOAT f_x_, FLOAT f_z_)
{
	// 文件里的是从左上角为原点，转到左下角为原点
	int nTitleIndex = (int)(f_x_/TILE_SCALE) + (p_map_info->n_height - (int)(f_z_/TILE_SCALE) - 1) * p_map_info->n_width;
	if (nTitleIndex >= p_map_info->map_block_data.size())
	{
		return false;
	}
	return (p_map_info->map_block_data[nTitleIndex] == 0);
	//return get_nav_map()->GetNPCNavMap()->IfCanGo(f_x_, f_z_);
}

//某个点是否可以通过 (动态阻挡)
inline int Map::if_can_dynamic_go(FLOAT f_x_, FLOAT f_z_)
{
	int nTitleIndex = (int)(f_x_/TILE_SCALE) + (p_map_info->n_height - (int)(f_z_/TILE_SCALE) - 1) * p_map_info->n_width;

	return p_map_info->map_dynamic_block[nTitleIndex];
}

inline int Map::if_can_dynamic_go(int nVisIndex)
{
	if(nVisIndex < 0)
		return 255;

	return p_map_info->map_dynamic_block[nVisIndex];
}

//------------------------------------------------------------------------------------------------------------------
// 玩家死亡
//------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_role_die(Role* p_role_, Unit* p_killer_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnRoleDie(p_role_, p_killer_, this);
	}
}

//------------------------------------------------------------------------------------------------------------------
// 玩家是否可以开启PK保护
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_set_safe_guard()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_set_safe_guard(this);
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------------
// 玩家是否可以使用物品
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_use_item(DWORD dw_data_id, INT64 n64_Item_Serial)
{
	if( VALID_POINT(p_script) ) 
	{
		return p_script->can_use_item(this, dw_data_id, n64_Item_Serial);
	}

	return TRUE;
}

inline VOID Map::onclock(INT nClock)
{
	if(VALID_POINT(p_script))
	{
		p_script->onclock(this, nClock);
	}
}

//------------------------------------------------------------------------------------------------------------------
// 玩家是否可以使用技能
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_use_skill(DWORD dw_data_id)
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_use_skill(this, dw_data_id);
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------------
// 玩家复活
//------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
						   INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->Revive(p_role_, e_type_, n_revive_hp_, n_revive_mp_, fx_, fy_, fz_, dw_reborn_map_id_);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 怪物死亡
//-------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_creature_die(Creature* p_creature_, Unit* p_killer_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnCreatureDie(p_creature_, p_killer_, this);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 怪物消失
//-------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_creature_disappear(Creature* p_creature_)
{
	if( VALID_POINT(p_script) ) 
	{
		p_script->OnCreatureDisappear(p_creature_, this);
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 玩家进入触发器
//--------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_enter_trigger(Role* p_role_, tag_map_trigger_info* p_info_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnEnterTrigger(p_role_, p_info_, this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// 玩家进入区域
//---------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_enter_area(Role* p_role_, tag_map_area_info* p_info_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnEnterArea(p_role_, p_info_, this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// 将可见单位同步给周围玩家
//---------------------------------------------------------------------------------------------------------------------
inline VOID Map::syn_visible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
											const LPVOID p_message, const DWORD dw_size)
{
	send_big_visible_tile_message(pVisTile, p_message, dw_size, pUnit);
}

//---------------------------------------------------------------------------------------------------------------------
// 是否允许邀请组队
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_invite_join_team()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_invite_join_team(this);
	}

	return 0;
}

// 是否允许 申请 加入队伍 [2016-5-25 by willing]
inline INT  Map::can_apply_join_team()
{
    if( VALID_POINT(p_script) )
    {
        return p_script->can_apply_join_team(this);
    }

    return 0;
}

// 是否允许 被邀请加入队伍 [2016-5-25 by willing]
inline INT  Map::can_be_invited_join_team()
{
    if( VALID_POINT(p_script) )
    {
        return p_script->can_be_invited_join_team(this);
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// 是否允许离开队伍
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_leave_team()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_leave_team(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// 是否允许移交队长
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_change_leader()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_change_leader(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// 是否可以踢掉玩家
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_kick_member()
{
	if( VALID_POINT(p_script) ) 
	{
		return p_script->can_kick_member(this);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------
// 敌我关系判断
//-----------------------------------------------------------------------------------------------------------------------
inline DWORD Map::get_friend_enemy_flag(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_)
{
	if( !VALID_POINT(p_script) )
	{
		b_ignore_ = FALSE;
		return 0;
	}

	return p_script->FriendEnemy(this, p_src_, p_target_, b_ignore_);
}

//-----------------------------------------------------------------------------------------------------------------------
// 坐标非法
//-----------------------------------------------------------------------------------------------------------------------
inline BOOL Map::is_position_valid(const Vector3& v_vector_)
{
	// 检测是否超出边界
	if( v_vector_.x < 0.0f || v_vector_.x >= FLOAT(p_map_info->n_width * TILE_SCALE) ||
		v_vector_.z < 0.0f || v_vector_.z >= FLOAT(p_map_info->n_height * TILE_SCALE) )
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------------------------------------------------------
// 修正坐标
//--------------------------------------------------------------------------------------------------------------------------
inline VOID Map::fix_position(Vector3& v_vector_)
{
	// 修正范围
	if( v_vector_.x < 0.0f )				v_vector_.x = 0.0f;
	if( v_vector_.z < 0.0f )				v_vector_.z = 0.0f;
	if( v_vector_.x >= FLOAT(p_map_info->n_width * TILE_SCALE) )		v_vector_.x = FLOAT(p_map_info->n_width * TILE_SCALE) - 1.0f;
	if( v_vector_.z >= FLOAT(p_map_info->n_height * TILE_SCALE) )	v_vector_.z = FLOAT(p_map_info->n_height * TILE_SCALE) - 1.0f;

	v_vector_.y = 0;
}

template<typename Operation>
VOID Map::foreach_unit_in_big_visible_tile(Unit* p_unit_, Operation oper_)
{
	if( !VALID_POINT(p_unit_) ) return;

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(p_unit_->GetVisTileIndex(), pVisTile);

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( VALID_POINT(pRole) )
			{
				oper_(pRole);				
			}
		}
	}
}

template<typename Operation, typename Predicate>
VOID Map::foreach_unit_in_map(Operation oper_, Predicate pred_)
{
	// 找到每个地图的人
	ROLE_MAP::map_iter itrR = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itrR, pRole) )
	{
		if( VALID_POINT(pRole) && pred_(pRole))
		{
			oper_(pRole);				
		}
	}

	// 找到每个地图的怪物
	CREATURE_MAP::map_iter itrC = map_creature.begin();
	Creature* pCreature = NULL;

	while( map_creature.find_next(itrC, pCreature) )
	{
		if( VALID_POINT(pCreature) && pred_(pCreature))
		{
			oper_(pCreature);				
		}
	}
}

template<typename RoleOperation>
VOID Map::for_each_role_in_map( RoleOperation oper_)
{
	// 找到每个地图的人
	ROLE_MAP::map_iter itrR = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itrR, pRole) )
		if(VALID_POINT(pRole)) oper_(pRole);			
}


// 从刷怪点组中随机一个没有怪的刷怪点
//class MapSpawnListHelper
//{
//public:
//	static BOOL RandSpwanPoint(Map* pMap, tag_map_spawn_point_list* spawnList, tag_map_spawn_point_info*& pMapSpawnInfo)
//	{
//		if (!VALID_POINT(spawnList))
//			return false;
//		package_map<DWORD,	tag_map_spawn_point_info*> list = spawnList->list;
//		DWORD dwSpawnID = 0;
//		tag_map_spawn_point_info* pInfo = NULL;
//		package_map<DWORD,	tag_map_spawn_point_info*>::map_iter it = list.begin();
//		while(list.find_next(it, dwSpawnID, pInfo))
//		{
//			if (pMap->is_point_has_uint(pInfo->dw_att_id))
//			{
//				list.erase(dwSpawnID);
//			}
//		}
//		if (list.size() <= 0)
//			return false;
//
//		return list.rand_find(dwSpawnID, pMapSpawnInfo);
//	}
//};

#endif