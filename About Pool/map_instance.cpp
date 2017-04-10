/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//普通副本地图

#include "StdAfx.h"

#include "..\_common\_server_define\_server_world_define\map_protocol.h"
#include "..\_common\_server_define\_server_world_define\MapAttDefine.h"
#include "..\_common\_server_define\_server_world_define\common_msg_define.h"

#include "map_instance.h"
#include "map_creator.h"
#include "team.h"
#include "group_mgr.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "pvp_mgr.h"
#include "item_creator.h"
#include "activity_mgr.h"


map_instance_normal::map_instance_normal() 
	: map_instance()
	, m_dw_creator_id(INVALID_VALUE)
	, m_dw_team_id(INVALID_VALUE)
	//, m_dw_start_tick(INVALID_VALUE)
	//, m_dw_end_tick(INVALID_VALUE)
	, m_e_instance_hare_mode(EIHM_NULL)
	, m_b_no_enter(TRUE)
	, m_p_boss(NULL)
	, m_dw_create_time(INVALID_VALUE)
	, m_dw_end_time(INVALID_VALUE)
	, m_dw_delete_time(INVALID_VALUE)
	, b_process(FALSE)
	, dw_save_process_time(0)
	, m_RoleDeadNum(0)
{
	memset(m_door_state, 0, sizeof(m_door_state));
	m_map_will_out_role_id.clear();
}

map_instance_normal::~map_instance_normal()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// 初始化
//------------------------------------------------------------------------------------------------------
BOOL map_instance_normal::init(const tag_map_info* p_info_, DWORD dw_instance_id_, Role* p_creator_, DWORD dw_misc_)
{
	ASSERT( VALID_POINT(p_info_) );
	ASSERT( EMT_Instance == p_info_->e_type );

	if( !VALID_POINT(p_creator_) ) 
		return FALSE;	// 必须要有创建者的

	// 读取副本静态属性
	m_pInstance = AttRes::GetInstance()->get_instance_proto(p_info_->dw_id);
	if( !VALID_POINT(m_pInstance) )	return FALSE;

	// 地图相关属性
	p_map_info = p_info_;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();
	m_list_creature_process.clear();
	b_process = FALSE;
	for(INT i = 0; i < MAX_INST_DOOR_NUM; i++)
	{
		m_door_state[i].dw_door_id = INVALID_VALUE;
		m_door_state[i].b_open = FALSE;
	}

	// 副本相关属性
	dw_id = p_map_info->dw_id;
	dw_instance_id = dw_instance_id_;
	m_dw_creator_id = p_creator_->GetID();
	m_e_instance_hare_mode = m_pInstance->difficulty;
	m_dw_team_id = p_creator_->GetTeamID();
	m_dw_create_time = g_world.GetWorldTime();
// 	m_dw_start_tick = g_world.GetWorldTick();
// 	m_dw_end_tick = INVALID_VALUE;
// 	m_dw_delete_tick = m_pInstance->dwEndTime * TICK_PER_SECOND;
// 	if (m_pInstance->dwEndTime < INSTANCE_COMPLETE_MIN_TIME)
// 	{
// 		m_dw_delete_tick = INSTANCE_COMPLETE_MIN_TIME * TICK_PER_SECOND;
// 	}
	m_dw_end_time = INVALID_VALUE;
	m_dw_delete_time = m_pInstance->dwEndTime;
	if (m_pInstance->dwEndTime < INSTANCE_COMPLETE_MIN_TIME)
	{
		m_dw_delete_time = INSTANCE_COMPLETE_MIN_TIME;
	}
	dw_save_process_time = SAVE_PROCESS_TIME;
	m_RoleDeadNum = 0; //角色死亡次数

	Role* pLeaderRole = NULL;
	const Team* pTeam = g_groupMgr.GetTeamPtr(p_creator_->GetTeamID());
	if(VALID_POINT(pTeam))
	{
		pLeaderRole = pTeam->get_member(0);
		if(VALID_POINT(pLeaderRole))
		{
			init_inst_process(pLeaderRole);
		}
		else
		{
			init_inst_process(p_creator_);
		}
	}
	else
	{
		init_inst_process(p_creator_);
	}
	
	
	p_npc_team_mgr = new NPCTeamMgr(this);
	if(!VALID_POINT(p_npc_team_mgr))
		return FALSE;
	
	// 初始化寻路系统
	//if (!initSparseGraph(p_info_))
	//	return FALSE;


	// 根据mapinfo来初始化地图的怪物，可视列表等信息
	if( FALSE == init_logical_info() )
	{
		return FALSE;
	}

	// 都创建成功了，设置玩家或其所在队伍的所属副本ID
	if( VALID_POINT(m_dw_team_id) )
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( VALID_POINT(pTeam) )
		{
			//if(pTeam->get_own_instanceid() == INVALID_VALUE && pTeam->get_own_instance_mapid() == INVALID_VALUE)
			//{
				pTeam->set_own_instance_mapid(p_map_info->dw_id);
				pTeam->set_own_instanceid(dw_instance_id);
			//}
			//else
			//{
			//	p_creator_->SetMyOwnInstanceMapID(p_map_info->dw_id);
			//	p_creator_->SetMyOwnInstanceID(dw_instance_id);
			//	p_creator_->SetMyOwnInstanceCreateTime(m_dw_create_time);
			//}
		}
	}
	else
	{
		p_creator_->SetMyOwnInstanceMapID(p_map_info->dw_id);
		p_creator_->SetMyOwnInstanceID(dw_instance_id);
		p_creator_->SetMyOwnInstanceCreateTime(m_dw_create_time);
	}

	return TRUE;		
}

// 初始化副本进度
VOID map_instance_normal::init_inst_process(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	if(pRole->get_list_inst_process().size() <= 0)
		return;

	package_list<s_inst_process*>::list_iter iter = pRole->get_list_inst_process().begin();
	s_inst_process* p = NULL;
	while(pRole->get_list_inst_process().find_next(iter, p))
	{
		if(!VALID_POINT(p))
			continue;

		if(p->dw_map_id != get_map_id() || p->n_mode != m_e_instance_hare_mode)
			continue;

		memcpy(m_door_state, p->st_door_state, sizeof(m_door_state));

		for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
		{
			if(p->dw_creature_id[i] == INVALID_VALUE)
				continue;
			m_list_creature_process.push_back(p->dw_creature_id[i]);
		}

		b_process = TRUE;
		
		break;
	}
}

// 保存副本进度
VOID map_instance_normal::add_inst_process_to_role(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	BOOL bFine = FALSE;
	package_list<s_inst_process*>::list_iter iter = pRole->get_list_inst_process().begin();
	s_inst_process* p_inst = NULL;
	while(pRole->get_list_inst_process().find_next(iter, p_inst))
	{
		if(!VALID_POINT(p_inst))
			continue;

		if(p_inst->dw_map_id != get_map_id() || p_inst->n_mode != m_e_instance_hare_mode)
			continue;

		memcpy(p_inst->st_door_state, m_door_state, sizeof(m_door_state));

		memset(p_inst->dw_creature_id, INVALID_VALUE, sizeof(p_inst->dw_creature_id));

		LIST_CREATURE_PRO::list_iter iter_pro = m_list_creature_process.begin();
		DWORD	dw_creature_id = INVALID_VALUE;
		while(m_list_creature_process.find_next(iter_pro, dw_creature_id))
		{
			for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
			{
				if(p_inst->dw_creature_id[i] == INVALID_VALUE && dw_creature_id != INVALID_VALUE)
				{
					p_inst->dw_creature_id[i] = dw_creature_id;
					break;
				}

			}
		}
		bFine = TRUE;

		break;
	}

	if(!bFine)
	{
		s_inst_process* p = new s_inst_process;
		if(!VALID_POINT(p))
			return;
		p->dw_map_id = get_map_id();
		p->n_mode = m_e_instance_hare_mode;
		p->n_type = m_pInstance->eInstanceEnterLimit;

		memcpy(p->st_door_state, m_door_state, sizeof(m_door_state));

		memset(p->dw_creature_id, INVALID_VALUE, sizeof(p->dw_creature_id));

		LIST_CREATURE_PRO::list_iter iter_pro = m_list_creature_process.begin();
		DWORD	dw_creature_id = INVALID_VALUE;
		while(m_list_creature_process.find_next(iter_pro, dw_creature_id))
		{
			for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
			{
				if(p->dw_creature_id[i] == INVALID_VALUE && dw_creature_id != INVALID_VALUE)
				{
					p->dw_creature_id[i] = dw_creature_id;
					break;
				}

			}
		}

		pRole->get_list_inst_process().push_back(p);
	}
}

//---------------------------------------------------------------------------------
// 设置最终boss
//---------------------------------------------------------------------------------
VOID map_instance_normal::set_boss(Creature* p_boss_)
{
	if(!VALID_POINT(p_boss_))
		return;
	//不需要难度判断，直接设置
	if(p_boss_->GetProto()->dw_data_id == m_pInstance->bossID)
	{
		m_p_boss = p_boss_;
	}
}

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID map_instance_normal::update()
{
	__try
	{
		Map::update();
		update_time_issue();
	}
	__except(serverdump::si_exception_filter(GetExceptionInformation()))
	{
		if (VALID_POINT(p_map_info))
			g_world.get_log()->write_log(_T("map_instance_normal::update() error throw mapid=%u\r\n"), p_map_info->dw_id);
	}
}

//---------------------------------------------------------------------------------
// 时限相关的更新
//---------------------------------------------------------------------------------
VOID map_instance_normal::update_time_issue()
{
	// 如果已经处于待删除状态，就不更新了
	if( is_delete() ) return;

	// 计算副本进度是否完成
	complete_estimate();

	//DWORD dw_tick = g_world.GetWorldTick();
	DWORD nowTime = g_world.GetWorldTime();
	// 时限副本
	if( is_time_limit() && !is_end() )
	{
		DWORD intervalTime = CalcTimeDiff(nowTime , m_dw_create_time);
		if( intervalTime >= m_pInstance->dwTimeLimit)
		{
			//守卫天关副本发放奖励,不需要奖励了
			//guardCustomsReward();
			m_dw_end_time = g_world.GetWorldTime();
			set_end();
			reSetTeamMapId();
		}
	}

	// 关闭倒计时
	if( is_end() )
	{
		DWORD intervalTime = CalcTimeDiff(nowTime , m_dw_end_time);
		if( intervalTime > m_dw_delete_time )
		{
			set_delete();
		}
		if (intervalTime >= m_dw_delete_time - INSTANCE_COMPLETE_START_TIME)
		{
			sendMapRoleInstanceComplete();
		}
	}

	// 进度完成关闭倒计时
	if(is_complete() && !is_end())
	{
		//m_dw_end_tick = g_world.GetWorldTick();
		m_dw_end_time = g_world.GetWorldTime();
		set_end();
		reSetTeamMapId();
	}

	// 更新所有待退出的角色的时间
	if( !m_map_will_out_role_id.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_map_will_out_role_id.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT n_tick = INVALID_VALUE;

		while( m_map_will_out_role_id.find_next(it, dw_role_id, n_tick) )
		{
			--n_tick;	// 减一下倒计时
			if( n_tick <= 0 )
			{
				// 时间到了，将玩家传送出去
				Role* p_role = find_role(dw_role_id);
				if( VALID_POINT(p_role) )
				{
					MapMgr* pMapMgr = g_mapCreator.get_map_manager(p_map_info->dw_id);
					if( VALID_POINT(pMapMgr) )
					{
						pMapMgr->RoleInstanceOut(p_role);
					}
				}
				
				m_map_will_out_role_id.erase(dw_role_id);
			}
			else
			{
				m_map_will_out_role_id.change_value(dw_role_id, n_tick);
			}
		}
	}

	dw_save_process_time--;
	if(dw_save_process_time <= 0)
	{
		ROLE_MAP::map_iter iter = map_role.begin();
		Role* pRole = NULL;
		while(map_role.find_next(iter, pRole))
		{
			if(!VALID_POINT(pRole))
				continue;

			// 添加副本进度
			add_inst_process_to_role(pRole);
		}
		dw_save_process_time = SAVE_PROCESS_TIME;
	}
}

//---------------------------------------------------------------------------------
// 完成判断
//---------------------------------------------------------------------------------

VOID map_instance_normal::complete_estimate(bool scriptComplete/* = false*/)
{
	if(is_complete())
		return;
	if (scriptComplete)
	{
		set_complete();
		//m_dw_complete_tick = g_world.GetWorldTick();
		reSetTeamMapId();
		return;
	}

	if(VALID_POINT(m_p_boss) && m_p_boss->IsDead())
	{
		set_complete();
		//m_dw_complete_tick = g_world.GetWorldTick();
		reSetTeamMapId();
	}

	//检测活动
	if (VALID_POINT(get_map_info()) && VALID_POINT(get_map_info()->dwActivityId))
	{
		activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(get_map_info()->dwActivityId);
		if (VALID_POINT( pActivity ))
		{
			if (pActivity->is_start() == FALSE)
			{
				set_complete();
				reSetTeamMapId();
			}
		}
		else
		{
			set_complete();
			reSetTeamMapId();
		}
	}
	// 是否进入等待关闭
	if( map_role.empty() && !is_end() && m_pInstance->dwEndTime != INVALID_VALUE && !m_b_no_enter)
	{
		//m_dw_end_tick = g_world.GetWorldTick();
		m_dw_end_time = g_world.GetWorldTime();
		b_end = TRUE;
		set_complete();
		reSetTeamMapId();
	}
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID map_instance_normal::destroy()
{
	m_list_creature_process.clear();
}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID map_instance_normal::add_role(Role* p_role_, Map* pSrcMap)
{
	Map::add_role(p_role_, pSrcMap);

// 	// 重置关闭等待
// 	if( is_end() )
// 	{
// 		m_dw_end_tick = INVALID_VALUE;
// 		b_end = FALSE;
// 		m_b_no_enter = TRUE;
// 	}

	// 如果是第一个进入副本的玩家
	if( m_b_no_enter )
	{
		m_b_no_enter = FALSE;
	}

	g_pvp_mgr.role_offline(p_role_);
	
	// 发送进入副本消息
	NET_SIS_enter_instance send;
	send.dw_error_code = E_Success;
	send.dwTimeLimit = cal_time_limit();
	send.dwSendTime = g_world.GetWorldTime();
	p_role_->SendMessage(&send, send.dw_size);

// 	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(p_map_info->dw_id);
// 	if (VALID_POINT(pInstance))
// 	{
// 		if (pInstance->eInstanceMapType == EIMT_Team)
// 		{
// 			int num = p_role_->GetDayClearTwoData(ERDCT_DuoRen_Fuben_Num);
// 			p_role_->SetRoleDayClearTwoData(ERDCT_DuoRen_Fuben_Num,num + 1);
// 			p_role_->SendRoleDayClearTwoData(ERDCT_DuoRen_Fuben_Num,num + 1);
// 		}
// 	}
	s_enter_map_limit* pEnterMapLimit = p_role_->GetMapLimitMap().find(p_map_info->dw_id);
	if(VALID_POINT(pEnterMapLimit)/* && m_pInstance->eInstanceEnterLimit != EEL_Week*/)
	{
		/*
		NET_SIS_instance_limit_info send;
		send.dw_enter_num = pEnterMapLimit->dw_enter_num_;
		send.dw_max_enter_num = m_pInstance->dwEnterNumLimit;
		p_role_->SendMessage(&send, send.dw_size);
		*/
		WorldPacket SendLimitMsg(NET_G2C_PROC_MAP_LIMIT_INFO);
		SendLimitMsg << (DWORD)1;
		SendLimitMsg << *pEnterMapLimit;
		p_role_->SendMessage((PVOID)SendLimitMsg.contents(), SendLimitMsg.size());
	}
}

//---------------------------------------------------------------------------------
// 玩家离开地图，只可能在主线程里面调用
//---------------------------------------------------------------------------------
VOID map_instance_normal::role_leave_map(DWORD dw_id_, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dw_id_);
	Role* pRole = g_roleMgr.get_role(dw_id_);
	if(VALID_POINT(pRole))
	{
		// 添加副本进度
		add_inst_process_to_role(pRole);
	}

	// 如果这个玩家在等待离开的列表里，则移除
	m_map_will_out_role_id.erase(dw_id_);

	// 是否进入等待关闭
	if( map_role.empty() && !is_end() && m_pInstance->dwEndTime != INVALID_VALUE )
	{
		//m_dw_end_tick = g_world.GetWorldTick();
		m_dw_end_time = g_world.GetWorldTime();

		//ZHJL:烟火屠魔副本要求5分钟走时结束才结束，玩家下线时不结束
		if (dw_id!=2887770572)
		{
			set_end();
			set_complete();
			reSetTeamMapId();
		}
	}
}
//---------------------------------------------------------------------------------
// 是否能进入副本
//---------------------------------------------------------------------------------
INT map_instance_normal::can_enter(Role *p_role_, DWORD dw_param_)
{
	// 先检测通用判断
	INT n_error_code = map_instance::can_enter(p_role_);
	if( E_Success != n_error_code ) return n_error_code;

	// 检查人数上限
	if( m_pInstance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;

	
	//if(dw_param_ == 0)
	//{
	//	// 等级限制
	//	if( m_pInstance->nLevelDownLimit > p_role_->get_level() )
	//		return E_Instance_Level_Down_Limit;

	//	if( m_pInstance->nLevelUpLimit < p_role_->get_level() )
	//		return E_Instance_Level_Up_Limit;
	//}
	//else
	//{
	//	// 等级限制
	//	if( m_pInstance->nLevelEliteDownLimit > p_role_->get_level() )
	//		return E_Instance_Level_Down_Limit;

	//	if( m_pInstance->nLevelEliteUpLimit < p_role_->get_level() )
	//		return E_Instance_Level_Up_Limit;
	//}

	// 检测队伍
	if( m_dw_team_id != INVALID_VALUE )
	{
		if( p_role_->GetTeamID() != m_dw_team_id )
			return E_Instance_Not_Same_Team;
	}
	// 检测玩家
	else
	{
		if( p_role_->GetID() != m_dw_creator_id )
			return E_Instance_Not_Same_Team;
	}

	// 不是同样难度副本
	if(dw_param_ != m_e_instance_hare_mode)
	{
		return E_Instance_diff_error;
	}

	// 判断权值限制 gx modify 2013.8.15
	/*s_enter_map_limit* pEnterMapLimit = p_role_->GetMapLimitMap().find(m_pInstance->dwMapID);
	if(VALID_POINT(pEnterMapLimit))
	{
		if(pEnterMapLimit->dw_enter_num_ >= m_pInstance->dwEnterNumLimit)
			return E_Instance_EnterNum_Limit;
		else
		{
			pEnterMapLimit->dw_enter_num_++;
		}
	}*/

	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL map_instance_normal::can_destroy()
{
	return map_instance::can_destroy();
}

//---------------------------------------------------------------------------------
// 初始化地图摆放怪物
//---------------------------------------------------------------------------------
BOOL map_instance_normal::init_all_fixed_creature(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
	// 一个一个的创建怪物
	tagMapCreatureInfo* p_creature_info = NULL;
	const tagCreatureProto* p_proto = NULL;
	p_map_info->map_creature_info.reset_iterator();
	while( p_map_info->map_creature_info.find_next(p_creature_info) )
	{
		p_proto = AttRes::GetInstance()->GetCreatureProto(p_creature_info->dw_type_id);
		if( !VALID_POINT(p_proto) )
		{
			print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), p_creature_info->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// 生成出生坐标和出生朝向
		Vector3 v_faceto;
		v_faceto.x = cosf(p_creature_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.z = sinf(p_creature_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// 生成怪物
		Creature* p_creature = Creature::Create(dw_creature_id, dw_id, this, p_proto, p_creature_info->v_pos, v_faceto, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, p_creature_info->b_collide, p_creature_info->list_patrol, INVALID_VALUE, dw_guild_id_);

		// 设置boss
		set_boss(p_creature);

		// 加入到地图中
		add_creature_on_load(p_creature);

		// 如果是巢穴，则加载巢穴怪物
		if( p_creature->IsNest() )
		{
			init_nest_creature(p_creature);
		}

		// 如果是怪物小队，则加载小队怪物
		if( p_creature->IsTeam() )
		{
			init_team_creature(p_creature);
		}
	}

	// 一个一个的创建门对象
	tag_map_door_info* p_door_info = NULL;
	//const tagCreatureProto* pProto = NULL;
	p_map_info->map_door_info.reset_iterator();
	while( p_map_info->map_door_info.find_next(p_door_info) )
	{
		p_proto = AttRes::GetInstance()->GetCreatureProto(p_door_info->dw_type_id);
		if( !VALID_POINT(p_proto) )
		{
			print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), p_door_info->dw_type_id);
			continue;
		}

		if ( !( p_proto->eType == ECT_GameObject && p_proto->nType2 == EGOT_Door ) )
		{
			print_message(_T("Detect a unknown door in map, dwObjID=%u\r\n"), p_door_info->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_door_id = OBJID_TO_DOORID(p_door_info->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id ));

		// 生成出生坐标和出生朝向
		Vector3 v_faceto;
		v_faceto.x = cosf(p_door_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.z = sinf(p_door_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// 生成怪物
		Creature* p_creature = Creature::Create(dw_door_id, dw_id, this, p_proto, p_door_info->v_pos, v_faceto, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);
	
		// 加入到地图中
		add_creature_on_load(p_creature);

		// 加入到门对象列表
		map_door.add( dw_door_id, p_creature );
	}

	return TRUE;
}

BOOL map_instance_normal::init_all_fixed_creature_ex(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
	// 一个一个的创建怪物
	tagMapMonsterInfo* pCreatureInfo = NULL;
	const tagCreatureProto* pProto = NULL;
	p_map_info->map_monster_info.reset_iterator();
	while( p_map_info->map_monster_info.find_next(pCreatureInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pCreatureInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pCreatureInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// 生成出生坐标和出生朝向
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		//大刀守卫特殊处理
		if (pCreatureInfo->dw_type_id == 2007013)
			fYaw = pCreatureInfo->f_yaw;
		Vector3 vFaceTo;
		vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_creature_id, dw_id, this, pProto, pCreatureInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL, INVALID_VALUE);

		if(pProto->bLoading)
		{
			tagNPCLoading* st_NPCLoading = new tagNPCLoading;
			st_NPCLoading->dw_npc_id = dw_creature_id;
			//st_NPCLoading->dw_Obj_id = pCreatureInfo->dw_att_id;
			list_NPCLoading.push_back(st_NPCLoading);
		}

		// 设置boss
		set_boss(pCreature);

		// 加入到地图中
		add_creature_on_load(pCreature);

		// 如果是巢穴，则加载巢穴怪物
		if( pCreature->IsNest() )
		{
			init_nest_creature(pCreature);
		}

		// 如果是怪物小队，则加载小队怪物
		if( pCreature->IsTeam() )
		{
			init_team_creature(pCreature);
		}
	}

	DWORD dwCreatureSize = (map_creature.size() * sizeof(Creature));

	// 一个一个的创建门对象
	tag_map_door_info* pDoorInfo = NULL;
	//const tagCreatureProto* pProto = NULL;
	p_map_info->map_door_info.reset_iterator();
	while( p_map_info->map_door_info.find_next(pDoorInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pDoorInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		if ( !( pProto->eType == ECT_GameObject && pProto->nType2 == EGOT_Door ) )
		{
			print_message(_T("Detect a unknown door in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_door_id = OBJID_TO_DOORID(pDoorInfo->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id) );

		// 生成出生坐标和出生朝向
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		Vector3 vFaceTo;
		vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_door_id, dw_id, this, pProto, pDoorInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);

		// 加入到地图中
		add_creature_on_load(pCreature);

		// 加入到门对象列表
		map_door.add(dw_door_id, pCreature );
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL map_instance_normal::init_all_spawn_point_creature(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
	/*if( EICM_Appoint == m_pInstance->eInstanceCreateMode )
		return TRUE;*/

	if( FALSE == recal_hard_mode() ) return FALSE;

	DWORD		dw_data_id = INVALID_VALUE;
	INT			n_base_level = 0;
	INT			n_level = 0;
	Vector3		vec3;

	// 计算随机怪物基本等级
	/*if( FALSE == get_creature_base_level(nBaseLevel) )
		return FALSE;*/

	tag_map_spawn_point_info* p_map_spawn_info = NULL;
	const tagRandSpawnPointInfo* p_spawn_proto = NULL;
	const tagCreatureProto* p_proto = NULL;

	p_map_info->map_spawn_point.reset_iterator();
	while( p_map_info->map_spawn_point.find_next(p_map_spawn_info) )
	{
		//DWORD dwSpawnPoint = transmit_big_id(nBaseLevel, pMapSpawnInfo);

		// 判断刷怪点是否有进度
		if(m_list_creature_process.is_exist(p_map_spawn_info->dw_att_id))
			continue;

		p_spawn_proto = AttRes::GetInstance()->GetSpawnPointProto(p_map_spawn_info->dw_spawn_point_id);
		if( !VALID_POINT(p_spawn_proto) )
		{
			print_message(_T("Detect a unknown Spawn Point in map, dwObjID=%u\r\n"), p_map_spawn_info->dw_spawn_point_id);
			continue;
		}

		dw_data_id = cal_creature_type_id(p_spawn_proto);
		if( !VALID_POINT(dw_data_id) )
		{
			//print_message(_T("Detect a unknown Instance Creature TypeID\n"));
			continue;
		}

		p_proto = AttRes::GetInstance()->GetCreatureProto(dw_data_id);
		if( !VALID_POINT(p_proto) )
		{
			//print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), pSpawnProto->dwSpawnPointID);
			continue;
		}

		// 取出一个ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// 随机一个朝向
		Vector3 v_faceto;
		//FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		v_faceto.x = cosf((270-p_map_spawn_info->f_angle) * 3.1415927f / 180.0f);
		v_faceto.z = sinf((270-p_map_spawn_info->f_angle) * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// 生成怪物
		Creature* p_creature = Creature::Create(dw_creature_id, dw_id, this, p_proto, p_map_spawn_info->v_pos, v_faceto, ECST_Load, p_map_spawn_info->dw_att_id, INVALID_VALUE, p_map_spawn_info->b_collide, p_map_spawn_info->list_patrol);

		// 设置最终boss
		set_boss(p_creature);
		// 加入到地图中
		add_creature_on_load(p_creature);

		// 如果是巢穴，则加载巢穴怪物
		if( p_creature->IsNest() )
		{
			init_nest_creature(p_creature);
		}

		// 如果是怪物小队，则加载小队怪物
		if( p_creature->IsTeam() )
		{
			init_team_creature(p_creature);
		}
	}

	return TRUE;


}

//---------------------------------------------------------------------------------
// 重新计算副本难度
//---------------------------------------------------------------------------------
BOOL map_instance_normal::recal_hard_mode()
{
	// 不可选副本难度
	if( !m_pInstance->bSelectHard )
	{
		m_e_instance_hare_mode = EIHM_Normal;
		return TRUE;
	}
	 // 可选
	else
	{
		switch(m_e_instance_hare_mode)
		{
			// 普通
		case EIHM_Normal:
			{
				if( !m_pInstance->bSelectNormal ) return FALSE;
				else return TRUE;
			}
			break;

			// 精英
		case EIHM_Elite:
			{
				if( !m_pInstance->bSelectElite ) return FALSE;
				else return TRUE;
			}
			break;

			// 魔王
		case EIHM_Devil:
			{
				if( !m_pInstance->bSelectDevil ) return FALSE;
				else return TRUE;
			}
			break;

		default:
			return FALSE;
			break;
		}

	}
}

//---------------------------------------------------------------------------------
// 根据副本生成规则，得到随机怪物的基本等级
//---------------------------------------------------------------------------------
BOOL map_instance_normal::get_creature_base_level(INT& n_base_level_)
{
	if( m_dw_team_id != INVALID_VALUE && m_pInstance->nNumUpLimit > 1 )
	{
		const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( !VALID_POINT(p_team) ) return FALSE;

		switch( m_pInstance->eInstanceCreateMode )
		{
			// 平均等级
		case EICM_AvgLevel:
			{
				n_base_level_ = p_team->get_average_level();
				return TRUE;
			}
			break;

			// 队长等级
		case EICM_LeaderLevel:
			{
				Role* p_leader = p_team->get_member(0);
				if( !VALID_POINT(p_leader) )
					return FALSE;
				else n_base_level_ = p_leader->get_level();
				return TRUE;
			}
			break;

			// 队伍最高等级
		case EICM_MaxLevel:
			{
				n_base_level_ = p_team->get_max_level();
				return TRUE;
			}
			break;

			// 队伍最低等级
		case EICM_MinLevel:
			{
				n_base_level_ = p_team->get_min_level();
				return TRUE;
			}
			break;

		default:
			return FALSE;
			break;
		}
	}
	else
	{
		Role* p_role = g_roleMgr.get_role(m_dw_creator_id);
		if( !VALID_POINT(p_role) )
			return FALSE;

		n_base_level_ = p_role->get_level();
		return TRUE;
	}
}


//---------------------------------------------------------------------------------
// 根据怪物基本等级得到随机怪物的TypeID
//---------------------------------------------------------------------------------
DWORD map_instance_normal::cal_creature_type_id(const tagRandSpawnPointInfo* p_rand_spawn_point_)
{	
	INT n_index = get_tool()->tool_rand() % RAND_CTEATUTE_NUM;
	
	switch(m_e_instance_hare_mode)
	{
	case EIHM_Normal:
		return p_rand_spawn_point_->dwNormalID[n_index];
	case EIHM_Elite:
		return p_rand_spawn_point_->dwEliteID[n_index];
	case EIHM_Devil:
		return p_rand_spawn_point_->dwDevilID[n_index];
	default:
		return INVALID_VALUE;
	}
}




//---------------------------------------------------------------------------------
// 根据副本基础等级，刷怪点的等级增加量，刷怪点小ID，计算刷怪点大ID
//---------------------------------------------------------------------------------
//DWORD map_instance_normal::transmit_big_id(INT n_base_level_, tag_map_spawn_point_info* p_map_spawn_info_)
//{
//	if(!VALID_POINT(p_map_spawn_info_))
//		return INVALID_VALUE;
//
//	const tagLevelMapping *p_level_mapping = AttRes::GetInstance()->GetLevelMapping(n_base_level_ + p_map_spawn_info_->n_level_increase);
//	return p_map_spawn_info_->dw_spawn_point_id + (DWORD)p_level_mapping->nTransmitLevel;
//}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID map_instance_normal::on_delete()
{
	// 移除所有在地图内的玩家
	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		if(!VALID_POINT(p_role))
			continue;
		p_role->SetMyOwnInstanceID(INVALID_VALUE);
		p_role->SetMyOwnInstanceMapID(INVALID_VALUE);
		tagDWORDTime st_Time;
		ZeroMemory(&st_Time, sizeof(tagDWORDTime));
		p_role->SetMyOwnInstanceCreateTime(st_Time);
		p_map_manager->RoleInstanceOut(p_role);
	}

	const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
	if(VALID_POINT(p_team))
	{
		if(p_team->get_team_id() == m_dw_team_id)
		{
			//gx modify 2014.2.16副本销毁不应该影响队伍的副本信息
			if (dw_id == p_team->get_own_instance_mapid())
			{
				p_team->set_own_instanceid(INVALID_VALUE);
				p_team->set_own_instance_mapid(INVALID_VALUE);
			}
		}
	}
}

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID map_instance_normal::on_destroy()
{
	// 清空一下队伍的副本ID
	if( VALID_POINT(m_dw_team_id) )
	{
		const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( VALID_POINT(p_team) )
		{
			//gx modify 2014.2.16副本销毁不应该影响队伍的副本信息
			if (dw_id == p_team->get_own_instance_mapid())
			{
				p_team->set_own_instance_mapid(INVALID_VALUE);
				p_team->set_own_instanceid(INVALID_VALUE);
			}
		}
	}
	// 清空一下玩家的副本ID
	else
	{
		/*Role* pCreator = g_roleMgr.get_role(m_dwCreatorID);
		if( VALID_POINT(pCreator) )
		{
			pCreator->SetMyOwnInstanceMapID(INVALID_VALUE);
			pCreator->SetMyOwnInstanceID(INVALID_VALUE);
		}*/
	}

}

//-----------------------------------------------------------------------------------
// 计算时限副本所剩时间
//-----------------------------------------------------------------------------------
DWORD map_instance_normal::cal_time_limit()
{
	DWORD dw_time_left = INVALID_VALUE;
	if(m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTimeLimit != INVALID_VALUE)
	{
// 		DWORD dw_current_tick = g_world.GetWorldTick();
// 		DWORD dw_time_pass = (dw_current_tick - m_dw_start_tick) / TICK_PER_SECOND;
// 		dw_time_left = m_pInstance->dwTimeLimit - dw_time_pass;
		DWORD nowTime = g_world.GetWorldTime();
		DWORD dw_time_pass = CalcTimeDiff(nowTime , m_dw_create_time);
		dw_time_left = m_pInstance->dwTimeLimit - dw_time_pass;
	}

	return dw_time_left;
}

//---------------------------------------------------------------------------------------------------
// 当队伍创建
//---------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_team_create(const Team* p_team_)
{
	if( !VALID_POINT(p_team_) ) return;

	if(m_dw_team_id == INVALID_VALUE)
	{
		m_dw_team_id = p_team_->get_team_id();
	}
}

//----------------------------------------------------------------------------------------------------
// 当队伍删除
//----------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_team_delete(const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;
	if( p_team_->get_member_number() > 1 ) return;
	
	//if( pTeam->GetMemID(0) != m_dwCreatorID ) return;

	m_dw_team_id = INVALID_VALUE;

	// 找到队伍里唯一一个玩家的玩家指针，重置为单人副本
	/*Role* pRole = g_roleMgr.get_role(m_dwCreatorID);
	if( VALID_POINT(pRole) )
	{
		pRole->SetMyOwnInstanceID(dw_instance_id);
		pRole->SetMyOwnInstanceMapID(m_dwID);
	}*/

	// 移除所有在地图内的玩家
	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		if(!VALID_POINT(p_role))
			continue;
		if (EIMT_SingleFight != m_pInstance->eInstanceMapType)
		{
			m_map_will_out_role_id.add(p_role->GetID(), ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN);
		}
	}
}

//-----------------------------------------------------------------------------------------------------
// 当玩家离队
//-----------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_role_leave_team(DWORD dw_role_id_, const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;

	// 如果该玩家是副本的创建者，则将创建者移交到当前队伍的队长
	/*if( dw_role_id == m_dwCreatorID )
	{
		ASSERT( pTeam->get_member_number() > 0 );
		m_dwCreatorID = pTeam->GetMemID(0);
	}*/

	// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时
	Role* p_role = find_role(dw_role_id_);
	if( VALID_POINT(p_role) )
	{
		if (EIMT_SingleFight != m_pInstance->eInstanceMapType)
		{
			m_map_will_out_role_id.add(dw_role_id_, ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN);
		}	
	}
}

//------------------------------------------------------------------------------------------------------
// 当队伍进入
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_role_enter_team(DWORD dw_role_id_, const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;

	// 如果该玩家目前在副本里，则清空玩家的离开副本倒计时
	Role* pRole = find_role(dw_role_id_);
	if( VALID_POINT(pRole) )
	{
		m_map_will_out_role_id.erase(dw_role_id_);
	}
}

//------------------------------------------------------------------------------------------------------
// 保存副本门开闭状态
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::set_door_state(DWORD	dw_door_id, BOOL b_open)
{
	for(INT i = 0; i < MAX_INST_DOOR_NUM; i++)
	{
		if(m_door_state[i].dw_door_id != dw_door_id)
			continue;
		m_door_state[i].b_open = b_open;
	}
}

//------------------------------------------------------------------------------------------------------
// 获取副本门开闭状态
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::get_door_state(INT nIndex, DWORD& dw_door_id, BOOL& b_open)
{
	if(nIndex < 0 || nIndex >= MAX_INST_DOOR_NUM)
		return;

	dw_door_id = m_door_state[nIndex].dw_door_id;
	b_open = m_door_state[nIndex].b_open;
}

VOID map_instance_normal::on_role_die( Role* p_role_, Unit* p_killer_ )
{
	m_RoleDeadNum++;
}

VOID map_instance_normal::guardCustomsReward()
{
	if (dw_id != GUARD_CUSTOMS_MAP)
		return;
	if (n_last_dead_boos == INVALID_VALUE)
		return;
	const tagInstanceDropProto* dropProto = AttRes::GetInstance()->GetInstanceDropProto(n_last_dead_boos);
	if (!VALID_POINT(dropProto))
		return;
	if (get_role_num() <= 0)
		return;
	//暂时只给背包有空间的发放奖励,
	Role* role =NULL;
	map_role.reset_iterator();
	while(map_role.find_next(role))
	{
		if (!VALID_POINT(role))
			continue;
		if (role->IsDead())
			continue;
		role->GetCurMgr().IncBagSilver(dropProto->money,EICM_FubenBossReward);
		role->ExpChange(dropProto->exp);
		int index = 0;
		int pro[5] = {0};
		for (int i = 0;i <5; ++i)
		{
			pro[i] = rand()%100;
			if (pro[i] >= dropProto->probability[i])
				continue;
			int size = role->GetItemMgr().GetBagFreeSize();
			if (size <= 0)
				continue;
			++index;
			tagItem *pNewItem = ItemCreator::Create(EICM_FubenBossReward, role->GetID(), dropProto->item[i], dropProto->num[i]);
			role->GetItemMgr().Add2Bag(pNewItem,elci_fuben_boss_reward,true);
		}
		if (index > 0)
		{
			WorldPacket SendMsg(NET_G2C_FUBEN_BOSS_REWARD);
			SendMsg << index;
			int num = 0;
			for (int i = 0;i <5; ++i)
			{
				if (num >= index)
					break;
				if (pro[i] >= dropProto->probability[i])
					continue;
				++num;
				SendMsg << dropProto->item[i];
				SendMsg << dropProto->num[i];
			}
			role->SendMessage((PVOID)SendMsg.contents(), SendMsg.size());
		}
	}
}

VOID map_instance_normal::reSetTeamMapId()
{
	if(!VALID_POINT(m_dw_team_id) )
		return;

	const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
	if( !VALID_POINT(p_team) )
		return;

	//gx modify 2014.2.16副本销毁不应该影响队伍的副本信息
	if (dw_id == p_team->get_own_instance_mapid())
	{
		p_team->set_own_instance_mapid(INVALID_VALUE);
		p_team->set_own_instanceid(INVALID_VALUE);
	}

}
