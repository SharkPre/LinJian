#ifndef _FOOTBALL_DB_ANALYZE_H_
#define _FOOTBALL_DB_ANALYZE_H_

#include <list>
using namespace std;

#include "connection.h"
#include "mysql_connection.h"
#include "property.h"
#include "mysqlImpl.h"
#include "db_request_list.h"
#include "redisclient.h"
#include "game_data_cache.h"
#include "game_common_def.h"
#include "singleton.h"


struct save_db_task_t
{
    uint64_t user_id;
    uint32_t table_flag;
};

#define SAVE_DB_TASK_NUM 100
#define LOAD_USER_DATA_LEVEL_OFFSET 10   //!服务器启动时加载当前最大等级向下偏移10个等级的所有玩家数据

#define FUNCTION_DEF1(FUNC_NAME) void FUNC_NAME(const string& data_, socket_ptr_t socket_)
#define MY_FUNCTION1(FUNC_NAME) void football_db_analyze_t::FUNC_NAME(const string& data_, socket_ptr_t socket_)

#define FUNCTION_DEF2(FUNC_NAME) void FUNC_NAME(long long userId_, socket_ptr_t socket_)
#define MY_FUNCTION2(FUNC_NAME) void football_db_analyze_t::FUNC_NAME(long long userId_, socket_ptr_t socket_)

#define config_inst singleton_t<game_resource_t>::instance()

class football_db_analyze_t
{
public:
	football_db_analyze_t();
	~football_db_analyze_t();

#pragma region Load data from database

	void initTestAccount();
    bool init_football_db();
	void initRobotName();
	void initPetPVPRobot();
	void initRolePvpRobot();
	void initMineAllInfo();
	void initMineOpenAreaInfo();
	void loadUserInfo();
	void loadUserRoleInfo();
	void loadBabyInfo();
	void loadSkillInfo();
	void loadEquipInfo();
	void loadPetItemInfo();
	void loadDrawCardInfo();
	void loadPveInfo();
	void loadWorldBossInfo();
	bool loadPetPvpInfo();
	void loadPetPvpHistory();
	void loadUserDelegateQuestList();
	void loadUserQuestList();
	void loadUserTalentInfo();
	void loadUserMailsInfo();
	void loadUserPetPvpTeamInfo();
	void loadPetEndlessFightingInfo();
	void loadBossBattleInfo();
	void loadMorpherInfo();
	void loadWarStandardInfo();
	void loadUserPvePetFragmentInfo();
	bool loadRobotName();
	void loadPveStarRewardInfo();
	bool loadUserRolePvpInfo();
	void loadUserRolePvpBattleHistory();
	void loadUserRolePvpTeamInfo();
	void loadRolePvpRankListReward();
	void loadRechargeInfo();
	//抢坑
	void loadMineTeamInfo();
	void loadMineBattleHistory();
	bool loadMineAllInfo();
	void loadMineRevengeInfo();
	void loadFriendInfo();
	void loadGuildInfo();
	void laodCelebrationInfo();
	bool loadGlobalInfo();
	void loadShopInfo();
	void loadOrderInfo();

#pragma endregion

	void justForTest();
	void initUserItem(uint64_t userId);
	bool registerNewAccount(const std::string& userAccount, const std::string& userPass, const std::string& deviceId, uint64_t u8UserId, uint64_t sdkUserId, string sdkUserName, uint64_t& newUserId);
	bool registerCreateRole(uint64_t userId, uint64_t templateId, uint64_t& roleId, std::vector<uint64_t>& vecTalent, uint64_t& talentLock);
	bool deleteRole(uint64_t roleId);
	bool enterGame(uint64_t userId, uint64_t roleId);
	bool unlockRoleSlots(uint64_t userId);
	bool switchRole(strUserInfo& userInfo, uint64_t roleId);
	bool buyPackageSize(strUserInfo& userInfo);
	bool petSummon(uint64_t userId, uint64_t petBase, uint64_t itemId, uint64_t itemNum, uint64_t& petId);
	bool petLevelUp(uint64_t petId, uint64_t newLevel, uint64_t newExp, uint64_t itemId, uint64_t itemNum);
	bool petEquip(uint64_t petId, uint64_t equipId, int index_id);
	bool roleEquip(uint64_t petId, uint64_t equipId, int index_id);
	bool petPetEquipCombin(uint64_t userId, uint64_t equipBase, std::vector<int>& vecItem, std::vector<int>&vecItemCount, uint64_t combinCost, uint64_t& newEquipId);
	bool petRankUp(uint64_t petId, uint64_t userId, uint64_t costGold);
	bool roleRankUp(uint64_t roleId, uint64_t userId, uint64_t costGold);
	bool petStarUp(uint64_t petId, uint64_t userId, uint64_t costGold, uint64_t itemId, uint64_t itemCount, strStarUpAttriValue& attribInfo);
	bool petSkillUp(uint64_t skillId, uint64_t newLv, uint64_t newExp, uint64_t itemId, uint64_t itemNum);
	bool petArtifactChoose(uint64_t petId, uint64_t artifactId, uint64_t userId, int costType, uint64_t costNum);
	bool petArtifactRankUp(uint64_t petId, uint64_t userId, uint64_t itemUse, uint64_t itemNum, uint64_t artifactNew);
	bool petArtifactLevelUp(uint64_t petId, uint64_t userId, uint64_t AP);

	bool skillLearn(uint64_t roleId, uint64_t slotId);
	bool equipLevelUp(uint64_t userId, uint64_t equipId, uint64_t equipId_index, uint64_t Equipcost);
	bool equipRankUp(uint64_t roleId);
	bool runesoltUnlock(uint64_t userId, uint64_t slotId);
	bool runeInlay(uint64_t userId, uint64_t index_id, uint64_t slotId, uint64_t type);
	bool runeTakeOut(uint64_t userId, uint64_t slotId);
	bool runeLevelUp(uint64_t userId, std::vector<uint64_t>& vecRuneId, std::vector<uint64_t>& vecExpId, std::vector<uint64_t>& vecLevel,
		std::vector<std::vector<uint64_t> >& vecItemId, std::vector<std::vector<uint64_t> >& vecItemNum);
	bool runeSlotRebuild(uint64_t userId, uint64_t runeId, uint64_t type);
	bool soulsoltUnlock(uint64_t userId, uint64_t slotId);
	bool soulslotLevelUp(uint64_t userId, uint64_t slotId, uint64_t costSP);
	bool soulInlay(uint64_t userId, uint64_t slotId, uint64_t petId);
	bool soulTakeOut(uint64_t userId, uint64_t slotId);
	bool talentsoltUnlock(uint64_t roleId, uint64_t slotId);
	bool talentLock(uint64_t userId, uint64_t slotId);
	bool talentUnlock(uint64_t userId, uint64_t slotId);
	bool talentRefresh(uint64_t userId, uint64_t talentIndex, uint64_t itemId, uint64_t itemNumber, uint64_t& talentValue);
	bool initTalentRateData(strUserTalentInfo* ptrTalentInfo);
	bool updateTalentRateData(strUserTalentInfo* ptrTalentInfo);
	bool talentBribe(uint64_t userId, uint64_t costType, uint64_t costNumber, uint64_t& talentLv);
	bool drawCardReward(uint64_t userId, uint64_t costType, uint64_t costNumber, std::vector<strDrawCardItems>& items, uint64_t actionType, uint64_t drawTime, uint64_t givePoint, std::vector<uint64_t>& discountTime);

	bool createNewBaby(uint64_t templateId, uint64_t& babyId);
	bool createNewRole(std::string& name_);
	bool useTitle(uint64_t userId, uint64_t titleId);
	bool setPetPvpGroup(uint64_t userId, uint64_t type, std::vector<uint64_t>& vecId);
	void getPetIDFromPvpGroup(strPetPvpGroupInfo* petPvpGroupInfo, int type_, int index_, uint64_t& pet_id)
	{
		if (type_ == 1)
		{
			if (index_ == 0) petPvpGroupInfo->team1_index1_pet_id; return;
			if (index_ == 1) petPvpGroupInfo->team1_index2_pet_id; return;
			if (index_ == 2) petPvpGroupInfo->team1_index3_pet_id; return;
			if (index_ == 3) petPvpGroupInfo->team1_index4_pet_id; return;
			if (index_ == 4) petPvpGroupInfo->team1_index5_pet_id; return;
			if (index_ == 5) petPvpGroupInfo->team1_index6_pet_id; return;
		}
		else if (type_ == 2)
		{
			if (index_ == 0) petPvpGroupInfo->team2_index1_pet_id; return;
			if (index_ == 1) petPvpGroupInfo->team2_index2_pet_id; return;
			if (index_ == 2) petPvpGroupInfo->team2_index3_pet_id; return;
			if (index_ == 3) petPvpGroupInfo->team2_index4_pet_id; return;
			if (index_ == 4) petPvpGroupInfo->team2_index5_pet_id; return;
			if (index_ == 5) petPvpGroupInfo->team2_index6_pet_id; return;
		}
	}
	bool setPetPveGroup(uint64_t userId, uint64_t type, uint64_t id1, uint64_t id2);
	void setPetPvpGroup_one(strPetPvpGroupInfo* petPvpGroupInfo, int type_, int index_, std::vector<uint64_t>& vecId);
	bool setPetCurPveGroup(uint64_t userId, uint64_t type);
	bool fastFinishPve(uint64_t userId, uint64_t id, uint64_t keyNumber, uint64_t time, std::vector<uint64_t>& items,std::vector<uint64_t>& itemNumber, std::vector<uint64_t>& newItemID,
		uint64_t exp, uint64_t level, uint64_t levelUpOrNot, int userStrength);
	bool finishPve(std::vector<uint64_t>& items,std::vector<uint64_t>& numbers, uint64_t level, uint64_t levelUpOrNot, uint64_t roleAddExp, 
		uint64_t roleLevel, uint64_t userId, uint64_t progress, uint64_t maxScore, std::vector<uint64_t>& id, int userStrength);
	void updateRoleLevel(strRoleInfo* roleInfo);

	bool fetchMailAttach(uint64_t userId, std::vector<uint64_t>& items, std::vector<uint64_t>& numbers, std::vector<uint64_t>& newItemId);
	bool fetchQuestReward(uint64_t userId, uint64_t addExp, std::vector<uint64_t>& itemId, std::vector<uint64_t>&  itemNum, uint64_t levelUp, uint64_t id, std::vector<uint64_t>& newItemId);
	bool fetchEndlessLevel(uint64_t userId, std::vector<uint64_t>& items, std::vector<uint64_t>& numbers, std::vector<uint64_t>& newItemId);
	bool init_celebration_info(const uint64_t user_id, UserCelebrationInfoMap& info);

#pragma region MSG Handler Func
	FUNCTION_DEF1(get_friend_captain_info);

	FUNCTION_DEF1(register_account);
	FUNCTION_DEF1(sdk_login);
	FUNCTION_DEF1(create_role);
	FUNCTION_DEF1(delete_role);
	FUNCTION_DEF1(enter_game);
	FUNCTION_DEF1(unlock_role_slots);
	FUNCTION_DEF1(buy_strength);
	FUNCTION_DEF1(switch_role);
	FUNCTION_DEF1(buy_package);
	FUNCTION_DEF1(pet_summon);
	FUNCTION_DEF1(pet_levelup);
	FUNCTION_DEF1(pet_equip);
	FUNCTION_DEF1(pet_equip_combin);
	FUNCTION_DEF1(pet_rank);
	FUNCTION_DEF1(pet_starup);
	FUNCTION_DEF1(pet_aritfact_choose);
	FUNCTION_DEF1(pet_artifact_rank_up);
	FUNCTION_DEF1(pet_artifact_levelup);

	FUNCTION_DEF1(update_user_shop_info);

	FUNCTION_DEF1(skill_learn);
	FUNCTION_DEF1(equip_level_up);
	FUNCTION_DEF1(equip_rank_up);
	FUNCTION_DEF1(runesolt_unlock);
	FUNCTION_DEF1(rune_inlay);
	FUNCTION_DEF1(rune_take_out);
	FUNCTION_DEF1(rune_level_up);
	FUNCTION_DEF1(soulsolt_unlock);
	FUNCTION_DEF1(soulslot_level_up);
	FUNCTION_DEF1(soul_inlay);
	FUNCTION_DEF1(soul_take_out);
	FUNCTION_DEF1(talentsolt_unlock);
	FUNCTION_DEF1(talent_lock);
	FUNCTION_DEF1(talent_unlock);
	FUNCTION_DEF1(talent_refresh);
	FUNCTION_DEF1(talent_bribe);

	FUNCTION_DEF1(draw_card_reward);
	FUNCTION_DEF1(update_user_currency);
	FUNCTION_DEF1(buy_something);
	FUNCTION_DEF1(user_use_title);

	FUNCTION_DEF1(set_pet_pvp_group);
	FUNCTION_DEF1(init_petPvpInfo);
	FUNCTION_DEF1(reset_pet_pvp_day_times);
	FUNCTION_DEF1(reset_pet_pvp_cd_time);
	FUNCTION_DEF1(set_pet_pve_group_req);
	FUNCTION_DEF1(set_pet_pve_cur_group_req);
	FUNCTION_DEF1(fast_finish_pve);

	FUNCTION_DEF1(get_delegate_quest_list);
	FUNCTION_DEF1(refresh_delegate_quest_list);
	FUNCTION_DEF1(fetch_delegate_quest_reward);
	FUNCTION_DEF1(start_delegate_quest);
	FUNCTION_DEF1(fast_finish_delegate_quest);
	FUNCTION_DEF1(finish_delegate_event);

	FUNCTION_DEF1(rune_rebuild);
	FUNCTION_DEF1(finish_pve);
	FUNCTION_DEF1(quest_list_info);
	FUNCTION_DEF1(update_ever_tower_info);
	//GM COMMAND
	FUNCTION_DEF1(update_user_vip_level);
	FUNCTION_DEF1(update_user_state);
	FUNCTION_DEF1(update_user_resource);
	FUNCTION_DEF1(update_user_base_resource);
	FUNCTION_DEF1(update_role_level);
	FUNCTION_DEF1(update_role_equip_level);
	FUNCTION_DEF1(update_soul_slot_level);
	FUNCTION_DEF1(update_pet_level);
	FUNCTION_DEF1(update_pet_equip);
	FUNCTION_DEF1(update_artifact);
	FUNCTION_DEF1(update_pet_skill);
	FUNCTION_DEF1(update_user_physical);
	FUNCTION_DEF1(update_user_add_item);
	//FUNCTION_DEF1(update_user_del_item);
	FUNCTION_DEF1(update_user_add_pet);
	FUNCTION_DEF1(update_user_del_pet);

	//mails
	FUNCTION_DEF1(send_mail);
	FUNCTION_DEF1(read_mail);
	FUNCTION_DEF1(fetch_mail_attach);
	FUNCTION_DEF1(delete_mail);

	FUNCTION_DEF1(fetch_quset_list);
	FUNCTION_DEF1(fetch_quest_point_reward);
	FUNCTION_DEF1(pet_pvp_team_level_up);
	FUNCTION_DEF1(pet_pvp_enter_game);
	FUNCTION_DEF1(pet_pvp_finish_game);

	FUNCTION_DEF1(finish_endless_level);
	FUNCTION_DEF1(reset_endless_level);

	FUNCTION_DEF1(revive_in_war);
	FUNCTION_DEF1(get_boss_battle_info);
	FUNCTION_DEF1(unlock_boss_battle_slot);
	FUNCTION_DEF1(update_boss_battle_info);

	FUNCTION_DEF1(exchange_gold);
	FUNCTION_DEF1(role_skill_level_up);
	FUNCTION_DEF1(refresh_times);
	FUNCTION_DEF1(update_configs);

	FUNCTION_DEF1(morpher_level_up);
	FUNCTION_DEF1(morpher_skill_level_up);
	FUNCTION_DEF1(set_active_morpher);

	FUNCTION_DEF1(set_war_standard);
	FUNCTION_DEF1(upgrade_war_standard);

	FUNCTION_DEF1(artifact_inlay);
	FUNCTION_DEF1(artifact_takeout);
	FUNCTION_DEF1(artifact_combine);
	FUNCTION_DEF1(artifact_compsite);
	FUNCTION_DEF1(artifact_decompsite);
	FUNCTION_DEF1(artifact_unlock_slot);

	FUNCTION_DEF1(sell_req);

	FUNCTION_DEF1(change_name);
	FUNCTION_DEF1(change_avatar);

	FUNCTION_DEF1(buy_remain_count);//购买次数
	FUNCTION_DEF1(fetch_pve_star_reward);//领取星数奖励

	FUNCTION_DEF1(update_pve_frag_info);

	//3v3pvp
	FUNCTION_DEF1(refresh_pvp_team_count);
	FUNCTION_DEF1(refresh_pvp_team_cd);
	FUNCTION_DEF1(set_team_group);
	FUNCTION_DEF1(finish_pvp_team_battle);
	FUNCTION_DEF1(fetch_pvp_team_reward);
	FUNCTION_DEF1(init_role_pvp_info);

	FUNCTION_DEF1(finish_new_guide);
	FUNCTION_DEF1(refresh_role_pvp_rank_reward);
	FUNCTION_DEF1(charge_for_gem);
	FUNCTION_DEF1(update_charge_order);
	FUNCTION_DEF1(fetch_monthly_sub_reward);

	//抢坑
	FUNCTION_DEF1(set_mine_team_group);
	FUNCTION_DEF1(enter_mine_battle);
	FUNCTION_DEF1(retreat_mine);
	FUNCTION_DEF1(finish_mine_battle);
	FUNCTION_DEF1(update_mine_status_and_reward);
	FUNCTION_DEF1(reset_mine_all_info);
	FUNCTION_DEF1(get_mine_reward);
	FUNCTION_DEF1(buy_mine_times);

	//friend
	FUNCTION_DEF1(add_friend);
	FUNCTION_DEF1(delete_friend);
	FUNCTION_DEF1(set_black_friend);
	FUNCTION_DEF1(response_add_friend);
	FUNCTION_DEF1(fetch_friend_gift);
	FUNCTION_DEF1(send_friend_gift);

	//user offline
	FUNCTION_DEF1(user_update_login_time);
	FUNCTION_DEF1(user_offline);

	//equip one key
	FUNCTION_DEF1(equip_all);

	FUNCTION_DEF1(fetch_turntable_reward);

	//guild
	FUNCTION_DEF1(create_guild);
	FUNCTION_DEF1(change_guild_desc);
	FUNCTION_DEF1(join_guild);
	FUNCTION_DEF1(response_join_guild);
	FUNCTION_DEF1(leave_guild);
	FUNCTION_DEF1(doante_guild);
	FUNCTION_DEF1(fetch_guild_donate_reward);
	FUNCTION_DEF1(kick_guild_member);
	FUNCTION_DEF1(change_guild_job);

	FUNCTION_DEF1(update_guild_boss_info);
	FUNCTION_DEF1(update_guild_boss_user_data);
	FUNCTION_DEF1(finish_guild_boss_battle);

	//get celebration reward
	FUNCTION_DEF1(fetch_celebration_reward);
	FUNCTION_DEF1(buy_celebration);
	FUNCTION_DEF1(update_celebration_info);
	FUNCTION_DEF1(fetch_seven_days_reward);

	//变身形态解锁
	FUNCTION_DEF1(unlock_morpher_req);

	//cdkey
	FUNCTION_DEF1(fetch_cdkey_gift);

	//!新装备系统
	FUNCTION_DEF1(new_equip_strenghten_up);
	FUNCTION_DEF1(new_equip_star_up);
	FUNCTION_DEF1(new_equip_rank_up);
	FUNCTION_DEF1(new_equip_potential_up);

	FUNCTION_DEF1(use_item);

	FUNCTION_DEF1(reset_game_data);

	//!宠物传承
	FUNCTION_DEF1(pet_inherit);

	//!宠物缘分
	FUNCTION_DEF1(pet_bind_upgrade);

	//!世界Boss
	FUNCTION_DEF1(get_world_boss_info);
	FUNCTION_DEF1(world_boss_sync_stat);
	FUNCTION_DEF1(fetch_world_boss_reward);
	FUNCTION_DEF1(enter_world_boss_battle);
	FUNCTION_DEF1(finish_world_boss_battle);
	FUNCTION_DEF1(revive_world_boss_pet);

	FUNCTION_DEF1(bind_account);
	FUNCTION_DEF1(update_gem_consume);
	FUNCTION_DEF1(update_gain_items);

	FUNCTION_DEF1(fetch_invite_code);
#pragma endregion

	inline void exe_sql()
	{
		if (!singleton_t<db_request_list>::instance().empty())
		{
			//clock_t start = clock();
			//logerror((LOG_SERVER, "mysql excute sql start [start: %ld]", start));
			int size = singleton_t<db_request_list>::instance().get_request_size();
			if (size == 0)
				return;

			m_db.set_autocommit(0);
			for (size_t i = 0; i < 1000; i++)
			{
				request req;
				if (singleton_t<db_request_list>::instance().pop_request_list(req))
				{
					m_db.hset(req.buf);
					logtrace((LOG_SERVER, "buf[%s]", req.buf.c_str()));
				}
				else
				{
					break;
				}
			}
			m_db.commit();
			m_db.set_autocommit(1);
			//clock_t end = clock();
			//float time = (float)(end - start) / CLOCKS_PER_SEC;
			//logerror((LOG_SERVER, "mysql excute sql [end: %ld] [total time:%f] [total sql:%d]", end, time, size > 1000 ? 1000 : size));
		}
	}

	inline void exe_sql_all()
	{
		std::queue<request> queReq;
		singleton_t<db_request_list>::instance().get_request_list(queReq);

		m_db.set_autocommit(0);
		while (queReq.size() > 0)
		{
			request req = queReq.front();
			m_db.hset(req.buf);
			queReq.pop();
		}
		m_db.commit();
		m_db.set_autocommit(1);
	}

private:

    mysqlImpl_t	m_db;
	std::map<std::string, uint64_t> m_mapLastInsertID;
private:
    CMysqlConnection	m_mysql;
	game_data_cache_t	m_data_cache;
    list<save_db_task_t>            m_db_tasks;         //!DB 操作的任务链,定时写入SAVE_DB_TASK_NUM条
    property_t<uint64_t, uint32_t>  m_user_db_flags;    //!玩家db操作标识，标识在任务链中是否有此DB操作任务

	void updateRoleTalent(strRoleInfo* roleInfo_, std::vector<uint64_t>& vecTalent, uint64_t& talentLock);
	void updatePetSkillUnlockInfo(strPetInfo* info, uint64_t petSkillLevel = 1);
	void updatePetSkillUnlock(strPetInfo* info, uint64_t skillId_, int index_, uint64_t petSkillLevel);
	void updateRoleSkillUnlockInfo(strRoleInfo* info_, uint64_t defaultLevel = 1);
	void updateRoleSkillUnlock(strRoleInfo* info_, uint64_t skillId_, int index_, uint64_t defaultLevel);
	void updateGetPetQuestState(uint64_t userId);
	void updatePetLevelUpQuestState(uint64_t userId, uint64_t oldLevel, uint64_t newLevel);
	void updatePetStarUpQuestState(uint64_t userId, uint64_t oldStarLevel, uint64_t newStarLevel);
	void updatePetRankUpQuestState(uint64_t userId, uint64_t oldRankUp, uint64_t newRankUp);
	void updateQuestStateCount(uint64_t userId, uint64_t type, uint64_t target, uint64_t count);
	void updateInviteCodeQuestState(uint64_t userId, uint64_t oldLevel, uint64_t newLevel);

	void updateDatabaseRoleInfo(strRoleInfo* roleInfo);
	void updateDatabasePetInfo(strPetInfo* petInfo);
	void send_add_role_skill_info(ss_msg_role_add_skill& info);
	void send_war_standard_info(ss_msg_add_war_standard& info);
	//void send_user_init_items(ss_msg_init_user_item& info);
	void send_user_talent_info(ss_msg_update_talent_info& info);
	//void send_mine_reward_info(ss_msg_update_mine_status_and_reward& info);
	bool theSameDay(time_t timeNow, time_t timeLast);
	void parseString2Vector(std::string &data, std::vector<uint64_t>& vecInfo);
	bool getPveRewardList(int level, std::vector<uint64_t>& normalItems, std::vector<uint64_t>& normalNumbers, std::vector<uint64_t>& geniusItems,
		std::vector<uint64_t>& geniusNumbers, std::vector<uint64_t>& bossItems, std::vector<uint64_t>& bossNumbers);

	void addMineHistory(const strMineHistory& history_);
	bool updateMineStatus(strMineAllInfo* mineInfo, int occupationTime, time_t timeNow);

	void updateWarStandardInfo(uint64_t userId);
	void updateDailyTaskCountInfo(uint64_t userId, int type, int count);
	void updateDailyTaskCountInfo_one(uint64_t target_type, std::map <std::uint64_t, boost::shared_ptr<strQuestList>>& mapQuest, int count, strUserInfo& userInfo);
	int getVipTimes(uint64_t userId, int type);
	uint64_t getAndUpdateLastInsertID(std::string);

	bool useItem(strItemInfo& itemInfo, const uint64_t needNum);
	bool useItem(const uint64_t itemId, strItemInfo& itemInfo, const uint64_t needNum);
	bool useItem(const uint64_t userId, const uint64_t itemBase, strItemInfo& itemInfo, const uint64_t needNum);
	bool addItemNumOrCreate(const uint64_t userId, const uint64_t itemBase, strItemInfo& itemInfo, const uint64_t addNum);

	bool useEquip(strEquipInfo& itemInfo);
	bool useEquip(const uint64_t equipId, strEquipInfo& equipInfo);
	bool useEquip(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo);
	bool unuseEquip(strEquipInfo& itemInfo);
	bool unuseEquip(const uint64_t equipId, strEquipInfo& equipInfo);
	bool unuseEquip(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo);
	bool addEquipNumOrCreate(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo, const uint64_t addNum);
	bool reduceEquip(strEquipInfo& itemInfo, const uint64_t needNum, const bool equiped = false);
	bool reduceEquip(const uint64_t itemId, strEquipInfo& itemInfo, const uint64_t needNum, const bool equiped = false);
	bool reduceEquip(const uint64_t userId, const uint64_t itemBase, strEquipInfo& itemInfo, const uint64_t needNum, const bool equiped = false);

	inline bool costGold(const uint64_t userId, const uint64_t costGold)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			userInfo.userGold -= costGold;
			userInfo.userCostGold += costGold;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` -  %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu", costGold, costGold, userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
			return true;
		}
		return false;
	}
	inline bool costGem(const uint64_t userId, const uint64_t costNum)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			userInfo.userGem -= costNum;
			userInfo.userCostGem += costNum;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu", userInfo.userGem, userInfo.userCostGem, userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
			return true;
		}
		return false;
	}
	inline bool gainGold(const uint64_t userId, const uint64_t num)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			userInfo.userGold += num;
			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` + %llu where `userId` = %llu", num, userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
			return true;
		}
		return false;
	}
	inline bool gainGem(const uint64_t userId, const uint64_t num)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			userInfo.userGem += num;
			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGem` = `userGem` + %llu where `userId` = %llu", num, userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
			return true;
		}
		return false;
	}

	void gainItems(const std::vector<uint64_t> items, const std::vector<uint64_t> numbers, const uint64_t userId, std::vector<uint64_t>& newItemID);
	void gainItem(const uint64_t itemBase_, const uint64_t num, strUserInfo& userInfo, std::vector<uint64_t>& newItemID);

	void sendMail(uint64_t userId_, const std::uint64_t mailId_, const std::string& to_user_, const std::string& title_, 
		const std::string& content_, const std::vector<uint64_t>& items_, const std::vector<uint64_t>& nums_, bool attach_);

	bool resetGameDataNewDay(uint64_t userId_);
	bool addNewsData(const uint64_t userId, const ENewsID id, const uint64_t time, const std::string& params);

//public:
//	template<typename T>
//	bool redis_get(const string& key, T& value);
//	template<typename T>
//	void redis_set(const string& key, const T& value);
};


//template<typename T>
//bool football_db_analyze_t::redis_get(const string& key, T& value)
//{
//	string val_str = m_redis->get(key);
//	if (val_str.length() > 0)
//	{
//		BinaryReadStream readStream(val_str.c_str(), val_str.size());
//		readStream >> value;
//		return true;
//	}
//	return false;
//}
//
//template<typename T>
//void football_db_analyze_t::redis_get(const string& key, const T& value)
//{
//	BinaryWriteStream writestream;
//	writestream << value;
//	m_redis->set(key, writestream.data());
//}

#endif