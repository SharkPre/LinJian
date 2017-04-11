#include <boost/lexical_cast.hpp>

#ifndef _WIN32
#include <unistd.h>
#endif // _WIN32

#include "football_db_analyze.h"
#include "server_config.h"
#include "singleton.h"
#include "BinaryWriteStream.h"
#include "json/json.h"
#include "error_code.h"
#include "log.h"
#include "base_server.h"
#include "utility.h"
#include "proxy_engine.h"
#include "db_request_list.h"
#include "game_common_def.h"
#include "game_resource.h"
#include "game_util.h"


football_db_analyze_t::football_db_analyze_t() : m_data_cache(), m_db(m_mysql, m_data_cache)
{
}

football_db_analyze_t::~football_db_analyze_t()
{
    //final();
}

// void football_db_analyze_t::final()
// {
//     //!TODO DO all the task
//     //m_mysql.disconnect();
// }

bool football_db_analyze_t::init_football_db()
{
    //mysql
    string mysql_ip = singleton_t<server_config_t>::instance().get("mysql_ip");
	string mysql_port = singleton_t<server_config_t>::instance().get("mysql_port");
	string mysql_user = singleton_t<server_config_t>::instance().get("mysql_user");
	string mysql_passwd = singleton_t<server_config_t>::instance().get("mysql_passwd");
	string mysql_database = singleton_t<server_config_t>::instance().get("mysql_database");

    if (m_mysql.connect(mysql_ip, mysql_port, mysql_user, mysql_passwd, mysql_database))
    {
        loginfo((LOG_SERVER, "init_football_db mysql select db error"));
        return false;
    }

	loginfo((LOG_SERVER, "connect mysql success."));

	//redis
    string redis_ip = singleton_t<server_config_t>::instance().get("redis_ip");
	int redis_port = boost::lexical_cast<int>(singleton_t<server_config_t>::instance().get("redis_port"));
	if (!m_data_cache.connect(redis_ip, redis_port))
    {
		loginfo((LOG_SERVER, "init_football_db m_data_cache connect error"));
        return false;
    }

	//clear all data
	//TODO: 将缓存数据转移至redis后 删除这行代码
	m_data_cache.clear_all();

	if (m_data_cache.dbsize() > 0)
	{
		loginfo((LOG_SERVER, "init_football_db redis already have data, will skip loading data."));
		return true;
	}

#pragma region Load data from database

	loginfo((LOG_SERVER, "=================start load data================="));
	loginfo((LOG_SERVER, "starting init data from db, and push to redis"));

	std::string init_test_account = singleton_t<server_config_t>::instance().get("init_test_account");
	if (atoi(init_test_account.c_str()) == 1)//是否初始化帐号名称
	{
		loginfo((LOG_SERVER, "initTestAccount"));
		initTestAccount();
	}

	//!初始化名称
	loginfo((LOG_SERVER, "loadRobotName"));
	if (loadRobotName()== 0)
	{
		loginfo((LOG_SERVER, "RobotName is empty, will init that."));
		loginfo((LOG_SERVER, "init RobotName"));
		initRobotName();
		loginfo((LOG_SERVER, "loadRobotName"));
		loadRobotName();
	}

	//!初始化宠物PVP机器人
	loginfo((LOG_SERVER, "loadPetPvpInfo"));
	if (!loadPetPvpInfo())
	{
		loginfo((LOG_SERVER, "PetPvpInfo is empty, will init that."));
		loginfo((LOG_SERVER, "init PetPvpInfoRobot"));
		initPetPVPRobot();
		loginfo((LOG_SERVER, "loadPetPvpInfo"));
		loadPetPvpInfo();
	}

	//!初始化主角PVP机器人
	loginfo((LOG_SERVER, "loadUserRolePvpInfo"));
	if (!loadUserRolePvpInfo())
	{
		loginfo((LOG_SERVER, "DB_UserRolePvpInfo is empty, will init that."));
		loginfo((LOG_SERVER, "init DB_UserRolePvpInfo Robot"));
		initRolePvpRobot();
		loginfo((LOG_SERVER, "loadUserRolePvpInfo"));
		loadUserRolePvpInfo();
	}

	//!加载账户信息
	loginfo((LOG_SERVER, "loadUserInfo"));
	loadUserInfo();

	//!加载角色信息
	loginfo((LOG_SERVER, "loadUserRoleInfo"));
	loadUserRoleInfo();

	//!加载宝宝信息
	loginfo((LOG_SERVER, "loadBabyInfo"));
	loadBabyInfo();

	//!加载装备信息
	loginfo((LOG_SERVER, "loadEquipInfo"));
	loadEquipInfo();

	//!加载技能信息
	loginfo((LOG_SERVER, "loadSkillInfo"));
	loadSkillInfo();

	//!加载宝宝Item信息
	loginfo((LOG_SERVER, "loadPetItemInfo"));
	loadPetItemInfo();

	//!加载抽卡信息
	loginfo((LOG_SERVER, "loadDrawCardInfo"));
	loadDrawCardInfo();

	//!加载pve信息
	loginfo((LOG_SERVER, "loadPveInfo"));
	loadPveInfo();

	//！加载世界BOSS信息
	loginfo((LOG_SERVER, "loadWorldBossInfo"));
	loadWorldBossInfo();

	//!加载宠物PVP信息
	loginfo((LOG_SERVER, "loadPetPvpInfo"));
	loadPetPvpInfo();

	//!加载宠物pvp对战历史记录
	loginfo((LOG_SERVER, "loadPetPvpHistory"));
	loadPetPvpHistory();

	//!加载委托任务记录
	loginfo((LOG_SERVER, "loadUserDelegateQuestList"));
	loadUserDelegateQuestList();

	//!加载任务成就信息
	loginfo((LOG_SERVER, "loadUserQuestList"));
	loadUserQuestList();

	//!加载天赋信息
	loginfo((LOG_SERVER, "loadUserTalentInfo"));
	loadUserTalentInfo();

	//!加载邮件信息
	loginfo((LOG_SERVER, "loadUserMailsInfo"));
	loadUserMailsInfo();

	//!加载宠物pvp编组信息
	loginfo((LOG_SERVER, "loadUserPetPvpTeamInfo"));
	loadUserPetPvpTeamInfo();

	//!加载无尽之塔宠物血量信息
	loginfo((LOG_SERVER, "loadPetEndlessFightingInfo"));
	loadPetEndlessFightingInfo();

	//!加载Boss连战宠物以及宠物槽解锁信息
	loginfo((LOG_SERVER, "loadBossBattleInfo"));
	loadBossBattleInfo();

	//!加载变身系统信息
	loginfo((LOG_SERVER, "loadMorpherInfo"));
	loadMorpherInfo();

	//!加载战旗信息
	loginfo((LOG_SERVER, "loadWarStandardInfo"));
	loadWarStandardInfo();

	//!加载pve关卡宠物碎片掉落信息
	loginfo((LOG_SERVER, "loadUserPvePetFragmentInfo"));
	loadUserPvePetFragmentInfo();

	//!加载pve星级评价奖励信息
	loginfo((LOG_SERVER, "loadPveStarRewardInfo"));
	loadPveStarRewardInfo();

	//!加载主角pvp信息
	loginfo((LOG_SERVER, "loadUserRolePvpInfo"));
	loadUserRolePvpInfo();

	//!加载主角PVP历史记录
	loginfo((LOG_SERVER, "loadUserRolePvpBattleHistory"));
	loadUserRolePvpBattleHistory();

	//!加载主角PVP编组信息
	loginfo((LOG_SERVER, "loadUserRolePvpTeamInfo"));
	loadUserRolePvpTeamInfo();

	//!加载主角PVP排行榜奖励信息
	loginfo((LOG_SERVER, "loadRolePvpRankListReward"));
	loadRolePvpRankListReward();

	//!加载充值信息
	loginfo((LOG_SERVER, "loadRechargeInfo"));
	loadRechargeInfo();

	//!加载抢坑军团编组信息
	loginfo((LOG_SERVER, "loadMineTeamInfo"));
	loadMineTeamInfo();

	//!加载抢坑对战历史记录
	loginfo((LOG_SERVER, "loadMineBattleHistory"));
	loadMineBattleHistory();

	//!加载矿图信息
	loginfo((LOG_SERVER, "loadMineAllInfo"));
	if (!loadMineAllInfo())
	{
		loginfo((LOG_SERVER, "DB_MineAllInfo is empty, will init that."));
		loginfo((LOG_SERVER, "init DB_MineAllInfo"));
		initMineAllInfo();
		loginfo((LOG_SERVER, "loadMineAllInfo"));
		loadMineAllInfo();
	}

	//!抢坑区域开放信息表
	loginfo((LOG_SERVER, "loadGlobalInfo"));
	if (!loadGlobalInfo())
	{
		loginfo((LOG_SERVER, "init DB_MineAllInfo"));
		initMineOpenAreaInfo();
		loginfo((LOG_SERVER, "loadGlobalInfo"));
		loadGlobalInfo();
	}

	//!加载好友信息
	loginfo((LOG_SERVER, "loadFriendInfo"));
	loadFriendInfo();

	//!加载复仇信息
	loginfo((LOG_SERVER, "loadMineRevengeInfo"));
	loadMineRevengeInfo();
	//justForTest();

	//!工会信息
	loginfo((LOG_SERVER, "loadGuildInfo"));
	loadGuildInfo();

	//!运营活动信息
	loginfo((LOG_SERVER, "laodCelebrationInfo"));
	laodCelebrationInfo();

	//!加载商店信息
	loginfo((LOG_SERVER, "loadShopInfo"));
	loadShopInfo();

	//!订单信息
	loginfo((LOG_SERVER, "loadOrderInfo"));
	loadOrderInfo();

	loginfo((LOG_SERVER, "=================finish load data================="));

#pragma endregion

    return true;
}

void football_db_analyze_t::initTestAccount()
{
	//account
	std::string userName = "superAcc";

	uint64_t roleBase[3] = { 1002, 1003, 1004};

	uint64_t GoldInit = 1000000;
	uint64_t GemInit = 100000;
	uint64_t Strength = 120;
	uint64_t RoleRank = 4;
	uint64_t RoleLevel = 40;
	uint64_t RoleSkillLevel = 35;

	uint64_t petBase[14] = { 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2016, 2027, 2030 };
	uint64_t petLevel = 35;
	uint64_t petRank = 4;
	uint64_t petStar = 1;
	uint64_t petSkillLevel = 35;
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_playerAttrib_t*>& mapPlayerAttrib = config.get_playerAttrib_config();
	for (int i = 0; i < 100; i++)
	{
		char userAccount[16] = { 0 };
		sprintf(userAccount, "%s%02d", userName.c_str(), i);
		char buf[1024] = { 0 };
		sprintf(buf, "insert into gameUserInfo(`userAccount`, `userGold`, `userGem`, `userStrength`) values('%s', %llu, %llu, %llu);",
			userAccount, GoldInit, GemInit, Strength);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameUserInfo);
		uint64_t newUserId = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameUserInfo);

		//Role
		uint64_t roleId = 0;
		std::vector<uint64_t> vecTalent;
		uint64_t talentLock = 0;
		strRoleInfo* roleInfo = new strRoleInfo;
		roleInfo->rolebase = roleBase[i % 3];
		roleInfo->level_ = 40;
		roleInfo->userid = newUserId;
		roleInfo->equipRank = RoleRank;
		roleInfo->curexp_ = 17268;

		uint64_t skillunlock = 0;
		for (int m = 0; m < 7; m++)
		{
			skillunlock |= 0x1 << m;
		}

		char bufRole[1024] = { 0 };
		sprintf(bufRole, "insert into userRoleInfo(`userId`, `BaseId`, `Level`, `equipRank`, `CurExp`, `skillLock`) values (%llu, %llu, %llu, %llu, 17268, %llu);", newUserId, roleBase[ i % 3], RoleLevel, RoleRank, skillunlock);
		singleton_t<db_request_list>::instance().push_request_list(bufRole, sql_insert, DB_UserRoleInfo);
		roleInfo->role = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserRoleInfo);
		updateRoleSkillUnlockInfo(roleInfo, RoleSkillLevel);
		game_util::updateRoleInfo(m_data_cache, roleInfo);
		updateDatabaseRoleInfo(roleInfo);

		char bufUpdateRoleId[1024] = { 0 };
		sprintf(bufUpdateRoleId, "update gameUserInfo set `userRole0` = %llu where userId = %llu;",roleInfo->role, newUserId);
		singleton_t<db_request_list>::instance().push_request_list(bufUpdateRoleId, sql_update, DB_UserRoleInfo);

		//Pet
		for (int j = 0; j < 14; j++)
		{
			strPetInfo* petInfo = new strPetInfo;
			petInfo->userId = newUserId;
			petInfo->base = petBase[j];
			petInfo->LV = petLevel;
			petInfo->Rank = petRank;
			petInfo->Star = petStar;
			petInfo->EquipsId0 = 0;
			petInfo->EquipsId1 = 0;
			petInfo->EquipsId2 = 0;
			petInfo->EquipsId3 = 0;
			petInfo->EquipsId4 = 0;
			petInfo->EquipsId5 = 0;
			petInfo->SoulMax = 0;
			petInfo->SoulRestore = 0;
			petInfo->EXP = 12948;
			petInfo->SkillsId0 = 0;
			petInfo->SkillsId1 = 0;
			petInfo->SkillsId2 = 0;
			petInfo->SkillsId3 = 0;

			char buf[1024] = { 0 };
			sprintf(buf, "insert into `gamePetInfo` (`Base`, `userId`, `Lv`, `Rank`, `Star`, `EXP`) values(%llu, %llu, %llu, %llu, %llu, 12948)", petBase[j], newUserId, petLevel, petRank, petStar);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GamePetInfo);
			petInfo->ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GamePetInfo);

			updatePetSkillUnlockInfo(petInfo, petSkillLevel);
			game_util::updatePetInfo(m_data_cache, petInfo);
			updateDatabasePetInfo(petInfo);

			delete petInfo;
		}

		//warstandard
		uint64_t warSandardId[4] = { 0 };
		int indexWar = 0;
		for (int index = 1001; index < 1005; index++)
		{
			char warStandard[1024] = { 0 };
			sprintf(warStandard, "insert into `userWarStandardInfo`(`userId`, `Base`, `Lv`) values (%llu, %d, 1);", newUserId, index);
			m_db.hset(warStandard);
			warSandardId[indexWar] = m_db.get_insert_id();
			indexWar++;
		}

		char bufRoleWarStandard[1024] = { 0 };
		sprintf(bufRoleWarStandard, "update `userRoleInfo` set `warStandard0` =  %llu, `warStandard1` = %llu, `warStandard2` = %llu, `warStandard3` = %llu where `Role` = %llu",
			warSandardId[0], warSandardId[1], warSandardId[2], bufRoleWarStandard[3], roleInfo->role);
		m_db.hset(bufRoleWarStandard);
	}
}

bool  football_db_analyze_t::createNewRole(std::string& name_)
{
	if (m_db.hsetUserRoleInfo(name_))
	{
		return true;
	}
	return false;
}

void football_db_analyze_t::initUserItem(uint64_t userId)
{
	//ss_msg_init_user_item info;
	//info.userId = userId;

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_itemInit_t*>& map = config.get_itemInit_config();

	std::vector<uint64_t> items, nums;
	std::map<uint64_t, config_itemInit_t*>::iterator iterItem = map.begin();
	for (iterItem; iterItem != map.end(); iterItem++)
	{
		items.push_back(iterItem->second->m_ID);
		nums.push_back(iterItem->second->m_Number);
	}

	std::vector<uint64_t> newIds;
	gainItems(items, nums, userId, newIds);
	//send_user_init_items(info);
}

bool  football_db_analyze_t::registerNewAccount(const std::string& userAccount, const std::string& userPass, 
	const std::string& deviceId, uint64_t u8UserId, uint64_t sdkUserId, string sdkUserName, uint64_t& newUserId)
{
	uint64_t userId = 0;
	if (m_data_cache.get_userIdByAccount(userAccount, userId))
	{
		logerror((LOG_SERVER, "user name already exists name[%s]", userAccount.c_str()));
		return false;
	}

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	uint64_t GoldInit = 0;
	uint64_t DiamondInit = 0;
	uint64_t ArtifactInit = 0;
	uint64_t MainpvpcoinInit = 0;
	uint64_t PetpvpcoinInit = 0;
	uint64_t SoulpointInit = 0;
	uint64_t StaminaInit = 0;

	std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("GoldInit");
	if (iterGlobal != mapGlobal.end())
	{
		GoldInit = atol(iterGlobal->second.c_str());
	}

	iterGlobal = mapGlobal.find("DiamondInit");
	if (iterGlobal != mapGlobal.end())
	{
		DiamondInit = atol(iterGlobal->second.c_str());
	}

	iterGlobal = mapGlobal.find("StrengthInit");
	if (iterGlobal != mapGlobal.end())
	{
		StaminaInit = atol(iterGlobal->second.c_str());
	}

// 	uint64_t exchangeGoldNum = 10;
// 	iterGlobal = mapGlobal.find("GoldTreeDailyCount");
// 	if (iterGlobal != mapGlobal.end())
// 	{
// 		exchangeGoldNum = atol(iterGlobal->second.c_str());
// 	}

	uint64_t timeNow = time(NULL);

	char buf[1024] = { 0 };
	sprintf(buf, "insert into gameUserInfo"
		"(`userAccount`,`userPassword`, `userGold`, `userGem`, `userSP`, `userAP`, `userStrength`, `userRolePVP`, `userPetPVP`, `userDeviceID`, `userExchangeGoldNum`, `createAt`, `lastLoginAt`, `u8UserId`, `sdkUserId`, `sdkUserName`, `stateFlag`) "
		"values('%s','%s', %llu, %llu, %llu, %llu, %llu, %llu, %llu, '%s', 0, %llu, %llu, %llu, %llu, '%s', 0);",
		userAccount.c_str(), userPass.c_str(), GoldInit, DiamondInit, SoulpointInit, ArtifactInit, StaminaInit, MainpvpcoinInit, PetpvpcoinInit, deviceId.c_str(), timeNow, timeNow, u8UserId, sdkUserId, sdkUserName.c_str());
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameUserInfo);
	newUserId = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameUserInfo);

	strUserInfo* userInfo = new strUserInfo;
	userInfo->userAccount = /*bufName*/userAccount;
	userInfo->userName = userAccount;
	userInfo->userPassword = userPass;
	userInfo->userId = newUserId;
	userInfo->userVip = 0;
	userInfo->userCurRole = 0;
	userInfo->userUnLock = 1;
	userInfo->userGold = GoldInit;
	userInfo->userCostGold = 0;
	userInfo->userTotalGold = 0;
	userInfo->userGem = DiamondInit;
	userInfo->userCostGem = 0;
	userInfo->userTotalGem = 0;
	userInfo->userSP = SoulpointInit;
	userInfo->userAP = ArtifactInit;
	userInfo->userPvpExp = 0;
	userInfo->userPvpLevel = 0;
	userInfo->userDeviceId = deviceId;
	userInfo->userStrength = StaminaInit;
	userInfo->userStrengthTime = 0;
	userInfo->userPackageSize = 0;
	userInfo->userRole0Id = 0;
	userInfo->userRole1Id = 0;
	userInfo->userRole2Id = 0;
	userInfo->userRolePVP = MainpvpcoinInit;
	userInfo->userPetPVP = PetpvpcoinInit;
	userInfo->userTeamPoint = 0;
	userInfo->CurPveTeam = 0;
	userInfo->ETProgress = 0;
	userInfo->CurETLevel = 0;
	userInfo->ETKey = 0;
	userInfo->DQExp = 0;
	userInfo->DQRefreshTimes = 0;
	userInfo->DQRefreshTime = 0;
	userInfo->userBuyStrengthCount = 0;
	userInfo->userTalentExchangLv = 1;
	userInfo->userExchangeGoldNum = 0;
	userInfo->userCurMorpherId = 0;
	userInfo->ArtifacetAccumulate = 0;
	userInfo->AvatarID = 0;
	userInfo->ChangeNameTimes = 0;
	userInfo->petPvpHighestRank = 0;
	userInfo->drawCardTimes = 0;
	userInfo->lastOfflineTime = 0;
	userInfo->questPoint = 0;
	userInfo->questStatus = 0;
	userInfo->guildId = 0;
	userInfo->guildProgress = 0;
	userInfo->guildFetched = 0;
	userInfo->guildTotalProgress = 0;
	userInfo->lastRewardTime = 0;
	userInfo->guildDonateStatus = 0;
	userInfo->leaveGuildCDTime = 0;
	userInfo->kickGuildCDTime = 0;
	userInfo->createAt = timeNow;
	userInfo->lastRefreshNewDayTime = 0;
	userInfo->lastLoginAt = timeNow;
	userInfo->u8UserId = u8UserId;
	userInfo->stateFlag = 0;
	userInfo->inviterId = 0;

	ostringstream oss;
	oss << newUserId;
	m_data_cache.set_userIdByAccount(userAccount, newUserId);
	m_data_cache.set_userIdByName(oss.str(), newUserId);
	m_data_cache.set_userInfo(newUserId, *userInfo);
	//m_cache_userName_map.insert(std::make_pair(userAccount, newUserId));
	//m_cache_userInfo_map.insert(std::make_pair(newUserId, userInfo));

	initUserItem(newUserId);
	delete userInfo;

	return true;
}

bool football_db_analyze_t::registerCreateRole(uint64_t userId, uint64_t templateId, uint64_t& roleId, std::vector<uint64_t>& vecTalent, uint64_t& talentLock)
{
	//get role config value
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_playerAttrib_t*>& mapPlayerAttrib = config.get_playerAttrib_config();

	std::map<uint64_t, config_playerAttrib_t*>::iterator iterRole = mapPlayerAttrib.find(templateId + 10000);
	if (iterRole == mapPlayerAttrib.end())
	{
		return false;
	}

	std::map<uint64_t, config_playerBase_t*>& mapPlayerBase = config.get_playerbase_config();
	std::map<uint64_t, config_playerBase_t*>::iterator iterPlayerBase = mapPlayerBase.find(templateId);
	if (iterPlayerBase == mapPlayerBase.end())
	{
		return false;
	}

	int skillunlock = 0;
	if (1 >= iterPlayerBase->second->m_Skill1UnlockLv) (skillunlock |= 0x1);
	if (1 >= iterPlayerBase->second->m_Skill2UnlockLv) (skillunlock |= 0x1 << 1);
	if (1 >= iterPlayerBase->second->m_Skill3UnlockLv) (skillunlock |= 0x1 << 2);
	if (1 >= iterPlayerBase->second->m_Skill4UnlockLv) (skillunlock |= 0x1 << 3);
	if (1 >= iterPlayerBase->second->m_Skill5UnlockLv) (skillunlock |= 0x1 << 4);
	if (1 >= iterPlayerBase->second->m_Skill6UnlockLv) (skillunlock |= 0x1 << 5);
	if (1 >= iterPlayerBase->second->m_Skill7UnlockLv) (skillunlock |= 0x1 << 6);

	uint64_t newEquip = 0;
	set_new_equip_strenghten(newEquip, 1);
	set_new_equip_star(newEquip, 1);
	set_new_equip_rank(newEquip, 0);

	uint64_t newEquipPotential = 0;
	set_new_equip_potential(newEquipPotential, 1, 1);
	set_new_equip_potential(newEquipPotential, 2, 1);
	set_new_equip_potential(newEquipPotential, 3, 1);

	char buf[4096] = { 0 };
	sprintf(buf, "insert into userRoleInfo(`userId`, `BaseId`,"
		"`HPMax`,"
		"`HPRestore`,"
		"`SoulMax`,"
		"`SoulRestore`,"
		"`PhysicalDamage`,"
		"`PhysicalDefense`,"
		"`MagicDamage`,"
		"`MagicDefense`,"
		"`Critical`,"
		"`Tough`,"
		"`Hit`,"
		"`Block`,"
		"`CriticalDamage`,"
		"`MoveSpeed`,"
		"`FastRate`,"
		"`StiffAdd`,"
		"`StiffSub`,"
		"`AbilityMax`,"
		"`AbHitAdd`,"
		"`AbRestore`,"
		"`AbUseAdd`,"
		"`skillLock`,"
		"`newEquip1`,"
		"`newEquip2`,"
		"`newEquip3`,"
		"`newEquip4`,"
		"`newEquip5`,"
		"`newEquip6`,"
		"`newEquipPotential`"
		") values(%llu, %llu, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %llu, %llu, %llu, %llu, %llu, %llu, %llu); ", userId, templateId,
		iterRole->second->m_HPMax,
		iterRole->second->m_HPRestore,
		iterRole->second->m_SoulMax,
		iterRole->second->m_SoulRestore,
		iterRole->second->m_PhysicalDamage,
		iterRole->second->m_PhysicalDefenseMagicDamage,
		iterRole->second->m_MagicDamage,
		iterRole->second->m_MagicDefense,
		iterRole->second->m_Critical,
		iterRole->second->m_Tough,
		iterRole->second->m_Hit,
		iterRole->second->m_Block,
		iterRole->second->m_CriticalDamage,
		iterRole->second->m_MoveSpeed,
		iterRole->second->m_FastRate,
		iterRole->second->m_StiffAdd,
		iterRole->second->m_StiffSub,
		iterRole->second->m_AbilityMax,
		iterRole->second->m_AbHitAdd,
		iterRole->second->m_AbRestore,
		iterRole->second->m_AbUseAdd,
		skillunlock,
		newEquip,
		newEquip,
		newEquip,
		newEquip,
		newEquip,
		newEquip,
		newEquipPotential);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserRoleInfo);
	roleId = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserRoleInfo);

	strRoleInfo* roleInfo = new strRoleInfo;
	roleInfo->userid = userId;
	roleInfo->role = roleId;
	roleInfo->rolebase = templateId;
	roleInfo->level_ = 1;
	roleInfo->equipRank = 1;
	roleInfo->talent0 = 0;
	roleInfo->talent1 = 0;
	roleInfo->battle_ = 0;
	roleInfo->curhp_ = iterRole->second->m_HPMax;
	roleInfo->curexp_ = 0;
	roleInfo->curability_ = 0;
	roleInfo->cursoul_ = 0;
	roleInfo->nextexp_ = 0;
	roleInfo->exp_ = 0;
	roleInfo->hpmax_ = iterRole->second->m_HPMax;
	roleInfo->hprestore_ = iterRole->second->m_HPRestore;
	roleInfo->soulmax_ = iterRole->second->m_SoulMax;
	roleInfo->soulrestore_ = iterRole->second->m_SoulRestore;
	roleInfo->physicaldamage_ = iterRole->second->m_PhysicalDamage;
	roleInfo->physicaldefense_ = iterRole->second->m_PhysicalDefenseMagicDamage;
	roleInfo->magicdamage_ = iterRole->second->m_MagicDamage;
	roleInfo->magicdefense_ = iterRole->second->m_MagicDefense;
	roleInfo->critical_ = iterRole->second->m_Critical;
	roleInfo->tough_ = iterRole->second->m_Tough;
	roleInfo->hit_ = iterRole->second->m_Hit;
	roleInfo->block_ = iterRole->second->m_Block;
	roleInfo->criticaldamage_ = iterRole->second->m_CriticalDamage;
	roleInfo->movespeed_ = iterRole->second->m_MoveSpeed;
	roleInfo->fastrate_ = iterRole->second->m_FastRate;
	roleInfo->stiffadd_ = iterRole->second->m_StiffAdd;
	roleInfo->stiffsub_ = iterRole->second->m_StiffSub;
	roleInfo->abilitymax_ = iterRole->second->m_AbilityMax;
	roleInfo->abhitadd_ = iterRole->second->m_AbHitAdd;
	roleInfo->abrestore_ = iterRole->second->m_AbRestore;
	roleInfo->abuseadd_ = iterRole->second->m_AbUseAdd;
	roleInfo->equipid0 = 0;
	roleInfo->equipid1 = 0;
	roleInfo->equipid2 = 0;
	roleInfo->equipid3 = 0;
	roleInfo->equipid4 = 0;
	roleInfo->equipid5 = 0;
	roleInfo->progress_ = 0;
	roleInfo->newguide_ = 0;
	roleInfo->runeslotnum = 0;
	roleInfo->runeslot0 = 0;
	roleInfo->runeslot1 = 0;
	roleInfo->runeslot2 = 0;
	roleInfo->runeslot3 = 0;
	roleInfo->runeslot4 = 0;
	roleInfo->runeslot5 = 0;
	roleInfo->runeslot6 = 0;
	roleInfo->runeslot7 = 0;
	roleInfo->runeslotAttrib0 = 0;
	roleInfo->runeslotAttrib1 = 0;
	roleInfo->runeslotAttrib2 = 0;
	roleInfo->runeslotAttrib3 = 0;
	roleInfo->runeslotAttrib4 = 0;
	roleInfo->runeslotAttrib5 = 0;
	roleInfo->runeslotAttrib6 = 0;
	roleInfo->runeslotAttrib7 = 0;
	roleInfo->soulSlotNum = 0;//灵魂链接槽的个数
	roleInfo->soulSlot0 = 0;//灵魂链接宠物等级
	roleInfo->soulSlot1 = 0;
	roleInfo->soulSlot2 = 0;
	roleInfo->soulSlotLevel0 = 1;//灵魂链接槽的等级
	roleInfo->soulSlotLevel1 = 1;
	roleInfo->soulSlotLevel2 = 1;
	roleInfo->skillunlock = skillunlock;
	roleInfo->skill0 = 0;
	roleInfo->skill1 = 0;
	roleInfo->skill2 = 0;
	roleInfo->skill3 = 0;
	roleInfo->skill4 = 0;
	roleInfo->skill5 = 0;
	roleInfo->skill6 = 0;
	roleInfo->curTitle = 0;
	roleInfo->talentLock = 0;
	roleInfo->talent2 = 0;
	roleInfo->talent3 = 0;
	roleInfo->talent4 = 0;
	roleInfo->warStandard0 = 0;
	roleInfo->warStandard1 = 0;
	roleInfo->warStandard2 = 0;
	roleInfo->warStandard3 = 0;
	roleInfo->warStandard4 = 0;
	roleInfo->warStandard5 = 0;
	roleInfo->artifact0 = 0;
	roleInfo->artifact1 = 0;
	roleInfo->artifact2 = 0;
	roleInfo->artifact3 = 0;
	roleInfo->artifact4 = 0;
	roleInfo->artifact5 = 0;
	roleInfo->lockArtifactSlot = 0;
	roleInfo->NewEquip1 = newEquip;
	roleInfo->NewEquip2 = newEquip;
	roleInfo->NewEquip3 = newEquip;
	roleInfo->NewEquip4 = newEquip;
	roleInfo->NewEquip5 = newEquip;
	roleInfo->NewEquip6 = newEquip;
	roleInfo->NewEquipPotential = newEquipPotential;

	//if curRole is null, set this role curRole
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		if (userInfo.userCurRole != 0)
		{
			userInfo.userCurRole = roleId;
		}
	}

	updateRoleTalent(roleInfo, vecTalent, talentLock);
	updateRoleSkillUnlockInfo(roleInfo);
	game_util::updateRoleInfo(m_data_cache, roleInfo);
	updateDatabaseRoleInfo(roleInfo);
	m_data_cache.set_roleInfo(roleInfo->userid, roleInfo->role, roleInfo->rolebase, *roleInfo);
	//m_cache_roleInfo_map.insert(std::make_pair(roleId, roleInfo));

	if (userInfo.userRole0Id == 0)
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userRole0` = %llu where `userId` = %llu;", roleId, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		userInfo.userRole0Id = roleId;
	}
	//else if (userInfo.userRole1Id == 0)
	//{
	//	char buf[1024] = { 0 };
	//	sprintf(buf, "update `gameUserInfo` set `userRole1` = %llu where `userId` = %llu;", roleId, userId);
	//	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	//	userInfo.userRole1Id = roleId;
	//}
	//else if (userInfo.userRole2Id == 0)
	//{
	//	char buf[1024] = { 0 };
	//	sprintf(buf, "update `gameUserInfo` set `userRole2` = %llu where `userId` = %llu;", roleId, userId);
	//	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	//	userInfo.userRole2Id = roleId;
	//}
	else
	{
		return false;
	}

	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	return true;
}

bool football_db_analyze_t::deleteRole(uint64_t roleId)
{
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, roleId, NULL, roleInfo))
	{
		return false;
	}
	else
	{
		m_data_cache.del_roleInfo(NULL, roleId);
	}

	char buf[1024] = { 0 };
	sprintf(buf, "delete from userRoleInfo where `role` = %llu;", roleId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	return true;
}

bool football_db_analyze_t::enterGame(uint64_t userId, uint64_t roleId)
{

	return false;
}

bool football_db_analyze_t::unlockRoleSlots(uint64_t userId)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		userInfo.userUnLock += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userRoleUnlock` = `userRoleUnlock` + 1 where `userId` = %llu;", userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::switchRole(strUserInfo& userInfo, uint64_t roleId)
{
	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, roleId, NULL, roleInfo))
	{
		userInfo.userCurRole = roleId;

		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userCurRole` = %llu where `userId` = %llu;", roleId, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::buyPackageSize(strUserInfo& userInfo)
{
	userInfo.userPackageSize += 10;

	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `userPackageSize` = `userPacakgeSize` + 10 where `userId` = %llu;", userInfo.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	return true;
}

bool football_db_analyze_t::petSummon(uint64_t userId, uint64_t petBase, uint64_t itemId, uint64_t itemNum, uint64_t& petId)
{
	//std::unordered_map<uint64_t, strItemInfo*>::iterator iterItem = m_cache_item_map.find(itemId);
	//if (iterItem != m_cache_item_map.end())
	strItemInfo itemInfo;
	useItem(itemId, itemInfo, itemNum);

	strPetInfo* petInfo = new strPetInfo;
	clear_strPetInfo(petInfo);
	petInfo->userId = userId;
	petInfo->base = petBase;
	petInfo->LV = 1;
	petInfo->Rank = 1;
	petInfo->Star = 1;

	uint64_t newEquip = 0;
	set_new_equip_strenghten(newEquip, 1);
	set_new_equip_star(newEquip, 1);
	set_new_equip_rank(newEquip, 0);
	petInfo->NewEquip1 = newEquip;
	petInfo->NewEquip2 = newEquip;
	petInfo->NewEquip3 = newEquip;
	petInfo->NewEquip4 = newEquip;
	petInfo->NewEquip5 = newEquip;
	petInfo->NewEquip6 = newEquip;

	uint64_t newEquipPotential = 0;
	set_new_equip_potential(newEquipPotential, 1, 1);
	set_new_equip_potential(newEquipPotential, 2, 1);
	set_new_equip_potential(newEquipPotential, 3, 1);
	petInfo->NewEquipPotential = newEquipPotential;

	petInfo->petBinds.clear();
	for (size_t i = 0; i < Max_Pet_Bind_Size; i++)
	{
		petInfo->petBinds.push_back(0);
	}

	char buf[1024] = { 0 };
	sprintf(buf, "insert into `gamePetInfo` (`Base`, `userId`, `Lv`, `Rank`, `Star`) values(%llu, %llu, %d, %d, %d)", petBase, userId, 1, 1, 1);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GamePetInfo);
	petInfo->ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GamePetInfo);
	petId = petInfo->ID;
	m_data_cache.set_petInfo(petInfo->userId, petInfo->ID, petInfo->base, *petInfo);

	updatePetSkillUnlockInfo(petInfo);
	game_util::updatePetInfo(m_data_cache, petInfo);
	updateDatabasePetInfo(petInfo);

	updateGetPetQuestState(userId);
	updateWarStandardInfo(userId);

	delete petInfo;
	return true;
}

bool football_db_analyze_t::petLevelUp(uint64_t petId, uint64_t newLevel, uint64_t newExp, uint64_t itemId, uint64_t itemNum)
{
	//std::unordered_map<uint64_t, strItemInfo*>::iterator iterItem = m_cache_item_map.find(itemId);
	//if (iterItem == m_cache_item_map.end())
	strItemInfo itemInfo;
	if (!useItem(itemId, itemInfo, itemNum))
	{
		return false;
	}

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		bool unlockArtifact = false;
		for (size_t slot = 1; slot <= 6; slot++)
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			int unlock = petInfo.LockedArtifactSlot & (1 << (slot - 1));
			int unlockLv = config.getArtifactUnlockLv(slot);
			int unlockCost = config.getArtifactUnlockCost(slot);
			if (unlock == 0 && unlockLv <= newLevel && unlockCost == 0)
			{
				petInfo.LockedArtifactSlot |= (1 << (slot - 1));
				unlockArtifact = true;
			}
		}

		petInfo.LV = newLevel;
		petInfo.EXP = newExp;

		if (unlockArtifact)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `Lv` = %llu, `EXP` = %llu, `lockedArtifactSlot` = %llu where `ID` = %llu;",
				newLevel, newExp, petInfo.LockedArtifactSlot, petId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		}
		else
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `Lv` = %llu, `EXP` = %llu where `ID` = %llu;", newLevel, newExp, petId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		}

		updatePetSkillUnlockInfo(&petInfo);
		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);

		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::petEquip(uint64_t petId, uint64_t equipId, int index_id)
{
	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		char buf[1024] = { 0 };
		if (index_id == 0)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId0` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId0 = equipId;
		}
		else if (index_id == 1)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId1` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId1 = equipId;
		}
		else if (index_id == 2)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId2` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId2 = equipId;
		}
		else if (index_id == 3)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId3` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId3 = equipId;
		}
		else if (index_id == 4)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId4` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId4 = equipId;
		}
		else if (index_id == 5)
		{
			sprintf(buf, "update `gamePetInfo` set `EquipsId5` = %llu where `ID` = %llu;", equipId, petId);
			petInfo.EquipsId5 = equipId;
		}
		else
		{
			return false;
		}
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

		strEquipInfo equipInfo;
		this->useEquip(equipId, equipInfo);

		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		return true;
	}
	return false;
}

bool football_db_analyze_t::roleEquip(uint64_t roleId, uint64_t equipId, int index_id)
{
	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, roleId, NULL, roleInfo))
	{
		char buf[1024] = { 0 };
		if (index_id == 0)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId0` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid0 = equipId;
		}
		else if (index_id == 1)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId1` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid1 = equipId;
		}
		else if (index_id == 2)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId2` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid2 = equipId;
		}
		else if (index_id == 3)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId3` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid3 = equipId;
		}
		else if (index_id == 4)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId4` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid4 = equipId;
		}
		else if (index_id == 5)
		{
			sprintf(buf, "update `userRoleInfo` set `EquipId5` = %llu where `Role` = %llu;", equipId, roleId);
			roleInfo.equipid5 = equipId;
		}
		else
		{
			return false;
		}
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

		strEquipInfo equipInfo;
		this->useEquip(equipId, equipInfo);

		game_util::updateRoleInfo(m_data_cache, &roleInfo);
		updateDatabaseRoleInfo(&roleInfo);
		return true;
	}
	return false;
}


bool football_db_analyze_t::petPetEquipCombin(uint64_t userId, uint64_t equipBase, std::vector<int>& vecItem, std::vector<int>&vecItemCount, uint64_t combinCost, uint64_t& newEquipId)
{
	this->costGold(userId, combinCost);

	for (size_t i = 0; i < vecItem.size(); i++)
	{
		strItemInfo itemInfo;
		useItem(vecItem[i], itemInfo, vecItemCount[i]);
	}

	//add new equip
	strEquipInfo equipInfo;
	this->addEquipNumOrCreate(userId, equipBase, equipInfo, 1);
	newEquipId = equipInfo.ID;
	return true;
}

bool football_db_analyze_t::petRankUp(uint64_t petId, uint64_t userId, uint64_t costGold)
{
	this->costGold(userId, costGold);

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `Rank` = `Rank` + 1, `EquipsId0` = 0, `EquipsId1` = 0, `EquipsId2` = 0, `EquipsId3` = 0, `EquipsId4` = 0, `EquipsId5` = 0 where `ID` = %llu;", petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

		strEquipInfo equipInfo;
		this->reduceEquip(petInfo.EquipsId0, equipInfo, 1, true);
		this->reduceEquip(petInfo.EquipsId1, equipInfo, 1, true);
		this->reduceEquip(petInfo.EquipsId2, equipInfo, 1, true);
		this->reduceEquip(petInfo.EquipsId3, equipInfo, 1, true);
		this->reduceEquip(petInfo.EquipsId4, equipInfo, 1, true);
		this->reduceEquip(petInfo.EquipsId5, equipInfo, 1, true);

		petInfo.Rank += 1;
		petInfo.EquipsId0 = 0;
		petInfo.EquipsId1 = 0;
		petInfo.EquipsId2 = 0;
		petInfo.EquipsId3 = 0;
		petInfo.EquipsId4 = 0;
		petInfo.EquipsId5 = 0;

		updatePetSkillUnlockInfo(&petInfo);
		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);
		updatePetRankUpQuestState(userId, petInfo.Rank - 1, petInfo.Rank);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::roleRankUp(uint64_t roleId, uint64_t userId, uint64_t costGold)
{
	this->costGold(userId, costGold);

	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, roleId, NULL, roleInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `equipRank` = `equipRank` + 1, `EquipId0` = 0, `EquipId1` = 0, `EquipId2` = 0, `EquipId3` = 0, `EquipId4` = 0, `EquipId5` = 0 where `Role` = %llu;", roleId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

		strEquipInfo equipInfo;
		this->reduceEquip(roleInfo.equipid0, equipInfo, 1, true);
		this->reduceEquip(roleInfo.equipid1, equipInfo, 1, true);
		this->reduceEquip(roleInfo.equipid2, equipInfo, 1, true);
		this->reduceEquip(roleInfo.equipid3, equipInfo, 1, true);
		this->reduceEquip(roleInfo.equipid4, equipInfo, 1, true);
		this->reduceEquip(roleInfo.equipid5, equipInfo, 1, true);

		roleInfo.equipRank += 1;
		roleInfo.equipid0 = 0;
		roleInfo.equipid1 = 0;
		roleInfo.equipid2 = 0;
		roleInfo.equipid3 = 0;
		roleInfo.equipid4 = 0;
		roleInfo.equipid5 = 0;

		game_util::updateRoleInfo(m_data_cache, &roleInfo);
		updateDatabaseRoleInfo(&roleInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::petStarUp(uint64_t petId, uint64_t userId, uint64_t costGold, uint64_t itemId, uint64_t itemCount, strStarUpAttriValue& attribInfo)
{
	this->costGold(userId, costGold);

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `Star` = `Star` +  1 where `ID` = %llu;", petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

		petInfo.Star += 1;
		updatePetSkillUnlockInfo(&petInfo);
		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);

		updatePetStarUpQuestState(userId, petInfo.Star - 1, petInfo.Star);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}

	//std::unordered_map<uint64_t, strItemInfo*>::iterator iterItem = m_cache_item_map.find(itemId);
	//if (iterItem != m_cache_item_map.end())
	strItemInfo itemInfo;
	if (m_data_cache.get_itemInfo(NULL, itemId, NULL, itemInfo))
	{
		itemInfo.Num -= itemCount;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameItemInfo` set `itemNum` = `itemNum` - %llu where `itemId` = %llu;", itemCount, itemId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameItemInfo);
		m_data_cache.set_itemInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::petSkillUp(uint64_t skillId, uint64_t newLv, uint64_t newExp, uint64_t itemId, uint64_t itemNum)
{
	//std::unordered_map<uint64_t, strItemInfo*>::iterator iterItem = m_cache_item_map.find(itemId);
	//if (iterItem == m_cache_item_map.end())
	strItemInfo itemInfo;
	if (!useItem(itemId, itemInfo, itemNum))
	{
		return false;
	}

	strSkillInfo skillInfo;
	if (m_data_cache.get_skillInfo(NULL, skillId, NULL, skillInfo))
	{
		skillInfo.Lv = newLv;
		skillInfo.CurExp = newExp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameSkillInfo` set `Lv` = %llu, `CurExp` = %llu where `ID` = %llu;", newLv, newExp, skillId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
		m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::petArtifactChoose(uint64_t petId, uint64_t artifactId, uint64_t userId, int costType, uint64_t costNum)
{
	strPetInfo petInfo;
	if (!m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		return false;
	}

	char buf[1024] = { 0 };
	if (costType == 1)
	{
		this->costGold(userId, costNum);
	}
	else if (costType == 2)
	{
		this->costGem(userId, costNum);
	}
	else
	{
		return false;
	}

	char bufUpdate[1024] = { 0 };
	sprintf(buf, "update `gamePetInfo` set `Artifact` = %llu, `ArtifactLv` = 1 where `ID` = %llu;", artifactId, petId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
	petInfo.Artifact = artifactId;
	petInfo.ArtifactLv = 1;
	m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	return true;
}

bool football_db_analyze_t::petArtifactRankUp(uint64_t petId, uint64_t userId, uint64_t itemUse, uint64_t itemNum, uint64_t artifactNew)
{
	strItemInfo itemInfo;
	if (!useItem(itemUse, itemInfo, itemNum))
	{
		return false;
	}

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `Artifact` = %llu where `ID` = %llu;",artifactNew, petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		petInfo.Artifact = artifactNew;
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}

	return true;
}

bool football_db_analyze_t::petArtifactLevelUp(uint64_t petId, uint64_t userId, uint64_t AP)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		userInfo.userAP -= AP;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userAP` = `userAP` -  %llu where `userId` = %llu", AP, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, petId, NULL, petInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `ArtifactLv` = `ArtifactLv` + 1 where `ID` = %llu;", petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		petInfo.ArtifactLv += 1;
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}

	return true;
}

bool football_db_analyze_t::skillLearn(uint64_t roleId, uint64_t slotId)
{
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, roleId, NULL, roleInfo))
	{
		return false;
	}

	if (!(roleInfo.skillunlock & (0x01 << slotId)))
	{
		return false;
	}

	roleInfo.skillunlock |= 0x1 << slotId;

	char buf[1024] = { 0 };
	sprintf(buf, "update `userRoleInfo` set `skillLock` = %d where `Role` = %llu;", roleId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);

	return false;
}

bool football_db_analyze_t::equipLevelUp(uint64_t userId, uint64_t equipId, uint64_t equip_index, uint64_t cost)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		userInfo.userGold -= cost;
		userInfo.userCostGold += cost;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` -  %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu", cost, cost, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		if (equip_index == 0)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId0` = `EquipId0` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid0 += 1;
		}
		else if (equip_index == 1)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId1` = `EquipId1` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid1 += 1;
		}
		else if (equip_index == 2)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId2` = `EquipId2` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid2 += 1;
		}
		else if (equip_index == 3)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId3` = `EquipId3` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid3 += 1;
		}
		else if (equip_index == 4)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId4` = `EquipId4` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid4 += 1;
		}
		else if (equip_index == 5)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `EquipId5` = `EquipId5` + 1 where `Role` = %llu;", userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.equipid5 += 1;
		}

		game_util::updateRoleInfo(m_data_cache, &roleInfo);
		updateDatabaseRoleInfo(&roleInfo);
	}

	return true;
}

bool football_db_analyze_t::equipRankUp(uint64_t userId)
{
	return false;
}

bool football_db_analyze_t::runesoltUnlock(uint64_t userId, uint64_t slotId)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	roleInfo.runeslotnum += 1;

	char buf[1024] = { 0 };
	sprintf(buf, "update `userRoleInfo` set `runeSlotNum` = `runeSlotNum` + 1 where `Role` = %llu;", userInfo.userCurRole);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);

	return true;
}

bool football_db_analyze_t::runeInlay(uint64_t userId, uint64_t index_id, uint64_t slotId, uint64_t type)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	strEquipInfo equipInfo;
	if (this->useEquip(slotId, equipInfo))
	{
		return false;
	}

	if (index_id == 0)
	{
		roleInfo.runeslot0 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot0` = %llu, `runeSlotAttrib0` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}

	if (index_id == 1)
	{
		roleInfo.runeslot1 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot1` = %llu, `runeSlotAttrib1` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}

	if (index_id == 2)
	{
		roleInfo.runeslot2 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot2` = %llu, `runeSlotAttrib2` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}


	if (index_id == 3)
	{
		roleInfo.runeslot3 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot3` = %llu, `runeSlotAttrib3` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}


	if (index_id == 4)
	{
		roleInfo.runeslot4 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot4` = %llu, `runeSlotAttrib4` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	if (index_id == 5)
	{
		roleInfo.runeslot5 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot5` = %llu, `runeSlotAttrib5` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}

	if (index_id == 6)
	{
		roleInfo.runeslot6 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot6` = %llu, `runeSlotAttrib6` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}

	if (index_id == 7)
	{
		roleInfo.runeslot7 = slotId;

		char bufTemp[1024] = { 0 };
		sprintf(bufTemp, "update `userRoleInfo` set `runeSlot7` = %llu, `runeSlotAttrib7` = %llu where `Role` = %llu;", slotId, type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(bufTemp, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::runeTakeOut(uint64_t userId, uint64_t index_id)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	uint64_t equipId = 0;
	if (index_id == 0)
	{
		equipId = roleInfo.runeslot0;
		roleInfo.runeslot0 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot0` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 1)
	{
		equipId = roleInfo.runeslot1;
		roleInfo.runeslot1 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot1` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 2)
	{
		equipId = roleInfo.runeslot2;
		roleInfo.runeslot2 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot2` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 3)
	{
		equipId = roleInfo.runeslot3;
		roleInfo.runeslot3 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot3` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 4)
	{
		equipId = roleInfo.runeslot4;
		roleInfo.runeslot4 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot4` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 5)
	{
		equipId = roleInfo.runeslot5;
		roleInfo.runeslot5 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot5` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 6)
	{
		equipId = roleInfo.runeslot6;
		roleInfo.runeslot6 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot6` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (index_id == 7)
	{
		equipId = roleInfo.runeslot7;
		roleInfo.runeslot7 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlot7` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else
	{
		return false;
	}

	strEquipInfo equipInfo;
	if (!this->unuseEquip(equipId, equipInfo))
	{
		return false;
	}
	return true;
}

bool football_db_analyze_t::runeSlotRebuild(uint64_t userId, uint64_t runeId, uint64_t type)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	if (runeId == 0)
	{
		roleInfo.runeslotAttrib0 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib0` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 1)
	{
		roleInfo.runeslotAttrib2 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib1` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 2)
	{
		roleInfo.runeslotAttrib2 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib2` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 3)
	{
		roleInfo.runeslotAttrib3 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib3` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 4)
	{
		roleInfo.runeslotAttrib4 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib4` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 5)
	{
		roleInfo.runeslotAttrib5 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib5` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 6)
	{
		roleInfo.runeslotAttrib6 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib6` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	else if (runeId == 7)
	{
		roleInfo.runeslotAttrib7 = type;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `runeSlotAttrib7` = %llu where `Role` = %llu;", type, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		return true;
	}
	return false;
}

bool football_db_analyze_t::runeLevelUp(uint64_t userId, std::vector<uint64_t>& vecRuneId, std::vector<uint64_t>& vecExp, std::vector<uint64_t>& vecLevel,
	std::vector<std::vector<uint64_t> >& vecItemId, std::vector<std::vector<uint64_t> >& vecItemNum)
{
	if (vecRuneId.size() != vecLevel.size() || vecLevel.size() != vecRuneId.size())
	{
		return false;
	}

	for (size_t i = 0; i < vecItemId.size(); i++)
	{
		for (size_t j = 0; j < vecItemId[i].size(); j++)
		{
			strItemInfo itemInfo;
			useItem(vecItemId[i][j], itemInfo, vecItemNum[i][j]);
		}
	}
	for (size_t i = 0; i < vecRuneId.size(); i++)
	{
		strEquipInfo equipInfo;
		if (!m_data_cache.get_equipInfo(NULL, vecRuneId[i], NULL, equipInfo))
		{
			return false;
		}

		if (vecLevel[i] < 1)
		{
			vecLevel[i] = 1;
		}

		equipInfo.Exp = vecExp[i];
		equipInfo.Lv = vecLevel[i];

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameEquipInfo` set `Lv` = %llu, `Exp` = %llu where `ID` = %llu", vecLevel[i], vecExp[i], vecRuneId[i]);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameEquipInfo);
		m_data_cache.set_equipInfo(equipInfo.userId, equipInfo.ID, equipInfo.Base, equipInfo);
	}
	return true;
}

bool football_db_analyze_t::soulsoltUnlock(uint64_t userId, uint64_t slotId)
{
	return false;
}

bool football_db_analyze_t::soulslotLevelUp(uint64_t userId, uint64_t slotId, uint64_t costSP)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}
	userInfo.userSP -= costSP;

	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `userSP` = `userSP` -  %llu where `userId` = %llu", costSP, userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	if (slotId == 1)
	{
		roleInfo.soulSlotLevel0 += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlotLevel0` = `soulSlotLevel0` + 1 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 2)
	{
		roleInfo.soulSlotLevel1 += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlotLevel1` = `soulSlotLevel1` + 1 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 3)
	{
		roleInfo.soulSlotLevel2 += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlotLevel2` = `soulSlotLevel2` + 1 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}

	return true;
}

bool football_db_analyze_t::soulInlay(uint64_t userId, uint64_t slotId, uint64_t petId)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	if (slotId == 1)
	{
		roleInfo.soulSlot0 = petId;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot0` = %llu where `Role` = %llu;", petId, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 2)
	{
		roleInfo.soulSlot0 = petId;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot1` = %llu where `Role` = %llu;", petId, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 3)
	{
		roleInfo.soulSlot2 = petId;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot2` = %llu where `Role` = %llu;", petId, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else
	{
		return false;
	}

	return true;
}

bool football_db_analyze_t::soulTakeOut(uint64_t userId, uint64_t slotId)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	if (slotId == 1)
	{
		roleInfo.soulSlot0 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot0` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 2)
	{
		roleInfo.soulSlot0 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot1` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else if (slotId == 3)
	{
		roleInfo.soulSlot2 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlot2` = 0 where `Role` = %llu;", userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
	else
	{
		return false;
	}

	return true;
}

bool football_db_analyze_t::talentsoltUnlock(uint64_t roleId, uint64_t slotId)
{
	return true;
}

bool football_db_analyze_t::talentLock(uint64_t userId, uint64_t slotId)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	roleInfo.talentLock |= 0x01 << slotId;

	char buf[1024] = { 0 };
	sprintf(buf, "update `userRoleInfo` set `talentLock` = %d where `Role` = %llu;", roleInfo.talentLock, userInfo.userCurRole);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	return true;
}

bool football_db_analyze_t::talentUnlock(uint64_t userId, uint64_t slotId)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	roleInfo.talentLock &= ~(0x1 << slotId);

	char buf[1024] = { 0 };
	sprintf(buf, "update `userRoleInfo` set `talentLock` = %d where `Role` = %llu;", roleInfo.talentLock, userInfo.userCurRole);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	return true;
}

bool football_db_analyze_t::talentRefresh(uint64_t userId, uint64_t talentIndex, uint64_t itemId, uint64_t itemNumber, uint64_t& talentValue)
{
	strItemInfo itemInfo;
	if (!useItem(itemId, itemInfo, itemNumber))
	{
		return false;
	}

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	game_resource_t& config = singleton_t<game_resource_t>::instance();

	if (talentIndex == 1)
	{
		if (config.refreshTalentId(roleInfo.rolebase, userInfo.userTalentExchangLv, roleInfo.talent0))
		{
			talentValue = roleInfo.talent0;

			char buf[1024] = { 0 };
			sprintf(buf,"update `userRoleInfo` set `talent0` = %llu where `Role` = %llu", roleInfo.talent0, roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}
	}
	else if (talentIndex == 2)
	{
		if (config.refreshTalentId(roleInfo.rolebase, userInfo.userTalentExchangLv, roleInfo.talent1))
		{
			talentValue = roleInfo.talent1;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `talent1` = %llu where `Role` = %llu", roleInfo.talent1, roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}
	}
	else if (talentIndex == 3)
	{
		if (config.refreshTalentId(roleInfo.rolebase, userInfo.userTalentExchangLv, roleInfo.talent2))
		{
			talentValue = roleInfo.talent2;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `talent2` = %llu where `Role` = %llu", roleInfo.talent2, roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}
	}
	else if (talentIndex == 4)
	{
		if (config.refreshTalentId(roleInfo.rolebase, userInfo.userTalentExchangLv, roleInfo.talent3))
		{
			talentValue = roleInfo.talent3;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `talent3` = %llu where `Role` = %llu", roleInfo.talent3, roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}
	}
	else if (talentIndex == 5)
	{
		if (config.refreshTalentId(roleInfo.rolebase, userInfo.userTalentExchangLv, roleInfo.talent4))
		{
			talentValue = roleInfo.talent4;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `talent4` = %llu where `Role` = %llu", roleInfo.talent4, roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}
	}
	else
	{
		return false;
	}

	strUserTalentInfo userTalentInfo;
	if (m_data_cache.hget<strUserTalentInfo>(userInfo.userCurRole, userTalentInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `userTalentInfo` set `curTalentLevel` = %llu, `levelRateD` = %llu, `levelRateC` = %llu, `levelRateB` = %llu, `levelRateA` = %llu, `levelRateS` = %llu where `userId` = %llu",
			userTalentInfo.talentLevel, userTalentInfo.levelRateA, userTalentInfo.levelRateB, userTalentInfo.levelRateC, userTalentInfo.levelRateD, userTalentInfo.levelRateS, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userTalentInfo");
		m_data_cache.hset<strUserTalentInfo>(userInfo.userCurRole, userTalentInfo);
		return true;
	}

	game_util::updateRoleInfo(m_data_cache, &roleInfo);
	updateDatabaseRoleInfo(&roleInfo);
	return true;
}

bool football_db_analyze_t::talentBribe(uint64_t userId, uint64_t costType, uint64_t costNumber, uint64_t& talentLv)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	if (costType == 1)
	{
		userInfo.userGold -= costNumber;
		userInfo.userCostGold += costNumber;
		userInfo.userTalentExchangLv += 1;
		talentLv = userInfo.userTalentExchangLv;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` - %llu, `userCostGold` = `userCostGold` + %llu,`talentExchangeLv` = `talentExchangeLv` +1 where `userId` =%llu",
			costNumber, costNumber, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
		return true;
	}
	else if (costType == 2)
	{
		userInfo.userGem -= costNumber;
		userInfo.userCostGem += costNumber;
		userInfo.userTalentExchangLv += 1;
		talentLv = userInfo.userTalentExchangLv;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu, `talentExchangeLv` = `talentExchangeLv` +1 where `userId` =%llu",
			userInfo.userGem, userInfo.userCostGem, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
		return true;
	}

	return false;
}

bool football_db_analyze_t::initTalentRateData(strUserTalentInfo* ptrInfo)
{
	char buf[1024] = { 0 };
	sprintf(buf, "insert into `userTalentInfo`(`userId`, `curTalentLevel`, `levelRateS`, `LevelRateD`, `LevelRateC`, `LevelRateB`, `LevelRateA`) values(%llu, %llu, %llu, %llu, %llu, %llu, %llu)",
		ptrInfo->userId, ptrInfo->talentLevel, ptrInfo->levelRateS, ptrInfo->levelRateD, ptrInfo->levelRateC, ptrInfo->levelRateB, ptrInfo->levelRateA);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, "userTalentInfo");
	m_data_cache.hset<strUserTalentInfo>(ptrInfo->userId, *ptrInfo);
	return true;
}

bool football_db_analyze_t::updateTalentRateData(strUserTalentInfo* ptrInfo)
{
	char buf[1024] = { 0 };
	sprintf(buf, "update `userTalentInfo`set `curTalentLevel` = %llu, `levelRateS` = %llu, `LevelRateD` = %llu, `LevelRateC` = %llu, `LevelRateB` = %llu, `LevelRateA` = %llu where `userId` = %llu",
		ptrInfo->talentLevel, ptrInfo->levelRateS, ptrInfo->levelRateD, ptrInfo->levelRateC, ptrInfo->levelRateB, ptrInfo->levelRateA, ptrInfo->userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userTalentInfo");
	m_data_cache.hset<strUserTalentInfo>(ptrInfo->userId, *ptrInfo);
	return true;
}

bool football_db_analyze_t::drawCardReward(uint64_t userId, uint64_t costType, uint64_t costNumber, std::vector<strDrawCardItems>& items, uint64_t actionType, uint64_t drawTime, uint64_t givePoint, std::vector<uint64_t>& discountTime)
{
	if (costType != 0)
	{
		if (costType == 1) { this->costGold(userId, costNumber); }
		else if (costType == 2) { this->costGem(userId, costNumber); }
		else if (costType > 2)
		{ 
			strItemInfo tokenItemInfo;
			if (!this->useItem(userId, costType, tokenItemInfo, costNumber))
				return false;
		}
		else { return false; }
	}

	strDrawCardInfo drawCardInfo;
	if (!m_data_cache.get_drawCardInfo(userId, drawCardInfo))
	{
		drawCardInfo.userId = userId;
		drawCardInfo.drawTime0 = 0;
		drawCardInfo.drawTime1 = 0;
		drawCardInfo.drawTime2 = 0;
		drawCardInfo.drawTime3 = 0;
		drawCardInfo.drawTimes0 = 0;
		drawCardInfo.drawTimes1 = 0;
		drawCardInfo.drawTimes2 = 0;
		drawCardInfo.drawTimes3 = 0;
		drawCardInfo.givePoint = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `drawCardInfo`(`userId`) values ( %llu )", userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "drawCardInfo");
		m_data_cache.set_drawCardInfo(userId, drawCardInfo);
	}

	if (drawTime != 0)
	{
		if (actionType == DRAW_CARD_TYPE::GOLD_ONE)
		{
			drawCardInfo.drawTime0 = drawTime;
			drawCardInfo.drawTimes0++;
		}
		else if (actionType == DRAW_CARD_TYPE::GEM_ONE)
		{
			drawCardInfo.drawTime1 = drawTime;
			drawCardInfo.drawTimes1++;
		}
	}

	drawCardInfo.givePoint = givePoint;

	char buf[1024] = { 0 };
	sprintf(buf, "update `drawCardInfo` set `drawTime0` = %llu,`drawTime1` = %llu, `drawTime2` = %llu,`drawTime3` = %llu, \
				 `drawTimes0` = %llu, `drawTimes1` = %llu,  `drawTimes2` = %llu, `drawTimes3` = %llu, \
				 `givePoint` = %llu where `userId` = %llu;",
				 drawCardInfo.drawTime0, drawCardInfo.drawTime1, drawCardInfo.drawTime2, drawCardInfo.drawTime3,
				 drawCardInfo.drawTimes0, drawCardInfo.drawTimes1, drawCardInfo.drawTimes2, drawCardInfo.drawTimes3,
				 drawCardInfo.givePoint, userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "drawCardInfo");
	m_data_cache.set_drawCardInfo(userId, drawCardInfo);

	//!钻石十连抽折扣记录
	if (actionType == GEM_TEN)
	{
		drawCardInfo.discountTime = discountTime;
		std::string discountTimeStr;
		parseVector2String(discountTime, discountTimeStr);
		char buf[1024] = { 0 };
		sprintf(buf, "update `drawCardInfo` set `discountTime` = '%s' where `userId` = %llu;", discountTimeStr.c_str(), userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "drawCardInfo");
		m_data_cache.set_drawCardInfo(userId, drawCardInfo);
	}

	for (size_t i = 0; i < items.size(); i++)
	{
		if (items[i].type == 1)//宠物
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_petFragDecompositon_t*>& mapPetFragDecompositon = config.get_petFragDecomposition_config();
			std::map<uint64_t, config_petFragDecompositon_t*>::iterator iterPetDecompositon = mapPetFragDecompositon.find(items[i].baseId);
			if (iterPetDecompositon == mapPetFragDecompositon.end())
			{
				return false;
			}

			bool exist = false;
			strPetInfo petInfo;
			if (m_data_cache.get_petInfo(userId, NULL, iterPetDecompositon->second->m_PetID, petInfo))
			{
				//宠物分解成碎片
				strItemInfo itemInfo;
				addItemNumOrCreate(userId, iterPetDecompositon->second->m_FragmentID, itemInfo, iterPetDecompositon->second->m_FragmentCount);
				items[i].id = itemInfo.ID;
				items[i].decompose = 1;
				exist = true;
			}
			else
			{
				strPetInfo* newPetInfo = new strPetInfo;
				clear_strPetInfo(newPetInfo);

				newPetInfo->userId = userId;
				newPetInfo->base = iterPetDecompositon->second->m_PetID;
				newPetInfo->LV = iterPetDecompositon->second->m_Level;
				newPetInfo->Rank = iterPetDecompositon->second->m_Rank;
				newPetInfo->Star = iterPetDecompositon->second->m_Star;

				uint64_t newEquip = 0;
				set_new_equip_strenghten(newEquip, 1);
				set_new_equip_star(newEquip, 1);
				set_new_equip_rank(newEquip, 0);
				newPetInfo->NewEquip1 = newEquip;
				newPetInfo->NewEquip2 = newEquip;
				newPetInfo->NewEquip3 = newEquip;
				newPetInfo->NewEquip4 = newEquip;
				newPetInfo->NewEquip5 = newEquip;
				newPetInfo->NewEquip6 = newEquip;

				uint64_t newEquipPotential = 0;
				set_new_equip_potential(newEquipPotential, 1, 1);
				set_new_equip_potential(newEquipPotential, 2, 1);
				set_new_equip_potential(newEquipPotential, 3, 1);
				newPetInfo->NewEquipPotential = newEquipPotential;

				newPetInfo->petBinds.clear();
				for (size_t i = 0; i < Max_Pet_Bind_Size; i++)
				{
					newPetInfo->petBinds.push_back(0);
				}

				char buf[1024] = { 0 };
				sprintf(buf, "insert into `gamePetInfo` (`Base`, `userId`, `Lv`, `Rank`, `Star`) values(%d, %llu, %d, %d, %d)",
					iterPetDecompositon->second->m_PetID, userId, iterPetDecompositon->second->m_Level, iterPetDecompositon->second->m_Rank, iterPetDecompositon->second->m_Star);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GamePetInfo);
				newPetInfo->ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GamePetInfo);
				items[i].id = newPetInfo->ID;
				items[i].decompose = 0;

				updatePetSkillUnlockInfo(newPetInfo);
				game_util::updatePetInfo(m_data_cache, newPetInfo);
				updateDatabasePetInfo(newPetInfo);

				updateGetPetQuestState(newPetInfo->userId);
				updateWarStandardInfo(newPetInfo->userId);

				delete newPetInfo;
			}
		}
		else if (items[i].type == 3)//装备
		{
			strEquipInfo equipInfo;
			this->addEquipNumOrCreate(userId, items[i].baseId, equipInfo, items[i].number);
			items[i].id = equipInfo.ID;
		}
		else if (items[i].type == 2)//道具
		{
			//gold
			if (items[i].baseId == CURRENCY_GOLD)
			{
				this->gainGold(userId, items[i].number);
				items[i].id = CURRENCY_GOLD;
			}
			//gem
			else if (items[i].baseId == CURRENCY_GEM)
			{
				this->gainGem(userId, items[i].number);
				items[i].id = CURRENCY_GEM;
			}
			else
			{
				strItemInfo itemInfo;
				addItemNumOrCreate(userId, items[i].baseId, itemInfo, items[i].number);
				items[i].id = itemInfo.ID;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool football_db_analyze_t::useTitle(uint64_t userId, uint64_t titleId)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			roleInfo.curTitle = titleId;

			char buf[128] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `curTitle` = %llu where `Role` = %llu;", titleId, userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			return true;
		}
	}
	return false;
}

bool football_db_analyze_t::setPetCurPveGroup(uint64_t userId, uint64_t type)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		userInfo.CurPveTeam = type;

		char buf[128] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `CurPveTeam` = %llu where `userId` = %llu;", type, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
		return true;
	}
	return false;
}

bool football_db_analyze_t::setPetPveGroup(uint64_t userId, uint64_t type, uint64_t id1, uint64_t id2)
{
	strPetPveGroupInfo petPveGroup;
	if (!m_data_cache.get_petPveGroupInfo(userId, petPveGroup))
	{
		petPveGroup.team1_index1_pet_id = 0;
		petPveGroup.team1_index2_pet_id = 0;
		petPveGroup.team2_index1_pet_id = 0;
		petPveGroup.team2_index2_pet_id = 0;
		petPveGroup.team3_index1_pet_id = 0;
		petPveGroup.team3_index2_pet_id = 0;
	}

	if (type == 1)
	{
		if (petPveGroup.team1_index1_pet_id != id1)
		{
			strPetInfo petInfo;
			if (id1 > 0 && m_data_cache.get_petInfo(NULL, id1, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby1` = 1 where `ID` = %llu;", id1);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby1 = 1;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team1_index1_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team1_index1_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby1` = 0 where `ID` = %llu;", petPveGroup.team1_index1_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby1 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team1_index1_pet_id = id1;
		}

		if (petPveGroup.team1_index2_pet_id != id2)
		{
			strPetInfo petInfo;
			if (id2 > 0 && m_data_cache.get_petInfo(NULL, id2, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby1` = 2 where `ID` = %llu;", id2);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby1 = 2;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team1_index2_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team1_index2_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby1` = 0 where `ID` = %llu;", petPveGroup.team1_index2_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby1 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team1_index2_pet_id = id2;
		}
	}
	else if (type == 2)
	{
		if (id1 > 0 && petPveGroup.team2_index1_pet_id != id1)
		{
			strPetInfo petInfo;
			if (m_data_cache.get_petInfo(NULL, id1, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby2` = 1 where `ID` = %llu;", id1);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby2 = 1;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team2_index1_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team2_index1_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby2` = 0 where `ID` = %llu;", petPveGroup.team2_index1_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby2 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team2_index1_pet_id = id1;
		}

		if (petPveGroup.team2_index2_pet_id != id2)
		{
			strPetInfo petInfo;
			if (id2 > 0 && m_data_cache.get_petInfo(NULL, id2, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby2` = 2 where `ID` = %llu;", id2);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby2 = 2;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team2_index2_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team2_index2_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby2` = 0 where `ID` = %llu;", petPveGroup.team2_index2_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby2 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team2_index2_pet_id = id2;
		}
	}
	else if (type == 3)
	{
		if (petPveGroup.team3_index1_pet_id != id1)
		{
			strPetInfo petInfo;
			if (id1 > 0 && m_data_cache.get_petInfo(NULL, id1, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby3` = 1 where `ID` = %llu;", id1);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby3 = 1;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team3_index1_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team3_index1_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby3` = 0 where `ID` = %llu;", petPveGroup.team3_index1_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby3 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team3_index1_pet_id = id1;
		}

		if (petPveGroup.team3_index2_pet_id != id2)
		{
			strPetInfo petInfo;
			if (id2 > 0 && m_data_cache.get_petInfo(NULL, id2, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby3` = 2 where `ID` = %llu;", id2);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby3 = 2;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			if (petPveGroup.team3_index2_pet_id > 0 && m_data_cache.get_petInfo(NULL, petPveGroup.team3_index2_pet_id, NULL, petInfo))
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PveStandby3` = 0 where `ID` = %llu;", petPveGroup.team3_index2_pet_id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PveStandby3 = 0;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}

			petPveGroup.team3_index2_pet_id = id2;
		}
	}

	m_data_cache.set_petPveGroupInfo(userId, petPveGroup);
	return true;
}

bool football_db_analyze_t::setPetPvpGroup(uint64_t userId, uint64_t type, std::vector<uint64_t>& vecId)
{
	strPetPvpGroupInfo petPvpGroup;
	if (!m_data_cache.get_petPvpGroupInfo(userId, petPvpGroup))
	{
		return false;
	}

	if (type == 1)
	{
		setPetPvpGroup_one(&petPvpGroup, 1, 0, vecId);
		setPetPvpGroup_one(&petPvpGroup, 1, 1, vecId);
		setPetPvpGroup_one(&petPvpGroup, 1, 2, vecId);
		setPetPvpGroup_one(&petPvpGroup, 1, 3, vecId);
		setPetPvpGroup_one(&petPvpGroup, 1, 4, vecId);
		setPetPvpGroup_one(&petPvpGroup, 1, 5, vecId);
	}
	else if (type == 2)
	{
		setPetPvpGroup_one(&petPvpGroup, 2, 0, vecId);
		setPetPvpGroup_one(&petPvpGroup, 2, 1, vecId);
		setPetPvpGroup_one(&petPvpGroup, 2, 2, vecId);
		setPetPvpGroup_one(&petPvpGroup, 2, 3, vecId);
		setPetPvpGroup_one(&petPvpGroup, 2, 4, vecId);
		setPetPvpGroup_one(&petPvpGroup, 2, 5, vecId);
	}

	m_data_cache.set_petPvpGroupInfo(userId, petPvpGroup);
	return true;
}

void football_db_analyze_t::setPetPvpGroup_one(strPetPvpGroupInfo* petPvpGroupInfo, int type_, int index_, std::vector<uint64_t>& vecId)
{
	uint64_t pet_id = 0;
	getPetIDFromPvpGroup(petPvpGroupInfo, type_, index_, pet_id);

	if (pet_id != vecId[index_])
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(NULL, vecId[index_], NULL, petInfo))
		{
			if (type_ == 1)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PvpStandby1` = %d where `ID` = %llu;", index_, vecId[index_]);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PvpStandby1 = index_;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}
			else if (type_ == 2)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `PvpStandby2` = %d where `ID` = %llu;", index_, vecId[index_]);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.PvpStandby2 = index_;
				m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
			}
		}

		if (m_data_cache.get_petInfo(NULL, pet_id, NULL, petInfo))
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `PvpStandby1` = 0 where `ID` = %llu;", petPvpGroupInfo->team1_index1_pet_id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
			m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		}

		pet_id = vecId[index_];
	}
}

bool football_db_analyze_t::fetchQuestReward(uint64_t userId, uint64_t addExp, std::vector<uint64_t>&  itemId, std::vector<uint64_t>&  itemNum, uint64_t levelUp, uint64_t id, std::vector<uint64_t>& newItemId)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			if (levelUp == 0)
			{
				roleInfo.curexp_ += addExp;

				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `CurExp` = %llu where `Role` = %llu;", roleInfo.curexp_, userInfo.userCurRole);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
			}
			else
			{
				int oldLevel = roleInfo.level_;
				roleInfo.level_ = levelUp;
				roleInfo.curexp_ += addExp;

				game_resource_t& config = singleton_t<game_resource_t>::instance();
				std::map<uint64_t, config_playerAttrib_t*>& mapPlayerAttrib = config.get_playerAttrib_config();
				std::map<uint64_t, config_playerAttrib_t*>::iterator iterPlayerAttrib = mapPlayerAttrib.find(roleInfo.rolebase + 10000 * levelUp);
				if (iterPlayerAttrib != mapPlayerAttrib.end())
				{
					userInfo.userStrength += iterPlayerAttrib->second->m_LvlupStrRestore;

					char buf[1024] = { 0 };
					sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu;", userInfo.userStrength, userId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
				}

				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `CurExp` = %llu, `Level` = %llu where `Role` = %llu;", roleInfo.curexp_, roleInfo.level_, userInfo.userCurRole);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);

                updateRoleLevel(&roleInfo);
        		game_util::updateRoleInfo(m_data_cache, &roleInfo);
        		updateDatabaseRoleInfo(&roleInfo);
        		updateWarStandardInfo(userId);

				if (userInfo.inviterId > 0)
				{
					updateInviteCodeQuestState(userInfo.inviterId, oldLevel, levelUp);
				}
			}
		}

		for (size_t i = 0; i < itemId.size(); i++)
		{
			this->gainItem(itemId[i], itemNum[i], userInfo, newItemId);
		}

		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuest = mapQuest.begin();
		for (; iterQuest != mapQuest.end(); iterQuest++)
		{
			if (iterQuest->second->questId == id)
			{
				iterQuest->second->state = EQuestState::GetReward_Quest;
				m_db.hUpdateUserQuestList(userId, mapQuest);
				break;
			}
		}
	}
	MV_SAFE_RELEASE(mapQuest);
	return true;
}

bool football_db_analyze_t::fetchEndlessLevel(uint64_t userId, std::vector<uint64_t>& items, std::vector<uint64_t>& numbers, std::vector<uint64_t>& newItemID)
{
	if (items.size() != numbers.size())
	{
		return false;
	}
	this->gainItems(items, numbers, userId, newItemID);
	return true;
}

bool football_db_analyze_t::fetchMailAttach(uint64_t userId, std::vector<uint64_t>& items, std::vector<uint64_t>& numbers, std::vector<uint64_t>& newItemID)
{
	if (items.size() != numbers.size())
	{
		return false;
	}

	this->gainItems(items, numbers, userId, newItemID);

	return true;
}

bool football_db_analyze_t::fastFinishPve(uint64_t userId, uint64_t id, uint64_t keyNumber, uint64_t times, std::vector<uint64_t>& items, std::vector<uint64_t>& numbers,
	std::vector<uint64_t>& newItemID, uint64_t exp, uint64_t level, uint64_t levelUp, int userStrength)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	//strength
	userInfo.userStrength = std::max(0, (int)userInfo.userStrength - userStrength);

	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu;", userInfo.userStrength, userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

	//exp
	if (levelUp == 1)
	{
		int oldLevel = roleInfo.level_;
		roleInfo.level_ = level;
		roleInfo.curexp_ += exp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `Level` = %llu, `CurExp` = %llu where `Role` = %llu;", roleInfo.level_, roleInfo.curexp_, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

		updateRoleLevel(&roleInfo);
		game_util::updateRoleInfo(m_data_cache, &roleInfo);
		updateDatabaseRoleInfo(&roleInfo);
		updateWarStandardInfo(userId);

		if (userInfo.inviterId > 0)
		{
			updateInviteCodeQuestState(userInfo.inviterId, oldLevel, level);
		}
	}
	else if (exp > 0)
	{
		roleInfo.curexp_ += exp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `CurExp` = %llu where `Role` = %llu;", roleInfo.curexp_, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_SceneList_t*>& mapSceneList = config.get_sceneList_config();
	std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
	std::map<uint64_t, config_SceneList_t*>::iterator iterScene = mapSceneList.find(id);
	if (iterScene == mapSceneList.end())
		return false;

	std::map<uint64_t, config_levelSetup_t*>::iterator iterLevelSetup = mapLevelSetUp.find(id);
	if (iterLevelSetup == mapLevelSetUp.end())
		return false;

	uint64_t type = iterScene->second->m_Type;
	uint64_t subType = iterScene->second->m_SubType;

	if (type == LEVEL_MAIN_ENDLESS)//无尽之塔
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `ETKey` = `ETKey` - %llu, `CurETLevel` = %llu where `userId` = %llu;", keyNumber, id, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

		userInfo.ETKey -= keyNumber;
		userInfo.CurETLevel = id;
	}

	if (userInfo.ETProgress < id)
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `ETProgress` = %llu where `userId` = %llu;", id, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		userInfo.ETProgress = id;
	}

	//add items
	for (size_t i = 0; i < items.size(); i++)
	{
		this->gainItem(items[i], numbers[i], userInfo, newItemID);
	}

	m_data_cache.set_userInfo(userInfo.userId, userInfo);

	//update pve info
	strPveInfo pveInfo;
	if (m_data_cache.get_pveInfo(userId, id, pveInfo))
	{
		int timeNow = time(NULL);

		if (type == LEVEL_MAIN_TIME) //限时本,所有难度共享次数
		{
			for (int i = 10001; i <= 10042; i++)
			{
				std::map<uint64_t, config_SceneList_t*>::iterator itertamp = mapSceneList.find(i);
				if (itertamp != mapSceneList.end())
				{
					if (itertamp->second->m_SubType == subType && itertamp->second->m_Type == type)
					{
						strPveInfo pveInfo2;
						if (m_data_cache.get_pveInfo(userId, i, pveInfo2))
						{
							pveInfo2.remainCount -= times;
							pveInfo2.passCount += times;
							pveInfo2.lastTime = timeNow;

							char buf[1024] = { 0 };
							sprintf(buf, "update `userPveInfo` set `passCount` = %llu, `remainCount` = %llu, `lastTime` = %llu where `userId` = %llu and `pveType` = %d;",
								pveInfo2.passCount, pveInfo2.remainCount, pveInfo2.lastTime, userId, i);
							singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
							m_data_cache.set_pveInfo(userId, i, pveInfo2);
						}
					}
				}
			}
		}
		else
		{
			pveInfo.remainCount -= times;

			//99 == 无限次
			if (iterLevelSetup->second->m_CountLimit == 99)
				pveInfo.remainCount = 99;

			pveInfo.passCount += times;
			pveInfo.lastTime = timeNow;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userPveInfo` set `passCount` = %llu, `remainCount` = %llu, `lastTime` = %llu where `userId` = %llu and `pveType` = %llu;",
				pveInfo.passCount, pveInfo.remainCount, pveInfo.lastTime, userId, id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
			m_data_cache.set_pveInfo(userId, id, pveInfo);
		}
	}

	return true;
}


void football_db_analyze_t::updateRoleLevel(strRoleInfo* roleInfo)
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<uint64_t, config_playerBase_t*>& mapPlayerBase = config.get_playerbase_config();

	//角色栏位开放 前端处理

	//主角技能解锁 前端处理 SkillLearnCmd
	std::map<uint64_t, config_playerBase_t*>::iterator iterPlayerBase = mapPlayerBase.find(roleInfo->rolebase);
	if (iterPlayerBase != mapPlayerBase.end())
	{
		//宝石栏位解锁
		bool unlockArtifact = false;
		for (size_t slot = 1; slot <= 6; slot++)
		{
			int unlock = roleInfo->lockArtifactSlot & (1 << (slot - 1));
			int unlockLv = config.getArtifactUnlockLv(slot);
			int unlockCost = config.getArtifactUnlockCost(slot);
			if (unlock == 0 && unlockLv <= roleInfo->level_ && unlockCost == 0)
			{
				roleInfo->lockArtifactSlot |= (1ULL << (slot - 1));
				unlockArtifact = true;
			}
		}

		if (unlockArtifact)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `lockedArtifactSlot` = %d where `Role` = %llu;",
				roleInfo->lockArtifactSlot, roleInfo->role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		}

		//天赋解锁以及初始化
		ss_msg_update_talent_info info;
		updateRoleTalent(roleInfo, info.talentId, info.talentLock);
		info.roleId = roleInfo->role;
		send_user_talent_info(info);
		updateRoleSkillUnlockInfo(roleInfo);
	}
}

bool football_db_analyze_t::finishPve(std::vector<uint64_t>& items, std::vector<uint64_t>& numbers, uint64_t level, uint64_t levelUpOrNot,
	uint64_t roleAddExp, uint64_t roleLevel, uint64_t userId, uint64_t progress, uint64_t maxScore, std::vector<uint64_t>& id, int userStrength)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return false;
	}

	userInfo.userStrength = std::max(0, (int)userInfo.userStrength - userStrength);

	//update user strength and etprogress
	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu;", userInfo.userStrength, userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

	if (userInfo.ETProgress < level)
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `ETProgress` = %llu where `userId` = %llu;", level, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		userInfo.ETProgress = level;
	}

	//add role exp and update level
	strRoleInfo roleInfo;
	if (!m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
	{
		return false;
	}

	if (levelUpOrNot == 1)
	{
		int oldLevel = roleInfo.level_;
		roleInfo.level_ = roleLevel;
		roleInfo.curexp_ += roleAddExp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `Level` = %llu, `CurExp` = `CurExp` + %llu where `Role` = %llu;", roleLevel, roleAddExp, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

		updateRoleLevel(&roleInfo);
		game_util::updateRoleInfo(m_data_cache, &roleInfo);
		updateDatabaseRoleInfo(&roleInfo);
		updateWarStandardInfo(userId);

		if (userInfo.inviterId > 0)
		{
			updateInviteCodeQuestState(userInfo.inviterId, oldLevel, roleLevel);
		}
	}
	else if (roleAddExp > 0)
	{
		roleInfo.curexp_ += roleAddExp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `CurExp` = `CurExp` + %llu where `Role` = %llu;", roleAddExp, userInfo.userCurRole);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}

	//add items
	for (size_t i = 0; i < items.size(); i++)
	{
		this->gainItem(items[i], numbers[i], userInfo, id);
	}

	m_data_cache.set_userInfo(userInfo.userId, userInfo);

	//update pve info
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
	std::map<uint64_t, config_SceneList_t*>& mapSceneList = config.get_sceneList_config();
	std::map<uint64_t, config_SceneList_t*>::iterator iterScene = mapSceneList.find(level);
	if (iterScene == mapSceneList.end())
		return false;

	std::map<uint64_t, config_levelSetup_t*>::iterator iterLevelSetup= mapLevelSetUp.find(level);
	if (iterLevelSetup == mapLevelSetUp.end())
		return false;

	uint64_t subType = iterScene->second->m_SubType;
	uint64_t type = iterScene->second->m_Type;
	time_t timeNow = time(NULL);

#pragma region 判断是否存在 不存在就插入新的关卡信息

	strPveInfo pveInfo;
	if (!m_data_cache.get_pveInfo(userId, level, pveInfo))
	{
		int remainCount = iterLevelSetup->second->m_CountLimit;

		pveInfo.userId = userId;
		pveInfo.pveType = level;
		pveInfo.passCount = 1;
		pveInfo.maxScore = 0;
		pveInfo.lastScore = 0;
		pveInfo.lastTime = timeNow;
		pveInfo.remainCount = remainCount;
		pveInfo.progress = progress;
		pveInfo.fetched = 0;
		pveInfo.refreshCount = 0;

		if (type == LEVEL_MAIN_TIME) //限时本
		{
			for (int i = 10001; i <= 10042; i++)
			{
				std::map<uint64_t, config_SceneList_t*>::iterator itertamp = mapSceneList.find(i);
				if (itertamp == mapSceneList.end())
					continue;

				if (itertamp->second->m_SubType == subType && itertamp->second->m_Type == type)
				{
					strPveInfo pveInfo2;
					if (!m_data_cache.get_pveInfo(userId, i, pveInfo2))
					{
						pveInfo2 = pveInfo;
						pveInfo2.pveType = i;

						char buf[1024] = { 0 };
						sprintf(buf, "insert into `userPveInfo`(`userId`, `pveType`, `passCount`, `maxScore`, `lastScore`, `lastTime`, `remainCount`, `progress`, `fetched`) values(%llu, %llu, %llu, %llu, %llu, %llu, %d, %llu, 0)",
							pveInfo2.userId, pveInfo2.pveType, pveInfo2.passCount, pveInfo2.maxScore, pveInfo2.lastTime, pveInfo2.lastTime, remainCount, progress);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userPveInfo");
						m_data_cache.set_pveInfo(userId, i, pveInfo2);
					}
				}
			}
		}
		else
		{
			char buf[1024] = { 0 };
			sprintf(buf, "insert into `userPveInfo`(`userId`, `pveType`, `passCount`, `maxScore`, `lastScore`, `lastTime`, `remainCount`, `progress`, `fetched`) values(%llu, %llu, %llu, %llu, %llu, %llu, %d, %llu, 0)",
				pveInfo.userId, pveInfo.pveType, pveInfo.passCount, pveInfo.maxScore, pveInfo.lastTime, pveInfo.lastTime, remainCount, progress);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
			m_data_cache.set_pveInfo(userId, level, pveInfo);
		}
	}

#pragma endregion

	if (type == LEVEL_MAIN_TIME) //限时本
	{
		for (int i = 10001; i <= 10042; i++)
		{
			std::map<uint64_t, config_SceneList_t*>::iterator itertamp = mapSceneList.find(i);
			if (itertamp == mapSceneList.end())
				continue;

			if (itertamp->second->m_SubType == subType && itertamp->second->m_Type == type)
			{
				strPveInfo pveInfo2;
				if (m_data_cache.get_pveInfo(userId, i, pveInfo2))
				{
					pveInfo2.remainCount -= 1;
					pveInfo2.passCount += 1;
					pveInfo2.lastTime = timeNow;

					if (i == level && maxScore > pveInfo.maxScore)
					{
						pveInfo2.maxScore = maxScore;
						pveInfo2.lastScore = maxScore;
					}

					char buf[1024] = { 0 };
					sprintf(buf, "update `userPveInfo` set `passCount` = %llu, `remainCount` = %llu, `maxScore` = %llu, `lastScore` = %llu, `progress` = %llu, `lastTime` = %llu where `userId` = %llu and `pveType` = %d;",
						pveInfo2.passCount, pveInfo2.remainCount, pveInfo2.maxScore, pveInfo2.lastScore, pveInfo2.progress, pveInfo2.lastTime, userId, i);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
					m_data_cache.set_pveInfo(userId, i, pveInfo2);
				}
			}
		}
	}
	else //普通关卡
	{
		pveInfo.remainCount -= 1;

		if (maxScore > pveInfo.maxScore)
		{
			pveInfo.maxScore = maxScore;
			pveInfo.lastScore = maxScore;
		}

		if (progress > pveInfo.progress)
		{
			pveInfo.progress = progress;
		}

		pveInfo.passCount += 1;
		pveInfo.lastTime = timeNow;

		//99 == 无限次
		if (iterLevelSetup->second->m_CountLimit == 99)
		{
			pveInfo.remainCount = 99;
		}

		char buf[1024] = { 0 };
		sprintf(buf, "update `userPveInfo` set `passCount` = %llu, `remainCount` = %llu, `maxScore` = %llu, `lastScore` = %llu, `progress` = %llu, `lastTime` = %llu where `userId` = %llu and `pveType` = %llu;",
			pveInfo.passCount, pveInfo.remainCount, pveInfo.maxScore, pveInfo.lastScore, pveInfo.progress, pveInfo.lastTime, userId, level);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
		m_data_cache.set_pveInfo(userId, level, pveInfo);
	}

	return true;
}

bool football_db_analyze_t::createNewBaby(uint64_t templateId, uint64_t& babyId)
{
	char buf[128] = { 0 };
	sprintf(buf, "insert into gameBabyInfo(`babyTemplateId`) values ( %llu );", templateId);
	m_db.hset(buf);
	babyId = m_db.get_insert_id();
	return true;
}

void football_db_analyze_t::justForTest()
{
	uint64_t userId = 0, babyId = 0;
	for (int i = 0; i < 5000; i++)
	{
		char buf[32] = { 0 };
		sprintf(buf, "useraccount_%d", i);
		std::string account(buf);
		std::string passwd("123456");
		std::string deviceid("abcdefghijklmnopqrstuvwxyz");
		if (!registerNewAccount(account, passwd, deviceid, 0, 0, "", userId))
		{
			logerror((LOG_SERVER, "register user account error! userId[%llu]", userId));
		}

		if (!createNewBaby(i, babyId))
		{
			logerror((LOG_SERVER, "register baby error! babyId[%llu]", babyId));
		}

		logerror((LOG_SERVER, "register user account userid[%d]", userId));
	}
}

void football_db_analyze_t::initPetPVPRobot()
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	config_userName_t* userName = config.get_userName_config();
	if (userName)
	{
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterPetPvpBirthPlace = mapGlobal.find("PetPvpBirthPlace");
		if (iterPetPvpBirthPlace == mapGlobal.end())
		{
			logerror((LOG_SERVER, "PetPvpBirthPlace in global table not find!"));
			return;
		}

		int count = atoi(iterPetPvpBirthPlace->second.c_str());

		std::vector<std::string> vecUserName;
		config.getUserName(vecUserName, count);
		size_t indexName = 0;
		std::map<uint64_t, config_robot_t*>&mapRobot = config.get_robot_config();
		std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.begin();

		for (; iterRobot != mapRobot.end();++iterRobot)
		{
			if (iterRobot->second->m_PVPType == 1 && indexName < vecUserName.size())
			{
				char buf[1024] = { 0 };
				sprintf(buf, "insert into `userPetPvpInfo`(`userId`, `name`, `rank`, `lastTime`, `remainCount`, `level`, `exp`, `power`) values(%d, '%s', %d, 0, 0, 1, 0, %d)",
					iterRobot->second->m_ID, vecUserName[indexName].c_str(), iterRobot->second->m_Ranking, 0);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, "userPvePvpInfo");
				indexName++;
			}
		}
	}
}

void football_db_analyze_t::initRolePvpRobot()
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	config_userName_t* userName = config.get_userName_config();
	if (userName)
	{
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterTeamPvpBirthPlace = mapGlobal.find("TeamPvpBirthPlace");
		if (iterTeamPvpBirthPlace == mapGlobal.end())
		{
			logerror((LOG_SERVER, "TeamPvpBirthPlace in global table not find!"));
			return;
		}
		int count = atoi(iterTeamPvpBirthPlace->second.c_str());
		std::vector<std::string> vecUserName;
		config.getUserName(vecUserName, count);
		size_t indexName = 0;
		std::map<uint64_t, config_robot_t*>&mapRobot = config.get_robot_config();
		std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.begin();

		for (; iterRobot != mapRobot.end(); ++iterRobot)
		{
			if (iterRobot->second->m_PVPType == 2 && indexName < vecUserName.size())
			{
				char buf[1024] = { 0 };
				sprintf(buf, "insert into `userRolePvpInfo`(`userId`, `name`, `rank`, `lastTime`, `remainCount`, `level`, `exp`, `power`, `battleCount`) values(%d, '%s', %d, 0, 0, 1, 0, %d, 0)",
					iterRobot->second->m_ID, vecUserName[indexName].c_str(), iterRobot->second->m_Ranking, 0);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserRolePvpInfo);
				indexName++;
			}
		}
	}
}

void football_db_analyze_t::initMineAllInfo()
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_mineMap_t*>& mineMap = config.get_mineMap_config();
	for (std::map<uint64_t, config_mineMap_t*>::iterator iter = mineMap.begin(); iter != mineMap.end(); iter++)
	{
		strMineAllInfo pMineAll;
		pMineAll.id = iter->first;
		pMineAll.mineSpot = iter->second->m_MineSpot;
		pMineAll.baseId = iter->second->m_MineID;
		pMineAll.owner = iter->second->m_RobotID;
		pMineAll.teamId = 0;
		pMineAll.status = 0;
		pMineAll.startTime = 0;
		pMineAll.lastFetchTime = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `mineAllInfo`(`id`, `mineSpot`, `baseId`, `owner`, `teamIndex`, `status`, `startTime`, `lastFetchTime`) values(%llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu);",
			pMineAll.id, pMineAll.mineSpot, pMineAll.baseId, pMineAll.owner, pMineAll.teamId, pMineAll.status, pMineAll.startTime, pMineAll.lastFetchTime);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_MineAllInfo);
		m_data_cache.set_mineInfo(pMineAll.id, pMineAll);
	}
}

void football_db_analyze_t::initMineOpenAreaInfo()
{
	std::vector<uint64_t> mineOpenAreaInfo;
	mineOpenAreaInfo.push_back(1001);
	mineOpenAreaInfo.push_back(1002);
	mineOpenAreaInfo.push_back(1003);
	mineOpenAreaInfo.push_back(1004);
	mineOpenAreaInfo.push_back(1005);
	m_db.hinitMineOpenEreaInfo(mineOpenAreaInfo);
}

void football_db_analyze_t::initRobotName()
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	config_userName_t* userName = config.get_userName_config();
	if (userName)
	{
		std::vector<std::string> userName;
		config.getAllName(userName);
		size_t indexName = 0;
		std::map<uint64_t, config_robot_t*>&mapRobot = config.get_robot_config();
		std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.begin();
		for (size_t i = 0; i < userName.size(); i++)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "insert into `robotName`(`name`) values('%s')", userName[i].c_str());
			//m_db.hset(buf);
			m_data_cache.set_robotName(i, userName[i]);
			indexName++;
		}
	}
}

//void football_db_analyze_t::loadUserName()
//{
//	m_db.hgetUserName(/*m_cache_userName_map*/);
//}

void football_db_analyze_t::loadUserRoleInfo()
{
	uint64_t lastInsertID = m_db.hgetUserRoleInfo(/*m_cache_roleInfo_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_UserRoleInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadUserInfo()
{
	uint64_t lastInsertID = m_db.hgetUserInfo(/*m_cache_userInfo_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GameUserInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadBabyInfo()
{
	uint64_t lastInsertID = m_db.hgetBabyInfo(/*m_cache_pet_map, m_cache_petPvpGroupInfo_map, m_cache_petPveGroupInfo_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GamePetInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadEquipInfo()
{
	uint64_t lastInsertID = m_db.hgetEquipInfo(/*m_cache_equip_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GameEquipInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadSkillInfo()
{
	uint64_t lastInsertID = m_db.hgetSkillInfo(/*m_cache_skill_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GameSkillInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadPetItemInfo()
{
	uint64_t lastInsertID = m_db.hgetPetItemInfo(/*m_cache_item_map*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GameItemInfo, lastInsertID, 200000);
}

void football_db_analyze_t::loadDrawCardInfo()
{
	m_db.hgetDrawCardInfo();
}

void football_db_analyze_t::loadPveInfo()
{
	m_db.hgetPveInfo(/*m_cache_pveInfo_map*/);
}

void football_db_analyze_t::loadWorldBossInfo()
{
	m_db.hgetWorldBossInfo(/*m_cache_worldBossInfo_map*/);
}
bool football_db_analyze_t::loadPetPvpInfo()
{
	return m_db.hgetPetPvpInfo(/*m_cache_petPvpInfo_map, m_cache_petPvpRankList*/);
}
void football_db_analyze_t::loadPetPvpHistory()
{
	m_db.hgetPetPvpHistory(/*m_cache_petPvpHistory_map*/);
}
void football_db_analyze_t::loadUserDelegateQuestList()
{
	m_db.hgetUserDelegateQuestList(/*m_cache_delegateQuest_map*/);
}
void football_db_analyze_t::loadUserQuestList()
{
	m_db.hgetUserQuestList(/*m_cache_questList_map*/);
}
void football_db_analyze_t::loadUserTalentInfo()
{
	m_db.hgetUserTalentInfo(/*m_cache_talent_map*/);
}
void football_db_analyze_t::loadUserMailsInfo()
{
	m_db.hgetUserMailsInfo(/*m_cache_mails_map*/);
}
void football_db_analyze_t::loadUserPetPvpTeamInfo()
{
	m_db.hgetUserPetPvpTeamInfo(/*m_cache_petPvpTeam_map*/);
}
void football_db_analyze_t::loadPetEndlessFightingInfo()
{
	m_db.hgetUserPetEndlessFightingInfo(/*m_cache_petEndlessFightingInfo*/);
}
void football_db_analyze_t::loadBossBattleInfo()
{
	m_db.hgetUserBossBattleInfo(/*m_cache_boss_battle_info*/);
}
void football_db_analyze_t::loadMorpherInfo()
{
	uint64_t lastInsertID =	m_db.hgetUserMorpherInfo(/*m_cache_morpher_info*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_UserMorpherInfo, lastInsertID, 100000);
}
void football_db_analyze_t::loadWarStandardInfo()
{
	uint64_t lastInsertID = m_db.hgetUserWarStandardInfo(/*m_cache_war_standard_info*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_UserWarStandardInfo, lastInsertID, 100000);
}

void football_db_analyze_t::loadUserPvePetFragmentInfo()
{
	m_db.hgetUserPvePetFragmentInfo(/*m_cache_pvePetFragment_info*/);
}

bool football_db_analyze_t::loadRobotName()
{
	return m_db.hgetRobotName(/*m_cache_robotName*/);
}

void football_db_analyze_t::loadPveStarRewardInfo()
{
	m_db.hgetPveSartRewardInfo(/*m_cache_pveStarRewardInfo*/);
}

bool football_db_analyze_t::loadUserRolePvpInfo()
{
	return m_db.hgetUserRolePvpInfo(/*m_cache_rolePvpInfo, m_cache_rolePvpRankList*/);
}

void football_db_analyze_t::loadUserRolePvpBattleHistory()
{
	m_db.hgetUserRolePvpBattleHistory(/*m_cache_rolePvpBattleHistory*/);
}

void football_db_analyze_t::loadUserRolePvpTeamInfo()
{
	m_db.hgetUserRolePvpTeamInfo(/*m_cache_rolePvpTeamInfo*/);
}

void football_db_analyze_t::loadRolePvpRankListReward()
{
	m_db.hgetRolePvpRankListReward(/*m_cache_rolePvpRankListReward*/);
}

void football_db_analyze_t::loadRechargeInfo()
{
	m_db.hgetRechargeInfo(/*m_cache_rechargeInfo*/);
}

void football_db_analyze_t::loadMineTeamInfo()
{
	m_db.hgetMineTeamInfo(/*m_cache_mineTeamInfo*/);
}

void football_db_analyze_t::loadMineBattleHistory()
{
	m_db.hgetMineHistory(/*m_cache_mineHistory*/);
}

bool football_db_analyze_t::loadMineAllInfo()
{
	return m_db.hgetMineAllInfo(/*m_cache_mineAllInfo*/);
}

void football_db_analyze_t::loadMineRevengeInfo()
{
	m_db.hgetMineRevengeInfo(/*m_cache_mineRevengeInfo*/);
}

void football_db_analyze_t::loadFriendInfo()
{
	m_db.hgetFriendInfo(/*m_cache_friendInfo*/);
}

void football_db_analyze_t::loadGuildInfo()
{
	uint64_t lastInsertID = m_db.hgetGuildInfo(/*m_cache_guildInfo*/);
	singleton_t<db_request_list>::instance().initInsertMap(DB_GameGuildInfo, lastInsertID, 100000);

	m_db.hgetGuildBossInfo();
	m_db.hgetGuildBossUserData();
}

void football_db_analyze_t::laodCelebrationInfo()
{
	m_db.hGetUserCelebrationInfo(/*m_cache_celebrationInfo*/);
}

bool football_db_analyze_t::loadGlobalInfo()
{
	return m_db.hGetGlobalInfo(/*m_cache_mineOpenAreaInfo*/);
}

void football_db_analyze_t::loadShopInfo()
{
	m_db.hGetShopInfo(/*m_cache_shopInfo*/);
}

void football_db_analyze_t::loadOrderInfo()
{
	m_db.hgetOrderInfo();
}

void football_db_analyze_t::send_add_role_skill_info(ss_msg_role_add_skill& info)
{
	BinaryWriteStream writeStream;
	writeStream << info;

	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
	socket_ptr_t socket_ptr_ = engine.get_gameserver_con();
	if (socket_ptr_)
	{
		socket_ptr_->async_write(writeStream);
	}
}

void football_db_analyze_t::send_war_standard_info(ss_msg_add_war_standard& info)
{
	BinaryWriteStream writeStream;
	writeStream << info;

	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
	socket_ptr_t socket_ptr_ = engine.get_gameserver_con();
	if (socket_ptr_)
	{
		socket_ptr_->async_write(writeStream);
	}
}

//void football_db_analyze_t::send_user_init_items(ss_msg_init_user_item& info)
//{
//	BinaryWriteStream writeStream;
//	writeStream << info;
//
//	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
//	socket_ptr_t socket_ptr_ = engine.get_gameserver_con();
//	if (socket_ptr_)
//	{
//		socket_ptr_->async_write(writeStream);
//	}
//}

void football_db_analyze_t::send_user_talent_info(ss_msg_update_talent_info& info)
{
	BinaryWriteStream writeStream;
	writeStream << info;

	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
	socket_ptr_t socket_ptr_ = engine.get_gameserver_con();
	if (socket_ptr_)
	{
		socket_ptr_->async_write(writeStream);
	}
}

//void football_db_analyze_t::send_mine_reward_info(ss_msg_update_mine_status_and_reward& info)
//{
//	BinaryWriteStream writeStream;
//	writeStream << info;
//
//	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
//	socket_ptr_t socket_ptr_ = engine.get_gameserver_con();
//	if (socket_ptr_)
//	{
//		socket_ptr_->async_write(writeStream);
//	}
//}

MY_FUNCTION1(get_friend_captain_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
// 	ss_msg_get_friend_captain_info_req msg;
// 	readStream >> msg;
//
// 	ss_msg_get_friend_captain_info_ret ret;
//
// 	ret.userId = msg.userId;
// 	ret.friendUid = msg.friendUid;
//
// 	BinaryWriteStream writeStream;
// 	writeStream << ret;
// 	socket_->async_write(writeStream);
}
MY_FUNCTION1(register_account)
{
	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
	engine.set_gameserver_con(socket_);

	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_register_account_req reg_req;
	readStream >> reg_req;

	ss_msg_register_account_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.sName = reg_req.sName;
	ret.sPassWd = reg_req.sPassWd;
	ret.userId = 0;
	ret.socket_id = reg_req.socket_id;
	ret.index_id = reg_req.index_id;

	uint64_t newUid = 0;
	if (registerNewAccount(reg_req.sName, reg_req.sPassWd, reg_req.sDeviceId, reg_req.u8UserId, reg_req.sdkUserId, reg_req.sdkUserName, newUid))
	{
		ret.nResult = (int)RET_SUCCESS;
		ret.userId = newUid;
		ret.sDeviceId = reg_req.sDeviceId;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(sdk_login)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_login_gt_req req;
	readStream >> req;

	ss_msg_login_gt_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.sName = req.sName;
	ret.sDeviceId = req.sDeviceId;
	ret.nResult = (int)RET_REQ_DATA_ERROR;

	uint64_t newUid = 0;
	if (registerNewAccount(req.sName, "", req.sDeviceId, req.u8UserId, req.sdkUserId, req.sdkUserName, newUid))
	{
		ret.nResult = (int)RET_SUCCESS;
		ret.uid = newUid;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(create_role)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_create_role_req create_role_req;
	readStream >> create_role_req;

	ss_msg_create_role_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = create_role_req.socket_id;
	ret.templateId = create_role_req.userTemplateId;
	ret.index_id = create_role_req.index_id;
	ret.userId = create_role_req.userId;

	ss_msg_update_talent_info info;
	uint64_t newRole = 0;
	if (registerCreateRole(create_role_req.userId, create_role_req.userTemplateId, newRole, info.talentId, info.talentLock))
	{
		ret.nResult = (int)RET_SUCCESS;
		ret.roleId = newRole;
	}

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.userName = create_role_req.name;
		m_data_cache.set_userIdByName(create_role_req.name, create_role_req.userId);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userName` = '%s' where `userId` = %llu", create_role_req.name.c_str(), create_role_req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	info.roleId = newRole;
	send_user_talent_info(info);
}

MY_FUNCTION1(delete_role)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_delete_role_req delete_role_req;
	readStream >> delete_role_req;

	ss_msg_delete_role_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = delete_role_req.socket_id;
	ret.roleId = delete_role_req.roleId;
	ret.index_id = delete_role_req.index_id;

	if (deleteRole(delete_role_req.roleId))
	{
		ret.nResult = (int)RET_SUCCESS;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(enter_game)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_enter_game_req req;
	readStream >> req;

	ss_msg_enter_game_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = req.socket_id;
	ret.roleId = req.roleId;
	ret.index_id = req.index_id;
	ret.userId = req.userId;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, req.roleId, NULL, roleInfo))
		{
			userInfo.userCurRole = req.roleId;

			if (userInfo.lastRefreshStrength > 0)
			{
				game_resource_t& config = singleton_t<game_resource_t>::instance();
				std::map<std::string, std::string>& mapGlobal = config.get_global_config();
				std::map<std::string, std::string>::iterator iterLimit = mapGlobal.find("StaminaSoftLimit");
				std::map<std::string, std::string>::iterator iterRecoverTime = mapGlobal.find("StaminaRecoverTime");

				if (iterLimit != mapGlobal.end() && iterRecoverTime != mapGlobal.end())
				{
					uint64_t staminaSoftLimit = atol(iterLimit->second.c_str());
					uint64_t staminaRecoverTime = atol(iterRecoverTime->second.c_str());

					if (userInfo.userStrength < staminaSoftLimit)
					{
						time_t now = time(NULL);
						uint64_t diff = now - userInfo.lastRefreshStrength;
						uint64_t amount = diff / (staminaRecoverTime * 60);

						if (amount > 0)
						{
							userInfo.lastRefreshStrength = now;
							userInfo.userStrength += amount;
							userInfo.userStrength = min(userInfo.userStrength, staminaSoftLimit);
						}
					}
				}
			}

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userCurRole` = %llu, `userStrength` = %llu, `lastRefreshStrength` = %llu where `userId` = %llu;",
				req.roleId, userInfo.userStrength, userInfo.lastRefreshStrength, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
			ret.nResult = (int)RET_SUCCESS;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	proxy_engine_t& engine = singleton_t<proxy_engine_t>::instance();
	engine.set_gameserver_con(socket_);
}

MY_FUNCTION1(unlock_role_slots)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_unlock_role_slot_req unlock_role_slots_req;
	readStream >> unlock_role_slots_req;

	ss_msg_unlock_role_slot_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = unlock_role_slots_req.socket_id;
	ret.userId = unlock_role_slots_req.userId;
	ret.index_id = unlock_role_slots_req.index_id;
	ret.costGem = unlock_role_slots_req.costGem;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(unlock_role_slots_req.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
	}
	else
	{
		if (unlockRoleSlots(unlock_role_slots_req.userId))
		{
			if (unlock_role_slots_req.costGem != 0)
			{
				userInfo.userCostGem += unlock_role_slots_req.costGem;
				userInfo.userGem -= unlock_role_slots_req.costGem;

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu",
					userInfo.userGem, userInfo.userCostGem, unlock_role_slots_req.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			}
			userInfo.userUnLock += 1;
			ret.nResult = (int)RET_SUCCESS;
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
		else
		{
			ret.nResult = (int)RET_USER_INFO_EMPTY;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(buy_strength)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_strength_req req;
	readStream >> req;

	ss_msg_buy_strength_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.index_id = req.index_id;
	ret.gemCost = req.gemCost;
	ret.strengthCount = req.strengthCount;
	ret.timeBuy = req.timeBuy;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(req.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		ret.strengthNum = 0;
		ret.strengthTime = 0;
	}
	else
	{
		userInfo.userStrength += req.strengthCount;
		userInfo.userCostGem += req.gemCost;
		userInfo.userGem -= req.gemCost;
		userInfo.userBuyStrengthCount++;
		userInfo.userStrengthTime = req.timeBuy;

		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userStrength` = %llu, `userGem` = %llu, `userCostGem` = %llu, `userBuyStrengthCount` = %llu, `userStrengthTime` = %llu where `userId` = %llu;",
			userInfo.userStrength, userInfo.userGem, userInfo.userCostGem, userInfo.userBuyStrengthCount, req.timeBuy, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);

		ret.nResult = (int)RET_SUCCESS;
		updateDailyTaskCountInfo(req.userId, dailyTaskCount::buyStrength_type, 1);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(switch_role)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_switch_role_req switch_role_req;
	readStream >> switch_role_req;

	ss_msg_switch_role_ret ret;
	ret.nResult = (int)RET_FAILED;
	ret.socket_id = switch_role_req.socket_id;
	ret.userId = switch_role_req.userId;
	ret.roleId = switch_role_req.roleId;
	ret.index_id = switch_role_req.index_id;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(switch_role_req.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
	}
	else
	{
		if (switchRole(userInfo, switch_role_req.roleId))
		{
			ret.nResult = (int)RET_SUCCESS;
		}
		else
		{
			ret.nResult = (int)RET_USER_INFO_EMPTY;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(buy_package)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_package_page_req buyPackagePageReq;
	readStream >> buyPackagePageReq;

	ss_msg_buy_package_page_ret buyPackagePageRet;
	buyPackagePageRet.socket_id = buyPackagePageReq.socket_id;
	buyPackagePageRet.userId = buyPackagePageReq.userId;
	buyPackagePageRet.index_id = buyPackagePageReq.index_id;
	buyPackagePageRet.nResult = (int)RET_FAILED;;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(buyPackagePageReq.userId, userInfo))
	{
		if (buyPackageSize(userInfo))
		{
			buyPackagePageRet.nResult = (int)RET_SUCCESS;
		}
		else
		{
			buyPackagePageRet.nResult = (int)RET_USER_INFO_EMPTY;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << buyPackagePageRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_summon)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_summon_req req;
	readStream >> req;

	ss_msg_pet_summon_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.petBase = req.petBase;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.nResult = (int)RET_FAILED;
	ret.petId = 0;

	strPetInfo petinfo;
	if (!m_data_cache.get_petInfo(req.userId, NULL, req.petBase, petinfo))
	{
		if (petSummon(ret.userId, ret.petBase, ret.itemId, ret.itemNum, ret.petId))
		{
			ret.nResult = (int)RET_SUCCESS;
		}
	}
	else
	{
		ret.nResult = (int)RET_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_levelup)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_level_up_req petLevelUpReq;
	readStream >> petLevelUpReq;

	ss_msg_pet_level_up_ret petLevelUpRet;
	petLevelUpRet.userId = petLevelUpReq.userId;
	petLevelUpRet.socket_id = petLevelUpReq.socket_id;
	petLevelUpRet.index_id = petLevelUpReq.index_id;
	petLevelUpRet.petId = petLevelUpReq.petId;
	petLevelUpRet.newExp = petLevelUpReq.newExp;
	petLevelUpRet.newLevel = petLevelUpReq.newLevel;
	petLevelUpRet.itemId = petLevelUpReq.itemId;
	petLevelUpRet.itemNum = petLevelUpReq.itemNum;
	petLevelUpRet.nResult = (int)RET_FAILED;

	strPetInfo petInfo;
	if (!m_data_cache.get_petInfo(NULL, petLevelUpReq.petId, NULL, petInfo))
	{
		petLevelUpRet.nResult = (int)RET_USER_INFO_EMPTY;
	}
	else
	{
		config_inst.rectify_pet_exp_from_level(petInfo.base, petInfo.LV, petInfo.EXP);

		if (petLevelUp(petLevelUpReq.petId, petLevelUpReq.newLevel, petLevelUpReq.newExp, petLevelUpReq.itemId, petLevelUpReq.itemNum))
		{
			updatePetLevelUpQuestState(petLevelUpRet.userId, petInfo.LV, petLevelUpReq.newLevel);
			petInfo.LV = petLevelUpReq.newLevel;
			petInfo.NextExp = petLevelUpReq.newExp;
			petLevelUpRet.nResult = (int)RET_SUCCESS;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << petLevelUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_equip)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_equip_req petEquipReq;
	readStream >> petEquipReq;

	ss_msg_equip_ret petEquipRet;
	petEquipRet.unitType = petEquipReq.unitType;
	petEquipRet.socket_id = petEquipReq.socket_id;
	petEquipRet.index_id = petEquipReq.index_id;
	petEquipRet.petId = petEquipReq.petId;
	petEquipRet.equipId = petEquipReq.equipId;
	petEquipRet.equip_index = petEquipReq.equip_index;
	petEquipRet.nResult = (int)RET_FAILED;

	if (petEquipRet.unitType == 1)
	{
		strPetInfo petInfo;
		if (!m_data_cache.get_petInfo(NULL, petEquipRet.petId, NULL, petInfo))
		{
			petEquipRet.nResult = (int)RET_USER_INFO_EMPTY;
		}
		else
		{
			if (petEquip(petEquipReq.petId, petEquipReq.equipId, petEquipReq.equip_index))
			{
				petEquipRet.nResult = (int)RET_SUCCESS;
			}
		}
	}
	else if (petEquipRet.unitType == 0)
	{
		strRoleInfo roleInfo;
		if (!m_data_cache.get_roleInfo(NULL, petEquipRet.petId, NULL, roleInfo))
		{
			petEquipRet.nResult = (int)RET_USER_INFO_EMPTY;
		}
		else
		{
			if (roleEquip(petEquipReq.petId, petEquipReq.equipId, petEquipReq.equip_index))
			{
				petEquipRet.nResult = (int)RET_SUCCESS;
			}
		}
	}

	BinaryWriteStream writeStream;
	writeStream << petEquipRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_equip_combin)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_equip_combin_req petEquipCombinReq;
	readStream >> petEquipCombinReq;

	ss_msg_equip_combin_ret petEquipCombinRet;
	petEquipCombinRet.socket_id = petEquipCombinReq.socket_id;
	petEquipCombinRet.index_id = petEquipCombinReq.index_id;
	petEquipCombinRet.equipBase = petEquipCombinReq.equipBase;
	petEquipCombinRet.equip_combin_vec = petEquipCombinReq.equip_combin_vec;
	petEquipCombinRet.userId = petEquipCombinReq.userId;
	petEquipCombinRet.nResult = (int)RET_FAILED;
	petEquipCombinRet.combinCost = petEquipCombinReq.combinCost;

	if (petPetEquipCombin(petEquipCombinReq.userId, petEquipCombinReq.equipBase, petEquipCombinReq.equip_combin_vec.vecCombinItem, petEquipCombinReq.equip_combin_vec.vecCombinItemCount,
		petEquipCombinReq.combinCost, petEquipCombinRet.newEquipId))
	{
		petEquipCombinRet.nResult = (int)RET_SUCCESS;
	}

	BinaryWriteStream writeStream;
	writeStream << petEquipCombinRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_rank)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_rank_up_req petRankUpReq;
	readStream >> petRankUpReq;

	ss_msg_rank_up_ret petRankUpRet;
	petRankUpRet.socket_id = petRankUpReq.socket_id;
	petRankUpRet.index_id = petRankUpReq.index_id;
	petRankUpRet.unitId = petRankUpReq.unitId;
	petRankUpRet.unitType = petRankUpReq.unitType;
	petRankUpRet.userId = petRankUpReq.userId;
	petRankUpRet.costGold = petRankUpReq.costGold;
	petRankUpRet.nResult = (int)RET_FAILED;
	if (petRankUpRet.unitType == 1)
	{
		strPetInfo petInfo;
		if (!m_data_cache.get_petInfo(NULL, petRankUpReq.unitId, NULL, petInfo))
		{
			petRankUpRet.nResult = (int)RET_USER_INFO_EMPTY;
		}
		else
		{
			if (petRankUp(petRankUpReq.unitId, petRankUpReq.userId, petRankUpReq.costGold))
			{
				petRankUpRet.nResult = (int)RET_SUCCESS;
			}
		}
	}
	else if (petRankUpRet.unitType == 0)
	{
		strRoleInfo roleInfo;
		if (!m_data_cache.get_roleInfo(NULL, petRankUpReq.unitId, NULL, roleInfo))
		{
			petRankUpRet.nResult = (int)RET_USER_INFO_EMPTY;
		}
		else
		{
			if (roleRankUp(petRankUpReq.unitId, petRankUpReq.userId, petRankUpReq.costGold))
			{
				petRankUpRet.nResult = (int)RET_SUCCESS;
			}
		}
	}

	BinaryWriteStream writeStream;
	writeStream << petRankUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_starup)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_star_up_req petStarUpReq;
	readStream >> petStarUpReq;

	ss_msg_pet_star_up_ret petStarUpRet;
	petStarUpRet.socket_id = petStarUpReq.socket_id;
	petStarUpRet.index_id = petStarUpReq.index_id;
	petStarUpRet.petId = petStarUpReq.petId;
	petStarUpRet.nResult = (int)RET_FAILED;

	petStarUpRet.costGold = petStarUpReq.costGold;
	petStarUpRet.userId = petStarUpReq.userId;
	petStarUpRet.itemId = petStarUpReq.itemId;
	petStarUpRet.itemCount = petStarUpReq.itemCount;
	petStarUpRet.attriInfo = petStarUpReq.attriInfo;

	if (petStarUp(petStarUpReq.petId, petStarUpReq.userId, petStarUpReq.costGold, petStarUpReq.itemId, petStarUpReq.itemCount,petStarUpReq.attriInfo))
	{
		petStarUpRet.nResult = (int)RET_SUCCESS;
	}

	BinaryWriteStream writeStream;
	writeStream << petStarUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_aritfact_choose)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_artifact_choose_req petArtifactChooseReq;
	readStream >> petArtifactChooseReq;

	ss_msg_pet_artifact_choose_ret petArtifactChooseRet;
	petArtifactChooseRet.socket_id = petArtifactChooseReq.socket_id;
	petArtifactChooseRet.index_id = petArtifactChooseReq.index_id;
	petArtifactChooseRet.petId = petArtifactChooseReq.petId;
	petArtifactChooseRet.artifactId = petArtifactChooseReq.artifactId;
	petArtifactChooseRet.userId = petArtifactChooseReq.userId;
	petArtifactChooseRet.costType = petArtifactChooseReq.costType;
	petArtifactChooseRet.costNum = petArtifactChooseReq.costNum;
	petArtifactChooseRet.nResult = (int)RET_FAILED;

	if (petArtifactChoose(petArtifactChooseReq.petId, petArtifactChooseReq.artifactId, petArtifactChooseReq.userId, petArtifactChooseRet.costType, petArtifactChooseRet.costNum))
	{
		petArtifactChooseRet.nResult = (int)RET_SUCCESS;
	}

	BinaryWriteStream writeStream;
	writeStream << petArtifactChooseRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_artifact_rank_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_artifact_rank_up_req petArtifactResetReq;
	readStream >> petArtifactResetReq;

	ss_msg_pet_artifact_rank_up_ret petArtifactResetRet;
	petArtifactResetRet.socket_id = petArtifactResetReq.socket_id;
	petArtifactResetRet.index_id = petArtifactResetReq.index_id;
	petArtifactResetRet.petId = petArtifactResetReq.petId;
	petArtifactResetRet.userId = petArtifactResetReq.userId;
	petArtifactResetRet.itemId = petArtifactResetReq.itemId;
	petArtifactResetRet.itemNum = petArtifactResetReq.itemNum;
	petArtifactResetRet.newArtifact = petArtifactResetReq.newArtifact;
	petArtifactResetRet.nResult = (int)RET_FAILED;

	if (petArtifactRankUp(petArtifactResetReq.petId, petArtifactResetReq.userId, petArtifactResetReq.itemId, petArtifactResetReq.itemNum, petArtifactResetReq.newArtifact))
	{
		petArtifactResetRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		petArtifactResetRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << petArtifactResetRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_user_shop_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_shop_data data;
	readStream >> data;

	char buf[20480] = { 0 };
	sprintf(buf, "replace into `shopInfo`(`userId`, `info`) values(%llu, '%s');", data.userId, data.info.c_str());
	//m_db.hset(buf);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_ShopInfo);
}

MY_FUNCTION1(pet_artifact_levelup)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_artifact_level_up_req petArtifactLevelUpReq;
	readStream >> petArtifactLevelUpReq;

	ss_msg_pet_artifact_level_up_ret petArtifactLevelUpRet;
	petArtifactLevelUpRet.socket_id = petArtifactLevelUpReq.socket_id;
	petArtifactLevelUpRet.index_id = petArtifactLevelUpReq.index_id;
	petArtifactLevelUpRet.petId = petArtifactLevelUpReq.petId;
	petArtifactLevelUpRet.artifactId = petArtifactLevelUpReq.artifactId;
	petArtifactLevelUpRet.userId = petArtifactLevelUpReq.userId;
	petArtifactLevelUpRet.AP = petArtifactLevelUpReq.AP;
	petArtifactLevelUpRet.nResult = (int)RET_FAILED;

	if (petArtifactLevelUp(petArtifactLevelUpReq.petId, petArtifactLevelUpReq.userId, petArtifactLevelUpReq.AP))
	{
		petArtifactLevelUpRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		petArtifactLevelUpRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << petArtifactLevelUpRet;
	socket_->async_write(writeStream);
}


MY_FUNCTION1(skill_learn)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_skill_learn_req skillLevelUpReq;
	readStream >> skillLevelUpReq;

	ss_msg_skill_learn_ret skillLevelUpRet;
	skillLevelUpRet.socket_id = skillLevelUpReq.socket_id;
	skillLevelUpRet.index_id = skillLevelUpReq.index_id;
	skillLevelUpRet.userId = skillLevelUpReq.userId;
	skillLevelUpRet.skillId = skillLevelUpReq.skillId;
	skillLevelUpRet.nResult = (int)RET_FAILED;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(skillLevelUpReq.userId, userInfo))
	{
		skillLevelUpRet.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << skillLevelUpRet;
		socket_->async_write(writeStream);
		return;
	}

	if (skillLearn(userInfo.userCurRole, skillLevelUpReq.skillId))
	{
		skillLevelUpRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		skillLevelUpRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << skillLevelUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(equip_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_equip_level_up_req equipLevelUpReq;
	readStream >> equipLevelUpReq;

	ss_msg_equip_level_up_ret equipLevelUpRet;
	equipLevelUpRet.socket_id = equipLevelUpReq.socket_id;
	equipLevelUpRet.index_id = equipLevelUpReq.index_id;
	equipLevelUpRet.equipId = equipLevelUpReq.equipId;
	equipLevelUpRet.userId = equipLevelUpReq.userId;
	equipLevelUpRet.equip_index = equipLevelUpReq.equip_index;
	equipLevelUpRet.cost = equipLevelUpReq.cost;
	equipLevelUpRet.nResult = (int)RET_FAILED;

	if (equipLevelUp(equipLevelUpReq.userId, equipLevelUpReq.equipId, equipLevelUpReq.equip_index, equipLevelUpReq.cost))
	{
		equipLevelUpRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		equipLevelUpRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << equipLevelUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(equip_rank_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_equip_rank_up_req equipRankUpReq;
	readStream >> equipRankUpReq;

	ss_msg_equip_rank_up_ret equipRankUpRet;
	equipRankUpRet.socket_id = equipRankUpReq.socket_id;
	equipRankUpRet.index_id = equipRankUpReq.index_id;
	equipRankUpRet.userId = equipRankUpReq.userId;
	equipRankUpRet.costGold = equipRankUpReq.costGold;
	equipRankUpRet.nResult = (int)RET_FAILED;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(equipRankUpReq.userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			userInfo.userGold -= equipRankUpRet.costGold;
			userInfo.userCostGold -= equipRankUpRet.costGold;
			roleInfo.equipRank += 1;

			char bufUser[1024] = { 0 };
			sprintf(bufUser, "update `gameUserInfo` set `userGold` = `userGold` - %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu",
				equipRankUpRet.costGold, equipRankUpRet.costGold, equipRankUpRet.userId);
			singleton_t<db_request_list>::instance().push_request_list(bufUser, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `equipRank` = `equipRank` + 1 where `Role` = %llu;", roleInfo.role);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			equipRankUpRet.nResult = (int)RET_SUCCESS;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << equipRankUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(runesolt_unlock)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_runesolt_unlock_req runesoltUnlockReq;
	readStream >> runesoltUnlockReq;

	ss_msg_runesolt_unlock_ret runesoltUnlockRet;
	runesoltUnlockRet.socket_id = runesoltUnlockReq.socket_id;
	runesoltUnlockRet.index_id = runesoltUnlockReq.index_id;
	runesoltUnlockRet.userId = runesoltUnlockReq.userId;
	runesoltUnlockRet.slotId = runesoltUnlockReq.slotId;
	runesoltUnlockRet.nResult = (int)RET_FAILED;

	if (runesoltUnlock(runesoltUnlockReq.userId, runesoltUnlockReq.slotId))
	{
		runesoltUnlockRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		runesoltUnlockRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << runesoltUnlockRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(rune_inlay)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_rune_inlay_req runeInlayReq;
	readStream >> runeInlayReq;

	ss_msg_rune_inlay_ret runeInlayRet;
	runeInlayRet.socket_id = runeInlayReq.socket_id;
	runeInlayRet.index_id = runeInlayReq.index_id;
	runeInlayRet.index_pos = runeInlayReq.index_pos;
	runeInlayRet.userId = runeInlayReq.userId;
	runeInlayRet.slotId = runeInlayReq.slotId;
	runeInlayRet.type = runeInlayReq.type;
	runeInlayRet.nResult = (int)RET_FAILED;

	if (runeInlay(runeInlayReq.userId, runeInlayReq.index_pos, runeInlayReq.slotId, runeInlayReq.type))
	{
		runeInlayRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		runeInlayRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << runeInlayRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(rune_take_out)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_rune_take_out_req runeTakeOutReq;
	readStream >> runeTakeOutReq;

	ss_msg_rune_take_out_ret runeTakeOutRet;
	runeTakeOutRet.socket_id = runeTakeOutReq.socket_id;
	runeTakeOutRet.index_id = runeTakeOutReq.index_id;
	runeTakeOutRet.userId = runeTakeOutReq.userId;
	runeTakeOutRet.slotId = runeTakeOutReq.slotId;
	runeTakeOutRet.nResult = (int)RET_FAILED;

	if (runeTakeOut(runeTakeOutReq.userId, runeTakeOutReq.slotId))
	{
		runeTakeOutRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		runeTakeOutRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << runeTakeOutRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(rune_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_rune_level_up_req runeLevelUpReq;
	readStream >> runeLevelUpReq;

	ss_msg_rune_level_up_ret runeLevelUpRet;
	runeLevelUpRet.socket_id = runeLevelUpReq.socket_id;
	runeLevelUpRet.index_id = runeLevelUpReq.index_id;
	runeLevelUpRet.userId = runeLevelUpReq.userId;
	runeLevelUpRet.slotId = runeLevelUpReq.slotId;
	runeLevelUpRet.vecRuneId = runeLevelUpReq.vecRuneId;
	runeLevelUpRet.vecExpId = runeLevelUpReq.vecExpId;
	runeLevelUpRet.vecLevel = runeLevelUpReq.vecLevel;
	runeLevelUpRet.vecExpItemId = runeLevelUpReq.vecExpItemId;
	runeLevelUpRet.vecExpItemNum = runeLevelUpReq.vecExpItemNum;
	runeLevelUpRet.nResult = (int)RET_FAILED;

	if (runeLevelUp(runeLevelUpReq.userId, runeLevelUpReq.vecRuneId, runeLevelUpReq.vecExpId, runeLevelUpReq.vecLevel, runeLevelUpReq.vecExpItemId, runeLevelUpReq.vecExpItemNum))
	{
		runeLevelUpRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		runeLevelUpRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << runeLevelUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(rune_rebuild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_rune_rebuild_req runeRebuild;
	readStream >> runeRebuild;

	ss_msg_rune_rebuild_ret runeRebuildRet;
	runeRebuildRet.socket_id = runeRebuild.socket_id;
	runeRebuildRet.index_id = runeRebuild.index_id;
	runeRebuildRet.userId = runeRebuild.userId;
	runeRebuildRet.runeId = runeRebuild.runeId;
	runeRebuildRet.type = runeRebuild.type;
	runeRebuildRet.nResult = (int)RET_FAILED;

	if (runeSlotRebuild(runeRebuildRet.userId, runeRebuildRet.runeId, runeRebuildRet.type))
	{
		runeRebuildRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		runeRebuildRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << runeRebuildRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(soulsolt_unlock)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_soulsolt_unlock_req soulsoltUnlockReq;
	readStream >> soulsoltUnlockReq;

	ss_msg_soulsolt_unlock_ret soulsoltUnlockRet;
	soulsoltUnlockRet.socket_id = soulsoltUnlockReq.socket_id;
	soulsoltUnlockRet.index_id = soulsoltUnlockReq.index_id;
	soulsoltUnlockRet.userId = soulsoltUnlockReq.userId;
	soulsoltUnlockRet.slotId = soulsoltUnlockReq.slotId;
	soulsoltUnlockRet.nResult = (int)RET_FAILED;

	if (soulsoltUnlock(soulsoltUnlockReq.userId, soulsoltUnlockReq.slotId))
	{
		soulsoltUnlockRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		soulsoltUnlockRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << soulsoltUnlockRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(soulslot_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_soulsolt_level_up_req soulslotLevelUpReq;
	readStream >> soulslotLevelUpReq;

	ss_msg_soulsolt_level_up_ret soulslotLevelUpRet;
	soulslotLevelUpRet.socket_id = soulslotLevelUpReq.socket_id;
	soulslotLevelUpRet.index_id = soulslotLevelUpReq.index_id;
	soulslotLevelUpRet.userId = soulslotLevelUpReq.userId;
	soulslotLevelUpRet.slotId = soulslotLevelUpReq.slotId;
	soulslotLevelUpRet.costSP = soulslotLevelUpReq.costSP;
	soulslotLevelUpRet.nResult = (int)RET_FAILED;

	if (soulslotLevelUp(soulslotLevelUpReq.userId, soulslotLevelUpReq.slotId, soulslotLevelUpReq.costSP))
	{
		soulslotLevelUpRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		soulslotLevelUpRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << soulslotLevelUpRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(soul_inlay)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_soul_inlay_req soulInlayReq;
	readStream >> soulInlayReq;

	ss_msg_soul_inlay_ret soulInlayRet;
	soulInlayRet.socket_id = soulInlayReq.socket_id;
	soulInlayRet.index_id = soulInlayReq.index_id;
	soulInlayRet.userId = soulInlayReq.userId;
	soulInlayRet.slotId = soulInlayReq.slotId;
	soulInlayRet.petId = soulInlayReq.petId;
	soulInlayRet.nResult = (int)RET_FAILED;

	if (soulInlay(soulInlayReq.userId, soulInlayReq.slotId, soulInlayReq.petId))
	{
		soulInlayRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		soulInlayRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << soulInlayRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(soul_take_out)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_soul_take_out_req soulTakeOutReq;
	readStream >> soulTakeOutReq;

	ss_msg_soul_take_out_ret soulTakeOutRet;
	soulTakeOutRet.socket_id = soulTakeOutReq.socket_id;
	soulTakeOutRet.index_id = soulTakeOutReq.index_id;
	soulTakeOutRet.userId = soulTakeOutReq.userId;
	soulTakeOutRet.slotId = soulTakeOutReq.slotId;
	soulTakeOutRet.nResult = (int)RET_FAILED;

	if (soulTakeOut(soulTakeOutReq.userId, soulTakeOutReq.slotId))
	{
		soulTakeOutRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		soulTakeOutRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << soulTakeOutRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(talentsolt_unlock)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_talentslot_unlock_req talentsoltUnlockReq;
	readStream >> talentsoltUnlockReq;

	ss_msg_talentslot_unlock_ret talentsoltUnlockRet;
	talentsoltUnlockRet.socket_id = talentsoltUnlockReq.socket_id;
	talentsoltUnlockRet.index_id = talentsoltUnlockReq.index_id;
	talentsoltUnlockRet.userId = talentsoltUnlockReq.userId;
	talentsoltUnlockRet.slotId = talentsoltUnlockReq.slotId;
	talentsoltUnlockRet.nResult = (int)RET_FAILED;

	if (talentsoltUnlock(talentsoltUnlockReq.userId, talentsoltUnlockReq.slotId))
	{
		talentsoltUnlockRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		talentsoltUnlockRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << talentsoltUnlockRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(talent_lock)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_talent_lock_req talentLockReq;
	readStream >> talentLockReq;

	ss_msg_talent_lock_ret talentLockRet;
	talentLockRet.socket_id = talentLockReq.socket_id;
	talentLockRet.index_id = talentLockReq.index_id;
	talentLockRet.userId = talentLockReq.userId;
	talentLockRet.slotId = talentLockReq.slotId;
	talentLockRet.nResult = (int)RET_FAILED;

	if (talentLock(talentLockReq.userId, talentLockReq.slotId))
	{
		talentLockRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		talentLockRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << talentLockRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(talent_unlock)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_talent_unlock_req talentUnlockReq;
	readStream >> talentUnlockReq;

	ss_msg_talent_unlock_ret talentUnlockRet;
	talentUnlockRet.socket_id = talentUnlockReq.socket_id;
	talentUnlockRet.index_id = talentUnlockReq.index_id;
	talentUnlockRet.userId = talentUnlockReq.userId;
	talentUnlockRet.slotId = talentUnlockReq.slotId;
	talentUnlockRet.nResult = (int)RET_FAILED;

	if (talentUnlock(talentUnlockReq.userId, talentUnlockReq.slotId))
	{
		talentUnlockRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		talentUnlockRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << talentUnlockRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(talent_refresh)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_talent_refresh_req talentRefreshReq;
	readStream >> talentRefreshReq;

	ss_msg_talent_refresh_ret talentRefreshRet;
	talentRefreshRet.socket_id = talentRefreshReq.socket_id;
	talentRefreshRet.index_id = talentRefreshReq.index_id;
	talentRefreshRet.userId = talentRefreshReq.userId;
	talentRefreshRet.talentIndex = talentRefreshReq.talentIndex;
	talentRefreshRet.itemId = talentRefreshReq.itemId;
	talentRefreshRet.itemNumber = talentRefreshReq.itemNumber;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(talentRefreshReq.userId, userInfo))
	{
		talentRefreshRet.nResult = (int)RET_REQ_DATA_ERROR;
		BinaryWriteStream writeStream;
		writeStream << talentRefreshRet;
		socket_->async_write(writeStream);
		return;
	}

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_playerTalentExchange_t*>& mapPlayerTalentExchange = config.get_playerTalentExchange_config();

	strUserTalentInfo userTalentInfo;
	if (!m_data_cache.hget<strUserTalentInfo>(userInfo.userCurRole, userTalentInfo))
	{
		//create new
		userTalentInfo.talentLevel = userInfo.userTalentExchangLv;
		userTalentInfo.userId = userInfo.userCurRole;

		std::map<uint64_t, config_playerTalentExchange_t*>::iterator iterPlayerTalentExchange = mapPlayerTalentExchange.find(5);
		if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
		{
			talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
			BinaryWriteStream writeStream;
			writeStream << talentRefreshRet;
			socket_->async_write(writeStream);
			return;
		}
		userTalentInfo.levelRateS = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;

		iterPlayerTalentExchange = mapPlayerTalentExchange.find(4);
		if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
		{
			talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
			BinaryWriteStream writeStream;
			writeStream << talentRefreshRet;
			socket_->async_write(writeStream);
			return;
		}
		userTalentInfo.levelRateD = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;

		iterPlayerTalentExchange = mapPlayerTalentExchange.find(3);
		if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
		{
			talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
			BinaryWriteStream writeStream;
			writeStream << talentRefreshRet;
			socket_->async_write(writeStream);
			return;
		}
		userTalentInfo.levelRateC = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;

		iterPlayerTalentExchange = mapPlayerTalentExchange.find(2);
		if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
		{
			talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
			BinaryWriteStream writeStream;
			writeStream << talentRefreshRet;
			socket_->async_write(writeStream);
			return;
		}
		userTalentInfo.levelRateB = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;

		iterPlayerTalentExchange = mapPlayerTalentExchange.find(1);
		if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
		{
			talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
			BinaryWriteStream writeStream;
			writeStream << talentRefreshRet;
			socket_->async_write(writeStream);
			return;
		}
		userTalentInfo.levelRateA = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
		userTalentInfo.userId = talentRefreshReq.userId;

		initTalentRateData(&userTalentInfo);
	}

	std::map<uint64_t, config_playerTalentExchange_t*>::iterator iterPlayerTalentExchange = mapPlayerTalentExchange.find(userInfo.userTalentExchangLv);
	if (iterPlayerTalentExchange == mapPlayerTalentExchange.end())
	{
		talentRefreshRet.nResult = (int)RET_CONFIG_NOT_FIND_THE_RECORD;
		BinaryWriteStream writeStream;
		writeStream << talentRefreshRet;
		socket_->async_write(writeStream);
		return;
	}

	//level or not
	if (userInfo.userTalentExchangLv == 1)
	{
		int indexRand = myRand(0, 100);
		if (indexRand < userTalentInfo.levelRateA)//level up
		{
			userTalentInfo.levelRateA = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
			userTalentInfo.talentLevel = 2;
		}
		else
		{
			int AccumulateAdd = myRand(iterPlayerTalentExchange->second->m_MinAccumulate / 100, iterPlayerTalentExchange->second->m_MaxAccumulate / 100);
			userTalentInfo.levelRateA += AccumulateAdd;
			userTalentInfo.talentLevel = 1;
			if (iterPlayerTalentExchange->second->m_Limit / 100 < userTalentInfo.levelRateA)
			{
				userTalentInfo.levelRateA = iterPlayerTalentExchange->second->m_Limit;
			}
		}
	}
	else if (userInfo.userTalentExchangLv == 2)
	{
		int indexRand = myRand(0, 100);
		if (indexRand < userTalentInfo.levelRateB)//level up
		{
			userTalentInfo.levelRateB = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
			userTalentInfo.talentLevel = 3;
		}
		else
		{
			int AccumulateAdd = myRand(iterPlayerTalentExchange->second->m_MinAccumulate / 100, iterPlayerTalentExchange->second->m_MaxAccumulate / 100);
			userTalentInfo.levelRateB += AccumulateAdd;
			userTalentInfo.talentLevel = 1;
			if (iterPlayerTalentExchange->second->m_Limit / 100 < userTalentInfo.levelRateB)
			{
				userTalentInfo.levelRateB = iterPlayerTalentExchange->second->m_Limit;
			}
		}
	}
	else if (userInfo.userTalentExchangLv == 3)
	{
		int indexRand = myRand(0, 100);
		if (indexRand < userTalentInfo.levelRateC)//level up
		{
			userTalentInfo.levelRateC = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
			userTalentInfo.talentLevel = 4;
		}
		else
		{
			int AccumulateAdd = myRand(iterPlayerTalentExchange->second->m_MinAccumulate / 100, iterPlayerTalentExchange->second->m_MaxAccumulate / 100);
			userTalentInfo.levelRateC += AccumulateAdd;
			userTalentInfo.talentLevel = 1;
			if (iterPlayerTalentExchange->second->m_Limit / 100 < userTalentInfo.levelRateC)
			{
				userTalentInfo.levelRateC = iterPlayerTalentExchange->second->m_Limit / 100;
			}
		}
	}
	else if (userInfo.userTalentExchangLv == 4)
	{
		int indexRand = myRand(0, 100);
		if (indexRand < userTalentInfo.levelRateD)//level up
		{
			userTalentInfo.levelRateD = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
			userTalentInfo.talentLevel = 5;
		}
		else
		{
			int AccumulateAdd = myRand(iterPlayerTalentExchange->second->m_MinAccumulate / 100, iterPlayerTalentExchange->second->m_MaxAccumulate / 100);
			userTalentInfo.levelRateD += AccumulateAdd;
			userTalentInfo.talentLevel = 1;
			if (iterPlayerTalentExchange->second->m_Limit / 100 < userTalentInfo.levelRateD)
			{
				userTalentInfo.levelRateD = iterPlayerTalentExchange->second->m_Limit / 100;
			}
		}
	}
	else if (userInfo.userTalentExchangLv == 5)
	{
		userTalentInfo.levelRateS = iterPlayerTalentExchange->second->m_ProbabilityBase / 100;
		userTalentInfo.talentLevel = 1;
	}
	else
	{
		talentRefreshRet.nResult = (int)RET_REQ_DATA_ERROR;
		BinaryWriteStream writeStream;
		writeStream << talentRefreshRet;
		socket_->async_write(writeStream);
		return;
	}

	updateTalentRateData(&userTalentInfo);

	if (talentRefresh(talentRefreshReq.userId, talentRefreshRet.talentIndex, talentRefreshRet.itemId, talentRefreshRet.itemNumber, talentRefreshRet.talentValue))
	{
		talentRefreshRet.nResult = (int)RET_SUCCESS;
		userInfo.userTalentExchangLv = userTalentInfo.talentLevel;
		talentRefreshRet.talentExchangeLv = userInfo.userTalentExchangLv;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `talentExchangeLv` = %d where `userId` =%llu", talentRefreshRet.talentExchangeLv, talentRefreshReq.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
	else
	{
		talentRefreshRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << talentRefreshRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(talent_bribe)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_talent_bribe_req talentRefreshReq;
	readStream >> talentRefreshReq;

	ss_msg_talent_bribe_ret talentRefreshRet;
	talentRefreshRet.socket_id = talentRefreshReq.socket_id;
	talentRefreshRet.index_id = talentRefreshReq.index_id;
	talentRefreshRet.userId = talentRefreshReq.userId;
	talentRefreshRet.costType = talentRefreshReq.costType;
	talentRefreshRet.costNumber = talentRefreshReq.costNumber;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(talentRefreshReq.userId, userInfo))
	{
		talentRefreshRet.nResult = (int)RET_REQ_DATA_ERROR;
		BinaryWriteStream writeStream;
		writeStream << talentRefreshRet;
		socket_->async_write(writeStream);
		return;
	}


	if (talentBribe(talentRefreshRet.userId, talentRefreshRet.costType, talentRefreshRet.costNumber, talentRefreshRet.talentExchangeLv))
	{
		talentRefreshRet.nResult = (int)RET_SUCCESS;
	}
	else
	{
		talentRefreshRet.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << talentRefreshRet;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(draw_card_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_draw_card_reward_req req;
	readStream >> req;

	ss_msg_draw_card_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.actionType = req.actionType;
	ret.costType = req.costType;
	ret.costNumber = req.costNumber;
	ret.tokenId = req.tokenId;
	ret.tokenCost = req.tokenCost;
	ret.drawTime = req.drawTime;
	ret.discountTime = req.discountTime;

	int costType = req.costType;
	int costNumber = req.costNumber;
	if (req.tokenId > 0 && req.tokenCost > 0)
	{
		costType = req.tokenId;
		costNumber = req.tokenCost;
	}

	if (drawCardReward(req.userId, costType, costNumber, req.items, req.actionType, req.drawTime, req.givePoint, req.discountTime))
	{
		ret.nResult = (int)RET_SUCCESS;
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(req.userId, userInfo))
		{
			userInfo.drawCardTimes++;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `drawCardTimes` = %llu where `userId` = %llu", userInfo.drawCardTimes, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	if (ret.actionType == DRAW_CARD_TYPE::GOLD_ONE)
		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::drawCard_Gold_type, 1);
	else if (ret.actionType == DRAW_CARD_TYPE::GEM_ONE)
		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::drawCard_Gem_type, 1);
	else if (ret.actionType == DRAW_CARD_TYPE::GOLD_TEN)
		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::drawCard_Gold_type, 10);
	else if (ret.actionType == DRAW_CARD_TYPE::GEM_TEN)
		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::drawCard_Gem_type, 10);

}

MY_FUNCTION1(buy_something)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_something_req req;
	readStream >> req;

	ss_msg_buy_something_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.itemId = req.itemId;
	ret.itemNumber = req.itemNumber;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		std::vector<uint64_t> newItemID;
		this->gainItem(req.itemId, req.itemNumber, userInfo, newItemID);
		
		if (newItemID.size() > 0)
			ret.itemNewId = newItemID[0];
	}

	updateDailyTaskCountInfo(req.userId, dailyTaskCount::buyShopItem_type, 1);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_user_currency)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_user_currency_req req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		for (size_t i = 0; i < req.currencyType.size(); i++)
		{
			if (req.currencyCount[i] >= INT_MAX)
				logerror((LOG_SERVER, "update_user_currency [lose] item may cross the border, [type:%llu count:%llu]", req.currencyType[i], req.currencyCount[i]));

			if (req.currencyType[i] == CURRENCY_GOLD)
			{
				userInfo.userGold = req.currencyCount[i];
				userInfo.userCostGold = req.totalCostGold;

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userGold` = %llu, `userCostGold` = %llu where `userId` = %llu", userInfo.userGold, userInfo.userCostGold, req.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			}
			else if (req.currencyType[i] == CURRENCY_GEM)
			{
				userInfo.userGem = req.currencyCount[i];
				userInfo.userCostGem = req.totalCostGem;

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu", userInfo.userGem, userInfo.userCostGem, req.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			}
			else
			{
				//std::unordered_map<uint64_t, strItemInfo*>::iterator iterItemInfo = m_cache_item_map.begin();
				//for (; iterItemInfo != m_cache_item_map.end(); iterItemInfo++)
				strItemInfo itemInfo;
				if (m_data_cache.get_itemInfo(req.userId, NULL, req.currencyType[i], itemInfo))
				{
					itemInfo.Num = req.currencyCount[i];

					char buf[1024] = { 0 };
					sprintf(buf, "update `gameItemInfo` set `itemNum` = %llu where `itemUser` = %llu and `itemBase` = %llu", itemInfo.Num, req.userId, req.currencyType[i]);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameItemInfo);
					m_data_cache.set_itemInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
				}
			}
		}

		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(user_use_title)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_use_title_req req;
	readStream >> req;

	ss_msg_use_title_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.titleId = req.titleId;
	ret.data = "";

	if (useTitle(req.userId, req.titleId))
	{
		ret.nResult = (int)RET_SUCCESS;
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(reset_pet_pvp_day_times)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_reset_pet_pvp_day_times_req req;
	readStream >> req;

	ss_msg_reset_pet_pvp_day_times_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.costGem = req.costGem;
	ret.nResult = (int)RET_SUCCESS;

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	//reset remain count
	strPetPvpInfo petPvpInfo;
	if (m_data_cache.get_petPvpInfo(req.userId, NULL, petPvpInfo))
	{
		/*std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
		std::map<uint64_t, config_levelSetup_t*>::iterator iter = mapLevelSetUp.find(9000);*/
		int remainCount = 1;//每次增加一次 暂时写死
		//std::string userName = "";
		/*if (iter != mapLevelSetUp.end())
		{
		remainCount = iter->second->m_CountLimit;
		}*/

		petPvpInfo.remainCount = remainCount;
		petPvpInfo.resetTimes++;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userPetPvpInfo` set `remainCount` = %d, `resetTimes` = `resetTimes` + 1 where `userId` = %llu", remainCount, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPetPvpInfo);
		m_data_cache.set_petPvpInfo(petPvpInfo.userId, petPvpInfo.rank, petPvpInfo);
	}

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlob = mapGlobal.find("PetPvpResetCost");
		if (iterGlob != mapGlobal.end())
		{
			int costGem = atoi(iterGlob->second.c_str());
			userInfo.userCostGem += costGem;
			userInfo.userGem -= costGem;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu", userInfo.userGem, userInfo.userCostGem, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(reset_pet_pvp_cd_time)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_reset_pet_pvp_cd_time_req req;
	readStream >> req;

	ss_msg_reset_pet_pvp_cd_time_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.costGem = req.costGem;
	ret.userId = req.userId;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userCostGem += ret.costGem;
		userInfo.userGem -= ret.costGem;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu", userInfo.userGem, userInfo.userCostGem, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strPetPvpInfo petPvpInfo;
	if (m_data_cache.get_petPvpInfo(req.userId, NULL, petPvpInfo))
	{
		petPvpInfo.lastTime = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userPetPvpInfo` set `lastTime` = 0 where `userId` = %llu", req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPetPvpInfo);
		m_data_cache.set_petPvpInfo(petPvpInfo.userId, petPvpInfo.rank, petPvpInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(init_petPvpInfo)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_pet_pvp_info_req req;
	readStream >> req;

	strPetPvpInfo petPvpInfo;
	if (!m_data_cache.get_petPvpInfo(req.userId, NULL, petPvpInfo))
	{
		char buf[1024] = { 0 };
		sprintf(buf, "insert into `userPetPvpInfo`(`userId`, `name`, `rank`, `lastTime`, `remainCount`, `level`, `exp`, `power`,`totalCount`, `winCount`) values(%llu, '%s', 0, 0, %llu, 1, 0, 0, 0, 0);",
			req.userId, req.name.c_str(), req.remainCount);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserPetPvpInfo);

		strPetPvpInfo* pPetPvp = new strPetPvpInfo;
		pPetPvp->userId = req.userId;
		pPetPvp->name = req.name;
		pPetPvp->rank = 0;
		pPetPvp->remainCount = req.remainCount;
		pPetPvp->level = 1;
		pPetPvp->exp = 0;
		pPetPvp->power = 0;
		pPetPvp->lastTime = 0;
		pPetPvp->totalCount = 0;
		pPetPvp->winCount = 0;
		pPetPvp->resetTimes = 0;

		m_data_cache.set_petPvpInfo(pPetPvp->userId, pPetPvp->rank, *pPetPvp);
		delete pPetPvp;
	}
}

MY_FUNCTION1(set_pet_pvp_group)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_pet_pvp_group_req req;
	readStream >> req;

	ss_msg_set_pet_pvp_group_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.vecInfo = req.vecInfo;
	ret.curTeam = req.curTeam;
	ret.data = "";

	strUserPetPvpTeamInfo petPvpTeamInfo;
	if (m_data_cache.get_petPvpTeamInfo(req.userId, petPvpTeamInfo))
	{
		petPvpTeamInfo.vecInfo = req.vecInfo;
		m_db.updateUserPetPvpTeamInfo(petPvpTeamInfo);
	}
	else
	{
		petPvpTeamInfo.userId = ret.userId;
		petPvpTeamInfo.vecInfo = req.vecInfo;
		m_db.initUserPetPvpTeamInfo(petPvpTeamInfo);
	}

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_pvp_team_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_pvp_team_level_up_req req;
	readStream >> req;

	ss_msg_pet_pvp_team_level_up_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.costGold = req.costGold;
	ret.formation = req.formation;
	ret.nResult = (int)RET_FAILED;
	/*
	std::unordered_map<uint64_t, strUserPetPvpTeamInfo* >::iterator iterTeamInfo = m_cache_petPvpTeam_map.find(req.userId);
	if (iterTeamInfo != m_cache_petPvpTeam_map.end())
	{
		for (size_t i = 0; i < iterTeamInfo->second->teamId.size(); i++)
		{
			if (iterTeamInfo->second->teamId[i] == req.formation)
			{
				std::unordered_map<uint64_t, strUserInfo*>::iterator iterUser = m_cache_userInfo_map.find(req.userId);
				if (iterUser != m_cache_userInfo_map.end())
				{
					char buf[1024] = { 0 };
					sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` - %llu where `userId` = %llu", req.costGold, req.userId);
					m_db.hset(buf);
					iterUser->second->userGold -= req.costGold;
				}
				iterTeamInfo->second->Level[i] += 1;
				break;
			}
		}

		for (size_t i = 0; i < iterTeamInfo->second->vecInfo.size(); i++)
		{
			if (iterTeamInfo->second->vecInfo[i].teamId = req.formation)
			{
				iterTeamInfo->second->vecInfo[i].Level += 1;
				break;
			}
		}
		iterTeamInfo = m_cache_petPvpTeam_map.find(req.userId);
		//m_db.updateUserPetPvpTeamInfo(iterTeamInfo->second);
		Json::Value info;
		Json::Value Info, Formations, teams;

		Info["curTeam"] = iterTeamInfo->second->curTeam;
		for (size_t i = 0; i < iterTeamInfo->second->teamId.size(); i++)
		{
			char buf[32] = { 0 };
			sprintf(buf, "%llu", iterTeamInfo->second->teamId[i]);
			Formations[buf] = iterTeamInfo->second->Level[i];
		}

		Info["Formations"] = Formations;

		for (size_t i = 0; i < iterTeamInfo->second->vecInfo.size(); i++)
		{
			Json::Value teamInfo, pet, charge;
			teamInfo["formation"] = iterTeamInfo->second->vecInfo[i].teamId;
			teamInfo["Lv"] = iterTeamInfo->second->vecInfo[i].Level;

			for (size_t j = 0; j < iterTeamInfo->second->vecInfo[i].vecPets.size(); j++)
			{
				pet.append((Json::Value::Int)iterTeamInfo->second->vecInfo[i].vecPets[j]);
			}
			teamInfo["pets"] = pet;

			for (size_t j = 0; j < iterTeamInfo->second->vecInfo[i].vecCharges.size(); j++)
			{
				charge.append((Json::Value::Int)iterTeamInfo->second->vecInfo[i].vecCharges[j]);
			}
			teamInfo["charge"] = charge;

// 			char buf[32] = { 0 };
// 			sprintf(buf, "%llu", iterTeamInfo->second->vecInfo[i].teamId);
// 			teams[buf] = teamInfo;
			teams.append(teamInfo);
		}
		Info["teams"] = teams;

		Json::Value contents;
		Json::FastWriter writer;
		contents["info"] = Info;
		std::string data = writer.write(contents);

		char bufContents[20480] = { 0 };
		sprintf(bufContents, "update `userPetPvpTeamInfo` set `curTeam` = %llu, `info` = '%s' where `userId` = %llu;", iterTeamInfo->second->curTeam, data.c_str(), iterTeamInfo->second->userId);
		m_db.hset(bufContents);
	}*/

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_pvp_enter_game)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_enter_pet_pvp_battle_req req;
	readStream >> req;

	ss_msg_enter_pet_pvp_battle_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.otherId = req.otherId;
	ret.curTeam = req.curTeam;
	ret.nResult = (int)RET_SUCCESS;

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_pvp_finish_game)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_pet_pvp_battle_req req;
	readStream >> req;

	ss_msg_finish_pet_pvp_battle_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.otherId = req.otherId;
	ret.items = req.items;
	ret.itemNumbers = req.itemNumbers;
	ret.win = req.win;
	ret.time = req.time;
	ret.otherRank = req.otherRank;
	ret.userRank = req.userRank;
	ret.oldRank = req.oldRank;
	ret.friendDamage = req.friendDamage;
	ret.enemyDamage = req.enemyDamage;
	ret.nResult = (int)RET_SUCCESS;

	strPetPvpInfo petPvpInfo;
	if (!m_data_cache.get_petPvpInfo(req.userId, NULL, petPvpInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	strPetPvpInfo otherPetPvpInfo;
	if (!m_data_cache.get_petPvpInfo(req.otherId, NULL, otherPetPvpInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	petPvpInfo.rank = req.userRank;
	otherPetPvpInfo.rank = req.otherRank;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	ret.newHeighestRank = 0;
	if (userInfo.petPvpHighestRank > petPvpInfo.rank || userInfo.petPvpHighestRank == 0)
	{
		ret.newHeighestRank = petPvpInfo.rank;
		userInfo.petPvpHighestRank = petPvpInfo.rank;
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `petPvpHighestRank` = %llu where userId = %llu", userInfo.petPvpHighestRank, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	//remain && time
	if (req.win)
	{
		petPvpInfo.totalCount++;
		petPvpInfo.winCount++;
	}
	else
	{
		petPvpInfo.totalCount++;
	}
	petPvpInfo.remainCount--;
	petPvpInfo.lastTime = req.time;

	char buf_self[1024] = { 0 };
	sprintf(buf_self, "update `userPetPvpInfo` set `rank` = %llu, `remainCount` = %llu, `lastTime` = %llu, `totalCount` = %llu, `winCount` = %llu where userId = %llu",
		petPvpInfo.rank, petPvpInfo.remainCount, petPvpInfo.lastTime, petPvpInfo.totalCount, petPvpInfo.winCount, petPvpInfo.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf_self, sql_update, DB_GameUserInfo);
	m_data_cache.set_petPvpInfo(petPvpInfo.userId, petPvpInfo.rank, petPvpInfo);

	char buf_other[1024] = { 0 };
	sprintf(buf_other, "update `userPetPvpInfo` set `rank` = %llu where userId = %llu",
		otherPetPvpInfo.rank, otherPetPvpInfo.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf_other, sql_update, DB_GameUserInfo);
	m_data_cache.set_petPvpInfo(otherPetPvpInfo.userId, otherPetPvpInfo.rank, otherPetPvpInfo);

#pragma region 更新自己战斗记录
	//battle history update
	std::vector<strPetPvpHistory> vecPetPvpHistory;
	if (!m_data_cache.get_petPvpHistory(ret.userId, vecPetPvpHistory))
		m_db.insertPetPvpHistory(req.userId, vecPetPvpHistory);

	strPetPvpHistory strHistory;
	strHistory.user = ret.otherId;
	strHistory.userId = ret.userId;
	strHistory.win = ret.win;
	strHistory.time = ret.time;
	strHistory.friendDamage = ret.friendDamage;
	strHistory.enemyDamage = ret.enemyDamage;
	if (req.win)
	{
		strHistory.rank = ret.userRank;
	}
	else
	{
		strHistory.rank = ret.otherRank;
	}
	strHistory.name = otherPetPvpInfo.name;
	strHistory.attack = true;

	if (ret.otherId < 100000)//robot
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_robot_t*>& mapRobot = config.get_robot_config();
		std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.find(ret.otherId);
		if (iterRobot != mapRobot.end())
		{
			strHistory.level = iterRobot->second->m_PlayerLevel;
			strHistory.base = iterRobot->second->m_PlayerIcon;
		}
	}
	else
	{
		strUserInfo otherUserInfo;
		if (m_data_cache.get_userInfo(req.otherId, otherUserInfo))
		{
			strRoleInfo otherRoleInfo;
			if (m_data_cache.get_roleInfo(NULL, otherUserInfo.userCurRole, NULL, otherRoleInfo))
			{
				strHistory.level = otherRoleInfo.level_;
				strHistory.base = otherRoleInfo.rolebase;
			}
		}
	}
	vecPetPvpHistory.push_back(strHistory);
	if (vecPetPvpHistory.size() > 10)
	{
		std::vector<strPetPvpHistory> vecHistoryNew;
		for (size_t i = vecPetPvpHistory.size() - 10; i < vecPetPvpHistory.size(); i++)
			vecHistoryNew.push_back(vecPetPvpHistory[i]);
		vecPetPvpHistory.assign(vecHistoryNew.begin(), vecHistoryNew.end());
	}
	m_db.updatePetPvpHistory(ret.userId, vecPetPvpHistory);
#pragma endregion

#pragma region 更新对方战斗记录
	//other battle history update
	if (ret.otherId >= 100000)
	{
		std::vector<strPetPvpHistory> vecPetPvpHistory;
		if (!m_data_cache.get_petPvpHistory(ret.otherId, vecPetPvpHistory))
			m_db.insertPetPvpHistory(req.otherId, vecPetPvpHistory);

		strPetPvpHistory strHistory;
		strHistory.user = ret.userId;
		strHistory.userId = ret.otherId;
		strHistory.win = ret.win == 1 ? 0 : 1;
		strHistory.time = ret.time;
		strHistory.friendDamage = ret.enemyDamage;
		strHistory.enemyDamage = ret.friendDamage;
		if (!req.win)
		{
			strHistory.rank = ret.userRank;
		}
		else
		{
			strHistory.rank = ret.otherRank;
		}
		strHistory.name = petPvpInfo.name;
		strHistory.attack = false;

		if (ret.otherId < 100000)//robot
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_robot_t*>& mapRobot = config.get_robot_config();
			std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.find(ret.userId);
			if (iterRobot != mapRobot.end())
			{
				strHistory.level = iterRobot->second->m_PlayerLevel;
				strHistory.base = iterRobot->second->m_PlayerIcon;
			}
		}
		else
		{
			strUserInfo userInfo;
			if (m_data_cache.get_userInfo(req.userId, userInfo))
			{
				strRoleInfo roleInfo;
				if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
				{
					strHistory.level = roleInfo.level_;
					strHistory.base = roleInfo.rolebase;
				}
			}
		}
		vecPetPvpHistory.push_back(strHistory);
		if (vecPetPvpHistory.size() > 10)
		{
			std::vector<strPetPvpHistory> vecHistoryNew;
			for (size_t i = vecPetPvpHistory.size() - 10; i < vecPetPvpHistory.size(); i++)
				vecHistoryNew.push_back(vecPetPvpHistory[i]);
			vecPetPvpHistory.assign(vecHistoryNew.begin(), vecHistoryNew.end());
		}
		m_db.updatePetPvpHistory(ret.otherId, vecPetPvpHistory);
	}
#pragma endregion

	this->gainItems(req.items, req.itemNumbers, req.userId, ret.itemNewId);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::petPvp_type, 1);
}

MY_FUNCTION1(finish_endless_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_level_req req;
	readStream >> req;

	ss_msg_fetch_level_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.itemNumber = req.itemNumber;
	ret.id = req.id;
	ret.data = "";

	if (fetchEndlessLevel(req.userId, req.items, req.itemNumber, ret.newItemId))
	{
		ret.nResult = (int)RET_SUCCESS;
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	strPveInfo pveInfo;
	if (m_data_cache.get_pveInfo(ret.userId, ret.id, pveInfo))
	{
		pveInfo.fetched = 1;
		char buf[1024] = { 0 };
		sprintf(buf, "update `userPveInfo` set `fetched` = 1 where `userId` = %llu and pveType = %llu;", ret.userId, ret.id);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userPveInfo");
		m_data_cache.set_pveInfo(ret.userId, ret.id, pveInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(reset_endless_level)//重置无尽时空
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_reset_level_req req;
	readStream >> req;

	ss_msg_reset_level_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.remainCount = req.remainCount;
	ret.data = "";
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	strPetEndlessFightingInfo petEndlessInfo;
	if (m_data_cache.get_petEndlessFightingInfo(ret.userId, petEndlessInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			petEndlessInfo.playerLevel = roleInfo.level_;
		}
		petEndlessInfo.vecPet.clear();
		petEndlessInfo.vecPetHP.clear();
		petEndlessInfo.vecMonster.clear();
		m_db.hupdatePetEndlessFightingInfo(&petEndlessInfo);

		for (int i = 12001; i < 12013; i++)
		{
			//std::unordered_map<uint64_t, strPveInfo*>::iterator iterPve = m_cache_pveInfo_map.find(100000 * i + ret.userId);
			//if (iterPve != m_cache_pveInfo_map.end())
			strPveInfo pveInfo;
			if (m_data_cache.get_pveInfo(ret.userId, i, pveInfo))
			{
				pveInfo.passCount = 0;
				pveInfo.maxScore = 0;
				pveInfo.lastScore = 0;
				pveInfo.lastTime = 0;
				pveInfo.progress = 0;
				pveInfo.fetched = 0;
				pveInfo.refreshCount = 0;

				char buf[1024] = { 0 };
				sprintf(buf, "update `userPveInfo` set  `passCount` = 0, `maxScore` = 0, `lastScore` = 0, `lastTime` = 0, `progress` = 0, `fetched` = 0, `RefreshedCount` = 0 where userId = %llu and pveType = %d", ret.userId, i);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
				m_data_cache.set_pveInfo(ret.userId, i, pveInfo);
			}
		}
	}

	userInfo.CurETLevel = 0;
	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `CurETLevel` = 0 where `userId` = %llu", ret.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);

	//std::unordered_map<uint64_t, strPveInfo*>::iterator iterPve = m_cache_pveInfo_map.find(100000 * 12001 + ret.userId);
	//if (iterPve == m_cache_pveInfo_map.end())
	strPveInfo pveInfo;
	if (!m_data_cache.get_pveInfo(ret.userId, 12001, pveInfo))
	{
		strPveInfo* newPveInfo = new strPveInfo;
		newPveInfo->userId = ret.userId;
		newPveInfo->pveType = 12001;
		newPveInfo->remainCount = ret.remainCount;
		newPveInfo->passCount = 0;
		newPveInfo->maxScore = 0;
		newPveInfo->lastScore = 0;
		newPveInfo->lastTime = 0;
		newPveInfo->progress = 0;
		newPveInfo->fetched = 0;
		newPveInfo->refreshCount = 1;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `userPveInfo`(`userId`, `pveType`) values (%llu, 12001);", ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserPveInfo);
		m_data_cache.set_pveInfo(ret.userId, 12001, *newPveInfo);

		delete newPveInfo;
	}
	else
	{
		pveInfo.refreshCount += 1;//重置次数
		//pveInfo.remainCount += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userPveInfo` set `RefreshedCount` = %llu where `userId` = %llu and pveType = 12001;", pveInfo.refreshCount, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserPveInfo);
		m_data_cache.set_pveInfo(ret.userId, 12001, pveInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(set_pet_pve_cur_group_req)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_pet_cur_pve_group_req req;
	readStream >> req;

	ss_msg_set_pet_cur_pve_group_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.type = req.type;
	ret.data = "";

	if (setPetCurPveGroup(req.userId, req.type))
	{
		ret.nResult = (int)RET_SUCCESS;
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(set_pet_pve_group_req)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_pet_pve_group_req req;
	readStream >> req;

	ss_msg_set_pet_pve_group_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.type = req.type;
	ret.id1 = req.id1;
	ret.id2 = req.id2;
	ret.data = "";

	if (setPetPveGroup(req.userId, req.type, req.id1, req.id2))
	{
		ret.nResult = (int)RET_SUCCESS;
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

void football_db_analyze_t::parseString2Vector(std::string &data, std::vector<uint64_t>& vecInfo)
{
	std::string patten("|");
	int index = 0;
	int pos = 0;
	while ((pos = data.find(patten, index + 1)) != std::string::npos)
	{
		std::string value;
		value = data.substr(index, pos - index);
		vecInfo.push_back(atol(value.c_str()));
		index = pos + 1;
	}
	vecInfo.push_back(atol(data.substr(index, data.length() - index).c_str()));
}

uint64_t football_db_analyze_t::getAndUpdateLastInsertID(std::string type)
{
	std::map<std::string, uint64_t>::iterator iterInsertID = m_mapLastInsertID.find(type);
	if (iterInsertID != m_mapLastInsertID.end())
	{
		iterInsertID->second++;
		return iterInsertID->second;
	}

	return 0;
}

int football_db_analyze_t::getVipTimes(uint64_t userId, int type)
{
	int vipLevel = 0;
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId, userInfo))
	{
		return 0;
	}

	vipLevel = userInfo.userVip;
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<uint64_t, config_VipBase_t*>& mapVipBase = config.get_vipBase_config();
	std::map<uint64_t, config_VipFunc_t*>& mapVipFunc = config.get_vipFunc_config();

	std::map<uint64_t, config_VipBase_t*>::iterator iterVipBase = mapVipBase.find(vipLevel);
	if (iterVipBase == mapVipBase.end())
	{
		return 0;
	}

	int canBuyTimes = 0;
	std::vector<uint64_t> vecFunc;
	parseString2Vector(iterVipBase->second->m_FuncID, vecFunc);
	for (size_t i = 0; i < vecFunc.size(); i++)
	{
		std::map<uint64_t, config_VipFunc_t*>::iterator iterTemp = mapVipFunc.find(vecFunc[i]);
		if (iterTemp == mapVipFunc.end())
		{
			return 0;
		}

		if (iterTemp->second->m_Type == type)
		{
			canBuyTimes = iterTemp->second->m_Param;
			break;
		}
	}

	return canBuyTimes;
}

void football_db_analyze_t::updateDailyTaskCountInfo(uint64_t userId, int type, int count)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
		if (m_data_cache.get_questListByUserId(userId, mapQuest))
		{
			updateDailyTaskCountInfo_one(type, mapQuest, count, userInfo);
			m_db.hUpdateUserQuestList(userId, mapQuest);
		}

		MV_SAFE_RELEASE(mapQuest);
	}
}

void football_db_analyze_t::updateDailyTaskCountInfo_one(uint64_t type, std::map <std::uint64_t, boost::shared_ptr<strQuestList>>& mapQuest, int count, strUserInfo& userInfo)
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, std::vector<config_questBase_t*>>& mapDailyQuestBase = config.get_questBaseByTarget_config();
	std::map<uint64_t, std::vector<config_questBase_t*>>::iterator iterVecQuestBase = mapDailyQuestBase.find(type);

	if (iterVecQuestBase == mapDailyQuestBase.end())
		return;

	for (size_t i = 0; i < iterVecQuestBase->second.size(); i++)
	{
		config_questBase_t* questBase = iterVecQuestBase->second[i];

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuest = mapQuest.find(questBase->m_ID);
		if (iterQuest == mapQuest.end())
			continue;

		//七日礼包任务
		if (questBase->m_Type == 9)
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_sevenDaysGift_t*>& sevenDaysGift = config.get_sevenDaysGift_config();

			uint64_t id = questBase->m_ID;
			std::map<uint64_t, config_sevenDaysGift_t*>::const_iterator  iterGift = std::find_if(
				sevenDaysGift.begin(),
				sevenDaysGift.end(),
				[id](std::pair<uint64_t, config_sevenDaysGift_t*> a){return a.second->m_QuestID == id; });

			if (iterGift == sevenDaysGift.end())
				continue;

			int week = 0, days = 0;
			int dday = get_days_from_create((time_t)userInfo.createAt, &week, &days);
			if (iterGift->second->m_Week != week || iterGift->second->m_Day > days)
				continue;
		}

		if (iterQuest->second->state < EQuestState::Completed_Quest)
		{
			iterQuest->second->progress += count;

			if (iterQuest->second->progress >= atol(questBase->m_Param.c_str()))
				iterQuest->second->state = EQuestState::Completed_Quest;
		}
	}
}


void football_db_analyze_t::addMineHistory(const strMineHistory& history_)
{
	std::vector<strMineHistory> mineHistory;
	if (!m_data_cache.get_mineHistory(history_.userId, mineHistory))
	{
		mineHistory.push_back(history_);
		m_db.hinitMineHistory(history_.userId, mineHistory);
	}
	else
	{
		mineHistory.push_back(history_);
		m_db.hupdateMineHistory(history_.userId, mineHistory);
	}
}

bool football_db_analyze_t::updateMineStatus(strMineAllInfo* mineInfo, int occupationTime, time_t timeNow)
{
	//时间到了就撤军
	if (mineInfo->owner >= 100000 && timeNow - mineInfo->startTime >= occupationTime * 60)
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_mineBase_t*>& mapMineBase = config.get_mineBase_config();
		std::map<uint64_t, config_mineBase_t*>::iterator iterMineBase = mapMineBase.find(mineInfo->baseId);
		if (iterMineBase == mapMineBase.end())
		{
			logerror((LOG_SERVER, "failed to find mine base, baseId:%llu mineId:%llu userId:%llu",
				mineInfo->baseId, mineInfo->id, mineInfo->owner));
			return false;
		}

		std::vector<uint64_t> items;
		std::vector<uint64_t> nums;
		items.push_back(iterMineBase->second->m_ItemID);
		double numPerSec = iterMineBase->second->m_OutPut / (5.0 * 60.0);
		if (mineInfo->lastFetchTime > 0)
			nums.push_back(ceil((timeNow - mineInfo->lastFetchTime) * numPerSec));
		else
			nums.push_back(ceil((timeNow - mineInfo->startTime) * numPerSec));
		std::vector<uint64_t> newItemId;
		this->gainItems(items, nums, mineInfo->owner, newItemId);

		if (nums.size() > 0 && nums[0] > 0)
		{
			strMineHistory strHistory;
			strHistory.userId = mineInfo->owner;
			strHistory.status = EMineLogStatus_Reward;
			strHistory.value = nums[0];
			strHistory.uid = mineInfo->owner;
			strHistory.owner = mineInfo->owner;
			strHistory.mineBase = mineInfo->baseId;
			strHistory.time = timeNow;
			addMineHistory(strHistory);

			ostringstream oss;
			oss << mineInfo->baseId << "|";
			oss << nums[0];
			addNewsData(mineInfo->owner, ENewsID::MineTimeout, (uint64_t)timeNow, oss.str());
		}

		strMineTeamAllInfo mineTeamAllInfo;
		if (m_data_cache.get_mineTeamInfo(mineInfo->owner, mineTeamAllInfo))
		{
			std::map<uint64_t, config_mineMap_t*>& mapMine = config.get_mineMap_config();
			std::map<uint64_t, config_mineMap_t*>::iterator iterMineMapConfig = mapMine.find(mineInfo->id);
			if (iterMineMapConfig != mapMine.end())
			{
				mineInfo->owner = iterMineMapConfig->second->m_RobotID;
				mineInfo->teamId = 0;
				mineInfo->status = 0;
				mineInfo->startTime = 0;
				mineInfo->lastFetchTime = 0;

				char buf[1024] = { 0 };
				sprintf(buf, "update `mineAllInfo` set `owner` = %llu, `teamIndex` = %llu, `status` = %llu, `startTime` = %llu, `lastFetchTime` = %llu where `id` = %llu",
					mineInfo->owner, mineInfo->teamId, mineInfo->status, mineInfo->startTime, mineInfo->lastFetchTime, mineInfo->id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_MineAllInfo);
				m_data_cache.set_mineInfo(mineInfo->id, *mineInfo);
			}

			std::vector<strMineTeamInfo>::iterator iterTeamInfo = mineTeamAllInfo.teams.begin();
			for (; iterTeamInfo != mineTeamAllInfo.teams.end(); ++iterTeamInfo)
			{
				if (iterTeamInfo->mineId == mineInfo->id)
				{
					mineTeamAllInfo.teams.erase(iterTeamInfo);
					break;
				}
			}

			m_db.hupdateMineTeamInfo(&mineTeamAllInfo);
		}
	}

	return true;
}

void football_db_analyze_t::updateWarStandardInfo(uint64_t userId)
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
//	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
// 	std::unordered_map<uint64_t, strUserInfo*>::iterator iterUser = m_cache_userInfo_map.find(userId);
// 	if (iterUser != m_cache_userInfo_map.end())
// 	{
// 		std::unordered_map<uint64_t, strRoleInfo*>::iterator iterRole = m_cache_roleInfo_map.find(iterUser->second->userCurRole);
// 		if (iterRole != m_cache_roleInfo_map.end())
// 		{
// 			std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("WarStandardUnlockLv");
// 			if (iterGlobal != mapGlobal.end())
// 			{
// 				if (iterRole->second->level_ < atol(iterGlobal->second.c_str()))
// 				{
// 					return;
// 				}
// 			}
// 		}
// 	}

	bool roleDirty = false;
	bool petDirty = false;

	std::map<uint64_t, config_warStandardBase_t*>& mapBase = config.get_warStandardBase_config();
	std::map<uint64_t, config_warStandardBase_t*>::iterator iterBase = mapBase.begin();
	for (; iterBase != mapBase.end(); iterBase++)
	{
		std::vector<uint64_t> vecPetBase, usePetBase;
		parseString2Vector(iterBase->second->m_Members, vecPetBase);
		for (size_t i = 0; i < vecPetBase.size(); i++)
		{
			strPetInfo petInfo;
			if (m_data_cache.get_petInfo(userId, NULL, vecPetBase[i], petInfo))
			{
				usePetBase.push_back(petInfo.base);
			}
		}

		if (usePetBase.size() < 2)
		{
			continue;
		}

		if (iterBase->second->m_Type == 1)
			roleDirty |= true;
		else if (iterBase->second->m_Type == 2)
			petDirty |= true;
		else
			logerror((LOG_SERVER, "error config WarStandardBase:Type:%d", iterBase->second->m_Type));

		strWarStandardInfo warStandardInfo;
		if (!m_data_cache.get_warStandardInfo(userId, NULL, iterBase->second->m_ID, warStandardInfo))
		{
			warStandardInfo.userId = userId;
			warStandardInfo.base = iterBase->second->m_ID;
			warStandardInfo.lv = 1;

			char buf[1024] = { 0 };
			sprintf(buf, "insert into `userWarStandardInfo`(`userId`, `Base`, `Lv`) values (%llu, %d, 1);", userId, iterBase->second->m_ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserWarStandardInfo);
			warStandardInfo.Id = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserWarStandardInfo);

			strUserInfo userInfo;
			if (m_data_cache.get_userInfo(userId, userInfo))
			{
				strRoleInfo roleInfo;
				if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
				{
					if (roleInfo.warStandard0 == 0)
					{
						roleInfo.warStandard0 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard0` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}
					else if (roleInfo.warStandard1 == 0)
					{
						roleInfo.warStandard1 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard1` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}
					else if (roleInfo.warStandard2 == 0)
					{
						roleInfo.warStandard2 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard2` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}
					else if (roleInfo.warStandard3 == 0)
					{
						roleInfo.warStandard3 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard3` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}
					else if (roleInfo.warStandard4 == 0)
					{
						roleInfo.warStandard4 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard4` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}
					else if (roleInfo.warStandard5 == 0)
					{
						roleInfo.warStandard5 = warStandardInfo.Id;
						char buf[1024] = { 0 };
						sprintf(buf, "update `userRoleInfo` set `warStandard5` =  %llu where `Role` = %llu", warStandardInfo.Id, userInfo.userCurRole);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
					}

					m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
				}
			}
		}

		if (usePetBase.size() > warStandardInfo.petBase.size())
		{
			warStandardInfo.petBase = usePetBase;

			std::string temp;
			for (size_t i = 0; i < usePetBase.size(); i++)
			{
				char buf[16] = { 0 };
				sprintf(buf, "%llu|", usePetBase[i]);
				temp += buf;
			}
			char buf[1024] = { 0 };
			sprintf(buf, "update `userWarStandardInfo` set `petBase` = '%s' where `userId` = %llu and `base` = %d;", temp.substr(0, temp.length() - 1).c_str(), userId, iterBase->second->m_ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserWarStandardInfo);
			m_data_cache.set_warStandardInfo(warStandardInfo);
		}
	}

	//TODO 优化
	if (roleDirty)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			strRoleInfo roleInfo;
			if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
			{
				game_util::updateRoleInfo(m_data_cache, &roleInfo);
				updateDatabaseRoleInfo(&roleInfo);
			}
		}
	}

	if (petDirty)
	{
		std::vector<boost::shared_ptr<strPetInfo>> vecPetInfo;
		m_data_cache.mget_petInfoByUserId(userId, vecPetInfo);
		for (size_t i = 0; i < vecPetInfo.size(); i++)
		{
			game_util::updatePetInfo(m_data_cache, vecPetInfo[i].get());
			updateDatabasePetInfo(vecPetInfo[i].get());
		}
		MV_SAFE_RELEASE(vecPetInfo);
	}
}

bool football_db_analyze_t::getPveRewardList(int level, std::vector<uint64_t>& normalItems, std::vector<uint64_t>& normalNumbers, std::vector<uint64_t>& geniusItems,
	std::vector<uint64_t>& geniusNumbers, std::vector<uint64_t>& bossItems, std::vector<uint64_t>& bossNumbers)
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_levelSetup_t*>& map = config.get_levelSetup_config();
	std::map<uint64_t, config_levelSetup_t*>::iterator iterLevelSetup = map.find(level);
	if (iterLevelSetup == map.end())
	{
		return false;
	}

	std::vector<uint64_t> vecNormalItems, vecNormalNumber, vecNormalWeight;
	std::vector<uint64_t> vecGeniusItems, vecGeniusNumber, vecGeniusWeight;
	std::vector<uint64_t> vecBossItems, vecBossNumber, vecBossWeight;
// 	if (iterLevelSetup->second->m_NormalDrop.length() > 4)
// 	{
// 		parseString2Vector(iterLevelSetup->second->m_NormalDrop, vecNormalItems);
// 		parseString2Vector(iterLevelSetup->second->m_NormalNum, vecNormalNumber);
// 		parseString2Vector(iterLevelSetup->second->m_NormalWeight, vecNormalWeight);
// 	}
//
// 	if (iterLevelSetup->second->m_BossDrop.length() > 4)
// 	{
// 		parseString2Vector(iterLevelSetup->second->m_BossDrop, vecBossItems);
// 		parseString2Vector(iterLevelSetup->second->m_BossNum, vecBossNumber);
// 		parseString2Vector(iterLevelSetup->second->m_BossWeight, vecBossWeight);
// 	}
//
// 	if (iterLevelSetup->second->m_GeniusDrop.length() > 4)
// 	{
// 		parseString2Vector(iterLevelSetup->second->m_GeniusDrop, vecGeniusItems);
// 		parseString2Vector(iterLevelSetup->second->m_GeniusNum, vecGeniusNumber);
// 		parseString2Vector(iterLevelSetup->second->m_GeniusWeight, vecGeniusWeight);
// 	}

	if (vecNormalItems.size() != vecNormalNumber.size() || vecNormalNumber.size() != vecNormalWeight.size() || vecNormalItems.size() != vecNormalWeight.size())
	{
		return false;
	}

	if (vecGeniusItems.size() != vecGeniusNumber.size() || vecGeniusNumber.size() != vecGeniusWeight.size() || vecGeniusItems.size() != vecGeniusWeight.size())
	{
		return false;
	}

	if (vecBossItems.size() != vecBossNumber.size() || vecBossNumber.size() != vecBossWeight.size() || vecBossItems.size() != vecBossWeight.size())
	{
		return false;
	}

	int weight = 0;
	for (size_t i = 0; i < vecNormalItems.size(); i++)
	{
		weight = myRand(0, 100);
		if (weight <= vecNormalWeight[i])
		{
			if (vecNormalItems[i] > 1)
			{
				normalItems.push_back(vecNormalItems[i]);
				normalNumbers.push_back(vecNormalNumber[i]);
			}
		}
	}

	weight = 0;
	for (size_t i = 0; i < vecGeniusItems.size(); i++)
	{
		weight = myRand(0, 100);
		if (weight <= vecNormalWeight[i])
		{
			if (vecGeniusItems[i] > 1)
			{
				geniusItems.push_back(vecGeniusItems[i]);
				geniusNumbers.push_back(vecGeniusNumber[i]);
			}
		}
	}

	weight = 0;
	for (size_t i = 0; i < vecBossItems.size(); i++)
	{
		weight = myRand(0, 100);
		if (weight <= vecBossWeight[i])
		{
			if (vecBossItems[i] > 1)
			{
				bossItems.push_back(vecBossItems[i]);
				bossNumbers.push_back(vecBossNumber[i]);
			}
		}
	}
	return true;
}

MY_FUNCTION1(fast_finish_pve)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fast_finish_pve_req req;
	readStream >> req;

	ss_msg_fast_finish_pve_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.time = req.time;
	ret.keyNumber = req.keyNumber;
	ret.oldLevel = req.oldLevel;
	ret.levelUpOrNot = req.levelUpOrNot;
	ret.roleAddExp = req.roleAddExp;
	ret.roleLevel = req.roleLevel;
	ret.userStrength = req.userStrength;
	ret.items = req.items;
	ret.itemNumber = req.itemNumber;
	ret.data = "";

	Json::Value profit;

	//return levelData
	Json::Value levelProfit, drops;

	//levelProfit
	levelProfit["Gold"] = (Json::Value::Int)0;
	levelProfit["Soul"] = (Json::Value::Int)0;

	char bufNormal[16] = { 0 };
	sprintf(bufNormal, "%d", (int)ECellType::Normal);
	if (req.items.size() == 0)
	{
		//drops[bufNormal] = Json::nullValue;
	}
	else
	{
		Json::Value Value, drop;
		for (size_t i = 0; i < req.items.size(); i++)
		{
			char buf[16] = { 0 };
			sprintf(buf, "%llu", req.items[i]);
			Value[buf] = req.itemNumber[i];
		}
		drop["drops"] = Value;
		drops[bufNormal] = drop;
	}

	levelProfit["Drops"] = drops;

	profit.append(levelProfit);

	if (fastFinishPve(req.userId, ret.id, ret.keyNumber, ret.time, ret.items, ret.itemNumber, ret.newItemId, ret.roleAddExp, ret.roleLevel, ret.levelUpOrNot, ret.userStrength))
	{
		Json::Value Profit;
		Json::FastWriter write;
		Profit["Profit"] = profit;
		ret.data = write.write(Profit);
		ret.nResult = (int)RET_SUCCESS;

		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();
		std::map<uint64_t, config_SceneList_t*>& mapSceneList = config.get_sceneList_config();

		//更新任务
		std::map<uint64_t, config_SceneList_t*>::iterator iterSceneList = mapSceneList.find(ret.id);
		if (iterSceneList != mapSceneList.end())
		{
			if (iterSceneList->second->m_SubType == 1 || iterSceneList->second->m_SubType == 2 || iterSceneList->second->m_SubType == 3)//主线关卡(日常任务)
			{
				updateDailyTaskCountInfo(ret.userId, dailyTaskCount::mainPve_type, ret.time);
			}
			else if (iterSceneList->second->m_SubType == 4 || iterSceneList->second->m_SubType == 5)//英雄关卡(日常任务)
			{
				updateDailyTaskCountInfo(ret.userId, dailyTaskCount::heroPve_type, ret.time);
				updateQuestStateCount(req.userId, 10, dailyTaskCount::heroPve_type, ret.time);
			}
			else if(iterSceneList->second->m_Type == 10)//限时本(日常任务)
			{
				updateDailyTaskCountInfo(ret.userId, dailyTaskCount::limitTime_type, ret.time);
				updateQuestStateCount(req.userId, 10, dailyTaskCount::limitTime_type, ret.time);
			}
		}
	}
	else
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(quest_list_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_user_quest_list_info req;
	readStream >> req;

	if (req.type == 2) //有新任务加入列表并更新任务
		m_db.hUpdateUserQuestList(req.userId, req.vecInfo);
	else if (req.type == 1) //初始化任务列表
		m_db.hSetUserQuestList(req.userId, req.vecInfo);
}

MY_FUNCTION1(update_ever_tower_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_ever_tower_info req;
	readStream >> req;

	strPetEndlessFightingInfo petEndlessInfo;
	if (!m_data_cache.get_petEndlessFightingInfo(req.data.userId, petEndlessInfo))
	{
		m_db.hInitPetEndlessFightingInfo(&req.data);
	}
	else
	{
		m_db.hupdatePetEndlessFightingInfo(&req.data);
	}
}

MY_FUNCTION1(update_user_vip_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_user_vip_level req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userVip = req.vipLevel;
		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userVip` = %llu where `userId` = %llu;", req.vipLevel, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);

		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_playerAttrib_t*>&mapPlayerAtt = config.get_playerAttrib_config();
			std::map<uint64_t, config_playerAttrib_t*>::iterator iterPlayerAttr = mapPlayerAtt.find(roleInfo.rolebase + 10000 * req.level);
			if (iterPlayerAttr != mapPlayerAtt.end())
			{
				roleInfo.level_ = req.level;
				roleInfo.curexp_ = iterPlayerAttr->second->m_NextExp - 1;
				char bufRole[1024] = { 0 };
				sprintf(bufRole, "update `userRoleInfo` set `Level` = %llu, `curTitle` = %llu, `CurExp` = %llu where `Role` = %llu", req.level, /*req.newId*/roleInfo.curTitle, roleInfo.curexp_, userInfo.userCurRole);
				singleton_t<db_request_list>::instance().push_request_list(bufRole, sql_update, DB_UserRoleInfo);
				m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);

				BinaryWriteStream writeStream;
				writeStream << req;
				socket_->async_write(writeStream);
			}
		}
	}
}

MY_FUNCTION1(update_user_state)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_user_state req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.stateFlag = req.state;
		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `stateFlag` = %llu where `userId` = %llu;", userInfo.stateFlag, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(update_user_resource)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_user_resource req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userGold = req.gold;
		userInfo.userGem = req.gem;
		userInfo.userPvpExp = req.rpp;
		userInfo.userPetPVP = req.ppp;
		userInfo.userAP = req.ap;
		userInfo.userSP = req.sp;
		userInfo.ETKey = req.etKey;
		userInfo.userStrength = req.phy;
		userInfo.userTeamPoint = req.gc;
		userInfo.userUnLock = req.roleNum;
		userInfo.DQExp = req.dqe;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = %llu, `userGem` = %llu, `userPvpExp` = %llu, `userPetPVP` = %llu, `userAP` = %llu, `userSP` = %llu, `ETKey` = %llu, `userStrength` = %llu, "
		"`userTeamPoint` = %llu, `userRoleUnlock` = %llu, `DQExp` = %llu where `userId` = %llu;", req.gold, req.gem, req.rpp, req.ppp, req.ap, req.sp, req.etKey, req.phy, req.gc, req.roleNum, req.dqe, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(update_user_base_resource)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_user_base_resource req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userGold = req.gold;
		userInfo.userGem = req.gem;
		userInfo.userStrength = req.strength;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = %llu, `userGem` = %llu, `userStrength` = %llu where `userId` = %llu;", req.gold, req.gem, userInfo.userStrength, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(update_role_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_role_level req;
	readStream >> req;

	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, req.roleId, NULL, roleInfo))
	{
		roleInfo.level_ = req.level;
		roleInfo.curexp_ = req.exp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `Level` = %llu, `CurExp` = %llu where `Role` = %llu;", req.level, req.exp, req.roleId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
}

MY_FUNCTION1(update_role_equip_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_role_equip_level req;
	readStream >> req;

// 	std::unordered_map<uint64_t, strRoleInfo*>::iterator iterRole = m_cache_roleInfo_map.find(req.roleId);
// 	if (iterRole != m_cache_roleInfo_map.end())
// 	{
// 		iterRole->second->equiplevel0 = req.levels[0];
// 		iterRole->second->equiplevel1 = req.levels[1];
// 		iterRole->second->equiplevel2 = req.levels[2];
// 		iterRole->second->equiplevel3 = req.levels[3];
// 		iterRole->second->equiplevel4 = req.levels[4];
// 		iterRole->second->equiplevel5 = req.levels[5];
//
// 		char buf[1024] = { 0 };
// 		sprintf(buf, "update `userRoleInfo` set `EquipLevel0` = %llu, `EquipLevel1` = %llu, `EquipLevel2` = %llu, `EquipLevel3` = %llu, `EquipLevel4` = %llu, `EquipLevel5` = %llu  where `Role` = %llu;",
// 			req.levels[0], req.levels[1], req.levels[2], req.levels[3], req.levels[4], req.levels[5], req.roleId);
// 		m_db.hset(buf);
// 	}
}

MY_FUNCTION1(update_soul_slot_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_soul_slot_level req;
	readStream >> req;

	strRoleInfo roleInfo;
	if (m_data_cache.get_roleInfo(NULL, req.roleId, NULL, roleInfo))
	{
		roleInfo.soulSlotLevel0 = req.levels[0];
		roleInfo.soulSlotLevel1 = req.levels[1];
		roleInfo.soulSlotLevel2 = req.levels[2];

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRoleInfo` set `soulSlotLevel0` = %llu, `soulSlotLevel1` = %llu, `soulSlotLevel2` = %llu where `Role` = %llu;",
			req.levels[0], req.levels[1], req.levels[2], req.roleId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
		m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
	}
}

MY_FUNCTION1(update_pet_level)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_pet_level req;
	readStream >> req;

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, req.petId, NULL, petInfo))
	{
		if (petInfo.Rank < req.rank)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `Rank` = `Rank` + 1, `EquipsId0` = 0, `EquipsId1` = 0, `EquipsId2` = 0, `EquipsId3` = 0, `EquipsId4` = 0, `EquipsId5` = 0 where `ID` = %llu;", req.petId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

			strEquipInfo equipInfo;
			this->unuseEquip(petInfo.EquipsId0, equipInfo);
			this->unuseEquip(petInfo.EquipsId1, equipInfo);
			this->unuseEquip(petInfo.EquipsId2, equipInfo);
			this->unuseEquip(petInfo.EquipsId3, equipInfo);
			this->unuseEquip(petInfo.EquipsId4, equipInfo);
			this->unuseEquip(petInfo.EquipsId5, equipInfo);

			petInfo.EquipsId0 = 0;
			petInfo.EquipsId1 = 0;
			petInfo.EquipsId2 = 0;
			petInfo.EquipsId3 = 0;
			petInfo.EquipsId4 = 0;
			petInfo.EquipsId5 = 0;

		}
		petInfo.LV = req.level;
		petInfo.Star = req.star;
		petInfo.Rank = req.rank;
		petInfo.EXP = req.petExp;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `Lv` = %d, `Rank` = %d, `Star` = %d, `EXP` = %d where `ID` = %llu;",
			req.level, req.rank, req.star, req.petExp, req.petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

		updatePetSkillUnlockInfo(&petInfo);
		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);
	}
}

MY_FUNCTION1(update_pet_equip)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_pet_equip req;
	readStream >> req;

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, req.petId, NULL, petInfo))
	{
		petInfo.EquipsId0 = req.items[0];
		petInfo.EquipsId1 = req.items[1];
		petInfo.EquipsId2 = req.items[2];
		petInfo.EquipsId3 = req.items[3];
		petInfo.EquipsId4 = req.items[4];
		petInfo.EquipsId5 = req.items[5];

		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `EquipsId0` = %llu, `EquipsId1` = %llu, `EquipsId2` = %llu, `EquipsId3` = %llu, `EquipsId4` = %llu, `EquipsId5` = %llu where `ID` = %llu;",
			req.items[0], req.items[1], req.items[2], req.items[3], req.items[4], req.items[5], req.petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}
}

MY_FUNCTION1(update_artifact)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_artifact req;
	readStream >> req;

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, req.petId, NULL, petInfo))
	{
		petInfo.Artifact = req.aType;
		petInfo.ArtifactLv = req.aLevel;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `Artifact` = %d, `ArtifactId` = %d where `ID` = %llu;",req.aType, req.aLevel, req.petId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}
}

MY_FUNCTION1(update_pet_skill)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_upadte_pet_skill req;
	readStream >> req;

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(NULL, req.pet, NULL, petInfo))
	{
		petInfo.SkillsId0 = req.Level[0];
		petInfo.SkillsId1 = req.Level[1];
		petInfo.SkillsId2 = req.Level[2];
		petInfo.SkillsId3 = req.Level[3];

		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `SkillsId0` = %llu, `SkillsId1` = %llu, `SkillsId2` = %llu, `SkillsId3` = %llu, where `ID` = %llu;",
			req.Level[0], req.Level[1], req.Level[2], req.Level[3], req.pet);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
	}
}

MY_FUNCTION1(update_user_physical)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_physical req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userStrength = req.physical;
		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userStrength` = %d where `userId` = %llu;", req.physical, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(update_user_add_item)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_add_item req;
	readStream >> req;

	ss_gm_msg_add_item_ret ret;
	ret.client_id = req.client_id;
	ret.userId = req.userId;
	ret.base = req.Item;
	ret.Item = 0;
	ret.number = req.number;

	std::vector<uint64_t> newItemID;
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		this->gainItem(req.Item, req.number, userInfo, newItemID);
		ret.Item = newItemID[0];
	}

	//send to game server
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

//MY_FUNCTION1(update_user_del_item)
//{
//	BinaryReadStream readStream(data_.c_str(), data_.size());
//	ss_gm_msg_del_item req;
//	readStream >> req;
//
//	if (req.items / 100000 == 1)
//	{
//		bool exist = false;
//		std::unordered_map<uint64_t, strEquipInfo*>::iterator iterEquip = m_cache_equip_map.find(req.items);
//		if(iterEquip != m_cache_equip_map.end())
//		{
//			char buf[1024] = { 0 };
//			sprintf(buf, "delete from `gameEquipInfo` where `ID` = %llu", req.items);
//			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameEquipInfo);
//
//			m_cache_equip_map.erase(req.items);
//		}
//	}
//	else
//	{
//		std::unordered_map<uint64_t, strItemInfo*>::iterator iterItems = m_cache_item_map.find(req.items);
//		if(iterItems != m_cache_item_map.end())
//		{
//			char buf[1024] = { 0 };
//			sprintf(buf, "delete from `gameItemInfo` where `itemId` = %llu", req.items);
//			singleton_t<db_request_list>::instance().push_request_list(buf, sql_delete, DB_GameItemInfo);
//			m_cache_item_map.erase(req.items);
//		}
//	}
//}

MY_FUNCTION1(update_user_add_pet)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_add_pet req;
	readStream >> req;

	ss_gm_add_pet_ret ret;
	strPetInfo* petInfo = new strPetInfo;
	petInfo->userId = req.userId;
	petInfo->base = req.petId;
	petInfo->LV = 1;
	petInfo->Rank = 1;
	petInfo->Star = 1;
	petInfo->NextExp = 0;
	petInfo->EXP = 0;
	petInfo->HPMax = 0;
	petInfo->HPRestore = 0;
	petInfo->SoulMax = 0;
	petInfo->SoulRestore = 0;
	petInfo->PhysicalDamage = 0;
	petInfo->PhysicalDefense = 0;
	petInfo->MagicDamage = 0;
	petInfo->MagicDefense = 0;
	petInfo->Critical = 0;
	petInfo->Tough = 0;
	petInfo->Hit = 0;
	petInfo->Block = 0;
	petInfo->CriticalDamage = 0;
	petInfo->MoveSpeed = 0;
	petInfo->FastRate = 0;
	petInfo->SkillsId0 = 0;
	petInfo->SkillsId1 = 0;
	petInfo->SkillsId2 = 0;
	petInfo->SkillsId3 = 0;
	petInfo->EquipsId0 = 0;
	petInfo->EquipsId1 = 0;
	petInfo->EquipsId2 = 0;
	petInfo->EquipsId3 = 0;
	petInfo->EquipsId4 = 0;
	petInfo->EquipsId5 = 0;
	petInfo->Artifact = 0;
	petInfo->ArtifactLv = 0;
	petInfo->PveStandby1 = 0;
	petInfo->PveStandby2 = 0;
	petInfo->PveStandby3 = 0;
	petInfo->PveStandby1 = 0;
	petInfo->PvpStandby2 = 0;
	petInfo->DelegateQuest = 0;
	petInfo->Artifact0 = 0;
	petInfo->Artifact1 = 0;
	petInfo->Artifact2 = 0;
	petInfo->Artifact3 = 0;
	petInfo->Artifact4 = 0;
	petInfo->LockedArtifactSlot = 0;

	char buf[1024] = { 0 };
	sprintf(buf, "insert into `gamePetInfo` (`Base`, `userId`, `Lv`, `Rank`, `Star`) values(%llu, %llu, 1, 1, 1)", req.petId, req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GamePetInfo);
	petInfo->ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GamePetInfo);

	updatePetSkillUnlockInfo(petInfo);
	game_util::updatePetInfo(m_data_cache, petInfo);
	updateDatabasePetInfo(petInfo);

	updateGetPetQuestState(petInfo->userId);
	updateWarStandardInfo(petInfo->userId);

	ret.petId = petInfo->ID;
	ret.base = req.petId;
	ret.userId = req.userId;
	ret.client_id = req.client_id;

	delete petInfo;

	//send to game server
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_user_del_pet)
{
	/*
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_gm_msg_update_user_vip_level req;
	readStream >> req;

	std::unordered_map<uint64_t, strUserInfo*>::iterator iterUser = m_cache_userInfo_map.find(req.userId);
	if (iterUser != m_cache_userInfo_map.end())
	{
		iterUser->second->userVip = req.vipLevel;
		char buf[1024] = { 0 };
		sprintf(buf, "update gameUserInfo set `userVip` = %d where `userId` = %llu;", req.vipLevel, req.userId);
		m_db.hset(buf);
	}
	*/
}

MY_FUNCTION1(send_mail)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_send_mail_req req;
	readStream >> req;

	if (req.all)
	{
		std::vector<uint64_t> userIds;
		m_data_cache.get_userIdByAccount_all(userIds);
		for (size_t i = 0; i < userIds.size(); i++)
		{
			sendMail(userIds[i], req.mailId, req.toUser, req.title, req.contents, req.items, req.itemNumber, req.attach);
		}
		logerror((LOG_SERVER, "send mail to all gays, count:%d", userIds.size()));
	}
	else
	{
		sendMail(req.userId, req.mailId, req.toUser, req.title, req.contents, req.items, req.itemNumber, req.attach);
	}
}

MY_FUNCTION1(read_mail)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_read_mail_req req;
	readStream >> req;

	ss_msg_read_mail_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;

	std::vector<strUserMailsInfo> mailInfo;
	if (m_data_cache.get_mailInfo(req.userId, mailInfo))
	{
		for (size_t i = 0; i < mailInfo.size(); i++)
		{
			if (mailInfo[i].mailId == req.mailId)
			{
				Json::Value mail;
				Json::FastWriter writer;
				mail["mailId"] = (Json::Value::UInt)mailInfo[i].mailId;
				mail["title"] = mailInfo[i].title;
				mail["sender"] = mailInfo[i].sender;
				mail["content"] = mailInfo[i].contents;
				mail["time"] = mailInfo[i].time;
				mail["attach"] = mailInfo[i].attach ? true : false;
				mail["flag"] = (Json::Value::Int)mailInfo[i].flag;
				Json::Value items;
				for (size_t j = 0; j < mailInfo[i].items.size(); j++)
				{
					Json::Value item;
					char buf[32] = { 0 };
					sprintf(buf, "%llu", mailInfo[i].items[j]);
					item[buf] = (Json::Value::Int)mailInfo[i].itemsNumber[j];
					items.append(item);
				}
				mail["items"] = items;
				ret.nResult = (int)RET_SUCCESS;
				ret.data = writer.write(mail);

				mailInfo[i].flag = EMailState::Read;
				m_db.hupdateUserMailInfo(req.userId, mailInfo);
			}
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(fetch_mail_attach)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_attach_req req;
	readStream >> req;

	ss_msg_fetch_attach_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.itemId;
	ret.itemNumbers = req.itemNumber;
	ret.nResult = (int)RET_DATA_ERROR;

	std::vector<strUserMailsInfo> newMailInfo;
	std::vector<strUserMailsInfo> mailInfo;
	if (m_data_cache.get_mailInfo(req.userId, mailInfo))
	{
		for (size_t i = 0; i < mailInfo.size(); i++)
		{
			bool find = false;
			for (size_t j = 0; j < req.vecMailId.size(); j++)
			{
				if (mailInfo[i].mailId == req.vecMailId[j])
				{
					mailInfo[i].flag = EMailState::Reward;
					ret.nResult = (int)RET_SUCCESS;
					find = true;
				}
			}

			if (!find)
			{
				newMailInfo.push_back(mailInfo[i]);
			}
		}


		m_db.hupdateUserMailInfo(req.userId, newMailInfo);
		if (fetchMailAttach(req.userId, req.itemId, req.itemNumber, ret.vecItemNewId))
		{
			ret.nResult = (int)RET_SUCCESS;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(delete_mail)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_delete_mail_req req;
	readStream >> req;

	ss_msg_delete_mail_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.mailId = req.mailId;
	ret.nResult = (int)RET_USER_MAIL_NOT_EXIST;

	std::vector<strUserMailsInfo> mailInfo;
	if (m_data_cache.get_mailInfo(req.userId, mailInfo))
	{
		for (size_t i = 0; i < mailInfo.size(); i++)
		{
			if (mailInfo[i].mailId == req.mailId)
			{
				mailInfo[i].flag = EMailState::Delete;
				m_db.hupdateUserMailInfo(req.userId, mailInfo);
				ret.nResult = (int)RET_SUCCESS;
			}
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(get_boss_battle_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_boss_battle_info_req req;
	readStream >> req;

	ss_msg_get_boss_battle_info_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.nResult = RET_SUCCESS;

	strBossBattleInfo bossBattleInfo;
	if (!m_data_cache.get_bossBattleInfo(req.userId, bossBattleInfo))
	{
		bossBattleInfo.userId = req.userId;
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.unlock = config_inst.getGlobalIntValue("PetTeamSlotDefault", 6);
		bossBattleInfo.rank = 0;
		bossBattleInfo.killNumber = 0;
		bossBattleInfo.totalDamage = 0;
		bossBattleInfo.lastTime = 0;

		m_db.hInitUserBossBattleInfo(&bossBattleInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(unlock_boss_battle_slot)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_unlock_boss_battle_slot_req unlockBossBattleSlot;
	readStream >> unlockBossBattleSlot;

	strBossBattleInfo bossBattleInfo;
	if (!m_data_cache.get_bossBattleInfo(unlockBossBattleSlot.userId, bossBattleInfo))
	{
		bossBattleInfo.userId = unlockBossBattleSlot.userId;
		bossBattleInfo.unlock = config_inst.getGlobalIntValue("PetTeamSlotDefault", 6);
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.vecPet.push_back(0);
		bossBattleInfo.unlock++;
		m_db.hInitUserBossBattleInfo(&bossBattleInfo);
	}
	else
	{
		bossBattleInfo.unlock++;
		m_db.hUpdateUserBossBattleInfo(&bossBattleInfo);
	}

	this->costGem(unlockBossBattleSlot.userId, unlockBossBattleSlot.cost);

	ss_msg_unlock_boss_battle_slot_ret ret;
	ret.socket_id = unlockBossBattleSlot.socket_id;
	ret.index_id = unlockBossBattleSlot.index_id;
	ret.userId = unlockBossBattleSlot.userId;
	ret.slot = unlockBossBattleSlot.slot;
	ret.cost = unlockBossBattleSlot.cost;
	ret.nResult = (int)RET_SUCCESS;

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(revive_in_war)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_revive_in_war_req reviveInWar;
	readStream >> reviveInWar;

	ss_msg_revive_in_war_ret ret;
	ret.socket_id = reviveInWar.socket_id;
	ret.index_id = reviveInWar.index_id;
	ret.costType = reviveInWar.costType;
	ret.cost = reviveInWar.cost;
	ret.userId = reviveInWar.userId;
	ret.nResult = (int)RET_SUCCESS;

	if (ret.costType == 1)
	{
		this->costGold(ret.userId, ret.cost);
	}
	else if (ret.costType == 2)
	{
		this->costGem(ret.userId, ret.cost);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(exchange_gold)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_exchange_gold_req exchangeGold;
	readStream >> exchangeGold;

	ss_msg_exchange_gold_ret ret;
	ret.socket_id = exchangeGold.socket_id;
	ret.index_id = exchangeGold.index_id;
	ret.userId = exchangeGold.userId;
	ret.batch = exchangeGold.batch;
	ret.costGem = exchangeGold.costGem;
	ret.getGold = exchangeGold.getGold;
	ret.mutiTimes = exchangeGold.mutiTimes;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.userExchangeGoldNum += ret.batch;
		userInfo.userGem -= ret.costGem;
		userInfo.userCostGem += ret.costGem;
		userInfo.userGold += ret.getGold;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userExchangeGoldNum` = %llu,  `userGem` = %llu, `userCostGem` = %llu, `userGold` = %llu where `userId` = %llu",
			userInfo.userExchangeGoldNum, userInfo.userGem, userInfo.userCostGem, userInfo.userGold, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);

	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::exchangeGold_type, ret.batch);
}

MY_FUNCTION1(update_configs)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_configs req;
	readStream >> req;

	//!更新活动配置表
	server_config_t& server_config = singleton_t<server_config_t>::instance();
	server_config.reload_game_resource(req.day, req.openServerTime, req.enable);
}

MY_FUNCTION1(refresh_times)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_refresh_times req;
	readStream >> req;

	if (req.type == RESET_NEW_DAY && req.userId >= 100000)
	{
		resetGameDataNewDay(req.userId);
	}
	else if (req.type == RESET_NEW_DAY_SYSTEM)
	{
		uint64_t last_reset_time = 0;
		std::string info;
		if (m_data_cache.get_globalInfo(global_key_newday, info))
		{
			try
			{
				last_reset_time = boost::lexical_cast<uint64_t>(info);
			}
			catch (boost::bad_lexical_cast& e)
			{
				cout << e.what() << endl;
			}
		}
		
		uint64_t changeHour = 0, changeMinites = 0;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("CountRefreshTime");
		if (iterGlobal != mapGlobal.end())
		{
			parseString2Time(iterGlobal->second, changeHour, changeMinites);
		}

		time_t deftime = changeHour * 60 * 60 + changeMinites * 60;

		time_t tNow = time(NULL);
		int days = 0;
		if (last_reset_time > 0)
		{
			days = DaysBetween2Time(tNow - deftime, (time_t)last_reset_time - deftime);
		}

		if (last_reset_time == 0 || days > 0)
		{
			loginfo((LOG_SERVER, "START RESET_NEW_DAY_SYSTEM"));

#pragma region 重置工会系统
			std::vector<boost::shared_ptr<strGuildInfo>> vecGuildInfo;
			m_data_cache.getall_guildInfo(vecGuildInfo);
			for (size_t i = 0; i < vecGuildInfo.size(); i++)
			{
				vecGuildInfo[i]->guildProgress = 0;
				char buf[1024] = { 0 };
				sprintf(buf, "update `gameGuildInfo` set `guildProgress` = 0 where `id` = %llu", vecGuildInfo[i]->id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
				m_data_cache.set_guildInfo(vecGuildInfo[i]->id, *vecGuildInfo[i].get());
			}
			MV_SAFE_RELEASE(vecGuildInfo);
#pragma endregion

			loginfo((LOG_SERVER, "FINISH RESET_NEW_DAY_SYSTEM"));
			info = boost::lexical_cast<std::string>(tNow);
			if (last_reset_time == 0)
				m_db.hinitGlobalInfo(global_key_newday, info);
			else
				m_db.hUpdateGlobalInfo(global_key_newday, info);

			//!重置每日全服充值
			std::string today_recharge_info = "0";
			if (m_data_cache.get_globalInfo(global_key_todayrecharge, today_recharge_info))
				m_db.hUpdateGlobalInfo(global_key_todayrecharge, "0");
			else
				m_db.hinitGlobalInfo(global_key_todayrecharge, today_recharge_info);

			std::string last_recharge_info = "0";
			if (m_data_cache.get_globalInfo(global_key_lastdayrecharge, last_recharge_info))
				m_db.hUpdateGlobalInfo(global_key_lastdayrecharge, today_recharge_info);
			else
				m_db.hinitGlobalInfo(global_key_lastdayrecharge, today_recharge_info);
		}
		
	}
	else if (req.type == RESET_TOP_LIST)
	{
#pragma region RESET_TOP_LIST

		uint64_t last_reset_time = 0;
		std::string info;
		if (m_data_cache.get_globalInfo(global_key_toplist, info))
		{
			try
			{
				last_reset_time = boost::lexical_cast<uint64_t>(info);
			}
			catch (boost::bad_lexical_cast& e)
			{
				cout << e.what() << endl;
			}
		}
		
		uint64_t changeHour = 0, changeMinites = 0;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("LeaderBoardPvpRewardTime");
		if (iterGlobal != mapGlobal.end())
		{
			parseString2Time(iterGlobal->second, changeHour, changeMinites);
		}

		time_t deftime = changeHour * 60 * 60 + changeMinites * 60;

		time_t tNow = time(NULL);
		int days = 0;
		if (last_reset_time > 0)
		{
			days = DaysBetween2Time(tNow - deftime, (time_t)last_reset_time - deftime);
		}

		if (last_reset_time == 0 || days > 0)
		{
			loginfo((LOG_SERVER, "START RESET_TOP_LIST"));

#pragma region 英雄对战排名奖励
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::vector<config_levelReward_t>& levelUpReward = config.get_levelReward_config();
			std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
			std::string contents = "英雄对战每日排行榜奖励";
			std::string title = "英雄对战每日排行榜奖励";
			std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Title_petpvpdailyrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				title = iterLanguage->second;
			}
			iterLanguage = mapServerLanguage.find("Mail_Text_petpvpdailyrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				contents = iterLanguage->second;
			}

			std::vector<config_levelReward_t> rewards;
			for (size_t i = 0; i < levelUpReward.size(); i++)
			{
				if (levelUpReward[i].m_ID == 9000)
					rewards.push_back(levelUpReward[i]);
			}

			std::vector<boost::shared_ptr<strPetPvpInfo>> vecPetPvpInfo;
			m_data_cache.mget_petPvpInfo_noRobot(vecPetPvpInfo);
			for (size_t i = 0; i < vecPetPvpInfo.size(); i++)
			{
				if (vecPetPvpInfo[i]->userId < 100000)//robot
					continue;

				if (vecPetPvpInfo[i]->rank == 0)
					continue;

				std::vector<uint64_t> items;
				std::vector<uint64_t> nums;

				for (size_t j = 0; j < rewards.size(); j++)
				{
					if (vecPetPvpInfo[i]->rank <= rewards[j].m_RewardCond && (j == 0 || vecPetPvpInfo[i]->rank > rewards[j - 1].m_RewardCond))
					{
						parseString2Vector(rewards[j].m_RewardItem, items);
						parseString2Vector(rewards[j].m_Count, nums);

						sendMail(vecPetPvpInfo[i]->userId, getMailId(), "", title, contents, items, nums, true);
						break;
					}
				}
			}
			MV_SAFE_RELEASE(vecPetPvpInfo);
#pragma endregion

#pragma region 战队突袭排名奖励
			std::string contents2 = "战队突袭排行榜奖励";
			std::string title2 = "战队突袭排行榜奖励";
			iterLanguage = mapServerLanguage.find("Mail_Title_Teampvpdaily");
			if (iterLanguage != mapServerLanguage.end())
			{
				title2 = iterLanguage->second;
			}
			iterLanguage = mapServerLanguage.find("Mail_Text_Teampvpdaily");
			if (iterLanguage != mapServerLanguage.end())
			{
				contents2 = iterLanguage->second;
			}

			std::vector<boost::shared_ptr<strRolePvpRankListReward>> vecRolePvpRankReward;
			m_data_cache.mget_rolePvpRankListReward_noRobot(vecRolePvpRankReward);
			for (size_t i = 0; i < vecRolePvpRankReward.size(); i++)
			{
				if (vecRolePvpRankReward[i]->userId < 100000)
					continue;

				if (vecRolePvpRankReward[i]->rewardItems.size() == 0)
					continue;

				int count = 0;
				for (size_t j = 0; j < vecRolePvpRankReward[i]->rewardCount.size(); j++)
					count += vecRolePvpRankReward[i]->rewardCount[j];
				if (count == 0) continue;

				sendMail(vecRolePvpRankReward[i]->userId, getMailId(), "", title2, contents2, vecRolePvpRankReward[i]->rewardItems, vecRolePvpRankReward[i]->rewardCount, true);
			}
			MV_SAFE_RELEASE(vecRolePvpRankReward);

			//清除所有数据
			char buf[1024] = { 0 };
			sprintf(buf, "delete from `rolePvpRankListReward`");
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_RolePvpRankListReward);
			m_data_cache.clearall_rolePvpRankListReward();

			/************************************************************************/
			/*                           排名奖励									*/
			/************************************************************************/
			contents = "战队突袭对战每日排行榜奖励";
			title = "战队突袭对战每日排行榜奖励";
			iterLanguage = mapServerLanguage.find("Mail_Title_rolepvpdailyrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				title = iterLanguage->second;
			}
			iterLanguage = mapServerLanguage.find("Mail_Text_rolepvpdailyrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				contents = iterLanguage->second;
			}

			rewards.clear();
			for (size_t i = 0; i < levelUpReward.size(); i++)
			{
				if (levelUpReward[i].m_ID == 6100)
					rewards.push_back(levelUpReward[i]);
			}

			std::vector<boost::shared_ptr<strRolePvpInfo>> vecRolePvpInfo;
			m_data_cache.mget_rolePvpInfo_noRobot(vecRolePvpInfo);
			for (size_t i = 0; i < vecRolePvpInfo.size(); i++)
			{
				if (vecRolePvpInfo[i]->userId < 100000)//robot
					continue;

				if (vecRolePvpInfo[i]->rank == 0)
					continue;

				std::vector<uint64_t> items;
				std::vector<uint64_t> nums;

				for (size_t j = 0; j < rewards.size(); j++)
				{
					if (vecRolePvpInfo[i]->rank <= rewards[j].m_RewardCond && (j == 0 || vecRolePvpInfo[i]->rank > rewards[j - 1].m_RewardCond))
					{
						parseString2Vector(rewards[j].m_RewardItem, items);
						parseString2Vector(rewards[j].m_Count, nums);

						sendMail(vecRolePvpInfo[i]->userId, getMailId(), "", title, contents, items, nums, true);
						break;
					}
				}
			}
			MV_SAFE_RELEASE(vecRolePvpInfo);

#pragma endregion

			loginfo((LOG_SERVER, "FINISH RESET_TOP_LIST"));

			info = boost::lexical_cast<std::string>(tNow);
			if (last_reset_time == 0)
				m_db.hinitGlobalInfo(global_key_toplist, info);
			else
				m_db.hUpdateGlobalInfo(global_key_toplist, info);
		}
#pragma endregion
	}
	else if (req.type == RESET_STRENGTH)//!体力回复
	{
		loginfo((LOG_SERVER, "START RESET_STRENGTH"));

#pragma region RESET_STRENGTH
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(req.userId, userInfo))
		{
			userInfo.lastRefreshStrength = time(NULL);
			userInfo.userStrength++;
			m_data_cache.set_userInfo(req.userId, userInfo);

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu, `lastRefreshStrength` = %llu where `userId` = %llu", userInfo.userStrength, userInfo.lastRefreshStrength, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		}

#pragma endregion

		loginfo((LOG_SERVER, "FINISH RESET_STRENGTH"));
	}
	else if (req.type == RESET_BOSS_BATTLE_TOP_REWARD)
	{
		uint64_t last_reset_time = 0;
		std::string info;
		if (m_data_cache.get_globalInfo(global_key_bossbattle, info))
		{
			try
			{
				last_reset_time = boost::lexical_cast<uint64_t>(info);
			}
			catch (boost::bad_lexical_cast& e)
			{
				cout << e.what() << endl;
			}
		}

		uint64_t changeHour = 0, changeMinites = 0;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("LeaderBoardBossRushRewardTime");
		if (iterGlobal != mapGlobal.end())
		{
			parseString2Time(iterGlobal->second, changeHour, changeMinites);
		}

		time_t deftime = changeHour * 60 * 60 + changeMinites * 60;

		time_t tNow = time(NULL);
		int days = 0;
		if (last_reset_time > 0)
		{
			days = DaysBetween2Time(tNow - deftime, (time_t)last_reset_time - deftime);
		}

		if (last_reset_time == 0 || days > 0)
		{
			loginfo((LOG_SERVER, "START RESET_BOSS_BATTLE_TOP_REWARD"));

#pragma region RESET_BOSS_BATTLE_TOP_REWARD

			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::vector<config_levelReward_t>& levelUpReward = config.get_levelReward_config();
			std::vector<uint64_t> userId;
			m_data_cache.zrange_bossBattleRank(0, 3000, userId);

			std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
			std::string contents = "英雄试炼每日排行榜奖励";
			std::string title = "英雄试炼每日排行榜奖励";
			std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Title_BossRush");
			if (iterLanguage != mapServerLanguage.end())
			{
				title = iterLanguage->second;
			}
			iterLanguage = mapServerLanguage.find("Mail_Text_BossRush");
			if (iterLanguage != mapServerLanguage.end())
			{
				contents = iterLanguage->second;
			}

			std::vector<config_levelReward_t> rewards;
			for (size_t i = 0; i < levelUpReward.size(); i++)
			{
				if (levelUpReward[i].m_ID == 13002)
					rewards.push_back(levelUpReward[i]);
			}

			for (size_t rank = 1; rank <= userId.size(); rank++)
			{
				//send reward mails
				std::vector<uint64_t> items;
				std::vector<uint64_t> nums;
				for (size_t i = 0; i < rewards.size(); i++)
				{
					if (rank <= rewards[i].m_RewardCond && (i == 0 || rank > rewards[i - 1].m_RewardCond))
					{
						parseString2Vector(rewards[i].m_RewardItem, items);
						parseString2Vector(rewards[i].m_Count, nums);
						sendMail(userId[rank - 1], getMailId(), "", title, contents, items, nums, true);
						break;
					}
				}
			}

			//boss battle clear data
			std::vector<boost::shared_ptr<strBossBattleInfo>> vecBossBattleInfo;
			m_data_cache.getall_bossBattleInfo(vecBossBattleInfo);
			for (size_t i = 0; i < vecBossBattleInfo.size(); i++)
			{
				vecBossBattleInfo[i]->totalDamage = 0;
				vecBossBattleInfo[i]->killNumber = 0;
				vecBossBattleInfo[i]->rank = 0;
				vecBossBattleInfo[i]->buyCount.clear();
				m_db.hUpdateUserBossBattleInfo(vecBossBattleInfo[i].get());
			}
			m_data_cache.clearall_bossBattleRank();
			MV_SAFE_RELEASE(vecBossBattleInfo);
#pragma endregion

			loginfo((LOG_SERVER, "FINISH RESET_BOSS_BATTLE_TOP_REWARD"));

			info = boost::lexical_cast<std::string>(tNow);
			if (last_reset_time == 0)
				m_db.hinitGlobalInfo(global_key_bossbattle, info);
			else
				m_db.hUpdateGlobalInfo(global_key_bossbattle, info);
		}
	}
	else if (req.type == RESET_WORLD_BOSS_TOP_REWARD)//!世界boss排名结算
	{
		uint64_t last_reset_time = 0;
		/*std::string info;
		if (m_data_cache.get_globalInfo(global_key_worldboss, info))
		{
			try
			{
				last_reset_time = boost::lexical_cast<uint64_t>(info);
			}
			catch (boost::bad_lexical_cast& e)
			{
				cout << e.what() << endl;
			}
		}*/

		if (last_reset_time == 0)
		{
			loginfo((LOG_SERVER, "START RESET_WORLD_BOSS_TOP_REWARD"));

#pragma region RESET_WORLD_BOSS_TOP_REWARD

			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::vector<config_levelReward_t>& levelUpReward = config.get_levelReward_config();
			std::vector<uint64_t> userId;
			m_data_cache.zrange_worldBossRank(0, 10, userId);

			loginfo((LOG_SERVER, "WORLD_BOSS_TOP_REWARD user size %ud", userId.size()));

			std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
			std::string contents = "世界Boss排行榜奖励";
			std::string title = "世界Boss排行榜奖励";
			std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Title_worldbossrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				title = iterLanguage->second;
			}
			iterLanguage = mapServerLanguage.find("Mail_Text_worldbossrank");
			if (iterLanguage != mapServerLanguage.end())
			{
				contents = iterLanguage->second;
			}

			std::vector<config_levelReward_t> rewards;
			for (size_t i = 0; i < levelUpReward.size(); i++)
			{
				if (levelUpReward[i].m_ID == 11000)
					rewards.push_back(levelUpReward[i]);
			}

			for (size_t rank = 1; rank <= userId.size(); rank++)
			{
				//send reward mails
				std::vector<uint64_t> items;
				std::vector<uint64_t> nums;
				for (size_t i = 0; i < rewards.size(); i++)
				{
					if (rank <= rewards[i].m_RewardCond && (i == 0 || rank > rewards[i - 1].m_RewardCond))
					{
						parseString2Vector(rewards[i].m_RewardItem, items);
						parseString2Vector(rewards[i].m_Count, nums);
						sendMail(userId[rank - 1], getMailId(), "", title, contents, items, nums, true);
						break;
					}
				}
			}

#pragma endregion

			loginfo((LOG_SERVER, "FINISH RESET_WORLD_BOSS_TOP_REWARD"));

			/*info = boost::lexical_cast<std::string>(tNow);
			if (last_reset_time == 0)
				m_db.hinitGlobalInfo(global_key_bossbattle, info);
			else
				m_db.hUpdateGlobalInfo(global_key_bossbattle, info);*/
		}
	}
	else if (req.type == RESET_GUILD_BOSS_TOP_REWARD)//!公会boss排名结算
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
		std::string contents1 = "公会战役个人排名奖励";
		std::string title1 = "在本次公会战役活动中,你的队伍凭借出色战技获得了以下奖励:";
		std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Title_GuildBoss_PersonalRank");
		if (iterLanguage != mapServerLanguage.end())
		{
			title1 = iterLanguage->second;
		}
		iterLanguage = mapServerLanguage.find("Mail_Text_GuildBoss_PersonalRank");
		if (iterLanguage != mapServerLanguage.end())
		{
			contents1 = iterLanguage->second;
		}

		std::string contents2 = "公会战役公会排名奖励";
		std::string title2 = "在本次公会战役活动中,你的公会凭借出色战技获得了以下奖励:";
		iterLanguage = mapServerLanguage.find("Mail_Title_GuildBoss_GuildRank");
		if (iterLanguage != mapServerLanguage.end())
		{
			title2 = iterLanguage->second;
		}
		iterLanguage = mapServerLanguage.find("Mail_Text_GuildBoss_GuildRank");
		if (iterLanguage != mapServerLanguage.end())
		{
			contents2 = iterLanguage->second;
		}

		std::vector<uint64_t> vecGuildId, vecScore;
		game_util::getGuildBossScoreRankList(m_data_cache, vecGuildId, vecScore, 50);

		std::vector<config_levelReward_t>& levelUpReward = config.get_levelReward_config();
		std::vector<config_levelReward_t> scoreRewards, interRewards;

		for (size_t i = 0; i < levelUpReward.size(); i++)
		{
			if (levelUpReward[i].m_ID == 14001)
				interRewards.push_back(levelUpReward[i]);
			else if (levelUpReward[i].m_ID == 14002)
				scoreRewards.push_back(levelUpReward[i]);
		}

		std::map<uint64_t, config_levelReward_t> mapGuildReward;

		for (size_t i = 0; i < vecGuildId.size(); i++)
		{
			int rank = i + 1;

			for (size_t j = 0; j < scoreRewards.size(); j++)
			{
				if (rank <= scoreRewards[j].m_RewardCond && (j == 0 || rank > scoreRewards[j - 1].m_RewardCond))
				{
					mapGuildReward[vecGuildId[i]] = scoreRewards[j];
					break;
				}
			}
		}

		std::map<uint64_t, config_guildBoss_t*>& mapGuildBoss = config.get_guildBoss_config();
		std::vector<boost::shared_ptr<strGuildBossInfo>> vecGuildBossInfo;
		m_data_cache.getall_guildBossInfo(vecGuildBossInfo);
		for (size_t i = 0; i < vecGuildBossInfo.size(); i++)
		{
			std::vector<strGuildBossStat> stats = vecGuildBossInfo[i]->vecStatCache;
			if (stats.size() == 0)
				continue;

			std::sort(stats.begin(), stats.end(), [](strGuildBossStat a, strGuildBossStat b){ return a.totalDamage > b.totalDamage; });

			int bonus = 0;
			for (size_t j = 0; j < vecGuildBossInfo[i]->vecBossId.size(); j++)
			{
				if (vecGuildBossInfo[i]->vecBossHp[j] == 0)
				{
					std::map<uint64_t, config_guildBoss_t*>::iterator iterGuildBoss = mapGuildBoss.find(vecGuildBossInfo[i]->vecBossId[j]);
					if (iterGuildBoss != mapGuildBoss.end())
					{
						bonus += iterGuildBoss->second->m_RankBonus;
					}
				}
			}

			for (size_t x = 0; x < stats.size(); x++)
			{
				int rank = x + 1;

				for (size_t y = 0; y < interRewards.size(); y++)
				{
					if (rank <= interRewards[y].m_RewardCond && (y == 0 || rank > interRewards[y - 1].m_RewardCond))
					{
						std::vector<uint64_t> items;
						std::vector<uint64_t> nums;

						//!会内排名奖励
						parseString2Vector(interRewards[y].m_RewardItem, items);
						parseString2Vector(interRewards[y].m_Count, nums);

						if (bonus > 0)
						{
							for (size_t z = 0; z < nums.size(); z++)
								nums[z] += nums[z] * bonus / 100;
						}

						sendMail(stats[x].userId, getMailId(), "", title1, contents1, items, nums, true);

						//！公会积分奖励
						std::map<uint64_t, config_levelReward_t>::iterator iter = mapGuildReward.find(vecGuildBossInfo[i]->guildId);
						if (iter != mapGuildReward.end())
						{
							std::vector<uint64_t> pItems;
							std::vector<uint64_t> pNums;

							parseString2Vector(iter->second.m_RewardItem, pItems);
							parseString2Vector(iter->second.m_Count, pNums);

							sendMail(stats[x].userId, getMailId(), "", title2, contents2, pItems, pNums, true);
						}
						
						break;
					}
				}
			}
		}
		MV_SAFE_RELEASE(vecGuildBossInfo);
	}
}

MY_FUNCTION1(morpher_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_morpher_level_up_req req;
	readStream >> req;

	ss_msg_morpher_level_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.vecSkillBase = req.vecSkillBase;
	ret.id = req.id;
	ret.itemId = req.itemId;
	ret.itemNumber = req.itemNumber;
	ret.costGold = req.costGold;

	ret.nResult = (int)RET_SUCCESS;

	strItemInfo itemInfo;
	useItem(req.itemId, itemInfo, req.itemNumber);

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userGold -= req.costGold;
		userInfo.userCostGold += req.costGold;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = `userGold` - %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu", req.costGold, req.costGold, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strUserMorpherInfo morpherInfo;
	if (m_data_cache.get_morpherInfo(req.id, morpherInfo))
	{
		for (size_t i = 0; i < req.vecSkillBase.size(); i++)
		{
			strSkillInfo skillInfo;
			skillInfo.Base = req.vecSkillBase[i];
			skillInfo.userId = req.userId;
			skillInfo.Lv = 1;
			skillInfo.petId = req.id;
			skillInfo.CurExp = 0;
			skillInfo.Equip = 1;
			skillInfo.SlotNum = 0;

			strSkillInfo checkSkillInfo;
			if (m_data_cache.get_skillInfo(req.userId, NULL, skillInfo.Base, checkSkillInfo))
			{
				logerror((LOG_SERVER, "morpher level up unlock skill duplication, base:%llu", skillInfo.Base));
				continue;
			}

			char buf[1024] = { 0 };
			sprintf(buf, "insert into `gameSkillInfo`(`Base`, `userId`, `PetId`, `Lv`, `CurExp`, `Equip`, `SlotNum`) values(%llu, %llu, %llu, 1, 0, 1, 0);", req.vecSkillBase[i], req.userId, req.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameSkillInfo);
			skillInfo.ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameSkillInfo);
			m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);
			ret.vecSkillId.push_back(skillInfo.ID);
		}

		if (ret.vecSkillId.size() == 0)
		{
			morpherInfo.lv = req.lv;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userMorpherInfo` set `Lv` = %llu where `ID` = %llu", req.lv, req.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserMorpherInfo);
		}
		else if (ret.vecSkillId.size() == 1)
		{
			if (morpherInfo.skill1 == 0)
			{
				morpherInfo.lv = req.lv;
				morpherInfo.skill1 = ret.vecSkillId[0];

				char buf[1024] = { 0 };
				sprintf(buf, "update `userMorpherInfo` set `Lv` = %llu, `Skill1` = %llu where `ID` = %llu", req.lv, ret.vecSkillId[0],  req.id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserMorpherInfo);
			}
			else
			{
				morpherInfo.lv = req.lv;
				morpherInfo.skill2 = ret.vecSkillId[0];

				char buf[1024] = { 0 };
				sprintf(buf, "update `userMorpherInfo` set `Lv` = %llu, `Skill2` = %llu where `ID` = %llu", req.lv, ret.vecSkillId[0], req.id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserMorpherInfo);
			}
		}
		else if (ret.vecSkillId.size() == 2)
		{
			morpherInfo.lv = req.lv;
			morpherInfo.skill1 = ret.vecSkillId[0];
			morpherInfo.skill2 = ret.vecSkillId[1];

			char buf[1024] = { 0 };
			sprintf(buf, "update `userMorpherInfo` set `Lv` = %llu, `Skill1` = %llu, `Skill2` = %llu where `ID` = %llu", req.lv, ret.vecSkillId[0], ret.vecSkillId[1], req.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserMorpherInfo);
		}
		m_data_cache.set_morpherInfo(morpherInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(morpher_skill_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_morpher_skill_up_req req;
	readStream >> req;

	ss_msg_morpher_skill_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.itemId = req.itemId;
	ret.itemNumber = req.itemNumber;
	ret.levels = req.levels;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.skillId = req.skillId;
	ret.nResult = (int)RET_SUCCESS;

	strItemInfo itemInfo;
	useItem(req.itemId, itemInfo, req.itemNumber);

	for (size_t i = 0; i < ret.skillId.size(); i++)
	{
		strSkillInfo skillInfo;
		if (m_data_cache.get_skillInfo(NULL, ret.skillId[i], NULL, skillInfo))
		{
			skillInfo.Lv = req.flevels[i];

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameSkillInfo` set `Lv` = %llu where `ID` = %llu", skillInfo.Lv, ret.skillId[i]);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
			m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(set_active_morpher)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_active_morpher_req req;
	readStream >> req;

	ss_msg_set_active_morpher_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userCurMorpherId = req.id;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `curMorpher` =  %llu where `userId` = %llu", req.id, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_takeout)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_artifact_takeout_req req;
	readStream >> req;

	ss_msg_artifact_takeout_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.type = req.type;
	ret.unitId = req.unitId;
	ret.slotId = req.slotId;
	ret.chipId = req.chipId;
	ret.cost = req.cost;
	ret.costNumber = req.costNumber;
	ret.nResult = (int)RET_SUCCESS;

	strEquipInfo equipInfo;
	this->unuseEquip(ret.chipId, equipInfo);

	//cost
	if (ret.cost == CURRENCY_GOLD)
	{
		this->costGold(ret.unitId, ret.costNumber);
	}
	//gem
	else if (ret.cost == CURRENCY_GEM)
	{
		this->costGem(ret.unitId, ret.costNumber);
	}

	if (ret.type == 0)//role
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, ret.unitId, NULL, roleInfo))
		{
			if (ret.slotId == 0)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact0` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact0 = 0;
			}
			else if (ret.slotId == 1)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact1` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact1 = 0;
			}
			else if (ret.slotId == 2)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact2` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact2 = 0;
			}
			else if (ret.slotId == 3)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact3` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact3 = 0;
			}
			else if (ret.slotId == 4)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact4` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact4 = 0;
			}
			else if (ret.slotId == 5)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact5` = 0 where `Role` = %llu;", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact5 = 0;
			}
			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
		}
	}
	else if (ret.type == 1)//pet
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(NULL, ret.unitId, NULL, petInfo))
		{
			if (ret.slotId == 0)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact0` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact0 = 0;
			}
			else if (ret.slotId == 1)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact1` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact1 = 0;
			}
			else if (ret.slotId == 2)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact2` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact2 = 0;
			}
			else if (ret.slotId == 3)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact3` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact3 = 0;
			}
			else if (ret.slotId == 4)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact4` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact4 = 0;
			}
			else if (ret.slotId == 5)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact5` = 0 where `ID` = %llu", ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact5 = 0;
			}
			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_inlay)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_artifact_inlay_req req;
	readStream >> req;

	ss_msg_artifact_inlay_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.unitId = req.unitId;
	ret.slotId = req.slotId;
	ret.type = req.type;
	ret.chipId = req.chipId;
	ret.costId = req.costId;
	ret.costNum = req.costNum;
	ret.nResult = (int)RET_SUCCESS;

	strEquipInfo equipInfo;
	this->useEquip(ret.chipId, equipInfo);

	if (ret.costId != 0 && ret.costNum != 0)
	{
		if (ret.costId == CURRENCY_GOLD)
		{
			this->costGold(ret.unitId, ret.costNum);
		}
		else if (ret.costId == CURRENCY_GEM)
		{
			this->costGem(ret.unitId, ret.costNum);
		}
	}

	if (ret.type == 0)//role
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, ret.unitId, NULL, roleInfo))
		{
			if (ret.slotId == 0)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact0` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact0 = ret.chipId;
			}
			else if (ret.slotId == 1)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact1` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact1 = ret.chipId;
			}
			else if (ret.slotId == 2)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact2` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact2 = ret.chipId;
			}
			else if (ret.slotId == 3)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact3` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact3 = ret.chipId;
			}
			else if (ret.slotId == 4)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact4` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact4 = ret.chipId;
			}
			else if (ret.slotId == 5)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `userRoleInfo` set `artifact5` = %llu where `Role` = %llu;", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
				roleInfo.artifact5 = ret.chipId;
			}
			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
		}
	}
	else if (ret.type == 1)//pet
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(NULL, ret.unitId, NULL, petInfo))
		{
			if (ret.slotId == 0)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact0` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact0 = ret.chipId;
			}
			else if (ret.slotId == 1)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact1` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact1 = ret.chipId;
			}
			else if (ret.slotId == 2)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact2` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact2 = ret.chipId;
			}
			else if (ret.slotId == 3)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact3` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact3 = ret.chipId;
			}
			else if (ret.slotId == 4)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact4` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact4 = ret.chipId;
			}
			else if (ret.slotId == 5)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gamePetInfo` set `artifact5` = %llu where `ID` = %llu", ret.chipId, ret.unitId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
				petInfo.Artifact5 = ret.chipId;
			}
			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_compsite)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_artifact_compsite_req req;
	readStream >> req;

	ss_msg_artifact_compsite_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.dst_base = req.dst_base;
	ret.dst_count = req.dst_count;
	ret.src_base = req.src_base;
	ret.src_count = req.src_count;
	ret.nResult = (int)RET_SUCCESS;

	//!TODO
	strEquipInfo src_equipInfo;
	this->reduceEquip(req.userId, req.src_base, src_equipInfo, req.src_count);

	strEquipInfo dst_equipInfo;
	this->addEquipNumOrCreate(req.userId, req.dst_base, dst_equipInfo, req.dst_count);

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_decompsite)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_artifact_decompsite_req req;
	readStream >> req;

	ss_msg_artifact_decompsite_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.quality = req.quality;
	ret.decompNum = req.decompNum;
	ret.id = req.id;
	ret.getItems = req.getItems;
	ret.chips = req.chips;
	ret.nResult = (int)RET_SUCCESS;

	for (size_t i = 0; i < ret.chips.size(); i++)
	{
		strEquipInfo equipInfo;
		this->reduceEquip(ret.chips[i], equipInfo, ret.decompNum[i]);
	}

	strItemInfo itemInfo;
	this->addItemNumOrCreate(req.userId, 44109, itemInfo, req.getItems);

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_combine)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_artifact_combine_req req;
	readStream >> req;

	ss_msg_artifact_combine_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.type = req.type;
	ret.count = req.count;
	ret.accumulate = req.accumulate;
	ret.costAccumulate = req.costAccumulate;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(req.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeRet;
		writeRet << ret;
		socket_->async_write(writeRet);
		return;
	}

	if (ret.accumulate)
	{
		userInfo.ArtifacetAccumulate += ret.accumulate;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `ArtifacetAccumulate` = %llu where `userId` = %llu", userInfo.ArtifacetAccumulate, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	}

	for (size_t i = 0; i < req.items.size(); i++)
	{
		std::vector<uint64_t> newIds;
		this->gainItem(req.items[i], 1, userInfo, newIds);
	}

	//cost
	if (ret.type == 2)
	{
		userInfo.ArtifacetAccumulate -= ret.costAccumulate;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `ArtifacetAccumulate` = %llu where `userId` = %llu", userInfo.ArtifacetAccumulate, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	}

	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::artifact_combin_type, req.count);

	if (req.type == 8)
	{
		updateQuestStateCount(req.userId, 10, dailyTaskCount::artifact_combin_type2, req.count);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(artifact_unlock_slot)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_artifact_unlock_slot_req req;
	readStream >> req;

	ss_msg_pet_artifact_unlock_slot_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.nResult = (int)RET_SUCCESS;

	if (req.type == 0) //主角
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(req.userId, req.unitId, NULL, roleInfo))
		{
			roleInfo.lockArtifactSlot = req.lockArtifactSlot;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `lockedArtifactSlot` = %d where `Role` = %llu;",
				roleInfo.lockArtifactSlot, req.unitId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
		}
	}
	else if (req.type == 1) //宠物
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(req.userId, req.unitId, NULL, petInfo))
		{
			petInfo.LockedArtifactSlot = req.lockArtifactSlot;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `lockedArtifactSlot` = %llu where `ID` = %llu;",
				petInfo.LockedArtifactSlot, req.unitId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
			m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		}

	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(change_name)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_change_name_req req;
	readStream >> req;

	ss_msg_change_name_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.name = req.name;
	ret.cost = req.cost;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.oldName = userInfo.userName;
		m_data_cache.rename_userIdByName(ret.oldName, req.name);

		userInfo.userName = req.name;
		userInfo.ChangeNameTimes += 1;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userName` = '%s', `ChangeNameTimes` = %llu where `userId` = %llu",
			req.name.c_str(), userInfo.ChangeNameTimes, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(buy_remain_count)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_remain_count_req req;
	readStream >> req;

	ss_msg_buy_remain_count_ret ret;
	ret.userId = req.userId;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.id = req.id;
	ret.costGem = req.costGem;
	ret.count = req.count;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.userGem -= ret.costGem;
		userInfo.userCostGem += ret.costGem;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` = %llu where `userId` = %llu", userInfo.userGem, userInfo.userCostGem, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	//std::unordered_map<uint64_t, strPveInfo*>::iterator iterPve = m_cache_pveInfo_map.find(100000 * req.id + req.userId);
	//if (iterPve != m_cache_pveInfo_map.end())
	strPveInfo pveInfo;
	if (m_data_cache.get_pveInfo(req.userId, req.id, pveInfo))
	{
		if (req.id >= 10001 && req.id <= 10042)
		{
			uint64_t subType = 0, Type = 0;
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_SceneList_t*>& mapSceneList = config.get_sceneList_config();
			std::map<uint64_t, config_SceneList_t*>::iterator iterSceneList = mapSceneList.find(ret.id);
			if (iterSceneList != mapSceneList.end())
			{
				subType = iterSceneList->second->m_SubType;
				Type = iterSceneList->second->m_Type;
			}

			std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
			for (int i = 10001; i <= 10042; i++)
			{
				std::map<uint64_t, config_SceneList_t*>::iterator itertamp = mapSceneList.find(i);
				if (itertamp != mapSceneList.end())
				{
					if (itertamp->second->m_SubType == subType && itertamp->second->m_Type == Type)
					{
						//std::unordered_map<uint64_t, strPveInfo*>::iterator iter = m_cache_pveInfo_map.find(100000 * i + ret.userId);
						//if (iter != m_cache_pveInfo_map.end())
						strPveInfo pveInfo2;
						if (m_data_cache.get_pveInfo(ret.userId, i, pveInfo2))
						{
							pveInfo2.refreshCount += 1;
							pveInfo2.remainCount = ret.count;

							char buf[1024] = { 0 };
							sprintf(buf, "update `userPveInfo` set `remainCount` = %llu, `RefreshedCount` = %llu where `userId` = %llu and `pveType` = %llu", ret.count, pveInfo2.refreshCount, req.userId, i);
							singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userPveInfo");
							m_data_cache.set_pveInfo(ret.userId, i, pveInfo2);
						}
					}
				}
			}
		}
		else
		{
			pveInfo.refreshCount += 1;
			pveInfo.remainCount = ret.count;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userPveInfo` set `remainCount` = %llu, `RefreshedCount` = %llu where `userId` = %llu and `pveType` = %llu", ret.count, pveInfo.refreshCount, req.userId, req.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userPveInfo");
			m_data_cache.set_pveInfo(req.userId, req.id, pveInfo);
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(update_pve_frag_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_frag_pve_info req;
	readStream >> req;

	strUserPvePetFragmentInfo pveFragInfo;
	if (m_data_cache.get_pvePetFragmentInfo(req.userId, pveFragInfo))
	{
		bool find = false;
		for (size_t i = 0; i < pveFragInfo.vecInfo.size(); i++)
		{
			if (pveFragInfo.vecInfo[i].pveId == req.info.pveId)
			{
				pveFragInfo.vecInfo[i] = req.info;
				m_db.updateUserPvePetFragmentInfo(pveFragInfo);
				find = true;
				break;
			}
		}

		if (!find)
		{
			pveFragInfo.vecInfo.push_back(req.info);
			m_db.updateUserPvePetFragmentInfo(pveFragInfo);
		}
	}
	else
	{
		strUserPvePetFragmentInfo info;
		info.userId = req.userId;
		info.vecInfo.push_back(req.info);
		m_db.initUserPvePetFragmentInfo(info);
	}
}

MY_FUNCTION1(fetch_pve_star_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_pve_star_reward_req req;
	readStream >> req;

	ss_msg_fetch_pve_star_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.chapterId = req.chapterId;
	ret.subId = req.subId;
	ret.item = req.item;
	ret.itemNum = req.itemNum;
	ret.rewardIndex = req.rewardIndex;
	ret.nResult = (int)RET_SUCCESS;

	this->gainItems(req.item, req.itemNum, req.userId, ret.newId);

	bool find = false;
	strPveStarRewardInfo pveStarRewardInfo;
	if (!m_data_cache.get_pveStarRewardInfo(req.userId, pveStarRewardInfo))
	{
		pveStarRewardInfo.userId = req.userId;
		pveStarRewardInfo.levelId = req.levelId;
		pveStarRewardInfo.rewardState = req.rewardState;

		m_db.initPveStarRewardInfo(pveStarRewardInfo);
	}

	for (size_t i = 0; i < pveStarRewardInfo.levelId.size(); i++)
	{
		if (pveStarRewardInfo.levelId[i] == req.chapterId * 10000 + req.subId)
		{
			pveStarRewardInfo.rewardState[i] += 0x01 << req.rewardIndex;
			break;
		}
	}

	m_db.updatePveStarRewardInfo(pveStarRewardInfo);

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(change_avatar)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_change_avatar_req req;
	readStream >> req;

	ss_msg_change_avatar_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.AvatarID = ret.id;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `AvatarID` = %llu where `userId` = %llu", ret.id, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(init_role_pvp_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_pvp_team_info_req req;
	readStream >> req;

	strRolePvpInfo rolePvpInfo;
	if (!m_data_cache.get_rolePvpInfo(req.userId, NULL, rolePvpInfo))
	{
		strRolePvpInfo* pRolePvp = new strRolePvpInfo;
		pRolePvp->userId = req.userId;
		pRolePvp->name = req.name;
		pRolePvp->rank = 0;
		pRolePvp->Level = 1;
		pRolePvp->exp = 0;
		pRolePvp->power = 0;
		pRolePvp->lastTime = 0;
		pRolePvp->remainCount = req.remainCount;
		pRolePvp->totalCount = 0;
		pRolePvp->winCount = 0;
		pRolePvp->resetTimes = 0;


		char buf[1024] = { 0 };
		sprintf(buf, "insert into `userRolePvpInfo`(`userId`,`name`, `remainCount`, `level`, `totalCount`, `winCount`) values(%llu, '%s', %llu, 1, 0, 0)", req.userId, req.name.c_str(), req.remainCount);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserRolePvpInfo);
		m_data_cache.set_rolePvpInfo(pRolePvp->userId, pRolePvp->rank, *pRolePvp);

		delete pRolePvp;
	}
}

MY_FUNCTION1(refresh_role_pvp_rank_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_refresh_role_pvp_rank_reward req;
	readStream >> req;

	loginfo((LOG_SERVER, "START refresh_role_pvp_rank_reward"));

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::vector<config_levelReward_t>& levelUpReward = config.get_levelReward_config();

	std::vector<config_levelReward_t> role_pvp_reward;
	std::vector<std::vector<uint64_t>> reward_items;
	std::vector<std::vector<uint64_t>> reward_nums;
	for (size_t i = 0; i < levelUpReward.size(); i++)
	{
		if (levelUpReward[i].m_ID == 6000)
		{
			role_pvp_reward.push_back(levelUpReward[i]);
			std::vector<uint64_t> items, nums;
			parseString2Vector(levelUpReward[i].m_RewardItem, items);
			parseString2Vector(levelUpReward[i].m_Count, nums);
			reward_items.push_back(items);
			reward_nums.push_back(nums);
		}
	}

	std::vector<boost::shared_ptr<strRolePvpInfo>> vecRolePvp;
	m_data_cache.mget_rolePvpInfo_noRobot(vecRolePvp);
	for (size_t i = 0; i < vecRolePvp.size(); i++)
	{
		if (vecRolePvp[i]->userId < 100000 || vecRolePvp[i]->rank == 0)
			continue;

		std::vector<uint64_t> items, nums;
		for (size_t j = 0; j < role_pvp_reward.size(); j++)
		{
			if (vecRolePvp[i]->rank <= role_pvp_reward[j].m_RewardCond)
			{
				if (j == 0 || vecRolePvp[i]->rank > role_pvp_reward[j - 1].m_RewardCond)
				{
					items.assign(reward_items[j].begin(), reward_items[j].end());
					nums.assign(reward_nums[j].begin(), reward_nums[j].end());
					break;
				}
			}
		}

		if (nums.size() < 1)
		{
			logerror((LOG_SERVER, "item nums size is zero userId[%llu] rank[%llu]", vecRolePvp[i]->userId, vecRolePvp[i]->rank));
			continue;
		}

		if (nums[0] > 0)
		{
			strRolePvpRankListReward rolePvpRankReward;
			if (m_data_cache.get_rolePvpRankListReward(vecRolePvp[i]->userId, rolePvpRankReward))
			{
				for (size_t j = 0; j < items.size(); j++)
				{
					bool find = false;
					for (size_t k = 0; k < rolePvpRankReward.rewardItems.size(); k++)
					{
						if (items[j] == rolePvpRankReward.rewardItems[k])
						{
							rolePvpRankReward.rewardCount[k] += nums[j];
							find = true;
							break;
						}
					}
					if (!find)
					{
						rolePvpRankReward.rewardItems.push_back(items[j]);
						rolePvpRankReward.rewardCount.push_back(nums[j]);
					}
				}
				m_db.hupdateRolePvpRankListReward(&rolePvpRankReward);
			}
			else
			{
				rolePvpRankReward.userId = vecRolePvp[i]->userId;
				rolePvpRankReward.rewardItems = items;
				rolePvpRankReward.rewardCount = nums;
				m_db.hInitRolePvpRankListReward(&rolePvpRankReward);
			}
		}

	}

	loginfo((LOG_SERVER, "FINISH refresh_role_pvp_rank_reward"));
}

MY_FUNCTION1(finish_new_guide)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_new_guide_req req;
	readStream >> req;

	ss_msg_finish_new_guide_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.nResult = (int)RET_DATA_ERROR;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			roleInfo.newguide_ |= 1ULL << req.id;

			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `NewGuide` = %llu where `Role` = %llu", roleInfo.newguide_, userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			m_data_cache.set_roleInfo(roleInfo.userid, roleInfo.role, roleInfo.rolebase, roleInfo);
			ret.nResult = (int)RET_SUCCESS;
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(charge_for_gem)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_charge_for_gem_req req;
	readStream >> req;

	ss_msg_charge_for_gem_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.itemNum = req.itemNum;
	ret.cost = req.cost;
	ret.totalcost = req.rechargeInfo.totalRecharge;
	ret.id = req.id;
	ret.vipLevel = req.vipLevel;
	ret.nResult = (int)RET_SUCCESS;
	ret.morphId = 0;

	strRechargeInfo rechargeInfo;
	if (!m_data_cache.get_rechargeInfo(req.userId, rechargeInfo))
	{
		rechargeInfo = req.rechargeInfo;
		m_db.hInitRechargeInfo(&rechargeInfo);
	}
	else
	{
		rechargeInfo = req.rechargeInfo;
		m_db.hupdateRechargeInfo(&rechargeInfo);
	}

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		int lastVipLevel = userInfo.userVip;
		if (req.vipLevel > lastVipLevel)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userVip` = %llu where `userId` = %llu", req.vipLevel, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			userInfo.userVip = req.vipLevel;

			if (req.vipLevel >= 10 && lastVipLevel < 10)
			{
				strUserMorpherInfo strMorpher;
				strMorpher.userId = ret.userId;
				strMorpher.base = 1103;
				strMorpher.lv = 1;
				strMorpher.skill1 = 0;
				strMorpher.skill2 = 0;

				char buf[1024] = { 0 };
				sprintf(buf, "insert into `userMorpherInfo`(`userId`, `Base`) values(%llu, 1103)", ret.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserMorpherInfo);
				strMorpher.Id = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserMorpherInfo);

				ret.morphId = strMorpher.Id;
				m_data_cache.set_morpherInfo(strMorpher);
			}
		}

		for (size_t i = 0; i < req.items.size(); i++)
		{
			this->gainItem(req.items[i], req.itemNum[i], userInfo, ret.newId);
		}

		//! 更新今日全服充值
		if (userInfo.u8UserId > 0 || req.receiptData.size() > 0 )
		{
			std::string today_recharge_info = "0";
			uint64_t today_recharge = 0;
			if (m_data_cache.get_globalInfo(global_key_todayrecharge, today_recharge_info))
			{
				try
				{
					today_recharge = boost::lexical_cast<uint64_t>(today_recharge_info);
				}
				catch (boost::bad_lexical_cast& e)
				{
					cout << "EGetGlobalData get today recharge err:" << e.what() << endl;
				}

				today_recharge += req.cost;

				try
				{
					today_recharge_info = boost::lexical_cast<std::string>(today_recharge);
				}
				catch (boost::bad_lexical_cast& e)
				{
					cout << "charge_for_gem err:" << e.what() << endl;
				}
				m_db.hUpdateGlobalInfo(global_key_todayrecharge, today_recharge_info);
			}
			else
			{
				today_recharge += req.cost;
				try
				{
					today_recharge_info = boost::lexical_cast<std::string>(today_recharge);
				}
				catch (boost::bad_lexical_cast& e)
				{
					cout << "charge_for_gem err:" << e.what() << endl;
				}
				m_db.hinitGlobalInfo(global_key_todayrecharge, today_recharge_info);
			}
		}
		
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(update_charge_order)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_charge_order_id_req req;
	readStream >> req;

	char buf[1024] = { 0 };
	sprintf(buf, "insert into `gameOrderInfo`(`userId`, `orderId`, `sku`, `purchaseTime`) values(%llu, %s, %s, %llu )", 
		req.orderInfo.userId, req.orderInfo.orderId.c_str(), req.orderInfo.sku.c_str(), req.orderInfo.purchaseTime);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameOrderInfo);
	m_data_cache.set_orderInfo(req.orderInfo);
}


MY_FUNCTION1(fetch_monthly_sub_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_monthly_sub_reward_req req;
	readStream >> req;

	ss_msg_fetch_monthly_sub_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.itemNum = req.itemNum;

	strRechargeInfo rechargeInfo;
	if (!m_data_cache.get_rechargeInfo(req.userId, rechargeInfo))
	{
		logerror((LOG_SERVER, "charge info not exsit userId[%llu]", req.userId));
		return;
	}

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(req.userId, userInfo))
	{
		logerror((LOG_SERVER, "user info not exsit userId[%llu]", req.userId));
		return;
	}

	if (req.special == 1)
	{
		rechargeInfo.monthlyFetchLastTime = req.monthlyFetchLastTime;
	}
	else if (req.special == 2)
	{
		rechargeInfo.utlimateFetchLastTime = req.utlimateFetchLastTime;
	}

	for (size_t i = 0; i < req.items.size(); i++)
	{
		this->gainItem(req.items[i], req.itemNum[i], userInfo, ret.newId);
	}

	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	m_db.hupdateRechargeInfo(&rechargeInfo);

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}


MY_FUNCTION1(refresh_pvp_team_count)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_refresh_pvp_team_count_req req;
	readStream >> req;

	ss_msg_refresh_pvp_team_count_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.remainCount = req.remainCount;
	ret.costGem = req.costGem;
	ret.nResult = (int)RET_SUCCESS;

	this->costGem(ret.userId, ret.costGem);

	strRolePvpInfo rolePvpInfo;
	if (m_data_cache.get_rolePvpInfo(ret.userId, NULL, rolePvpInfo))
	{
		rolePvpInfo.remainCount = ret.remainCount;
		rolePvpInfo.resetTimes++;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRolePvpInfo` set `remainCount` = %llu, `resetTimes` = %llu where `userId` = %llu", ret.remainCount, rolePvpInfo.resetTimes, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRolePvpInfo);
		m_data_cache.set_rolePvpInfo(rolePvpInfo.userId, rolePvpInfo.rank, rolePvpInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(refresh_pvp_team_cd)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_refresh_pvp_team_cd_req req;
	readStream >> req;

	ss_msg_refresh_pvp_team_cd_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.costGem = req.costGem;
	ret.nResult = (int)RET_SUCCESS;

	strRolePvpInfo rolePvpInfo;
	if (m_data_cache.get_rolePvpInfo(req.userId, NULL, rolePvpInfo))
	{
		rolePvpInfo.lastTime = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRolePvpInfo` set `lastTime` = 0 where `userId` = %llu", req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRolePvpInfo);
		m_data_cache.set_rolePvpInfo(rolePvpInfo.userId, rolePvpInfo.rank, rolePvpInfo);
	}

	this->costGem(ret.userId, ret.costGem);

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(set_team_group)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_team_group_req req;
	readStream >> req;

	ss_msg_set_team_group_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.pets = req.pets;
	ret.nResult = (int)RET_SUCCESS;

	strRolePvpTeamInfo rolePvpTeamInfo;
	if (m_data_cache.get_rolePvpTeamInfo(ret.userId, rolePvpTeamInfo))
	{
		rolePvpTeamInfo.pet1 = ret.pets[0];
		rolePvpTeamInfo.pet2 = ret.pets[1];

		char buf[1024] = { 0 };
		sprintf(buf, "update `userRolePvpTeamInfo` set `pet1` = %llu, `pet2` = %llu where `userId` = %llu", rolePvpTeamInfo.pet1, rolePvpTeamInfo.pet2, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRolePvpTeamInfo);
		m_data_cache.set_rolePvpTeamInfo(rolePvpTeamInfo.userId, rolePvpTeamInfo);
	}
	else
	{
		rolePvpTeamInfo.userId = ret.userId;
		rolePvpTeamInfo.pet1 = ret.pets[0];
		rolePvpTeamInfo.pet2 = ret.pets[1];

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `userRolePvpTeamInfo`(`userId`, `pet1`, `pet2`) values(%llu, %llu, %llu)", rolePvpTeamInfo.userId, rolePvpTeamInfo.pet1, rolePvpTeamInfo.pet2);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserRolePvpTeamInfo);
		m_data_cache.set_rolePvpTeamInfo(rolePvpTeamInfo.userId, rolePvpTeamInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(finish_pvp_team_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_pvp_team_battle_req req;
	readStream >> req;

	ss_msg_finish_pvp_team_battle_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.uid = req.uid;
	ret.item = req.item;
	ret.itemNum = req.itemNum;
	ret.win = req.win;
	ret.time = req.time;
	ret.otherRank = req.otherRank;
	ret.userRank = req.userRank;
	ret.oldRank = req.oldRank;
	ret.friendDamage = req.friendDamage;
	ret.enemyDamage = req.enemyDamage;
	ret.nResult = (int)RET_SUCCESS;

	strRolePvpInfo rolePvpInfo;
	if (!m_data_cache.get_rolePvpInfo(req.userId, NULL, rolePvpInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	strRolePvpInfo otherRolePvpInfo;
	if (!m_data_cache.get_rolePvpInfo(req.uid, NULL, otherRolePvpInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	rolePvpInfo.rank = req.userRank;
	otherRolePvpInfo.rank = req.otherRank;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	ret.newHeighestRank = 0;
	if (userInfo.pvpTeamHightestRank > rolePvpInfo.rank || userInfo.pvpTeamHightestRank == 0)
	{
		ret.newHeighestRank = rolePvpInfo.rank;
		userInfo.pvpTeamHightestRank = rolePvpInfo.rank;
		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `pvpTeamHightestRank` = %llu where userId = %llu", userInfo.pvpTeamHightestRank, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	//remain && time
	if (req.win)
	{
		rolePvpInfo.totalCount++;
		rolePvpInfo.winCount++;
	}
	else
	{
		rolePvpInfo.totalCount++;
	}
	rolePvpInfo.remainCount--;
	rolePvpInfo.lastTime = req.time;

	char buf_self[1024] = { 0 };
	sprintf(buf_self, "update `userRolePvpInfo` set `rank` = %llu, `remainCount` = %llu, `lastTime` = %llu, `totalCount` = %llu, `winCount` = %llu where userId = %llu",
		rolePvpInfo.rank, rolePvpInfo.remainCount, rolePvpInfo.lastTime, rolePvpInfo.totalCount, rolePvpInfo.winCount, rolePvpInfo.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf_self, sql_update, DB_UserRolePvpInfo);
	m_data_cache.set_rolePvpInfo(rolePvpInfo.userId, rolePvpInfo.rank, rolePvpInfo);

	char buf_other[1024] = { 0 };
	sprintf(buf_other, "update `userRolePvpInfo` set `rank` = %llu where userId = %llu",
		otherRolePvpInfo.rank, otherRolePvpInfo.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf_other, sql_update, DB_UserRolePvpInfo);
	m_data_cache.set_rolePvpInfo(otherRolePvpInfo.userId, otherRolePvpInfo.rank, otherRolePvpInfo);

#pragma region 更新自己战斗记录
	//my battle history update
	std::vector<strRolePvpBattleHistory> vecRolePvpHistory;
	if (!m_data_cache.get_rolePvpHistory(ret.userId, vecRolePvpHistory))
		m_db.insertRolePvpHistory(req.userId, vecRolePvpHistory);

	strRolePvpBattleHistory strHistory;
	strHistory.user = ret.uid;
	strHistory.userId = ret.userId;
	strHistory.win = ret.win;
	strHistory.time = ret.time;
	strHistory.friendDamage = ret.friendDamage;
	strHistory.enemyDamage = ret.enemyDamage;
	if (req.win)
	{
		strHistory.rank = ret.userRank;
	}
	else
	{
		strHistory.rank = ret.otherRank;
	}
	strHistory.name = otherRolePvpInfo.name;
	strHistory.attack = true;

	if (ret.uid < 100000)//robot
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_robot_t*>& mapRobot = config.get_robot_config();
		std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.find(ret.uid);
		if (iterRobot != mapRobot.end())
		{
			strHistory.level = iterRobot->second->m_PlayerLevel;
			strHistory.base = iterRobot->second->m_PlayerIcon;
		}
	}
	else
	{
		strUserInfo otherUserInfo;
		if (m_data_cache.get_userInfo(req.uid, otherUserInfo))
		{
			strRoleInfo otherRoleInfo;
			if (m_data_cache.get_roleInfo(NULL, otherUserInfo.userCurRole, NULL, otherRoleInfo))
			{
				strHistory.level = otherRoleInfo.level_;
				strHistory.base = otherRoleInfo.rolebase;
			}
		}
	}

	vecRolePvpHistory.push_back(strHistory);
	if (vecRolePvpHistory.size() > 10)
	{
		std::vector<strRolePvpBattleHistory> vecHistoryNew;
		for (size_t i = vecRolePvpHistory.size() - 10; i < vecRolePvpHistory.size(); i++)
		{
			vecHistoryNew.push_back(vecRolePvpHistory[i]);
		}
		vecRolePvpHistory.clear();
		vecRolePvpHistory.assign(vecHistoryNew.begin(), vecHistoryNew.end());
	}
	m_db.updateRolePvpHistory(req.userId, vecRolePvpHistory);
#pragma endregion

#pragma region 更新对方战斗记录
	//other battle history update
	if (ret.uid >= 100000)
	{
		std::vector<strRolePvpBattleHistory> vecRolePvpHistory;
		if (!m_data_cache.get_rolePvpHistory(ret.uid, vecRolePvpHistory))
			m_db.insertRolePvpHistory(req.uid, vecRolePvpHistory);

		strRolePvpBattleHistory strHistory;
		strHistory.user = ret.userId;
		strHistory.userId = ret.uid;
		strHistory.win = ret.win == 1 ? 0 : 1;
		strHistory.time = ret.time;
		strHistory.friendDamage = ret.enemyDamage;
		strHistory.enemyDamage = ret.friendDamage;
		if (!req.win)
		{
			strHistory.rank = ret.userRank;
		}
		else
		{
			strHistory.rank = ret.otherRank;
		}
		strHistory.name = rolePvpInfo.name;
		strHistory.attack = false;

		if (ret.userId < 100000)//robot
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_robot_t*>& mapRobot = config.get_robot_config();
			std::map<uint64_t, config_robot_t*>::iterator iterRobot = mapRobot.find(ret.uid);
			if (iterRobot != mapRobot.end())
			{
				strHistory.level = iterRobot->second->m_PlayerLevel;
				strHistory.base = iterRobot->second->m_PlayerIcon;
			}
		}
		else
		{
			strUserInfo userInfo;
			if (m_data_cache.get_userInfo(ret.userId, userInfo))
			{
				strRoleInfo roleInfo;
				if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
				{
					strHistory.level = roleInfo.level_;
					strHistory.base = roleInfo.rolebase;
				}
			}
		}
		vecRolePvpHistory.push_back(strHistory);
		if (vecRolePvpHistory.size() > 10)
		{
			std::vector<strRolePvpBattleHistory> vecHistoryNew;
			for (size_t i = vecRolePvpHistory.size() - 10; i < vecRolePvpHistory.size(); i++)
			{
				vecHistoryNew.push_back(vecRolePvpHistory[i]);
			}
			vecRolePvpHistory.clear();
			vecRolePvpHistory.assign(vecHistoryNew.begin(), vecHistoryNew.end());
		}
		m_db.updateRolePvpHistory(req.uid, vecRolePvpHistory);
	}
#pragma endregion

	this->gainItems(req.item, req.itemNum, req.userId, ret.newId);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::rolePvp_type, 1);
}

MY_FUNCTION1(fetch_pvp_team_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_pvp_team_reward_req req;
	readStream >> req;

	ss_msg_fetch_pvp_team_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.items = req.items;
	ret.count = req.count;
	ret.nResult = (int)RET_SUCCESS;

	strRolePvpRankListReward rolePvpRankReward;
	if (m_data_cache.get_rolePvpRankListReward(ret.userId, rolePvpRankReward))
	{
		std::vector<uint64_t> newIds;
		this->gainItems(rolePvpRankReward.rewardItems, rolePvpRankReward.rewardCount, req.userId, newIds);
		rolePvpRankReward.rewardItems.clear();
		rolePvpRankReward.rewardCount.clear();
		m_db.hupdateRolePvpRankListReward(&rolePvpRankReward);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(sell_req)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_sell_req req;
	readStream >> req;

	ss_msg_sell_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.gold = req.gold;
	ret.count = req.count;
	ret.baseId = req.baseId;
	req.isEquip = req.isEquip;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.userGold += ret.gold;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = %llu where `userId` = %llu", userInfo.userGold, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	if (req.isEquip == 0)
	{
		strItemInfo itemInfo;
		if (!this->useItem(ret.id, itemInfo, ret.count))
		{
			ret.nResult = RET_ITEM_NUMBER_NOT_ENOUGH;
		}
	}
	else
	{
		strEquipInfo equipInfo;
		if (!this->reduceEquip(ret.id, equipInfo, ret.count))
		{
			ret.nResult = RET_EQUIP_NUMBER_NOT_ENOUGH;
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(set_war_standard)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_war_standard_req req;
	readStream >> req;

	ss_msg_set_war_standard_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.ids = req.ids;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			char buf[1024] = { 0 };
			sprintf(buf, "update `userRoleInfo` set `warStandard0` =  %llu, `warStandard1` = %llu, `warStandard2` = %llu, `warStandard3` = %llu, `warStandard4` = %llu, `warStandard5` = %llu where `Role` = %llu",
				req.ids[0], req.ids[1], req.ids[2], req.ids[3], req.ids[4], req.ids[5], userInfo.userCurRole);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
			roleInfo.warStandard0 = req.ids[0];
			roleInfo.warStandard1 = req.ids[1];
			roleInfo.warStandard2 = req.ids[2];
			roleInfo.warStandard3 = req.ids[3];
			roleInfo.warStandard4 = req.ids[4];
			roleInfo.warStandard5 = req.ids[5];

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
		}
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(upgrade_war_standard)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_upgrade_war_standard_req req;
	readStream >> req;

	ss_msg_upgrade_war_standard_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.costGold = req.costGold;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userGold -= req.costGold;
		userInfo.userCostGold += req.costGold;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` =  `userGold` - %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu", req.costGold, req.costGold, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strWarStandardInfo warStandardInfo;
	if (m_data_cache.get_warStandardInfo(NULL, req.id, NULL, warStandardInfo))
	{
		warStandardInfo.lv = req.Level;

		char buf[1024] = { 0 };
		sprintf(buf, "update `userWarStandardInfo` set `Lv` = %llu where `ID` = %llu", warStandardInfo.lv, req.id);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserWarStandardInfo);
		m_data_cache.set_warStandardInfo(warStandardInfo);
	}

	for (size_t i = 0; i < req.itemId.size(); i++)
	{
		strItemInfo itemInfo;
		this->useItem(req.itemId[i], itemInfo, req.itemNum[i]);
	}

	if (req.type == 1)
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
		{
			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
		}
	}
	else if (req.type == 2)
	{
		std::vector<boost::shared_ptr<strPetInfo>> vecPetInfo;
		m_data_cache.mget_petInfoByUserId(ret.userId, vecPetInfo);
		for (size_t i = 0; i < vecPetInfo.size(); i++)
		{
			game_util::updatePetInfo(m_data_cache, vecPetInfo[i].get());
			updateDatabasePetInfo(vecPetInfo[i].get());
		}
		MV_SAFE_RELEASE(vecPetInfo);
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);
}

MY_FUNCTION1(role_skill_level_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_skill_level_up_req req;
	readStream >> req;

	ss_msg_skill_level_up_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.costGold = req.costGold;
	ret.userId = req.userId;
	ret.skillId = req.skillId;
	ret.levels = req.levels;
	ret.unitId = req.unitId;
	ret.unitType = req.unitType;
	ret.itemId = req.itemId;
	ret.skillBookNumber = req.skillBookNumber;
	ret.nResult = (int)RET_DATA_ERROR;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		for (size_t i = 0; i < req.skillId.size(); i++)
		{
			strSkillInfo skillInfo;
			if (m_data_cache.get_skillInfo(NULL, req.skillId[i], NULL, skillInfo))
			{
				skillInfo.Lv = req.flevels[i];

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameSkillInfo` set `Lv` = %llu where `ID` = %llu", skillInfo.Lv, req.skillId[i]);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
				m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);
			}
		}

		userInfo.userGold -= req.costGold;
		userInfo.userCostGold += req.costGold;

		char bufUser[1024] = { 0 };
		sprintf(bufUser, "update `gameUserInfo` set `userGold` = `userGold` - %llu, `userCostGold` = `userCostGold` + %llu where `userId` = %llu", req.costGold, req.costGold, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(bufUser, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);

		strItemInfo itemInfo;
		this->useItem(req.itemId, itemInfo, req.skillBookNumber);
		ret.nResult = (int)RET_SUCCESS;
	}

	BinaryWriteStream writeRet;
	writeRet << ret;
	socket_->async_write(writeRet);

	int count = 0;
	for (size_t i = 0; i < ret.levels.size(); i++)
	{
		count += ret.levels[i];
	}

	if (ret.nResult == (int)RET_SUCCESS)
	{
		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::skillLevelUp_type, count);
	}
}

MY_FUNCTION1(update_boss_battle_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_boss_battle_info_req req;
	readStream >> req;

	strBossBattleInfo bossBattleInfo;
	if (!m_data_cache.get_bossBattleInfo(req.userId, bossBattleInfo))
	{
		bossBattleInfo.userId = req.userId;
		bossBattleInfo.unlock = req.unlock;
		bossBattleInfo.vecPet = req.vecPets;
		bossBattleInfo.buyCount = req.vecBuyCount;
		bossBattleInfo.killNumber = req.killNumber;
		bossBattleInfo.rank = req.rank;
		bossBattleInfo.totalDamage = req.totalDamage;
		bossBattleInfo.lastTime = req.lastTime;

		m_db.hInitUserBossBattleInfo(&bossBattleInfo);
	}
	else
	{
		bossBattleInfo.unlock = req.unlock;
		bossBattleInfo.vecPet = req.vecPets;
		bossBattleInfo.buyCount = req.vecBuyCount;
		bossBattleInfo.killNumber = req.killNumber;
		bossBattleInfo.rank = req.rank;
		bossBattleInfo.totalDamage = req.totalDamage;
		bossBattleInfo.lastTime = req.lastTime;

		m_db.hUpdateUserBossBattleInfo(&bossBattleInfo);
	}
}

MY_FUNCTION1(fetch_quest_point_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_quest_point_reward_req req;
	readStream >> req;

	ss_msg_fetch_quest_point_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.id = req.id;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(req.userId, userInfo))
	{
		ret.nResult = (int)RET_ROLE_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	for (size_t i = 0; i < ret.itemId.size(); i++)
	{
		this->gainItem(ret.itemId[i], ret.itemNum[i], userInfo, ret.newItemId);
	}

	userInfo.questStatus |= 0x1 << req.id;;
	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `questStatus` = %llu where `userId` = %llu", userInfo.questStatus, ret.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_quset_list)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_quest_reward_req req;
	readStream >> req;

	ss_msg_fetch_quest_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.addExp = req.addExp;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.id = req.id;
	ret.levelUp = req.levelUp;
	ret.oldLevel = req.oldLevel;
	ret.quesePoint = req.questPoint;

	if (fetchQuestReward(ret.userId, ret.addExp, ret.itemId, ret.itemNum, ret.levelUp, ret.id, ret.newItemId))
	{
		ret.nResult = (int)RET_SUCCESS;
	}

	if (ret.quesePoint != 0)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(ret.userId, userInfo))
		{
			userInfo.questPoint += ret.quesePoint;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `questPoint` = %llu where `userId` = %llu", userInfo.questPoint, ret.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(finish_pve)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_pve_req req;
	readStream >> req;

	ss_msg_finish_pve_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.progress = req.progress;
	ret.maxScore = req.maxScore;
	ret.time = req.time;
	ret.statData = req.statData;
	ret.levelUpOrNot = req.levelUpOrNot;
	ret.oldLevel = req.oldLevel;
	ret.items = req.items;
	ret.numbers = req.numbers;
	ret.roleAddExp = req.roleAddExp;
	ret.roleLevel = req.roleLevel;
	ret.vecPets = req.vecPets;
	ret.vecPetHP = req.vecPetHP;
	ret.vecMonster = req.vecMonster;
	ret.morphBase = req.morphBase;
	ret.userStrength = req.userStrength;
	ret.nResult = RET_SUCCESS;

	if (ret.morphBase != 0)
	{
		strUserMorpherInfo strMorpher;
		strMorpher.userId = ret.userId;
		strMorpher.base = ret.morphBase;
		strMorpher.lv = 1;
		strMorpher.skill1 = 0;
		strMorpher.skill2 = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `userMorpherInfo`(`userId`, `Base`) values(%llu, %llu)", ret.userId, ret.morphBase);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserMorpherInfo);
		strMorpher.Id = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserMorpherInfo);
		ret.morphId = strMorpher.Id;
		m_data_cache.set_morpherInfo(strMorpher);
	}

	if (ret.id >= 12001 && ret.id <= 12012)//!无尽之塔
	{
		strPetEndlessFightingInfo petEndlessInfo;
		if (m_data_cache.get_petEndlessFightingInfo(ret.userId, petEndlessInfo))
		{
			for (size_t i = 0; i < ret.vecPets.size(); i++)
			{
				bool find = false;
				for (size_t j = 0; j < petEndlessInfo.vecPet.size(); j++)
				{
					if (ret.vecPets[i] == petEndlessInfo.vecPet[j])
					{
						find = true;
						petEndlessInfo.vecPetHP[j] = ret.vecPetHP[i];
						break;
					}
				}
				if (!find)
				{
					petEndlessInfo.vecPet.push_back(ret.vecPets[i]);
					petEndlessInfo.vecPetHP.push_back(ret.vecPetHP[i]);
				}
			}
			petEndlessInfo.vecMonster = ret.vecMonster;
			m_db.hupdatePetEndlessFightingInfo(&petEndlessInfo);
		}
		else
		{
			strPetEndlessFightingInfo* pEndless = new strPetEndlessFightingInfo;
			strUserInfo userInfo;
			if (m_data_cache.get_userInfo(ret.userId, userInfo))
			{
				strRoleInfo roleInfo;
				if (m_data_cache.get_roleInfo(NULL, userInfo.userCurRole, NULL, roleInfo))
				{
					pEndless->playerLevel = roleInfo.level_;
				}
			}
			pEndless->userId = ret.userId;
			pEndless->vecPet = ret.vecPets;
			pEndless->vecPetHP = ret.vecPetHP;
			pEndless->vecMonster = ret.vecMonster;

			m_db.hInitPetEndlessFightingInfo(pEndless);
			delete pEndless;
		}

		if (req.win)
		{
			strUserInfo userInfo;
			if (m_data_cache.get_userInfo(ret.userId, userInfo))
			{
				userInfo.CurETLevel = ret.id + 1;

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `CurETLevel` = %llu where `userId` = %llu", ret.id + 1, req.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
				m_data_cache.set_userInfo(userInfo.userId, userInfo);
			}
		}
	}

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();
	std::map<uint64_t, config_SceneList_t*>& mapSceneList = config.get_sceneList_config();
	std::map<uint64_t, config_SceneList_t*>::iterator iterSceneList = mapSceneList.find(ret.id);
	if (iterSceneList != mapSceneList.end())
	{
		if (req.win)
		{
			if (finishPve(ret.items, ret.numbers, ret.id, ret.levelUpOrNot, ret.roleAddExp, ret.roleLevel, ret.userId, ret.progress, ret.maxScore, ret.newID, ret.userStrength))
			{
				ret.nResult = (int)RET_SUCCESS;

				//更新每日任务
				if (iterSceneList->second->m_Type == LEVEL_MAIN_NORMAL && (iterSceneList->second->m_SubType == 1 || iterSceneList->second->m_SubType == 2 || iterSceneList->second->m_SubType == 3))//主线关卡(日常任务)
				{
					updateDailyTaskCountInfo(ret.userId, dailyTaskCount::mainPve_type, 1);
				}
				else if (iterSceneList->second->m_Type == LEVEL_MAIN_NORMAL && (iterSceneList->second->m_SubType == 4 || iterSceneList->second->m_SubType == 5))//英雄关卡(日常任务)
				{
					updateDailyTaskCountInfo(ret.userId, dailyTaskCount::heroPve_type, 1);
					updateQuestStateCount(req.userId, 10, dailyTaskCount::heroPve_type, 1);
				}
				else if (iterSceneList->second->m_Type == LEVEL_MAIN_TIME)//限时本(日常任务)
				{
					updateDailyTaskCountInfo(ret.userId, dailyTaskCount::limitTime_type, 1);
					updateQuestStateCount(req.userId, 10, dailyTaskCount::limitTime_type, 1);
				}
			}
			else
			{
				ret.nResult = (int)RET_DATA_ERROR;
			}
		}

		if (iterSceneList->second->m_Type == LEVEL_MAIN_ENDLESS)//无尽之塔
		{
			updateDailyTaskCountInfo(ret.userId, dailyTaskCount::endLess_type, 1);
		}
		else if (iterSceneList->second->m_Type == LEVEL_MAIN_BOSS_CONTINUE)//Boss连战
		{
			updateDailyTaskCountInfo(ret.userId, dailyTaskCount::heroFight_type, 1);
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(get_delegate_quest_list)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_delegate_quest_list_req req;
	readStream >> req;

	ss_msg_get_delegate_quest_list_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.nResult = RET_SUCCESS;

	std::vector<strDelegateQuest> delegateQuests;
	if (!m_data_cache.get_delegateQuest(req.userId, delegateQuests))
	{
		m_db.hinitUserDelegateQuestList(req.userId, req.vecDelegateQuest, req.vecDelegateEvent);
	}
	else
	{
		m_db.updateUserDelegateQuestList(req.userId, req.vecDelegateQuest, req.vecDelegateEvent);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(refresh_delegate_quest_list)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_refresh_delegate_quest_list_req req;
	readStream >> req;

	ss_msg_refresh_delegate_quest_list_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.id = ret.id;
	ret.type = req.type;
	ret.userId = req.userId;
	ret.DQExp = req.DQExp;
	ret.itemNum = req.itemNum;
	ret.itemId = req.itemId;
	ret.costGem = req.costGem;
	ret.rewardGold = req.rewardGold;
	ret.delegateCount = req.delegateCount;

	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	if (req.type == 0)//重置
	{
		for (size_t i = 0; i < req.itemId.size(); i++)
		{
			this->gainItem(req.itemId[i], req.itemNum[i], userInfo, ret.newItemId);
		}

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `DQExp` =  `DQExp` + %llu, `userGold` = `userGold` + %llu where `userId` = %llu;", req.DQExp, req.rewardGold, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		userInfo.DQExp += req.DQExp;
		userInfo.userGold += req.rewardGold;

		//刷新宠物状态
		std::vector<boost::shared_ptr<strPetInfo>> vecPetInfo;
		m_data_cache.mget_petInfoByUserId(req.userId, vecPetInfo);
		for (size_t j = 0; j < vecPetInfo.size(); j++)
		{
			if (vecPetInfo[j]->DelegateQuest != 0)
			{
				vecPetInfo[j]->DelegateQuest = 0;
				m_data_cache.set_petInfo(vecPetInfo[j]->userId, vecPetInfo[j]->ID, vecPetInfo[j]->base, *vecPetInfo[j].get());
			}
		}
		MV_SAFE_RELEASE(vecPetInfo);

		char buf_user[1024] = { 0 };
		sprintf(buf_user, "update `gamePetInfo` set `DelegateQuest` = 0 where `userId` = %llu;", req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

		for (size_t i = 0; i < req.quests.size(); i++)
		{
			req.quests[i].vecPet.clear();
		}

		updateDailyTaskCountInfo(ret.userId, dailyTaskCount::delegateQuest_type, req.delegateCount);
	}
	else if (req.type == 1)//刷新
	{
		if (req.costGem != 0)
		{
			userInfo.userCostGem += req.costGem;
			userInfo.userGem -= req.costGem;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `userGem` = %llu, `userCostGem` =  %llu where `userId` = %llu;", userInfo.userGem, userInfo.userCostGem, req.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		}
	}

	if (userInfo.DQRefreshTime != req.refreshTime || userInfo.DQRefreshTimes != req.refreshTimes)
	{
		userInfo.DQRefreshTime = req.refreshTime;
		userInfo.DQRefreshTimes = req.refreshTimes;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `DQRefreshTimes` =  %llu, `DQRefreshTime` = %llu where `userId` = %llu", userInfo.DQRefreshTimes, userInfo.DQRefreshTime, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	}

	if (req.type == 0) //!重置清除事件
		req.events.clear();

	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	m_db.updateUserDelegateQuestList(req.userId, req.quests, req.events);

	ret.nResult = RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fast_finish_delegate_quest)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_delegate_quest_req req;
	readStream >> req;

	ss_msg_finish_delegate_quest_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.all = req.all;
	ret.id = req.id;
	ret.cost = req.cost;
	ret.nResult = (int)RET_SUCCESS;
	ret.data = "";

	this->costGem(req.userId, req.cost);

	if (req.id != 0)
	{
		for (size_t i = 0; i < req.quests.size(); i++)
		{
			if (req.quests[i].taskId == req.id)
			{
				req.quests[i].status = Completed;
			}
		}
	}
	else if (req.all)
	{
		for (size_t i = 0; i < req.quests.size(); i++)
		{
			if (req.quests[i].status == Accepted)
			{
				req.quests[i].status = Completed;
			}
		}
	}

	m_db.updateUserDelegateQuestList(req.userId, req.quests, req.events);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(start_delegate_quest)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_start_delegate_quest_req req;
	readStream >> req;

	ss_msg_start_delegate_quest_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.vecInfo = req.vecInfo;
	ret.id = req.id;
	ret.startTime = req.startTime;

	for (size_t i = 0; i < req.vecInfo.size(); i++)
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(NULL, req.vecInfo[i], NULL, petInfo))
		{
			petInfo.DelegateQuest = req.id;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `DelegateQuest` = %llu where `ID` = %llu;", req.id, req.vecInfo[i]);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
			m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
		}
	}

	m_db.updateUserDelegateQuestList(req.userId, req.quests, req.events);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(finish_delegate_event)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_delegate_event_req req;
	readStream >> req;

	ss_msg_finish_delegate_event_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.eventId = req.eventId;
	ret.questId = req.questId;
	ret.success = req.success;
	ret.nResult = (int)RET_SUCCESS;

	std::vector<uint64_t> newItemIds;
	this->gainItems(req.items, req.itemsNum, req.userId, newItemIds);

	m_db.updateUserDelegateQuestList(req.userId, req.quests, req.events);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(set_mine_team_group)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_mine_team_group_req req;
	readStream >> req;

	ss_msg_set_mine_team_group_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.pets = req.pets;

	/*strMineTeamInfo mineTeamInfo;
	if (m_data_cache.get_mineTeamInfo(ret.userId, mineTeamInfo))
	{
	for (size_t i = 0; i < req.pets.size(); i++)
	{
	mineTeamInfo.position[i] = req.pets[i];
	}
	m_db.hupdateMineTeamInfo(&mineTeamInfo);
	}
	else
	{
	strMineTeamInfo pTeam;
	pTeam.userId = req.userId;
	pTeam.status.push_back(0);
	pTeam.status.push_back(0);
	pTeam.status.push_back(0);
	pTeam.alreadyTime.push_back(0);
	pTeam.alreadyTime.push_back(0);
	pTeam.alreadyTime.push_back(0);
	pTeam.lastTime.push_back(0);
	pTeam.lastTime.push_back(0);
	pTeam.lastTime.push_back(0);
	pTeam.mineId.push_back(0);
	pTeam.mineId.push_back(0);
	pTeam.mineId.push_back(0);
	pTeam.lastRewardTime.push_back(0);
	pTeam.lastRewardTime.push_back(0);
	pTeam.lastRewardTime.push_back(0);
	for (size_t i = 0; i < req.pets.size(); i++)
	{
	pTeam.position.push_back(req.pets[i]);
	}

	m_db.hinitMineTeamInfo(&pTeam);
	}*/

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(enter_mine_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_enter_mine_battle_req req;
	readStream >> req;

	ss_msg_enter_mine_battle_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	//ret.teamId = req.teamId;
	ret.mineId = req.mineId;
	ret.pets = req.pets;

	strMineTeamAllInfo mineTeamAllInfo;
	if (!m_data_cache.get_mineTeamInfo(req.userId, mineTeamAllInfo))
	{
		int resWarAttackCount = 0;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterResWarAttackCount = mapGlobal.find("ResWarAttackCount");
		if (iterResWarAttackCount != mapGlobal.end())
			resWarAttackCount = atol(iterResWarAttackCount->second.c_str());

		mineTeamAllInfo.userId = req.userId;
		mineTeamAllInfo.remainTimes = resWarAttackCount;
		mineTeamAllInfo.boughtTimes = 0;

		m_db.hinitMineTeamInfo(&mineTeamAllInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(retreat_mine)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_retreat_mine_req req;
	readStream >> req;

	ss_msg_retreat_mine_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.mineId = req.mineId;
	ret.nResult = (int)RET_SUCCESS;

	game_resource_t& config = singleton_t<game_resource_t>::instance();

	strMineAllInfo mineInfo;
	if (m_data_cache.get_mineInfo(req.mineId, mineInfo))
	{
		std::vector<uint64_t> items;
		std::vector<uint64_t> nums;
		std::vector<uint64_t> newItemId;
		items.push_back(req.item);
		nums.push_back(req.num);
		this->gainItems(items, nums, mineInfo.owner, newItemId);

		if (nums.size() > 0 && nums[0] > 0)
		{
			strMineHistory strHistory;
			strHistory.userId = mineInfo.owner;
			strHistory.status = EMineLogStatus_Reward;
			strHistory.value = req.num;
			strHistory.uid = mineInfo.owner;
			strHistory.owner = mineInfo.owner;
			strHistory.mineBase = mineInfo.baseId;
			strHistory.time = req.time;
			addMineHistory(strHistory);
		}

		strMineTeamAllInfo mineTeamAllInfo;
		if (m_data_cache.get_mineTeamInfo(mineInfo.owner, mineTeamAllInfo))
		{
			std::vector<strMineTeamInfo>::iterator iterTeamInfo = mineTeamAllInfo.teams.begin();
			for (; iterTeamInfo != mineTeamAllInfo.teams.end(); ++iterTeamInfo)
			{
				if (iterTeamInfo->mineId == req.mineId)
				{
					mineTeamAllInfo.teams.erase(iterTeamInfo);
					break;
				}
			}

			m_db.hupdateMineTeamInfo(&mineTeamAllInfo);
		}

		std::map<uint64_t, config_mineMap_t*>& mapMine = config.get_mineMap_config();
		std::map<uint64_t, config_mineMap_t*>::iterator iterMineMapConfig = mapMine.find(mineInfo.id);
		if (iterMineMapConfig != mapMine.end())
		{
			mineInfo.owner = iterMineMapConfig->second->m_RobotID;
			mineInfo.teamId = 0;
			mineInfo.status = 0;
			mineInfo.startTime = 0;
			mineInfo.lastFetchTime = 0;

			char buf[1024] = { 0 };
			sprintf(buf, "update `mineAllInfo` set `owner` = %llu, `teamIndex` = %llu, `status` = %llu, `startTime` = %llu, `lastFetchTime` = %llu where `id` = %llu",
				mineInfo.owner, mineInfo.teamId, mineInfo.status, mineInfo.startTime, mineInfo.lastFetchTime, mineInfo.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_MineAllInfo);
			m_data_cache.set_mineInfo(mineInfo.id, mineInfo);
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(reset_mine_all_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_reset_mine_all_info req;
	readStream >> req;

	uint64_t last_reset_time = 0;
	std::string info;
	if (m_data_cache.get_globalInfo(global_key_minebattle, info))
	{
		try
		{
			last_reset_time = boost::lexical_cast<uint64_t>(info);
		}
		catch (boost::bad_lexical_cast& e)
		{
			cout << e.what() << endl;
		}
	}

	uint64_t changeHour = 0, changeMinites = 0;
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("ResWarRefreshTime");
	if (iterGlobal != mapGlobal.end())
	{
		parseString2Time(iterGlobal->second, changeHour, changeMinites);
	}

	time_t deftime = changeHour * 60 * 60 + changeMinites * 60;

	time_t tNow = time(NULL);
	int days = 0;
	if (last_reset_time > 0)
	{
		days = DaysBetween2Time(tNow - deftime, (time_t)last_reset_time - deftime);
	}

	if (last_reset_time == 0 || days > 0)
	{
		loginfo((LOG_SERVER, "START RESET_MINE_ALL"));

		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_mineMap_t*>& mapMine = config.get_mineMap_config();

		std::vector<boost::shared_ptr<strMineAllInfo>> vecMineInfo;
		m_data_cache.getall_mineInfo(vecMineInfo);
		for (size_t i = 0; i < vecMineInfo.size(); i++)
		{
			std::map<uint64_t, config_mineMap_t*>::iterator iterMineMapConfig = mapMine.find(vecMineInfo[i]->id);
			if (iterMineMapConfig != mapMine.end())
			{
				vecMineInfo[i]->owner = iterMineMapConfig->second->m_RobotID;
				vecMineInfo[i]->teamId = 0;
				vecMineInfo[i]->status = 0;
				vecMineInfo[i]->startTime = 0;
				vecMineInfo[i]->lastFetchTime = 0;

				char buf[1024] = { 0 };
				sprintf(buf, "update `mineAllInfo` set `owner` = %llu, `teamIndex` = %llu, `status` = %llu, `startTime` = %llu, `lastFetchTime` = %llu where `id` = %llu",
					vecMineInfo[i]->owner, vecMineInfo[i]->teamId, vecMineInfo[i]->status, vecMineInfo[i]->startTime, vecMineInfo[i]->lastFetchTime, vecMineInfo[i]->id);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_MineAllInfo);
				m_data_cache.set_mineInfo(vecMineInfo[i]->id, *vecMineInfo[i].get());
			}
		}
		MV_SAFE_RELEASE(vecMineInfo);

		//清除所有战斗记录
		m_data_cache.clearall_mineHistory();

		char buf[1024] = { 0 };
		sprintf(buf, "delete from `mineBattleHistory`");
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_delete, DB_MineBattleHistory);


		//清除所有复仇信息
		m_data_cache.clearall_mineRevengeInfo();
		char buf1[1024] = { 0 };
		sprintf(buf1, "delete from `mineRevengeInfo`");
		singleton_t<db_request_list>::instance().push_request_list(buf1, sql_delete, DB_MineRevengeInfo);

		//!清除队伍信息
		int resWarAttackCount = 0;
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterResWarAttackCount = mapGlobal.find("ResWarAttackCount");
		if (iterResWarAttackCount != mapGlobal.end())
			resWarAttackCount = atol(iterResWarAttackCount->second.c_str());

		std::vector<boost::shared_ptr<strMineTeamAllInfo>> vecMineTeamInfo;
		m_data_cache.getall_mineTeamInfo(vecMineTeamInfo);
		for (size_t i = 0; i < vecMineTeamInfo.size(); i++)
		{
			vecMineTeamInfo[i]->remainTimes = resWarAttackCount;
			vecMineTeamInfo[i]->boughtTimes = 0;
			vecMineTeamInfo[i]->teams.clear();
			m_db.hupdateMineTeamInfo(vecMineTeamInfo[i].get());
		}
		MV_SAFE_RELEASE(vecMineTeamInfo);

		//!重置矿区
		std::vector<uint64_t> mineOpenAreaInfo;
		mineOpenAreaInfo.push_back(1001);
		mineOpenAreaInfo.push_back(1002);
		mineOpenAreaInfo.push_back(1003);
		mineOpenAreaInfo.push_back(1004);
		mineOpenAreaInfo.push_back(1005);
		m_db.hUpdateMineOpenEreaInfo(mineOpenAreaInfo);

		loginfo((LOG_SERVER, "FINISH RESET_MINE_ALL"));

		info = boost::lexical_cast<std::string>(tNow);
		if (last_reset_time == 0)
			m_db.hinitGlobalInfo(global_key_minebattle, info);
		else
			m_db.hUpdateGlobalInfo(global_key_minebattle, info);

	}
}

MY_FUNCTION1(update_mine_status_and_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_mine_status req;
	readStream >> req;

	time_t timeNow = time(NULL);
	int ResWarOccupationTime = 0;
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("ResWarOccupationTime");
	if (iterGlobal != mapGlobal.end())
		ResWarOccupationTime = atol(iterGlobal->second.c_str());

	if (req.type == 0)
	{
		std::vector<boost::shared_ptr<strMineAllInfo>> vecMineInfo;
		m_data_cache.mget_mineInfo_noRobot(vecMineInfo);
		for (size_t i = 0; i < vecMineInfo.size(); i++)
		{
			updateMineStatus(vecMineInfo[i].get(), ResWarOccupationTime, timeNow);
		}
	}
	else
	{
		strMineAllInfo mineInfo;
		if (m_data_cache.get_mineInfo(req.mineId, mineInfo))
		{
			updateMineStatus(&mineInfo, ResWarOccupationTime, timeNow);
		}
	}

}

MY_FUNCTION1(finish_mine_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_mine_battle_req req;
	readStream >> req;

	ss_msg_finish_mine_battle_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.teamId = req.teamId;
	ret.enmeyDetail = req.enmeyDetail;
	ret.friendDetail = req.friendDetail;
	ret.mineId = req.mineId;
	ret.battleData = req.battleData;
	ret.win = req.win;
	ret.uid = req.uid;
	ret.time = req.time;
	ret.ereaId = req.ereaId;

	if (ret.ereaId != 0)
	{
		bool find = false;
		std::vector<uint64_t> mineOpenAreaInfo;
		if (m_data_cache.get_mineOpenAreaInfo(mineOpenAreaInfo))
		{
			for (size_t i = 0; i < mineOpenAreaInfo.size(); i++)
			{
				if (mineOpenAreaInfo[i] == ret.ereaId)
				{
					find = true;
					break;
				}
			}

			if (!find)
			{
				mineOpenAreaInfo.push_back(ret.ereaId);
				m_db.hUpdateMineOpenEreaInfo(mineOpenAreaInfo);
			}
		}
	}

	uint64_t otherTeamId = 0;
	uint64_t otherUid = 0;

	strMineAllInfo mineInfo;
	if (m_data_cache.get_mineInfo(req.mineId, mineInfo))
	{
		//add history
		otherUid = mineInfo.owner;
		otherTeamId = mineInfo.teamId;

#pragma region 历史战斗记录
		if (req.win)
		{
			strMineHistory strHistory;
			strHistory.userId = req.userId;
			strHistory.status = EMineLogStatus_Occupy;
			strHistory.value = 0;
			strHistory.uid = mineInfo.owner;
			strHistory.owner = mineInfo.owner;
			strHistory.mineBase = mineInfo.baseId;
			strHistory.time = req.time;
			addMineHistory(strHistory);
		}

		//add other history
		strMineHistory strHistory;
		strHistory.userId = otherUid;
		strHistory.value = 0;
		strHistory.uid = req.userId;
		strHistory.owner = req.userId;
		strHistory.status = req.win == 0 ? EMineLogStatus_Defense : EMineLogStatus_Fail;
		strHistory.mineBase = mineInfo.baseId;
		strHistory.time = req.time;
		addMineHistory(strHistory);

#pragma endregion

		if (req.win)
		{
			mineInfo.owner = req.userId;
			mineInfo.teamId = req.teamId;
			mineInfo.lastFetchTime = 0;
			mineInfo.status = EMineStatus_Owner;
			mineInfo.startTime = req.time;
			mineInfo.lastFetchTime = 0;

			char buf[1024] = { 0 };
			sprintf(buf, "update `mineAllInfo` set `owner` = %llu, `teamIndex` = %llu, `status` = %llu, `startTime` = %llu, `lastFetchTime` = %llu where `id` = %llu",
				mineInfo.owner, mineInfo.teamId, mineInfo.status, mineInfo.startTime, mineInfo.lastFetchTime, req.mineId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_MineAllInfo);
			m_data_cache.set_mineInfo(mineInfo.id, mineInfo);

			//add team
			strMineTeamAllInfo mineTeamAllInfo;
			if (m_data_cache.get_mineTeamInfo(req.userId, mineTeamAllInfo))
			{
				strMineTeamInfo mineTeamInfo;
				mineTeamInfo.userId = req.userId;
				mineTeamInfo.mineId = req.mineId;
				mineTeamInfo.teamId = req.teamId;
				mineTeamInfo.position = req.position;
				mineTeamInfo.status = 0;

				mineTeamAllInfo.teams.push_back(mineTeamInfo);
				mineTeamAllInfo.remainTimes -= 1;
				m_db.hupdateMineTeamInfo(&mineTeamAllInfo);
			}

			//add reward
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_mineMap_t*>& mineMap = config.get_mineMap_config();
			std::map<uint64_t, config_mineBase_t*>& mineBase = config.get_mineBase_config();
			std::map<uint64_t, config_mineMap_t*>::iterator iterMineMap = mineMap.find(mineInfo.id);
			if (iterMineMap != mineMap.end())
			{
				std::map<uint64_t, config_mineBase_t*>::iterator iterBase = mineBase.find(iterMineMap->second->m_MineID);
				if (iterBase != mineBase.end())
				{
					strUserInfo userInfo;
					if (m_data_cache.get_userInfo(mineInfo.owner, userInfo))
					{
						std::vector<uint64_t> newItems;
						this->gainItem(iterBase->second->m_PillageItem, iterBase->second->m_pillageCount, userInfo, newItems);
						m_data_cache.set_userInfo(userInfo.userId, userInfo);
					}
				}
			}
		}
	}

	if (req.win && otherUid >= 100000)
	{
		//!失败后未领取奖励
		std::vector<uint64_t> items;
		std::vector<uint64_t> nums;
		std::vector<uint64_t> newItemId;
		items.push_back(req.item);
		nums.push_back(req.num);
		this->gainItems(items, nums, otherUid, newItemId);

		if (nums.size() > 0 && nums[0] > 0)
		{
			strMineHistory strHistory;
			strHistory.userId = otherUid;
			strHistory.status = EMineLogStatus_Reward;
			strHistory.value = req.num;
			strHistory.uid = otherUid;
			strHistory.owner = otherUid;
			strHistory.mineBase = mineInfo.baseId;
			strHistory.time = req.time;
			addMineHistory(strHistory);
		}

		strMineTeamAllInfo otherMineTeamInfo;
		if (m_data_cache.get_mineTeamInfo(otherUid, otherMineTeamInfo))
		{
			std::vector<strMineTeamInfo>::iterator iterTeamInfo = otherMineTeamInfo.teams.begin();
			for (; iterTeamInfo != otherMineTeamInfo.teams.end(); ++iterTeamInfo)
			{
				if (iterTeamInfo->mineId == req.mineId)
				{
					otherMineTeamInfo.teams.erase(iterTeamInfo);
					break;
				}
			}

			//如果打的对手是玩家，则给玩家增加复仇信息
			if (mineInfo.owner >= 100000)
			{
				std::vector<strMineRevengeInfo> revengeInfo;
				if (m_data_cache.get_mineRevengeInfo(mineInfo.owner, revengeInfo))
				{
					strMineRevengeInfo strRevenge;
					strRevenge.userId = mineInfo.owner;
					strRevenge.uid = ret.userId;

					revengeInfo.push_back(strRevenge);
					m_db.hUpdateMineRevengeInfo(mineInfo.owner, revengeInfo);
				}
				else
				{
					strMineRevengeInfo strRevenge;
					strRevenge.userId = mineInfo.owner;
					strRevenge.uid = ret.userId;

					revengeInfo.push_back(strRevenge);
					m_db.hInitMineRevengeInfo(mineInfo.owner, revengeInfo);
				}
			}
		}
		m_db.hupdateMineTeamInfo(&otherMineTeamInfo);
	}

	//清除战斗信息
	m_data_cache.del_mineBattleInfo(req.userId);

	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::mineBattle_type, 1);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(get_mine_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_mine_reward_req req;
	readStream >> req;

	ss_msg_get_mine_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.mineId = req.mineId;
	ret.time = req.time;
	ret.item = req.item;
	ret.num = req.num;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.ownerId, userInfo))
	{
		std::vector<uint64_t> newItems;
		this->gainItem(req.item, req.num, userInfo, newItems);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	strMineAllInfo mineInfo;
	if (m_data_cache.get_mineInfo(req.mineId, mineInfo))
	{
		mineInfo.lastFetchTime = req.time;
		m_data_cache.set_mineInfo(req.mineId, mineInfo);
	}

	strMineHistory strHistory;
	strHistory.userId = req.userId;
	strHistory.status = EMineLogStatus_Reward;
	strHistory.value = req.num;
	strHistory.uid = mineInfo.owner;
	strHistory.owner = mineInfo.owner;
	strHistory.mineBase = mineInfo.baseId;
	strHistory.time = req.time;
	addMineHistory(strHistory);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(buy_mine_times)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_mine_times_req req;
	readStream >> req;

	ss_msg_buy_mine_times_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;

	strMineTeamAllInfo mineTeamAllInfo;
	if (m_data_cache.get_mineTeamInfo(req.userId, mineTeamAllInfo))
	{
		mineTeamAllInfo.boughtTimes++;
		mineTeamAllInfo.remainTimes++;
		m_db.hupdateMineTeamInfo(&mineTeamAllInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(add_friend)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_add_friend_req req;
	readStream >> req;

	ss_msg_add_friend_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.uid = req.uid;
	ret.userName = req.userName;
	ret.nResult = (int)RET_SUCCESS;

	strFriendInfo friendInfo;
	if (!m_data_cache.get_friendInfo(req.uid, friendInfo))
	{
		friendInfo.userId = req.uid;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("FriendStaGetMaximum");
		if (iterGlobal != mapGlobal.end())
			friendInfo.leftGetStrengthTimes = atol(iterGlobal->second.c_str());

		iterGlobal = mapGlobal.find("FriendStaGainMaximum");
		if (iterGlobal != mapGlobal.end())
			friendInfo.leftSendStrengthTimes = atol(iterGlobal->second.c_str());

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `friendInfo`(`userId`, `leftGetStrengthTimes`, `leftSendStrengthTimes`) values(%llu, %llu, %llu)", friendInfo.userId, friendInfo.leftGetStrengthTimes, friendInfo.leftSendStrengthTimes);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserFriendInfo);
	}
	bool find = false;
	for (size_t i = 0; i < friendInfo.responseList.size(); i++)
	{
		if (req.userId == friendInfo.responseList[i])
		{
			find = true;
			break;
		}
	}
	if (!find)
	{
		friendInfo.responseList.push_back(req.userId);

		//!限制在30人
		if (friendInfo.responseList.size() > 30)
		{
			std::reverse(friendInfo.responseList.begin(), friendInfo.responseList.end());
			friendInfo.responseList.resize(30);
			std::reverse(friendInfo.responseList.begin(), friendInfo.responseList.end());
		}
	}

	Json::Value responseList = Json::arrayValue;
	Json::FastWriter write;
	for (size_t i = 0; i < friendInfo.responseList.size(); i++)
	{
		responseList.append(friendInfo.responseList[i]);
	}

	std::string contents = write.write(responseList);

	char buf[1024 * 4] = { 0 };
	sprintf(buf, "update `friendInfo` set `responseList` = '%s' where `userId` = %llu", contents.c_str(), req.uid);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
	m_data_cache.set_friendInfo(friendInfo.userId, friendInfo);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(delete_friend)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_delete_friend_req req;
	readStream >> req;

	ss_msg_delete_friend_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.uid = req.uid;
	ret.nResult = (int)RET_SUCCESS;

	strFriendInfo friendInfo;
	if (m_data_cache.get_friendInfo(req.userId, friendInfo))
	{
		std::vector<uint64_t> vecFriend;
		for (size_t i = 0; i < friendInfo.friendList.size(); i++)
		{
			if (friendInfo.friendList[i] == req.uid)
			{
				continue;
			}
			vecFriend.push_back(friendInfo.friendList[i]);
		}

		friendInfo.friendList = vecFriend;
	}

	Json::Value friendList = Json::arrayValue;
	Json::FastWriter write;
	for (size_t i = 0; i < friendInfo.friendList.size(); i++)
	{
		friendList.append(friendInfo.friendList[i]);
	}
	std::string contents = write.write(friendList);

	char buf[1024 * 4] = { 0 };
	sprintf(buf, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents.c_str(), req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
	m_data_cache.set_friendInfo(friendInfo.userId, friendInfo);

#pragma region 对方也要移除自己

	//!对方也要移除自己
	strFriendInfo otherFriendInfo;
	if (m_data_cache.get_friendInfo(req.uid, otherFriendInfo))
	{
		std::vector<uint64_t> vecFriend;
		for (size_t i = 0; i < otherFriendInfo.friendList.size(); i++)
		{
			if (otherFriendInfo.friendList[i] == req.userId)
			{
				continue;
			}
			vecFriend.push_back(otherFriendInfo.friendList[i]);
		}

		otherFriendInfo.friendList = vecFriend;

		friendList.clear();
		for (size_t i = 0; i < otherFriendInfo.friendList.size(); i++)
		{
			friendList.append(otherFriendInfo.friendList[i]);
		}
		std::string contents2 = write.write(friendList);

		char buf2[1024 * 4] = { 0 };
		sprintf(buf2, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents2.c_str(), req.uid);
		singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_UserFriendInfo);
		m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);
	}

#pragma endregion

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(set_black_friend)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_set_black_friend_req req;
	readStream >> req;

	ss_msg_set_black_friend_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.black = req.black;
	ret.uid = req.uid;
	ret.nResult = (int)RET_SUCCESS;

	strFriendInfo friendInfo;
	if (!m_data_cache.get_friendInfo(req.userId, friendInfo))
	{
		friendInfo.userId = req.userId;
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("FriendStaGetMaximum");
		if (iterGlobal != mapGlobal.end())
			friendInfo.leftGetStrengthTimes = atol(iterGlobal->second.c_str());

		iterGlobal = mapGlobal.find("FriendStaGainMaximum");
		if (iterGlobal != mapGlobal.end())
			friendInfo.leftSendStrengthTimes = atol(iterGlobal->second.c_str());

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `friendInfo`(`userId`, `leftGetStrengthTimes`, `leftSendStrengthTimes`) values(%llu, %llu, %llu)", friendInfo.userId, friendInfo.leftGetStrengthTimes, friendInfo.leftSendStrengthTimes);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserFriendInfo);
	}

	if (req.black)
	{
		bool find = false;
		for (size_t i = 0; i < friendInfo.blackList.size(); i++)
		{
			if (friendInfo.blackList[i] == ret.uid)
			{
				find = true;
			}
		}
		if (!find)
		{
			friendInfo.blackList.push_back(ret.uid);
		}

		std::vector<uint64_t> vecFriend;
		for (size_t i = 0; i < friendInfo.friendList.size(); i++)
		{
			if (friendInfo.friendList[i] == req.uid)
			{
				continue;;
			}
			vecFriend.push_back(friendInfo.friendList[i]);
		}
		friendInfo.friendList = vecFriend;

#pragma region 对方也要移除自己

		strFriendInfo otherFriendInfo;
		if (m_data_cache.get_friendInfo(req.uid, otherFriendInfo))
		{
			std::vector<uint64_t> vecFriend;
			for (size_t i = 0; i < otherFriendInfo.friendList.size(); i++)
			{
				if (otherFriendInfo.friendList[i] == req.userId)
				{
					continue;
				}
				vecFriend.push_back(otherFriendInfo.friendList[i]);
			}

			otherFriendInfo.friendList = vecFriend;

			Json::Value friendList = Json::arrayValue;
			Json::FastWriter write;
			for (size_t i = 0; i < otherFriendInfo.friendList.size(); i++)
			{
				friendList.append(otherFriendInfo.friendList[i]);
			}
			std::string contents2 = write.write(friendList);

			char buf2[1024 * 4] = { 0 };
			sprintf(buf2, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents2.c_str(), req.uid);
			singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_UserFriendInfo);
			m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);
		}

#pragma endregion

	}
	else
	{
		std::vector<uint64_t> vecBlack;
		for (size_t i = 0; i < friendInfo.blackList.size(); i++)
		{
			if (friendInfo.blackList[i] == req.uid)
			{
				continue;;
			}
			vecBlack.push_back(friendInfo.blackList[i]);
		}
		friendInfo.blackList = vecBlack;
	}

	Json::Value blackList = Json::arrayValue;
	Json::FastWriter write;
	for (size_t i = 0; i < friendInfo.blackList.size(); i++)
	{
		blackList.append(friendInfo.blackList[i]);
	}
	std::string contents = write.write(blackList);

	char buf[1024 * 4] = { 0 };
	sprintf(buf, "update `friendInfo` set `blackList` = '%s' where `userId` = %llu", contents.c_str(), req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);

	Json::Value friendList = Json::arrayValue;
	for (size_t i = 0; i < friendInfo.friendList.size(); i++)
	{
		friendList.append(friendInfo.friendList[i]);
	}
	std::string contents2 = write.write(friendList);

	char buf2[1024 * 4] = { 0 };
	sprintf(buf2, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents2.c_str(), req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_UserFriendInfo);
	m_data_cache.set_friendInfo(friendInfo.userId, friendInfo);
	m_data_cache.set_friendInfo(friendInfo.userId, friendInfo);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(response_add_friend)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_response_add_friend_req req;
	readStream >> req;

	ss_msg_response_add_friend_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.accept = req.accept;
	ret.all = req.all;
	ret.uid = req.uid;
	ret.nResult = (int)RET_SUCCESS;

	strFriendInfo friendInfo;
	if (m_data_cache.get_friendInfo(req.userId, friendInfo))
	{
		if (req.accept)
		{
			int friendMaximum = 50;
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::string friendMaximumValue;
			if (config.getGlobalValue("FriendMaximum", friendMaximumValue))
				friendMaximum = atoi(friendMaximumValue.c_str());

			std::vector<uint64_t> vecResponse;
			for (size_t i = 0; i < friendInfo.responseList.size(); i++)
			{
				if (!req.all && friendInfo.responseList[i] != req.uid)
				{
					//!单个同意的时候保存没有添加的用户
					vecResponse.push_back(friendInfo.responseList[i]);
				}
				else
				{
					//friendInfo.friendList.push_back(req.uid);
					strFriendInfo otherFriendInfo;
					if (!m_data_cache.get_friendInfo(friendInfo.responseList[i], otherFriendInfo))
					{
						otherFriendInfo.userId = friendInfo.responseList[i];
						game_resource_t& config = singleton_t<game_resource_t>::instance();
						std::map<std::string, std::string>& mapGlobal = config.get_global_config();
						std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("FriendStaGetMaximum");
						if (iterGlobal != mapGlobal.end())
						{
							otherFriendInfo.leftGetStrengthTimes = atol(iterGlobal->second.c_str());
						}
						iterGlobal = mapGlobal.find("FriendStaGainMaximum");
						if (iterGlobal != mapGlobal.end())
						{
							otherFriendInfo.leftSendStrengthTimes = atol(iterGlobal->second.c_str());
						}

						char buf[1024] = { 0 };
						sprintf(buf, "insert into `friendInfo`(`userId`, `leftGetStrengthTimes`, `leftSendStrengthTimes`) values(%llu, %llu, %llu)",
							otherFriendInfo.userId, otherFriendInfo.leftGetStrengthTimes, otherFriendInfo.leftSendStrengthTimes);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
						m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);
					}

					if (otherFriendInfo.friendList.size() >= friendMaximum)
						continue;

					bool find = false;
					for (size_t ii = 0; ii < otherFriendInfo.friendList.size(); ii++)
					{
						if (otherFriendInfo.friendList[ii] == req.userId)
						{
							find = true;
							break;
						}
					}
					if (!find)
					{
						otherFriendInfo.friendList.push_back(req.userId);
						Json::Value friendList = Json::arrayValue;
						Json::FastWriter write;
						for (size_t ii = 0; ii < otherFriendInfo.friendList.size(); ii++)
						{
							friendList.append(otherFriendInfo.friendList[ii]);
						}
						std::string contents = write.write(friendList);

						char buf[1024 * 4] = { 0 };
						sprintf(buf, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents.c_str(), friendInfo.responseList[i]);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
						m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);
					}

					//add other
					find = false;
					for (size_t ii = 0; ii < friendInfo.friendList.size(); ii++)
					{
						if (friendInfo.friendList[ii] == friendInfo.responseList[i])
						{
							find = true;
							break;
						}
					}

					if (!find)
					{
						friendInfo.friendList.push_back(friendInfo.responseList[i]);
						Json::Value friendList = Json::arrayValue;
						Json::FastWriter write;
						for (size_t ii = 0; ii < friendInfo.friendList.size(); ii++)
						{
							friendList.append(friendInfo.friendList[ii]);
						}
						std::string contents = write.write(friendList);

						char buf[1024 * 4] = { 0 };
						sprintf(buf, "update `friendInfo` set `friendList` = '%s' where `userId` = %llu", contents.c_str(), req.userId);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
					}
				}
			}

			friendInfo.responseList = vecResponse;
		}
		else
		{
			if (req.all)
			{
				friendInfo.responseList.clear();
			}
			else
			{
				friendInfo.responseList.erase(std::remove(friendInfo.responseList.begin(), friendInfo.responseList.end(), req.uid), friendInfo.responseList.end());
			}
		}
	}

	Json::Value responseList = Json::arrayValue;
	Json::FastWriter write;

	for (size_t i = 0; i < friendInfo.responseList.size(); i++)
	{
		responseList.append(friendInfo.responseList[i]);
	}
	std::string contents = write.write(responseList);

	char buf[1024*4] = { 0 };
	sprintf(buf, "update `friendInfo` set `responseList` = '%s' where `userId` = %llu", contents.c_str(), req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
	m_data_cache.set_friendInfo(req.userId, friendInfo);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_friend_gift)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_friend_gift_req req;
	readStream >> req;

	ss_msg_fetch_friend_gift_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.userIds = req.userIds;
	ret.nResult = (int)RET_SUCCESS;

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<std::string, std::string>::iterator iterValue = mapGlobal.find("FriendStaGetCount");
	int friendStaGetCount = 2;
	if (iterValue != mapGlobal.end())
	{
		friendStaGetCount = atol(iterValue->second.c_str());
	}

	int iNumber = 0;
	strFriendInfo friendInfo;
	if (m_data_cache.get_friendInfo(req.userId, friendInfo))
	{
		std::vector<uint64_t> newGetGiftList;
		for (size_t i = 0; i < friendInfo.getGiftList.size(); i++)
		{
			bool find = false;
			for (size_t j = 0; j < req.userIds.size(); j++)
			{
				if (req.userIds[j] == friendInfo.getGiftList[i])
				{
					iNumber += friendStaGetCount;
					find = true;
					break;
				}
			}

			if (!find)
			{
				newGetGiftList.push_back(friendInfo.getGiftList[i]);
			}
		}

		friendInfo.getGiftList = newGetGiftList;
		friendInfo.leftGetStrengthTimes -= req.userIds.size();
	}

	Json::Value getGiftList = Json::arrayValue;
	Json::FastWriter write;
	for (size_t i = 0; i < friendInfo.getGiftList.size(); i++)
	{
		getGiftList.append(friendInfo.getGiftList[i]);
	}
	std::string contents = write.write(getGiftList);

	char buf[1024 * 4] = { 0 };
	sprintf(buf, "update `friendInfo` set `getGiftList` = '%s', `leftGetStrengthTimes` = %llu where `userId` = %llu", contents.c_str(), friendInfo.leftGetStrengthTimes, req.userId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
	m_data_cache.set_friendInfo(req.userId, friendInfo);

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.userStrength += iNumber;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu", userInfo.userStrength, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(user_update_login_time)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_user_update_login_time_req req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.lastLoginAt = req.timeNow;
		userInfo.userDeviceId = req.sDeviceId;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `lastLoginAt` = %llu, `userDeviceID` = '%s' where `userId` = %llu", req.timeNow, req.sDeviceId.c_str(), req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

MY_FUNCTION1(user_offline)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_user_offline_req req;
	readStream >> req;

	uint64_t userId = 0;
	if (m_data_cache.get_userIdByAccount(req.userName, userId))
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(userId, userInfo))
		{
			userInfo.lastOfflineTime = req.timeNow;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `lastOfflineTime` = %llu where `userId` = %llu", req.timeNow, userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
	}
}

MY_FUNCTION1(create_guild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_create_guild_req req;
	readStream >> req;

	ss_msg_create_guild_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.name = req.name;
	ret.notice = req.notice;

	Json::Value info, members;
	members.append(req.userId);
	info["members"] = members;

	Json::FastWriter writer;
	std::string contents = writer.write(info);
	char buf[1024] = { 0 };
	sprintf(buf, "insert into `gameGuildInfo`(`guildName`, `leaderId`,`members`, `lv`, `notice`) values('%s', %llu, '%s', 1, '%s');", req.name.c_str(), req.userId, contents.c_str(), req.notice.c_str());
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameGuildInfo);
	ret.guildId = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameGuildInfo);

	strGuildInfo* pGuild = new strGuildInfo;
	pGuild->id = ret.guildId;
	pGuild->leaderId = req.userId;
	pGuild->guildName = req.name;
	pGuild->lv = 1;
	pGuild->exp = 0;
	pGuild->guildProgress = 0;
	pGuild->members.push_back(req.userId);
	pGuild->notice = req.notice;

	m_data_cache.set_guildInfo(pGuild->id, *pGuild);

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		userInfo.guildId = ret.guildId;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `guildId` = %llu where `userId` = %llu", ret.guildId, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	delete pGuild;
}

MY_FUNCTION1(change_guild_desc)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_change_guild_desc_req req;
	readStream >> req;

	strGuildInfo guildInfo;
	if (m_data_cache.get_guildInfo(req.guildId, guildInfo))
	{
		guildInfo.notice = req.desc;

		char buf[1024 * 4] = { 0 };
		sprintf(buf, "update `gameGuildInfo` set `notice` = '%s' where `id` =%llu;", req.desc.c_str(), req.guildId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
		m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
	}
}

MY_FUNCTION1(join_guild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_join_guild_req req;
	readStream >> req;

	ss_msg_join_guild_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;

	strGuildInfo guildInfo;
	if (m_data_cache.get_guildInfo(req.id, guildInfo))
	{
		guildInfo.response.push_back(req.userId);

		//!限制在30人
		if (guildInfo.response.size() > 30)
		{
			std::reverse(guildInfo.response.begin(), guildInfo.response.end());
			guildInfo.response.resize(30);
			std::reverse(guildInfo.response.begin(), guildInfo.response.end());
		}

		Json::Value info, response;
		for (size_t i = 0; i < guildInfo.response.size(); i++)
		{
			response.append(guildInfo.response[i]);
		}
		info["response"] = response;

		Json::FastWriter writer;
		std::string contents = writer.write(info);
		char buf[1024*4] = { 0 };
		sprintf(buf, "update `gameGuildInfo` set `response` = '%s' where `id` =%llu;", contents.c_str(), req.id);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
		m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
	}

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(response_join_guild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_response_join_guild_req req;
	readStream >> req;

	ss_msg_response_join_guild_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.uid = req.uid;
	ret.accept = req.accept;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		std::vector<uint64_t> vecResponse;

		strGuildInfo guildInfo;
		if (m_data_cache.get_guildInfo(userInfo.guildId, guildInfo))
		{
			//移除请求列表
			for (size_t i = 0; i < guildInfo.response.size(); i++)
			{
				if (guildInfo.response[i] == req.uid)
					continue;
				vecResponse.push_back(guildInfo.response[i]);
			}
			guildInfo.response = vecResponse;

			if (req.accept)
			{
				guildInfo.members.push_back(req.uid);
				userInfo.guildId = guildInfo.id;

				strUserInfo otherUserInfo;
				if (m_data_cache.get_userInfo(req.uid, otherUserInfo))
				{
					otherUserInfo.guildId = guildInfo.id;
					char buf[1024] = { 0 };
					sprintf(buf, "update `gameUserInfo` set `guildId` = %llu where `userId` = %llu", guildInfo.id, otherUserInfo.userId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
					m_data_cache.set_userInfo(otherUserInfo.userId, otherUserInfo);
				}
			}

			Json::Value members, response, info1, info2;
			for (size_t i = 0; i < guildInfo.response.size(); i++)
				response.append(guildInfo.response[i]);
			info2["response"] = response;

			for (size_t i = 0; i < guildInfo.members.size(); i++)
				members.append(guildInfo.members[i]);
			info1["members"] = members;

			Json::FastWriter writer;
			std::string contents = writer.write(info2);
			std::string contents2 = writer.write(info1);

			char buf[1024*4] = { 0 };
			sprintf(buf, "update `gameGuildInfo` set `members` = '%s', `response` = '%s' where `id` = %llu", contents2.c_str(), contents.c_str(), guildInfo.id);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
			m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
		}
	}

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(leave_guild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_leave_guild_req req;
	readStream >> req;

	ss_msg_leave_guild_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.leaveTime = req.leaveTime;

	strGuildInfo guildInfo;
	if (m_data_cache.get_guildInfo(req.id, guildInfo))
	{
		std::vector<uint64_t> vecMembers;
		for (size_t i = 0; i < guildInfo.members.size(); i++)
		{
			if (guildInfo.members[i] == ret.userId)
			{
				continue;
			}
			vecMembers.push_back(guildInfo.members[i]);
		}
		guildInfo.members = vecMembers;

		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(ret.userId, userInfo))
		{
			userInfo.guildId = 0;
			userInfo.leaveGuildCDTime = ret.leaveTime;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `guildId` = 0, `leaveGuildCDTime` = %llu where `userId` = %llu", ret.leaveTime, ret.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}

		Json::Value members, info;
		for (size_t i = 0; i < guildInfo.members.size(); i++)
		{
			members.append(guildInfo.members[i]);
		}
		info["members"] = members;

		Json::FastWriter writer;
		std::string contents = writer.write(info);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameGuildInfo` set `members` = '%s' where `id` = %llu", contents.c_str(), ret.id);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
		m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(doante_guild)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_donate_guild_req req;
	readStream >> req;

	ss_msg_donate_guild_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.donateItem = req.donateItem;
	ret.donateCount = req.donateCount;
	ret.guildExp = req.guildExp;
	ret.guildCoin = req.guildCoin;
	ret.donateScore = req.donateScore;
	ret.guildLv = req.guildLv;
	ret.type = req.type;

	//gold
	if (ret.donateItem == CURRENCY_GOLD)
	{
		this->costGold(req.userId, ret.donateCount);
	}
	//gem
	else if (ret.donateItem == CURRENCY_GEM)
	{
		this->costGem(req.userId, ret.donateCount);
	}

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		strGuildInfo guildInfo;
		if (m_data_cache.get_guildInfo(userInfo.guildId, guildInfo))
		{
			guildInfo.exp = ret.guildExp;
			guildInfo.lv = ret.guildLv;
			guildInfo.guildProgress += ret.donateScore;
			char buf[1024] = { 0 };
			sprintf(buf, "update `gameGuildInfo` set `lv` = %llu, `exp` = %llu, `guildProgress` = %llu where `id` = %llu", 
				guildInfo.lv, guildInfo.exp, guildInfo.guildProgress, userInfo.guildId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
			m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
		}

		userInfo.guildProgress += ret.donateScore;
		userInfo.guildTotalProgress += ret.donateScore;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `guildProgress` = %llu, `guildTotalProgress` = %llu where `userId` = %llu",
			userInfo.guildProgress, userInfo.guildTotalProgress, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

		userInfo.guildDonateStatus = ret.type;

		char buf2[1024] = { 0 };
		sprintf(buf2, "update `gameUserInfo` set `guildDonateStatus` = %llu where `userId` = %llu", userInfo.guildDonateStatus, ret.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	//add item
	strItemInfo itemInfo;
	if (addItemNumOrCreate(req.userId, 44104, itemInfo, ret.guildCoin))
	{
		ret.newItemId = itemInfo.ID;
	}

	updateDailyTaskCountInfo(req.userId, dailyTaskCount::guildContribution_type, 1);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_guild_donate_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_guild_donate_reward_req req;
	readStream >> req;

	ss_msg_fetch_guild_donate_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index = req.index;
	ret.index_id = req.index_id;
	ret.rewardCount = req.rewardCount;
	ret.rewardItem = req.rewardItem;
	ret.rewardScore = req.rewardScore;
	ret.userId = req.userId;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.guildFetched |= 0x01 << req.index;

		this->gainItem(req.rewardItem, req.rewardCount, userInfo, ret.newItemIds);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `guildFetched` = %llu where `userId` = %llu", userInfo.guildFetched, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(kick_guild_member)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_kick_guild_member_req req;
	readStream >> req;

	ss_msg_kick_guild_member_ret ret;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.index_id = req.index_id;
	ret.uid = req.uid;
	ret.leaveTime = req.leaveTime;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		strGuildInfo guildInfo;
		if (m_data_cache.get_guildInfo(userInfo.guildId, guildInfo))
		{
			guildInfo.members.erase(std::remove(guildInfo.members.begin(), guildInfo.members.end(), req.uid), guildInfo.members.end());

			Json::Value members, info;
			for (size_t i = 0; i < guildInfo.members.size(); i++)
				members.append(guildInfo.members[i]);

			info["members"] = members;

			Json::FastWriter writer;
			std::string members_json = writer.write(info);

			if (req.otherJob == EGuildJob::ASSISTANT)
			{
				guildInfo.assistantIds.erase(std::remove(guildInfo.assistantIds.begin(), guildInfo.assistantIds.end(), req.uid), guildInfo.assistantIds.end());

				Json::Value assistant;
				for (size_t i = 0; i < guildInfo.assistantIds.size(); i++)
					assistant.append(guildInfo.assistantIds[i]);

				Json::FastWriter writer;
				std::string assistants_json = writer.write(assistant);

				char buf[2048] = { 0 };
				sprintf(buf, "update `gameGuildInfo` set `members` = '%s', `assistants` = '%s' where `id` = %llu", members_json.c_str(), assistants_json.c_str(), userInfo.guildId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
			}
			else
			{
				char buf[2048] = { 0 };
				sprintf(buf, "update `gameGuildInfo` set `members` = '%s' where `id` = %llu", members_json.c_str(), userInfo.guildId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
			}

			m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
		}

		strUserInfo otherUserInfo;
		if (m_data_cache.get_userInfo(ret.uid, otherUserInfo))
		{
			otherUserInfo.guildId = 0;
			otherUserInfo.kickGuildCDTime = ret.leaveTime;

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `guildId` = 0, `kickGuildCDTime` = %llu where `userId` = %llu", ret.leaveTime, ret.uid);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(otherUserInfo.userId, otherUserInfo);
		}
	}


	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(change_guild_job)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_change_guild_job_req req;
	readStream >> req;

	ss_msg_change_guild_job_ret ret;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.socket_id = req.socket_id;
	ret.otherJob = req.otherJob;
	ret.uid = req.uid;
	ret.nResult = (int)RET_SUCCESS;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		strGuildInfo guildInfo;
		if (m_data_cache.get_guildInfo(userInfo.guildId, guildInfo))
		{
			//! 会员 -> 副会长
			if (req.job == EGuildJob::MEMBER && req.otherJob == EGuildJob::ASSISTANT)
			{
				guildInfo.assistantIds.erase(std::remove(guildInfo.assistantIds.begin(), guildInfo.assistantIds.end(), req.uid), guildInfo.assistantIds.end());

				Json::Value assistant;
				for (size_t i = 0; i < guildInfo.assistantIds.size(); i++)
					assistant.append(guildInfo.assistantIds[i]);

				Json::FastWriter writer;
				std::string assistants_json = writer.write(assistant);

				char buf[2048] = { 0 };
				sprintf(buf, "update `gameGuildInfo` set `assistants` = '%s' where `id` = %llu", assistants_json.c_str(), userInfo.guildId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
				m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
			}
			//! 副会长 -> 会员
			else if (req.job == EGuildJob::ASSISTANT && req.otherJob == EGuildJob::MEMBER)
			{
				guildInfo.assistantIds.push_back(req.uid);

				Json::Value assistant;
				for (size_t i = 0; i < guildInfo.assistantIds.size(); i++)
					assistant.append(guildInfo.assistantIds[i]);

				Json::FastWriter writer;
				std::string assistants_json = writer.write(assistant);

				char buf[2048] = { 0 };
				sprintf(buf, "update `gameGuildInfo` set `assistants` = '%s' where `id` = %llu", assistants_json.c_str(), userInfo.guildId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
				m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
			}
			else if (req.job == EGuildJob::LEADER)
			{
				guildInfo.leaderId = req.uid;
				if (req.otherJob == EGuildJob::ASSISTANT)
				{
					guildInfo.assistantIds.erase(std::remove(guildInfo.assistantIds.begin(), guildInfo.assistantIds.end(), req.uid), guildInfo.assistantIds.end());

					Json::Value assistant;
					for (size_t i = 0; i < guildInfo.assistantIds.size(); i++)
						assistant.append(guildInfo.assistantIds[i]);

					Json::FastWriter writer;
					std::string assistants_json = writer.write(assistant);

					char buf[2048] = { 0 };
					sprintf(buf, "update `gameGuildInfo` set `leaderId` = %llu, `assistants` = '%s' where `id` = %llu", guildInfo.leaderId, assistants_json.c_str(), userInfo.guildId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
					m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
				}
				else
				{
					char buf[2048] = { 0 };
					sprintf(buf, "update `gameGuildInfo` set `leaderId` = %llu where `id` = %llu", guildInfo.leaderId, userInfo.guildId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
					m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
				}
			}
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_guild_boss_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_guild_boss_info req;
	readStream >> req;

	strGuildBossInfo guildBossInfo;
	if (!m_data_cache.get_guildBossInfo(req.guildId, guildBossInfo))
	{
		m_db.hInitGuildBossInfo(req.info);
	}
	else
	{
		m_db.hUpdateGuildBossInfo(req.info);
	}
}
MY_FUNCTION1(update_guild_boss_user_data)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_guild_boss_user_data req;
	readStream >> req;

	strGuildBossUserData guildBossUserData;
	if (!m_data_cache.get_guildBossUserData(req.userId, guildBossUserData))
	{
		m_db.hInitGuildBossUserData(req.info);
	}
	else
	{
		m_db.hUpdateGuildBossUserData(req.info);
	}
}

MY_FUNCTION1(finish_guild_boss_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_guild_boss_battle_req req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		strGuildInfo guildInfo;
		if (m_data_cache.get_guildInfo(userInfo.guildId, guildInfo))
		{
			guildInfo.exp = req.guildExp;
			guildInfo.lv = req.guildLv;
			char buf[1024] = { 0 };
			sprintf(buf, "update `gameGuildInfo` set `lv` = %llu, `exp` = %llu where `id` = %llu",
				guildInfo.lv, guildInfo.exp, userInfo.guildId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameGuildInfo);
			m_data_cache.set_guildInfo(guildInfo.id, guildInfo);
		}
	}
}

MY_FUNCTION1(unlock_morpher_req)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_unlock_morpher_req req;
	readStream >> req;

	ss_msg_unlock_morpher_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.baseId = req.baseId;
	ret.costGem = req.costGem;

	if (ret.costGem > 0)
	{
		this->costGem(req.userId, req.costGem);
	}

	strUserMorpherInfo strMorpher;
	strMorpher.userId = ret.userId;
	strMorpher.base = ret.baseId;
	strMorpher.lv = 1;
	strMorpher.skill1 = 0;
	strMorpher.skill2 = 0;

	char buf[1024] = { 0 };
	sprintf(buf, "insert into `userMorpherInfo`(`userId`, `Base`) values(%llu, %llu)", ret.userId, ret.baseId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserMorpherInfo);
	strMorpher.Id = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserMorpherInfo);
	ret.morpherId = strMorpher.Id;
	m_data_cache.set_morpherInfo(strMorpher);

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_cdkey_gift)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_cdkey_gift_req req;
	readStream >> req;

	ss_msg_fetch_cdkey_gift_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.vecItem = req.vecItem;
	ret.vecItemNum = req.vecItemNum;

	this->gainItems(req.vecItem, req.vecItemNum, req.userId, ret.newItemId);

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(new_equip_strenghten_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_new_equip_strenghten_up_req req;
	readStream >> req;

	ss_msg_new_equip_strenghten_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;

	if (req.unit_type == 0) //!当前角色
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(req.userId, req.unit_id, NULL, roleInfo))
		{
			for (size_t i = 0; i < req.position.size(); i++)
			{
				if (req.position[i] == 1) roleInfo.NewEquip1 = req.cur_new_equip[i];
				else if (req.position[i] == 2) roleInfo.NewEquip2 = req.cur_new_equip[i];
				else if (req.position[i] == 3) roleInfo.NewEquip3 = req.cur_new_equip[i];
				else if (req.position[i] == 4) roleInfo.NewEquip4 = req.cur_new_equip[i];
				else if (req.position[i] == 5) roleInfo.NewEquip5 = req.cur_new_equip[i];
				else if (req.position[i] == 6) roleInfo.NewEquip6 = req.cur_new_equip[i];
			}

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			ret.nResult = RET_SUCCESS;
		}
	}
	else //!宠物
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(req.userId, req.unit_id, NULL, petInfo))
		{
			for (size_t i = 0; i < req.position.size(); i++)
			{
				if (req.position[i] == 1) petInfo.NewEquip1 = req.cur_new_equip[i];
				else if (req.position[i] == 2) petInfo.NewEquip2 = req.cur_new_equip[i];
				else if (req.position[i] == 3) petInfo.NewEquip3 = req.cur_new_equip[i];
				else if (req.position[i] == 4) petInfo.NewEquip4 = req.cur_new_equip[i];
				else if (req.position[i] == 5) petInfo.NewEquip5 = req.cur_new_equip[i];
				else if (req.position[i] == 6) petInfo.NewEquip6 = req.cur_new_equip[i];
			}

			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
			ret.nResult = RET_SUCCESS;
		}
	}

	//!更新每日任务
	int totalLevel = 0;
	for (size_t i = 0; i < req.level.size(); i++)
		totalLevel += req.level[i];


	if (ret.nResult == (int)RET_SUCCESS)
	{
		updateDailyTaskCountInfo(req.userId, dailyTaskCount::newEquipStren_type, totalLevel);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(new_equip_star_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_new_equip_star_up_req req;
	readStream >> req;

	ss_msg_new_equip_star_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;

	uint64_t curNewEquip = NULL;
	if (req.unit_type == 0) //!当前角色
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(req.userId, req.unit_id, NULL, roleInfo))
		{
			for (size_t i = 0; i < req.position.size(); i++)
			{
				if (req.position[i] == 1) roleInfo.NewEquip1 = req.cur_new_equip[i];
				else if (req.position[i] == 2) roleInfo.NewEquip2 = req.cur_new_equip[i];
				else if (req.position[i] == 3) roleInfo.NewEquip3 = req.cur_new_equip[i];
				else if (req.position[i] == 4) roleInfo.NewEquip4 = req.cur_new_equip[i];
				else if (req.position[i] == 5) roleInfo.NewEquip5 = req.cur_new_equip[i];
				else if (req.position[i] == 6) roleInfo.NewEquip6 = req.cur_new_equip[i];
			}

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			ret.nResult = RET_SUCCESS;
		}
	}
	else //!宠物
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(req.userId, req.unit_id, NULL, petInfo))
		{
			for (size_t i = 0; i < req.position.size(); i++)
			{
				if (req.position[i] == 1) petInfo.NewEquip1 = req.cur_new_equip[i];
				else if (req.position[i] == 2) petInfo.NewEquip2 = req.cur_new_equip[i];
				else if (req.position[i] == 3) petInfo.NewEquip3 = req.cur_new_equip[i];
				else if (req.position[i] == 4) petInfo.NewEquip4 = req.cur_new_equip[i];
				else if (req.position[i] == 5) petInfo.NewEquip5 = req.cur_new_equip[i];
				else if (req.position[i] == 6) petInfo.NewEquip6 = req.cur_new_equip[i];
			}

			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
			ret.nResult = RET_SUCCESS;
		}
	}

	//!更新每日任务
	int totalLevel = 0;
	for (size_t i = 0; i < req.level.size(); i++)
		totalLevel += req.level[i];

	if (ret.nResult == (int)RET_SUCCESS)
	{
		updateDailyTaskCountInfo(req.userId, dailyTaskCount::newEquipStar_type, totalLevel);
	}


	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(new_equip_rank_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_new_equip_rank_up_req req;
	readStream >> req;

	ss_msg_new_equip_rank_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;

	uint64_t curNewEquip = NULL;
	if (req.unit_type == 0) //!当前角色
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(req.userId, req.unit_id, NULL, roleInfo))
		{
			if (req.position == 1) roleInfo.NewEquip1 = req.cur_new_equip;
			else if (req.position == 2) roleInfo.NewEquip2 = req.cur_new_equip;
			else if (req.position == 3) roleInfo.NewEquip3 = req.cur_new_equip;
			else if (req.position == 4) roleInfo.NewEquip4 = req.cur_new_equip;
			else if (req.position == 5) roleInfo.NewEquip5 = req.cur_new_equip;
			else if (req.position == 6) roleInfo.NewEquip6 = req.cur_new_equip;

			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			ret.nResult = RET_SUCCESS;
		}
	}
	else //!宠物
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(req.userId, req.unit_id, NULL, petInfo))
		{
			if (req.position == 1) petInfo.NewEquip1 = req.cur_new_equip;
			else if (req.position == 2) petInfo.NewEquip2 = req.cur_new_equip;
			else if (req.position == 3) petInfo.NewEquip3 = req.cur_new_equip;
			else if (req.position == 4) petInfo.NewEquip4 = req.cur_new_equip;
			else if (req.position == 5) petInfo.NewEquip5 = req.cur_new_equip;
			else if (req.position == 6) petInfo.NewEquip6 = req.cur_new_equip;

			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
			ret.nResult = RET_SUCCESS;
		}
	}

	if (ret.nResult == (int)RET_SUCCESS)
	{
		//updateDailyTaskCountInfo(req.userId, dailyTaskCount::newEquipStar_type, totalLevel);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(new_equip_potential_up)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_new_equip_potential_up_req req;
	readStream >> req;

	ss_msg_new_equip_potential_up_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;

	uint64_t curNewEquip = NULL;
	if (req.unit_type == 0) //!当前角色
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(req.userId, req.unit_id, NULL, roleInfo))
		{
			roleInfo.NewEquipPotential = req.cur_potential;
			game_util::updateRoleInfo(m_data_cache, &roleInfo);
			updateDatabaseRoleInfo(&roleInfo);
			ret.nResult = RET_SUCCESS;
		}
	}
	else //!宠物
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(req.userId, req.unit_id, NULL, petInfo))
		{
			petInfo.NewEquipPotential = req.cur_potential;
			game_util::updatePetInfo(m_data_cache, &petInfo);
			updateDatabasePetInfo(&petInfo);
			ret.nResult = RET_SUCCESS;
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(use_item)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_use_item_req req;
	readStream >> req;

	ss_msg_use_item_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.gain_items = req.gain_items;
	ret.gain_nums = req.gain_nums;
	ret.nResult = RET_SUCCESS;

	if (req.item_base == 44133)
	{

#pragma region VIP 经验道具

		strRechargeInfo rechargeInfo;
		if (!m_data_cache.get_rechargeInfo(req.userId, rechargeInfo))
		{
			rechargeInfo.userId = req.userId;
			rechargeInfo.monthlyEndTime = 0;
			rechargeInfo.utlimateEndTime = 0;
			rechargeInfo.totalRecharge = 0;
			rechargeInfo.dailyRecharge = 0;
			rechargeInfo.isUltimate = 0;
			rechargeInfo.monthlyFetchLastTime = 0;
			rechargeInfo.utlimateFetchLastTime = 0;
			rechargeInfo.extralVal = req.item_num;
			m_db.hInitRechargeInfo(&rechargeInfo);
		}
		else
		{
			rechargeInfo.extralVal += req.item_num;
			m_db.hupdateRechargeInfo(&rechargeInfo);
		}

		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_VipBase_t*>& mapVipBase = config.get_vipBase_config();

		//vip level
		int vipLevel = 0;
		strUserInfo userInfo;
		if (!m_data_cache.get_userInfo(req.userId, userInfo))
		{
			ret.nResult = RET_USER_INFO_EMPTY;
		}
		else
		{
			std::map<uint64_t, config_VipBase_t*>::iterator iterVip = mapVipBase.begin();
			for (; iterVip != mapVipBase.end(); iterVip++)
			{
				if (rechargeInfo.totalRecharge + rechargeInfo.extralVal >= iterVip->second->m_Cost)
				{
					vipLevel = iterVip->second->m_VIP;
				}
				else
				{
					break;
				}
			}

			int lastVipLevel = userInfo.userVip;
			if (vipLevel > lastVipLevel)
			{
				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userVip` = %llu where `userId` = %llu", vipLevel, req.userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
				userInfo.userVip = vipLevel;
				m_data_cache.set_userInfo(userInfo.userId, userInfo);

				if (vipLevel >= 10 && lastVipLevel < 10)
				{
					strUserMorpherInfo strMorpher;
					strMorpher.userId = ret.userId;
					strMorpher.base = 1103;
					strMorpher.lv = 1;
					strMorpher.skill1 = 0;
					strMorpher.skill2 = 0;

					char buf[1024] = { 0 };
					sprintf(buf, "insert into `userMorpherInfo`(`userId`, `Base`) values(%llu, 1103)", ret.userId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserMorpherInfo);
					strMorpher.Id = singleton_t<db_request_list>::instance().getLastInsertID(DB_UserMorpherInfo);

					m_data_cache.set_morpherInfo(strMorpher);
				}
			}
		}

#pragma endregion

	}

	if (req.gain_items.size() > 0)
	{
		std::vector<uint64_t> newItemId;
		this->gainItems(req.gain_items, req.gain_nums, req.userId, newItemId);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(reset_game_data)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_reset_game_data_req req;
	readStream >> req;

	ss_msg_reset_game_data_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;

	resetGameDataNewDay(req.userId);

	ret.nResult = RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_inherit)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_inherit_req req;
	readStream >> req;

	ss_msg_pet_inherit_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.firstId = req.firstId;
	ret.lastId = req.lastId;
	ret.nResult = RET_SUCCESS;

	strPetInfo fromPetInfo;
	if (!m_data_cache.get_petInfo(req.userId, req.firstId, NULL, fromPetInfo))
	{
		ret.nResult = RET_PET_NOT_EXIST;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	strPetInfo toPetInfo;
	if (!m_data_cache.get_petInfo(req.userId, req.lastId, NULL, toPetInfo))
	{
		ret.nResult = RET_PET_NOT_EXIST;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

#pragma region 等级 品质 科技 装备

	swap(fromPetInfo.LV, toPetInfo.LV);
	swap(fromPetInfo.EXP, toPetInfo.EXP);
	swap(fromPetInfo.NextExp, toPetInfo.NextExp);
	swap(fromPetInfo.Rank, toPetInfo.Rank);

	swap(fromPetInfo.NewEquip1, toPetInfo.NewEquip1);
	swap(fromPetInfo.NewEquip2, toPetInfo.NewEquip2);
	swap(fromPetInfo.NewEquip3, toPetInfo.NewEquip3);
	swap(fromPetInfo.NewEquip4, toPetInfo.NewEquip4);
	swap(fromPetInfo.NewEquip5, toPetInfo.NewEquip5);
	swap(fromPetInfo.NewEquip6, toPetInfo.NewEquip6);
	swap(fromPetInfo.NewEquipPotential, toPetInfo.NewEquipPotential);

	swap(fromPetInfo.EquipsId0, toPetInfo.EquipsId0);
	swap(fromPetInfo.EquipsId1, toPetInfo.EquipsId1);
	swap(fromPetInfo.EquipsId2, toPetInfo.EquipsId2);
	swap(fromPetInfo.EquipsId3, toPetInfo.EquipsId3);
	swap(fromPetInfo.EquipsId4, toPetInfo.EquipsId4);
	swap(fromPetInfo.EquipsId5, toPetInfo.EquipsId5);

	char buf[1024] = { 0 };
	sprintf(buf, "update `gamePetInfo` set `EquipsId0` = %llu, `EquipsId1` = %llu, `EquipsId2` = %llu, `EquipsId3` = %llu, `EquipsId4` = %llu, `EquipsId5` = %llu where `ID` = %llu;",
		fromPetInfo.EquipsId0, fromPetInfo.EquipsId1, fromPetInfo.EquipsId2, fromPetInfo.EquipsId3, fromPetInfo.EquipsId4, fromPetInfo.EquipsId5, req.firstId);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

	char buf2[1024] = { 0 };
	sprintf(buf2, "update `gamePetInfo` set `EquipsId0` = %llu, `EquipsId1` = %llu, `EquipsId2` = %llu, `EquipsId3` = %llu, `EquipsId4` = %llu, `EquipsId5` = %llu where `ID` = %llu;",
		toPetInfo.EquipsId0, toPetInfo.EquipsId1, toPetInfo.EquipsId2, toPetInfo.EquipsId3, toPetInfo.EquipsId4, toPetInfo.EquipsId5, req.lastId);
	singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_GamePetInfo);

#pragma endregion

#pragma region 技能

	for (size_t i = 0; i < 4; i++)
	{
		uint64_t *fromSkillId = NULL;
		uint64_t *toSkillId = NULL;
		int fromSkillLv = 0, toSkillLv = 0;
		uint64_t fromExp = 0, toExp = 0;

		if (i == 0) { fromSkillId = &fromPetInfo.SkillsId0; toSkillId = &toPetInfo.SkillsId0; }
		else if (i == 2) { fromSkillId = &fromPetInfo.SkillsId2; toSkillId = &toPetInfo.SkillsId2; }
		else if (i == 3) { fromSkillId = &fromPetInfo.SkillsId3; toSkillId = &toPetInfo.SkillsId3; }
		else if (i == 1) { fromSkillId = &fromPetInfo.SkillsId1; toSkillId = &toPetInfo.SkillsId1; }

		strSkillInfo fromSkillInfo;
		if (*fromSkillId > 0)
		{
			if (m_data_cache.get_skillInfo(NULL, *fromSkillId, NULL, fromSkillInfo))
			{
				fromSkillLv = fromSkillInfo.Lv;
				fromExp = fromSkillInfo.CurExp;
			}
		}

		strSkillInfo toSkillInfo;
		if (*toSkillId > 0)
		{
			if (m_data_cache.get_skillInfo(NULL, *toSkillId, NULL, toSkillInfo))
			{
				toSkillLv = toSkillInfo.Lv;
				toExp = toSkillInfo.CurExp;
			}
		}

		if (*toSkillId == 0 && *fromSkillId > 0)
		{
			updatePetSkillUnlock(&toPetInfo, *toSkillId, i, fromSkillLv);

			char buf[1024] = { 0 };
			sprintf(buf, "delete  from `gameSkillInfo` where `ID` = %llu;", fromSkillInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
			m_data_cache.del_skillInfo(*fromSkillId);

			*fromSkillId = 0;

			char bufPet[1024] = { 0 };
			sprintf(bufPet, "update `gamePetInfo` set `SkillsId%d` = 0 where `ID` = %llu;", i, fromPetInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(bufPet, sql_update, DB_GamePetInfo);
		}
		else if (*toSkillId > 0 && *fromSkillId == 0)
		{
			updatePetSkillUnlock(&fromPetInfo, *fromSkillId, i, toSkillLv);

			char buf[1024] = { 0 };
			sprintf(buf, "delete  from `gameSkillInfo` where `ID` = %llu;", toSkillInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
			m_data_cache.del_skillInfo(*toSkillId);

			*toSkillId = 0;

			char bufPet[1024] = { 0 };
			sprintf(bufPet, "update `gamePetInfo` set `SkillsId%d` = 0 where `ID` = %llu;", i, toPetInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(bufPet, sql_update, DB_GamePetInfo);
		}
		else if (*toSkillId > 0 && *fromSkillId > 0)
		{
			swap(fromSkillInfo.Lv, toSkillInfo.Lv);
			swap(fromSkillInfo.CurExp, toSkillInfo.CurExp);

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameSkillInfo` set `Lv` = %llu, `CurExp` = %llu where `ID` = %llu;", fromSkillInfo.Lv, fromSkillInfo.CurExp, fromSkillInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameSkillInfo);
			m_data_cache.set_skillInfo(fromSkillInfo.userId, fromSkillInfo.ID, fromSkillInfo.Base, fromSkillInfo);

			char buf2[1024] = { 0 };
			sprintf(buf2, "update `gameSkillInfo` set `Lv` = %llu, `CurExp` = %llu where `ID` = %llu;", toSkillInfo.Lv, toSkillInfo.CurExp, toSkillInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_GameSkillInfo);
			m_data_cache.set_skillInfo(toSkillInfo.userId, toSkillInfo.ID, toSkillInfo.Base, toSkillInfo);
		}

		fromSkillId = NULL;
		toSkillId = NULL;
	}

#pragma endregion

#pragma region 宝石

	strEquipInfo equipInfo;
	if (fromPetInfo.Artifact0) this->unuseEquip(fromPetInfo.Artifact0, equipInfo);
	if (fromPetInfo.Artifact1) this->unuseEquip(fromPetInfo.Artifact1, equipInfo);
	if (fromPetInfo.Artifact2) this->unuseEquip(fromPetInfo.Artifact2, equipInfo);
	if (fromPetInfo.Artifact3) this->unuseEquip(fromPetInfo.Artifact3, equipInfo);
	if (fromPetInfo.Artifact4) this->unuseEquip(fromPetInfo.Artifact4, equipInfo);
	if (fromPetInfo.Artifact5) this->unuseEquip(fromPetInfo.Artifact5, equipInfo);
	fromPetInfo.Artifact0 = 0;
	fromPetInfo.Artifact1 = 0;
	fromPetInfo.Artifact2 = 0;
	fromPetInfo.Artifact3 = 0;
	fromPetInfo.Artifact4 = 0;
	fromPetInfo.Artifact5 = 0;

	if (toPetInfo.Artifact0) this->unuseEquip(toPetInfo.Artifact0, equipInfo);
	if (toPetInfo.Artifact1) this->unuseEquip(toPetInfo.Artifact1, equipInfo);
	if (toPetInfo.Artifact2) this->unuseEquip(toPetInfo.Artifact2, equipInfo);
	if (toPetInfo.Artifact3) this->unuseEquip(toPetInfo.Artifact3, equipInfo);
	if (toPetInfo.Artifact4) this->unuseEquip(toPetInfo.Artifact4, equipInfo);
	if (toPetInfo.Artifact5) this->unuseEquip(toPetInfo.Artifact5, equipInfo);
	toPetInfo.Artifact0 = 0;
	toPetInfo.Artifact1 = 0;
	toPetInfo.Artifact2 = 0;
	toPetInfo.Artifact3 = 0;
	toPetInfo.Artifact4 = 0;
	toPetInfo.Artifact5 = 0;

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	for (size_t slot = 1; slot <= 6; slot++)
	{
		int unlockLv = config.getArtifactUnlockLv(slot);
		int unlockCost = config.getArtifactUnlockCost(slot);
		if (unlockLv <= fromPetInfo.LV && unlockCost == 0)
		{
			fromPetInfo.LockedArtifactSlot |= (1ULL << (slot - 1));
		}
		else if (unlockLv > fromPetInfo.LV && unlockCost == 0)
		{
			fromPetInfo.LockedArtifactSlot &= ~(1ULL << (slot - 1));
		}
	}

	for (size_t slot = 1; slot <= 6; slot++)
	{
		int unlockLv = config.getArtifactUnlockLv(slot);
		int unlockCost = config.getArtifactUnlockCost(slot);
		if (unlockLv <= toPetInfo.LV && unlockCost == 0)
		{
			toPetInfo.LockedArtifactSlot |= (1ULL << (slot - 1));
		}
		else if (unlockLv > toPetInfo.LV && unlockCost == 0)
		{
			toPetInfo.LockedArtifactSlot &= ~(1ULL << (slot - 1));
		}
	}


	char bufs[1024] = { 0 };
	sprintf(bufs, "update `gamePetInfo` set `artifact0` = 0, `artifact1` = 0, `artifact2` = 0, `artifact3` = 0, `artifact4` = 0, `artifact5` = 0, `lockedArtifactSlot` = %llu where `ID` = %llu",
		fromPetInfo.LockedArtifactSlot, req.firstId);
	singleton_t<db_request_list>::instance().push_request_list(bufs, sql_update, DB_GamePetInfo);

	char bufs2[1024] = { 0 };
	sprintf(bufs2, "update `gamePetInfo` set `artifact0` = 0, `artifact1` = 0, `artifact2` = 0, `artifact3` = 0, `artifact4` = 0, `artifact5` = 0, `lockedArtifactSlot` = %llu where `ID` = %llu",
		toPetInfo.LockedArtifactSlot, req.lastId);
	singleton_t<db_request_list>::instance().push_request_list(bufs2, sql_update, DB_GamePetInfo);

#pragma endregion

	game_util::updatePetInfo(m_data_cache, &fromPetInfo);
	updateDatabasePetInfo(&fromPetInfo);

	game_util::updatePetInfo(m_data_cache, &toPetInfo);
	updateDatabasePetInfo(&toPetInfo);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(pet_bind_upgrade)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_pet_bind_upgrade_req req;
	readStream >> req;

	ss_msg_pet_bind_upgrade_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.petId = req.petId;
	ret.bindId = req.bindId;
	ret.nResult = RET_SUCCESS;

	strPetInfo petInfo;
	if (m_data_cache.get_petInfo(req.userId, req.petId, NULL, petInfo))
	{
		petInfo.petBinds = req.petBinds;

		std::string petBindsStr;
		parseVector2String(petInfo.petBinds, petBindsStr);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gamePetInfo` set `petBinds` = '%s' where `ID` = %llu and `userId` = %llu;", petBindsStr.c_str(), petInfo.ID, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
		m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);

		game_util::updatePetInfo(m_data_cache, &petInfo);
		updateDatabasePetInfo(&petInfo);

		if (req.bind_lv > 1)
			updateQuestStateCount(req.userId, 10, dailyTaskCount::petbindupgrade_type, 1);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(get_world_boss_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_get_world_boss_info_req req;
	readStream >> req;

	//!更新boss
	if (req.newBattle)
	{
		std::string boosLvStr = "";
		if (!m_data_cache.get_globalInfo(global_key_worldbosslv, boosLvStr))
		{
			boosLvStr = boost::lexical_cast<std::string>(req.bossLv);
			m_db.hinitGlobalInfo(global_key_worldbosslv, boosLvStr);
		}
		else
		{
			boosLvStr = boost::lexical_cast<std::string>(req.bossLv);
			m_db.hUpdateGlobalInfo(global_key_worldbosslv, boosLvStr);
		}
	}

	//!更新玩家
	if (req.refreshInfo)
	{
		strWorldBossInfo worldBossInfo(req.userId);
		if (!m_data_cache.get_worldBossInfo(req.userId, worldBossInfo))
		{
			worldBossInfo.fightTimes = 0;
			worldBossInfo.totalDamage = 0;
			worldBossInfo.figRewardStatus = ERewardState::ERS_NoFetch;
			worldBossInfo.parRewardStatus = ERewardState::ERS_NoFetch;
			worldBossInfo.pets.clear();

			m_db.hinitWorldBossInfo(worldBossInfo);

		}
		else
		{
			worldBossInfo.fightTimes = 0;
			worldBossInfo.totalDamage = 0;
			worldBossInfo.figRewardStatus = ERewardState::ERS_NoFetch;
			worldBossInfo.parRewardStatus = ERewardState::ERS_NoFetch;
			worldBossInfo.pets.clear();

			m_db.hUpdateWorldBossInfo(worldBossInfo);
		}
	}
}

MY_FUNCTION1(world_boss_sync_stat)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_sync_world_boss_stat_req req;
	readStream >> req;

	ss_msg_sync_world_boss_stat_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.need_detail = req.need_detail;
	ret.damage = req.damage;
	ret.killed = req.killed;

	if (req.in_battle && req.damage > 0)
	{
		strWorldBossInfo worldBossInfo(req.userId);
		if (!m_data_cache.get_worldBossInfo(req.userId, worldBossInfo))
		{
			worldBossInfo.totalDamage = req.damage;
			worldBossInfo.lastDamage = req.damage;

			m_db.hinitWorldBossInfo(worldBossInfo);
		}
		else
		{
			worldBossInfo.totalDamage += req.damage;
			worldBossInfo.lastDamage += req.damage;

			m_db.hUpdateWorldBossInfo(worldBossInfo);
		}

		m_data_cache.zadd_worldBossRank(req.userId, worldBossInfo.totalDamage);

		if (req.killed)
		{
			std::string costTimeStr = "";
			if (!m_data_cache.get_globalInfo(global_key_worldbosscosttime, costTimeStr))
			{
				costTimeStr = boost::lexical_cast<std::string>(req.lastCostTime);
				m_db.hinitGlobalInfo(global_key_worldbosscosttime, costTimeStr);
			}
			else
			{
				costTimeStr = boost::lexical_cast<std::string>(req.lastCostTime);
				m_db.hUpdateGlobalInfo(global_key_worldbosscosttime, costTimeStr);
			}
		}
	}


	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_world_boss_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_world_boss_reward_req req;
	readStream >> req;

	ss_msg_fetch_world_boss_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.type = req.type;
	ret.vecItems = req.vecItems;
	ret.vecNums = req.vecNums;

	strWorldBossInfo worldBossInfo(req.userId);
	m_data_cache.get_worldBossInfo(req.userId, worldBossInfo);

	if (req.type == 1) //!单场奖励
	{
		worldBossInfo.figRewardStatus = ERewardState::ERS_NoFetch;
		worldBossInfo.lastDamage = 0;

		std::vector<uint64_t> newItemIds;
		gainItems(req.vecItems, req.vecNums, req.userId, newItemIds);
		ret.nResult = (int)RET_SUCCESS;

		m_db.hUpdateWorldBossInfo(worldBossInfo);
	}
	else if (req.type == 2) //!参与奖励
	{
		worldBossInfo.parRewardStatus = ERewardState::ERS_Fetched;

		std::vector<uint64_t> newItemIds;
		gainItems(req.vecItems, req.vecNums, req.userId, newItemIds);
		ret.nResult = (int)RET_SUCCESS;

		m_db.hUpdateWorldBossInfo(worldBossInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(enter_world_boss_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_enter_world_boss_battle_req req;
	readStream >> req;

	ss_msg_enter_world_boss_battle_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.nResult = RET_SUCCESS;

	strWorldBossInfo worldBossInfo(req.userId);
	if (!m_data_cache.get_worldBossInfo(req.userId, worldBossInfo))
	{
		worldBossInfo.lastTime = req.lastTime;
		worldBossInfo.fightTimes = req.fightTimes;
		worldBossInfo.lastDamage = 0;
		worldBossInfo.figRewardStatus = ERewardState::ERS_CanFetch;

		for (size_t i = 0; i < req.petIds.size(); i++)
		{
			strWorldBossPetInfo wbPetInfo;
			wbPetInfo.petId = req.petIds[i];
			wbPetInfo.dead = 1;
			wbPetInfo.reviveTimes = 0;
			worldBossInfo.pets.push_back(wbPetInfo);
		}
		
		m_db.hinitWorldBossInfo(worldBossInfo);
	}
	else
	{
		worldBossInfo.lastTime = req.lastTime;
		worldBossInfo.fightTimes = req.fightTimes;
		worldBossInfo.lastDamage = 0;
		worldBossInfo.figRewardStatus = ERewardState::ERS_CanFetch;

		for (size_t i = 0; i < req.petIds.size(); i++)
		{
			bool find = false;
			for (size_t j = 0; j < worldBossInfo.pets.size(); j++)
			{
				if (worldBossInfo.pets[j].petId == req.petIds[i])
				{
					find = true;
					worldBossInfo.pets[j].dead = 1;
					break;
				}
			}

			if (!find)
			{
				strWorldBossPetInfo wbPetInfo;
				wbPetInfo.petId = req.petIds[i];
				wbPetInfo.dead = 1;
				wbPetInfo.reviveTimes = 0;
				worldBossInfo.pets.push_back(wbPetInfo);
			}
		}

		Json::Value petInfo = Json::arrayValue;
		for (size_t i = 0; i < worldBossInfo.pets.size(); i++)
		{
			Json::Value info;
			info["id"] = Json::UInt(worldBossInfo.pets[i].petId);
			info["times"] = Json::UInt(worldBossInfo.pets[i].reviveTimes);
			info["dead"] = Json::UInt(worldBossInfo.pets[i].dead);

			petInfo.append(info);
		}
		Json::FastWriter write;
		std::string data = write.write(petInfo);

		m_db.hUpdateWorldBossInfo(worldBossInfo);
	}

	updateDailyTaskCountInfo(req.userId, dailyTaskCount::worldBoss_type, 1);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(finish_world_boss_battle)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_finish_world_boss_battle_req req;
	readStream >> req;

	ss_msg_finish_world_boss_battle_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.damage = req.damage;


	strWorldBossInfo worldBossInfo(req.userId);
	m_data_cache.get_worldBossInfo(req.userId, worldBossInfo);



	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(revive_world_boss_pet)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_revive_world_boss_pet_req req;
	readStream >> req;

	ss_msg_revive_world_boss_pet_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.petId = req.petId;

	strWorldBossInfo worldBossInfo(req.userId);
	if (m_data_cache.get_worldBossInfo(req.userId, worldBossInfo))
	{
		bool find = false;
		for (size_t i = 0; i < worldBossInfo.pets.size(); i++)
		{
			if (worldBossInfo.pets[i].petId == req.petId &&  worldBossInfo.pets[i].dead)
			{
				worldBossInfo.pets[i].dead = 0;
				worldBossInfo.pets[i].reviveTimes++;
				find = true;
				break;
			}
		}

		Json::Value petInfo = Json::arrayValue;
		for (size_t i = 0; i < worldBossInfo.pets.size(); i++)
		{
			Json::Value info;
			info["id"] = Json::UInt(worldBossInfo.pets[i].petId);
			info["times"] = Json::UInt(worldBossInfo.pets[i].reviveTimes);
			info["dead"] = Json::UInt(worldBossInfo.pets[i].dead);

			petInfo.append(info);
		}
		Json::FastWriter write;
		std::string data = write.write(petInfo);

		m_db.hUpdateWorldBossInfo(worldBossInfo);
	}

	ret.nResult = (int)RET_SUCCESS;
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(bind_account)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_bind_account_req req;
	readStream >> req;

	ss_msg_bind_account_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.new_account = req.new_account;
	ret.channel_id = req.channel_id;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		uint64_t checkUserId = 0;
		if (m_data_cache.get_userIdByAccount(req.new_account, checkUserId))
		{
			//!清除之前绑定的号
			if (checkUserId != req.userId)
			{
				strUserInfo checkUserInfo;
				if (m_data_cache.get_userInfo(checkUserId, checkUserInfo))
				{
					std::string oldAccount = checkUserInfo.userAccount;
					std::string oldName = checkUserInfo.userName;
					checkUserInfo.userAccount = "";

					char buf[1024] = { 0 };
					sprintf(buf, "update `gameUserInfo` set `userAccount` = '%s' where `userId` = %llu", "", checkUserInfo.userId);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
					m_data_cache.set_userInfo(checkUserInfo.userId, checkUserInfo);
					m_data_cache.del_userIdByAccount(oldAccount);
					m_data_cache.del_userIdByName(oldName);
				}
			}
		}

		std::string oldAccount = userInfo.userAccount;
		std::string oldName = userInfo.userName;
		userInfo.userAccount = req.new_account;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userAccount` = '%s' where `userId` = %llu", req.new_account.c_str(), req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(req.userId, userInfo);
		if (req.new_account.size() == 0)
		{
			m_data_cache.del_userIdByAccount(oldAccount);
			m_data_cache.del_userIdByName(oldName);
		}
		else
		{
			m_data_cache.rename_userIdByAccount(oldAccount, req.new_account);
		}
		ret.nResult = (int)RET_SUCCESS;
	}
	else
	{
		ret.nResult = (int)RET_USER_INFO_EMPTY;
	}
	
	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_gem_consume)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_gem_consume req;
	readStream >> req;

	if (req.count > 0)
		this->updateDailyTaskCountInfo(req.userId, dailyTaskCount::costgem_type, req.count);
}

MY_FUNCTION1(update_gain_items)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_gain_items req;
	readStream >> req;

	std::vector<uint64_t> newItemIds;
	this->gainItems(req.items, req.nums, req.userId, newItemIds);
}

MY_FUNCTION1(fetch_invite_code)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_invite_code_req req;
	readStream >> req;

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.inviterId = req.uid;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `inviterId` = %llu where `userId` = %llu", userInfo.inviterId, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

bool football_db_analyze_t::init_celebration_info(const uint64_t user_id, UserCelebrationInfoMap& info)
{
	/*info.clear();
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_celebrationBase_t*>& mapCelebrationBase = config.get_celebrationBase_conifg();

	std::map<uint64_t, config_celebrationBase_t*>::iterator iterCelebration = mapCelebrationBase.begin();
	for (; iterCelebration != mapCelebrationBase.end(); iterCelebration++)
	{
		UserCelebrationInfoMap::iterator iterCelebrationInfo = info.find(iterCelebration->second->m_Type);
		if (iterCelebrationInfo != info.end())
		{
			iterCelebrationInfo->second.insert(std::make_pair(iterCelebration->second->m_Rank, 0));
		}
		else
		{
			std::map<int64_t, uint64_t> map;
			map.insert(std::make_pair(iterCelebration->second->m_Rank, 0));
			info.insert(std::make_pair(iterCelebration->second->m_Type, map));
		}
	}*/

	m_db.hInitCelebrationInfo(user_id, info);
	return true;
}

MY_FUNCTION1(fetch_celebration_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_celebration_reward_req req;
	readStream >> req;

	ss_msg_fetch_celebration_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.vecItem = req.vecItem;
	ret.vecItemNum = req.vecItemNum;
	ret.id = req.id;
	ret.type = req.type;
	ret.rank = req.rank;
	ret.count = req.count;
	ret.extra = req.extra;

	UserCelebrationInfoMap celebrationInfo;
	if (!m_data_cache.get_celebrationInfo(req.userId, celebrationInfo))
	{
		init_celebration_info(req.userId, celebrationInfo);
	}

	UserCelebrationInfoMap::iterator iterMapSec = celebrationInfo.find(req.id);
	if (iterMapSec != celebrationInfo.end())
	{
		std::map<int64_t, uint64_t>& mapThr = iterMapSec->second;
		std::map<int64_t, uint64_t>::iterator iterMapThr = mapThr.find(req.rank);
		if (iterMapThr != mapThr.end())
		{
			if (req.type == CELEBRATION_TYPE_RES_SELL||
				req.type == CELEBRATION_TYPE_FESTIVAL_EXCHANGE)
			{
				iterMapThr->second += req.count;
			}
			else if (req.type == CELEBRATION_TYPE_GIFT_WEEKLY)
			{
				iterMapThr->second = time(NULL);
			}
			else
			{
				iterMapThr->second = 1;
			}
			m_db.hUpdateCelebrationInfo(req.userId, celebrationInfo);
		}
	}

	if (ret.type == CELEBRATION_TYPE_CHECK_DAY)
	{
		strUserInfo userInfo;
		if (m_data_cache.get_userInfo(ret.userId, userInfo))
		{
			userInfo.lastRewardTime = time(NULL);

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameUserInfo` set `lastRewardTime` = %llu where `userId` = %llu", userInfo.lastRewardTime, ret.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			m_data_cache.set_userInfo(userInfo.userId, userInfo);
		}
	}

	this->gainItems(req.vecItem, req.vecItemNum, req.userId, ret.newItemId);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(buy_celebration)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_buy_celebration_req req;
	readStream >> req;

	ss_msg_buy_celebration_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.id = req.id;
	ret.type = req.type;

	UserCelebrationInfoMap celebrationInfo;
	if (!m_data_cache.get_celebrationInfo(req.userId, celebrationInfo))
	{
		init_celebration_info(req.userId, celebrationInfo);
	}

	if (req.type != 5 && req.type != 6)
	{
		ret.nResult = (int)RET_REQ_DATA_ERROR;
		BinaryWriteStream write;
		write << ret;
		socket_->async_write(write);
		return;
	}

	UserCelebrationInfoMap::iterator iterMapSec = celebrationInfo.find(req.id);
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(ret.userId, userInfo))
	{
		ret.nResult = (int)RET_DATA_ERROR;
		BinaryWriteStream write;
		write << ret;
		socket_->async_write(write);
		return;
	}

	if (iterMapSec != celebrationInfo.end())
	{
		std::map<int64_t, uint64_t>& mapThr = iterMapSec->second;
		std::map<int64_t, uint64_t>::iterator iterMapThr = mapThr.find(CELEBRATION_OPENT_TIME);
		if (iterMapThr == mapThr.end())
		{
			iterMapThr = mapThr.insert(make_pair(CELEBRATION_OPENT_TIME, 0)).first;
		}
		iterMapThr->second = time(NULL);
		ret.buyTime = iterMapThr->second;
		m_db.hUpdateCelebrationInfo(req.userId, celebrationInfo);

		userInfo.userGem -= req.gemCost;
		userInfo.userCostGem += req.gemCost;
		ret.gemCost = req.gemCost;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGem` = `userGem` - %llu where `userId` = %llu", req.gemCost, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(update_celebration_info)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_update_celebration_info req;
	readStream >> req;


	UserCelebrationInfoMap celebrationInfo;
	if (!m_data_cache.get_celebrationInfo(req.userId, celebrationInfo))
	{
		init_celebration_info(req.userId, req.info);
	}

	m_db.hUpdateCelebrationInfo(req.userId, req.info);
}

MY_FUNCTION1(fetch_seven_days_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_sevendaysgift_reward_req req;
	readStream >> req;

	ss_msg_fetch_sevendaysgift_reward_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.ids = req.ids;
	ret.vecItem = req.vecItem;
	ret.vecItemNum = req.vecItemNum;
	ret.nResult = RET_SUCCESS;

	int count = 0;
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(req.userId, mapQuest))
	{
		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuest = mapQuest.begin();
		for (; iterQuest != mapQuest.end(); iterQuest++)
		{
			for (size_t i = 0; i < req.questIds.size(); i++)
			{
				if (iterQuest->second->questId == req.questIds[i])
				{
					iterQuest->second->state = EQuestState::GetReward_Quest;
					count++;
					break;
				}
			}

			if (count == req.questIds.size())
				break;
		}
		m_db.hUpdateUserQuestList(req.userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);

	this->gainItems(req.vecItem, req.vecItemNum, req.userId, ret.newItemId);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_turntable_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_turntable_reward_req req;
	readStream >> req;

	ss_msg_fetch_turntable_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.userId = req.userId;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.multiple = req.multiple;

	this->gainItems(req.itemId, req.itemNum, req.userId, ret.newItemId);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(equip_all)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_equip_all_req req;
	readStream >> req;

	ss_msg_equip_all_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.nResult = (int)RET_SUCCESS;

	ret.slot = req.slot;
	ret.unitID = req.unitID;
	ret.unitType = req.unitType;
	ret.needCombine = req.needCombine;
	ret.equipId = req.equipId;
	ret.equipIndex = req.equipIndex;
	ret.equip_combin_vec = req.equip_combin_vec;
	ret.combinCost = req.combinCost;
	ret.equipBase = req.equipBase;
	ret.equipBaseCombine = req.equipBaseCombine;
	int j = 0, k = 0;
	if (req.unitType == 0)//role
	{
		strRoleInfo roleInfo;
		if (m_data_cache.get_roleInfo(NULL, req.unitID, NULL, roleInfo))
		{
			for (size_t i = 0; i < req.slot.size(); i++)
			{
				if (!req.needCombine[i])
				{
					roleEquip(req.unitID, req.equipId[j], req.equipIndex[j]);
					j++;

				}
				else
				{
					uint64_t newEquipId = 0;
					petPetEquipCombin(req.userId, req.equipBaseCombine[k], req.equip_combin_vec[k].vecCombinItem, req.equip_combin_vec[k].vecCombinItemCount, req.combinCost[k], newEquipId);
					k++;
					ret.equipNewId.push_back(newEquipId);
					roleEquip(req.unitID, newEquipId, req.slot[i]);
				}
			}
		}

	}
	else if (req.unitType == 1)//pet
	{
		strPetInfo petInfo;
		if (m_data_cache.get_petInfo(NULL, req.unitID, NULL, petInfo))
		{
			for (size_t i = 0; i < req.slot.size(); i++)
			{
				if (!req.needCombine[i])
				{
					petEquip(req.unitID, req.equipId[j], req.equipIndex[j]);
					j++;
				}
				else
				{
					uint64_t newEquipId = 0;
					petPetEquipCombin(req.userId, req.equipBaseCombine[k], req.equip_combin_vec[k].vecCombinItem, req.equip_combin_vec[k].vecCombinItemCount, req.combinCost[k], newEquipId);
					k++;
					ret.equipNewId.push_back(newEquipId);
					petEquip(req.unitID, newEquipId, req.slot[i]);
				}
			}
		}
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(send_friend_gift)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_send_friend_gift_req req;
	readStream >> req;

	ss_msg_send_friend_gift_ret ret;
	ret.index_id = req.index_id;
	ret.socket_id = req.socket_id;
	ret.userId = req.userId;
	ret.userIds = req.userIds;
	ret.nResult = (int)RET_SUCCESS;

	strFriendInfo friendInfo;
	if (m_data_cache.get_friendInfo(req.userId, friendInfo))
	{
		for (size_t i = 0; i < ret.userIds.size(); i++)
		{
			friendInfo.sendGiftList.push_back(ret.userIds[i]);
		}

		Json::Value sendGiftList = Json::arrayValue;
		Json::FastWriter write;
		for (size_t i = 0; i < friendInfo.sendGiftList.size(); i++)
		{
			sendGiftList.append(friendInfo.sendGiftList[i]);
		}
		std::string contents = write.write(sendGiftList);

		char buf[1024] = { 0 };
		sprintf(buf, "update `friendInfo` set `sendGiftList` = '%s' where `userId` = %llu", contents.c_str(), req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
		m_data_cache.set_friendInfo(req.userId, friendInfo);
	}

	int count = 0;
	for (size_t i = 0; i < ret.userIds.size(); i++)
	{
		strFriendInfo otherFriendInfo;
		if (!m_data_cache.get_friendInfo(ret.userIds[i], otherFriendInfo))
		{
			otherFriendInfo.userId = ret.userIds[i];
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<std::string, std::string>& mapGlobal = config.get_global_config();
			std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("FriendStaGetMaximum");
			if (iterGlobal != mapGlobal.end())
			{
				otherFriendInfo.leftGetStrengthTimes = atol(iterGlobal->second.c_str());
			}
			iterGlobal = mapGlobal.find("FriendStaGainMaximum");
			if (iterGlobal != mapGlobal.end())
			{
				otherFriendInfo.leftSendStrengthTimes = atol(iterGlobal->second.c_str());
			}

			char buf[1024] = { 0 };
			sprintf(buf, "insert into `friendInfo`(`userId`, `leftGetStrengthTimes`, `leftSendStrengthTimes`) values(%llu, %llu, %llu)", otherFriendInfo.userId, otherFriendInfo.leftGetStrengthTimes, otherFriendInfo.leftSendStrengthTimes);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_UserFriendInfo);
			m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);
		}

		bool find = false;
		for (size_t j = 0; j < otherFriendInfo.getGiftList.size(); j++)
		{
			if (otherFriendInfo.getGiftList[j] == ret.userId)
			{
				find = true;
				break;
			}
		}

		if (!find)
		{
			otherFriendInfo.getGiftList.push_back(ret.userId);

			Json::Value getGiftList = Json::arrayValue;
			Json::FastWriter write;
			for (size_t i = 0; i < otherFriendInfo.getGiftList.size(); i++)
			{
				getGiftList.append(otherFriendInfo.getGiftList[i]);
			}
			std::string contents = write.write(getGiftList);

			char buf[4096] = { 0 };
			sprintf(buf, "update `friendInfo` set `getGiftList` = '%s', `leftGetStrengthTimes` = %llu where `userId` = %llu", contents.c_str(), otherFriendInfo.leftGetStrengthTimes, otherFriendInfo.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserFriendInfo);
			m_data_cache.set_friendInfo(otherFriendInfo.userId, otherFriendInfo);

			count++;
		}
	}

	m_data_cache.set_friendInfo(req.userId, friendInfo);
	updateDailyTaskCountInfo(req.userId, dailyTaskCount::giveStrength_type, count);

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);
}

MY_FUNCTION1(fetch_delegate_quest_reward)
{
	BinaryReadStream readStream(data_.c_str(), data_.size());
	ss_msg_fetch_delegate_quest_reward_req req;
	readStream >> req;

	ss_msg_fetch_delegate_quest_reward_ret ret;
	ret.socket_id = req.socket_id;
	ret.index_id = req.index_id;
	ret.itemId = req.itemId;
	ret.itemNum = req.itemNum;
	ret.userId = req.userId;
	ret.DQExp = req.DQExp;
	ret.rewardGold = req.rewardGold;
	ret.id = req.id;
	ret.all = req.all;
	ret.nResult = (int)RET_SUCCESS;

	std::vector<strDelegateQuest> delegateQuests;
	if (!m_data_cache.get_delegateQuest(req.userId, delegateQuests))
	{
		ret.nResult = (int)RET_DATA_ERROR;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	std::vector<strDelegateEvent> vecDelegateEvent;
	if (!m_data_cache.get_delegateEvent(req.userId, vecDelegateEvent))
	{
		ret.nResult = (int)RET_DATA_ERROR;
		BinaryWriteStream writeStream;
		writeStream << ret;
		socket_->async_write(writeStream);
		return;
	}

	int finishCount = 0;
	if (req.id != 0)
	{
		for (size_t i = 0; i < delegateQuests.size(); i++)
		{
			if (delegateQuests[i].taskId == req.id)
			{
				for (size_t j = 0; j < delegateQuests[i].vecPet.size(); j++)
				{
					strPetInfo petInfo;
					if (m_data_cache.get_petInfo(req.userId, delegateQuests[i].vecPet[j], NULL, petInfo))
					{
						petInfo.DelegateQuest = 0;

						char buf[1024] = { 0 };
						sprintf(buf, "update `gamePetInfo` set `DelegateQuest` = 0 where `ID` = %llu and `userId` = %llu;", petInfo.ID, req.userId);
						singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
						m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
					}
				}

				finishCount++;
				delegateQuests[i].status = GetReward;
				delegateQuests[i].vecPet.clear();
				break;
			}
		}

		m_db.updateUserDelegateQuestList(req.userId, delegateQuests, vecDelegateEvent);
	}

	if (req.all)
	{
		for (size_t i = 0; i < delegateQuests.size(); i++)
		{
			for (size_t k = 0; k < req.completedIds.size(); k++)
			{
				if (delegateQuests[i].taskId == req.completedIds[k])
				{
					for (size_t j = 0; j < delegateQuests[i].vecPet.size(); j++)
					{
						strPetInfo petInfo;
						if (m_data_cache.get_petInfo(req.userId, delegateQuests[i].vecPet[j], NULL, petInfo))
						{
							petInfo.DelegateQuest = 0;

							char buf[1024] = { 0 };
							sprintf(buf, "update `gamePetInfo` set `DelegateQuest` = 0 where `ID` = %llu and `userId` = %llu;", petInfo.ID, req.userId);
							singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
							m_data_cache.set_petInfo(petInfo.userId, petInfo.ID, petInfo.base, petInfo);
						}
					}

					finishCount++;
					delegateQuests[i].status = GetReward;
					delegateQuests[i].vecPet.clear();
				}
			}
		}

		m_db.updateUserDelegateQuestList(req.userId, delegateQuests, vecDelegateEvent);
	}

	this->gainItems(req.itemId, req.itemNum, req.userId, ret.itemUniqId);

	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(req.userId, userInfo))
	{
		userInfo.DQExp += req.DQExp;
		userInfo.userGold += req.rewardGold;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `DQExp` =  `DQExp` + %llu, `userGold` = `userGold` + %llu where `userId` = %llu;", req.DQExp, req.rewardGold, req.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}

	BinaryWriteStream writeStream;
	writeStream << ret;
	socket_->async_write(writeStream);

	updateDailyTaskCountInfo(ret.userId, dailyTaskCount::delegateQuest_type, finishCount);
}

void football_db_analyze_t::updateRoleTalent(strRoleInfo* roleInfo_, std::vector<uint64_t>& vecTalent, uint64_t& talentLock)
{
	//默认初始化2个天赋
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<std::string, std::string>& mapGlobal = config.get_global_config();
	std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("TalentUnlockLv");
	if (iterGlobal == mapGlobal.end())
	{
		return;
	}

	if (roleInfo_->level_ < atol(iterGlobal->second.c_str()))
	{
		return;
	}

	std::map<uint64_t, config_playerBase_t*>& mapPlayerBase = config.get_playerbase_config();
	std::map<uint64_t, config_playerBase_t*>::iterator iterPlayBase = mapPlayerBase.find(roleInfo_->rolebase);
	if (iterPlayBase == mapPlayerBase.end())
	{
		return;
	}

	if (iterPlayBase->second->m_Talent1UnlockLv <= roleInfo_->level_ && roleInfo_->talent0 == 0)
	{
		if (config.createTalentId(roleInfo_->rolebase, roleInfo_->talent0))
		{
			roleInfo_->talentLock &= ~(0x01 << 1);
		}
	}

	if (iterPlayBase->second->m_Talent2UnlockLv <= roleInfo_->level_ && roleInfo_->talent1 == 0)
	{
		if (config.createTalentId(roleInfo_->rolebase, roleInfo_->talent1))
		{
			roleInfo_->talentLock &= ~(0x01 << 2);
		}
	}

	if (iterPlayBase->second->m_Talent3UnlockLv <= roleInfo_->level_ && roleInfo_->talent2 == 0)
	{
		if (config.createTalentId(roleInfo_->rolebase, roleInfo_->talent2))
		{
			roleInfo_->talentLock &= ~(0x01 << 3);
		}
	}

	if (iterPlayBase->second->m_Talent4UnlockLv <= roleInfo_->level_ && roleInfo_->talent3 == 0)
	{
		if (config.createTalentId(roleInfo_->rolebase, roleInfo_->talent3))
		{
			roleInfo_->talentLock &= ~(0x01 << 4);
		}
	}

	if (iterPlayBase->second->m_Talent5UnlockLv <= roleInfo_->level_ && roleInfo_->talent4 == 0)
	{
		if (config.createTalentId(roleInfo_->rolebase, roleInfo_->talent4))
		{
			roleInfo_->talentLock &= ~(0x01 << 5);
		}
	}

	char buf[2048] = { 0 };
	sprintf(buf, "update `userRoleInfo` set `talent0` = %llu, `talent1` = %llu, `talent2` = %llu, `talent3` = %llu, `talent4` = %llu, `talentLock` = %d where `Role` = %llu; ",
		roleInfo_->talent0, roleInfo_->talent1, roleInfo_->talent2, roleInfo_->talent3, roleInfo_->talent4, roleInfo_->talentLock, roleInfo_->role);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);

	vecTalent.push_back(roleInfo_->talent0);
	vecTalent.push_back(roleInfo_->talent1);
	vecTalent.push_back(roleInfo_->talent2);
	vecTalent.push_back(roleInfo_->talent3);
	vecTalent.push_back(roleInfo_->talent4);

	talentLock = roleInfo_->talentLock;
}

void football_db_analyze_t::updateQuestStateCount(uint64_t userId, uint64_t type, uint64_t target, uint64_t count)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuestList = mapQuest.begin();
		for (; iterQuestList != mapQuest.end(); ++iterQuestList)
		{
			if (iterQuestList->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuestList->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Type != type ||
					iterQuestBase->second->m_Target != target)
					continue;

				iterQuestList->second->progress += count;
				if (iterQuestList->second->progress >= atol(iterQuestBase->second->m_Param.c_str()))
					iterQuestList->second->state = EQuestState::Completed_Quest;
			}
		}
		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updateInviteCodeQuestState(uint64_t userId, uint64_t oldLevel, uint64_t newLevel)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();
		
		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuestList = mapQuest.begin();
		for (; iterQuestList != mapQuest.end(); ++iterQuestList)
		{
			if (iterQuestList->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuestList->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Type != 11)//!宠物升星过滤
					continue;

				std::vector<uint64_t> vecParam;
				parseString2Vector(iterQuestBase->second->m_Param, vecParam);

				if (newLevel >= vecParam[0] && vecParam[0] > oldLevel)
					iterQuestList->second->progress += 1;

				if (iterQuestList->second->progress >= vecParam[1])
					iterQuestList->second->state = EQuestState::Completed_Quest;
			}
		}
		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updatePetStarUpQuestState(uint64_t userId, uint64_t oldStarLevel, uint64_t newStarLevel)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if(m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuestList = mapQuest.begin();
		for (; iterQuestList != mapQuest.end();++iterQuestList)
		{
			if (iterQuestList->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuestList->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Type != 22)//!宠物升星过滤
					continue;

				std::vector<uint64_t> vecParam;
				parseString2Vector(iterQuestBase->second->m_Param, vecParam);

				if (newStarLevel >= vecParam[0] && vecParam[0] > oldStarLevel)
					iterQuestList->second->progress += 1;

				if (iterQuestList->second->progress >= vecParam[1])
					iterQuestList->second->state = EQuestState::Completed_Quest;
			}
		}
		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updatePetRankUpQuestState(uint64_t userId, uint64_t oldRank, uint64_t newRankUp)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuestList = mapQuest.begin();
		for (; iterQuestList != mapQuest.end();++iterQuestList)
		{
			if (iterQuestList->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuestList->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Target != 21)//!宠物升阶过滤
					continue;

				std::vector<uint64_t> vecParam;
				parseString2Vector(iterQuestBase->second->m_Param, vecParam);

				if (newRankUp >= vecParam[0] && vecParam[0] > oldRank)
				{
					iterQuestList->second->progress += 1;
				}

				if (iterQuestList->second->progress >= vecParam[1])
				{
					iterQuestList->second->state = EQuestState::Completed_Quest;
				}
			}
		}

		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updatePetLevelUpQuestState(uint64_t userId, uint64_t oldLevel, uint64_t newLevel)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuest = mapQuest.begin();
		for (; iterQuest != mapQuest.end();++iterQuest)
		{
			if (iterQuest->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuest->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Target != 20) //!宠物升级过滤
					continue;

				std::vector<uint64_t> vecParam;
				parseString2Vector(iterQuestBase->second->m_Param, vecParam);

				if (newLevel >= vecParam[0] && vecParam[0] > oldLevel)
					iterQuest->second->progress += 1;

				if (iterQuest->second->progress >= vecParam[1])
					iterQuest->second->state = EQuestState::Completed_Quest;
			}
		}
		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updateGetPetQuestState(uint64_t userId)
{
	std::map <std::uint64_t, boost::shared_ptr<strQuestList>> mapQuest;
	if (m_data_cache.get_questListByUserId(userId, mapQuest))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();

		std::map <std::uint64_t, boost::shared_ptr<strQuestList>>::iterator iterQuest = mapQuest.begin();
		for (; iterQuest != mapQuest.end();++iterQuest)
		{
			if (iterQuest->second->state < EQuestState::Completed_Quest)
			{
				std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iterQuest->first);
				if (iterQuestBase == mapQuestBase.end())
					continue;

				if (iterQuestBase->second->m_Target != 19) //!过滤收集宠物
					continue;

				iterQuest->second->progress += 1;
				if (iterQuest->second->progress >= atoi(iterQuestBase->second->m_Param.c_str()))
					iterQuest->second->state = EQuestState::Completed_Quest;
			}
		}
		m_db.hUpdateUserQuestList(userId, mapQuest);
	}
	MV_SAFE_RELEASE(mapQuest);
}

void football_db_analyze_t::updateRoleSkillUnlock(strRoleInfo* info_, uint64_t skillId_, int index_, uint64_t defaultLevel)
{
	if (skillId_ == 0)
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_skillBase_t*>& mapSkillBase = config.get_skillBase_config();
		std::map<uint64_t, config_playerBase_t*>& mapPlayerBase = config.get_playerbase_config();

		std::map<uint64_t, config_playerBase_t*>::iterator iterPlayerBase = mapPlayerBase.find(info_->rolebase);
		if (iterPlayerBase != mapPlayerBase.end())
		{
			int skillBase = 0, unlockLv = 0;
			if (index_ == 0) {skillBase = iterPlayerBase->second->m_Skill1ID; unlockLv = iterPlayerBase->second->m_Skill1UnlockLv;}
			else if (index_ == 1) {skillBase = iterPlayerBase->second->m_Skill2ID; unlockLv = iterPlayerBase->second->m_Skill2UnlockLv;}
			else if (index_ == 2) {skillBase = iterPlayerBase->second->m_Skill3ID; unlockLv = iterPlayerBase->second->m_Skill3UnlockLv;}
			else if (index_ == 3) {skillBase = iterPlayerBase->second->m_Skill4ID; unlockLv = iterPlayerBase->second->m_Skill4UnlockLv;}
			else if (index_ == 4) {skillBase = iterPlayerBase->second->m_Skill5ID; unlockLv = iterPlayerBase->second->m_Skill5UnlockLv;}
			else if (index_ == 5) {skillBase = iterPlayerBase->second->m_Skill6ID; unlockLv = iterPlayerBase->second->m_Skill6UnlockLv;}
			else if (index_ == 6) {skillBase = iterPlayerBase->second->m_Skill7ID; unlockLv = iterPlayerBase->second->m_Skill7UnlockLv;}

			if (skillBase == 0)
			{
				logerror((LOG_SERVER, "failed to find skillbase by index %d, player base:%llu", index_, info_->rolebase));
				return;
			}

			std::map<uint64_t, config_skillBase_t*>::iterator iterSkillBase = mapSkillBase.find(skillBase);
			if (iterSkillBase != mapSkillBase.end())
			{
				if (unlockLv <= info_->level_)
				{
					char buf[1024] = { 0 };
					sprintf(buf, "insert into `gameSkillInfo`(`Base`, `userId`, `PetId`, `Lv`, `CurExp`, `Equip`, `SlotNum`) values(%d, %llu, %llu, %llu, 0, 1, 0);", skillBase, info_->userid, info_->role, defaultLevel);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameSkillInfo);
					uint64_t skillId = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameSkillInfo);

					strSkillInfo skillInfo;
					skillInfo.Base = skillBase;
					skillInfo.userId = info_->userid;
					skillInfo.Lv = defaultLevel;
					skillInfo.petId = info_->role;
					skillInfo.CurExp = 0;
					skillInfo.Equip = 1;
					skillInfo.SlotNum = 0;
					skillInfo.ID = skillId;

					m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);

					if (index_ == 0) info_->skill0 = skillId;
					else if (index_ == 1) info_->skill1 = skillId;
					else if (index_ == 2) info_->skill2 = skillId;
					else if (index_ == 3) info_->skill3 = skillId;
					else if (index_ == 4) info_->skill4 = skillId;
					else if (index_ == 5) info_->skill5 = skillId;
					else if (index_ == 6) info_->skill6 = skillId;

					char bufPet[1024] = { 0 };
					sprintf(bufPet, "update `userRoleInfo` set `skill%d` = %llu where `Role` = %llu;", index_, skillId, info_->role);
					singleton_t<db_request_list>::instance().push_request_list(bufPet, sql_update, DB_UserRoleInfo);
				}
			}
		}
	}
}

void football_db_analyze_t::updateRoleSkillUnlockInfo(strRoleInfo* info_, uint64_t defaultLevel)
{
	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_skillBase_t*>& mapSkillBase = config.get_skillBase_config();
	std::map<uint64_t, config_playerBase_t*>& mapPlayerBase = config.get_playerbase_config();

	this->updateRoleSkillUnlock(info_, info_->skill0, 0, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill1, 1, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill2, 2, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill3, 3, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill4, 4, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill5, 5, defaultLevel);
	this->updateRoleSkillUnlock(info_, info_->skill6, 6, defaultLevel);

	std::map<uint64_t, config_playerBase_t*>::iterator iterPlayerBase = mapPlayerBase.find(info_->rolebase);
	if (iterPlayerBase != mapPlayerBase.end())
	{
		if (info_->level_ >= iterPlayerBase->second->m_Skill1UnlockLv)
		{
			info_->skillunlock |= 0x1;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill2UnlockLv)
		{
			info_->skillunlock |= 0x1 << 1;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill3UnlockLv)
		{
			info_->skillunlock |= 0x1 << 2;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill4UnlockLv)
		{
			info_->skillunlock |= 0x1 << 3;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill5UnlockLv)
		{
			info_->skillunlock |= 0x1 << 4;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill6UnlockLv)
		{
			info_->skillunlock |= 0x1 << 5;
		}

		if (info_->level_ >= iterPlayerBase->second->m_Skill7UnlockLv)
		{
			info_->skillunlock |= 0x1 << 6;
		}

		char bufPet[1024] = { 0 };
		sprintf(bufPet, "update `userRoleInfo` set `skillLock` = %d where `Role` = %llu;", info_->skillunlock, info_->role);
		singleton_t<db_request_list>::instance().push_request_list(bufPet, sql_update, DB_UserRoleInfo);
	}
}

void football_db_analyze_t::updatePetSkillUnlock(strPetInfo* info, uint64_t skillId_, int index_, uint64_t petSkillLevel)
{
	if (skillId_ == 0)
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_skillBase_t*>& mapSkillBase = config.get_skillBase_config();
		std::map<uint64_t, config_petBase_t*>& mapPetBase = config.get_petBase_config();

		std::map<uint64_t, config_petBase_t*>::iterator iterPetBase = mapPetBase.find(info->base);
		if (iterPetBase != mapPetBase.end())
		{
			int skillBase = 0;
			if (index_ == 0) skillBase = iterPetBase->second->m_Skill1;
			else if (index_ == 1) skillBase = iterPetBase->second->m_Skill2;
			else if (index_ == 2) skillBase = iterPetBase->second->m_Skill3;
			else if (index_ == 3) skillBase = iterPetBase->second->m_Skill4;

			if (skillBase == 0)
			{
				logerror((LOG_SERVER, "failed to find skillbase by index %d, pet base:%llu", index_, info->base));
				return;
			}

			std::map<uint64_t, config_skillBase_t*>::iterator iterSkillBase = mapSkillBase.find(skillBase);
			if (iterSkillBase != mapSkillBase.end())
			{
				if (iterSkillBase->second->m_UnlockLevel <= info->LV && iterSkillBase->second->m_UnlockRank <= info->Rank && iterSkillBase->second->m_UnlockStar <= info->Star)
				{
					char buf[1024] = { 0 };
					sprintf(buf, "insert into `gameSkillInfo`(`Base`, `userId`, `PetId`, `Lv`, `CurExp`, `Equip`, `SlotNum`) values(%d, %llu, %llu, %llu, 0, 1, 0);", skillBase, info->userId, info->ID, petSkillLevel);
					singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameSkillInfo);
					uint64_t skillId = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameSkillInfo);

					strSkillInfo skillInfo;
					skillInfo.Base = skillBase;
					skillInfo.userId = info->userId;
					skillInfo.Lv = petSkillLevel;
					skillInfo.petId = info->ID;
					skillInfo.CurExp = 0;
					skillInfo.Equip = 1;
					skillInfo.SlotNum = 0;
					skillInfo.ID = skillId;

					m_data_cache.set_skillInfo(skillInfo.userId, skillInfo.ID, skillInfo.Base, skillInfo);

					if (index_ == 0) info->SkillsId0 = skillId;
					else if (index_ == 1) info->SkillsId1 = skillId;
					else if (index_ == 2) info->SkillsId2 = skillId;
					else if (index_ == 3) info->SkillsId3 = skillId;

					char bufPet[1024] = { 0 };
					sprintf(bufPet, "update `gamePetInfo` set `SkillsId%d` = %llu where `ID` = %llu;", index_, skillId, info->ID);
					singleton_t<db_request_list>::instance().push_request_list(bufPet, sql_update, DB_GamePetInfo);
				}
			}
		}
	}
}

void football_db_analyze_t::updatePetSkillUnlockInfo(strPetInfo* info, uint64_t petSkillLevel)
{
	this->updatePetSkillUnlock(info, info->SkillsId0, 0, petSkillLevel);
	this->updatePetSkillUnlock(info, info->SkillsId1, 1, petSkillLevel);
	this->updatePetSkillUnlock(info, info->SkillsId2, 2, petSkillLevel);
	this->updatePetSkillUnlock(info, info->SkillsId3, 3, petSkillLevel);
}

void football_db_analyze_t::updateDatabaseRoleInfo(strRoleInfo* roleInfo)
{
	char buf[2048] = { 0 };
	sprintf(buf, "update `userRoleInfo` set "
		"`HPMax` = %llu,"
		"`HPRestore` = %llu,"
		"`SoulMax` = %llu,"
		"`SoulRestore` = %llu,"
		"`PhysicalDamage` = %llu,"
		"`PhysicalDefense` = %llu,"
		"`MagicDamage` = %llu,"
		"`MagicDefense` = %llu,"
		"`Critical` = %llu,"
		"`Tough` = %llu,"
		"`Hit` = %llu,"
		"`Block` = %llu,"
		"`CriticalDamage` = %llu,"
		"`MoveSpeed` = %llu,"
		"`FastRate` = %llu,"
		"`StiffAdd` = %llu,"
		"`StiffSub` = %llu,"
		"`AbilityMax` = %llu,"
		"`AbHitAdd` = %llu,"
		"`AbRestore` = %llu,"
		"`AbUseAdd` = %llu, "
		"`runeSlotNum` = %llu, "
		"`soulSlotNum` = %llu, "
		"`talentLock` = %d, "
		"`talent0` = %llu, "
		"`talent1` = %llu, "
		"`talent2` = %llu, "
		"`talent3` = %llu, "
		"`talent4` = %llu, "
		"`newEquip1` = %llu, "
		"`newEquip2` = %llu, "
		"`newEquip3` = %llu, "
		"`newEquip4` = %llu, "
		"`newEquip5` = %llu, "
		"`newEquip6` = %llu, "
		"`newEquipPotential` = %llu "
		"where `Role` = %llu; ",
		roleInfo->hpmax_,
		roleInfo->hprestore_,
		roleInfo->soulmax_,
		roleInfo->soulrestore_,
		roleInfo->physicaldamage_,
		roleInfo->physicaldefense_,
		roleInfo->magicdamage_,
		roleInfo->magicdefense_,
		roleInfo->critical_,
		roleInfo->tough_,
		roleInfo->hit_,
		roleInfo->block_,
		roleInfo->criticaldamage_,
		roleInfo->movespeed_,
		roleInfo->fastrate_,
		roleInfo->stiffadd_,
		roleInfo->stiffsub_,
		roleInfo->abilitymax_,
		roleInfo->abhitadd_,
		roleInfo->abrestore_,
		roleInfo->abuseadd_,
		roleInfo->runeslotnum,
		roleInfo->soulSlotNum,
		roleInfo->talentLock,
		roleInfo->talent0,
		roleInfo->talent1,
		roleInfo->talent2,
		roleInfo->talent3,
		roleInfo->talent4,
		roleInfo->NewEquip1,
		roleInfo->NewEquip2,
		roleInfo->NewEquip3,
		roleInfo->NewEquip4,
		roleInfo->NewEquip5,
		roleInfo->NewEquip6,
		roleInfo->NewEquipPotential,
		roleInfo->role);

	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_UserRoleInfo);
	m_data_cache.set_roleInfo(roleInfo->userid, roleInfo->role, roleInfo->rolebase, *roleInfo);
}

void football_db_analyze_t::updateDatabasePetInfo(strPetInfo* petInfo)
{
	char buf[2048] = { 0 };
	sprintf(buf, "update `gamePetInfo` set "
		"`Lv` = %llu, "
		"`Rank` = %llu, "
		"`Star` = %llu, "
		"`EXP` = %llu, "
		"`NextExp` = %llu, "
		"`HPMax` = %llu, "
		"`HPRestore` = %llu, "
		"`SoulMax` = %llu, "
		"`SoulRestore` = %llu, "
		"`PhysicalDamage` = %llu, "
		"`PhysicalDefense` = %llu, "
		"`MagicDamage` = %llu, "
		"`MagicDefense` = %llu, "
		"`Critical` = %llu, "
		"`Tough` = %llu, "
		"`Hit` = %llu, "
		"`Block` = %llu, "
		"`CriticalDamage` = %llu, "
		"`MoveSpeed` = %llu, "
		"`FastRate` = %llu, "
		"`newEquip1` = %llu, "
		"`newEquip2` = %llu, "
		"`newEquip3` = %llu, "
		"`newEquip4` = %llu, "
		"`newEquip5` = %llu, "
		"`newEquip6` = %llu, "
		"`newEquipPotential` = %llu "
		"where `ID` = %llu; ",
		petInfo->LV,
		petInfo->Rank,
		petInfo->Star,
		petInfo->EXP,
		petInfo->NextExp,
		petInfo->HPMax,
		petInfo->HPRestore,
		petInfo->SoulMax,
		petInfo->SoulRestore,
		petInfo->PhysicalDamage,
		petInfo->PhysicalDefense,
		petInfo->MagicDamage,
		petInfo->MagicDefense,
		petInfo->Critical,
		petInfo->Tough,
		petInfo->Hit,
		petInfo->Block,
		petInfo->CriticalDamage,
		petInfo->MoveSpeed,
		petInfo->FastRate,
		petInfo->NewEquip1,
		petInfo->NewEquip2,
		petInfo->NewEquip3,
		petInfo->NewEquip4,
		petInfo->NewEquip5,
		petInfo->NewEquip6,
		petInfo->NewEquipPotential,
		petInfo->ID);

	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);
	m_data_cache.set_petInfo(petInfo->userId, petInfo->ID, petInfo->base, *petInfo);
}

#pragma region Helper Function
bool football_db_analyze_t::useItem(strItemInfo& itemInfo, const uint64_t needNum)
{
	if (itemInfo.Num >= needNum)
	{
		itemInfo.Num -= needNum;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameItemInfo` set `itemNum` = `itemNum` - %llu where `itemId` = %llu;", needNum, itemInfo.ID);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameItemInfo);
		m_data_cache.set_itemInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
	}
	else
	{
		logerror((LOG_SERVER, "item check error, id:%llu own:%llu need:%llu", itemInfo.ID, itemInfo.Num, needNum));
		return false;
	}
	return true;
}

bool football_db_analyze_t::useItem(const uint64_t itemId, strItemInfo& itemInfo, const uint64_t needNum)
{
	if (m_data_cache.get_itemInfo(NULL, itemId, NULL, itemInfo))
	{
		return useItem(itemInfo, needNum);
	}
	return false;
}

bool football_db_analyze_t::useItem(const uint64_t userId, const uint64_t itemBase, strItemInfo& itemInfo, const uint64_t needNum)
{
	if (m_data_cache.get_itemInfo(userId, NULL, itemBase, itemInfo))
	{
		return useItem(itemInfo, needNum);
	}
	return false;
}

bool football_db_analyze_t::addItemNumOrCreate(const uint64_t userId, const uint64_t itemBase, strItemInfo& itemInfo, const uint64_t addNum)
{
	if (m_data_cache.get_itemInfo(userId, NULL, itemBase, itemInfo))
	{
		itemInfo.Num += addNum;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameItemInfo` set `itemNum` = %llu where `itemId` = %llu", itemInfo.Num, itemInfo.ID);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameItemInfo);
		m_data_cache.set_itemInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
	}
	else
	{
		itemInfo.userId = userId;
		itemInfo.Base = itemBase;
		itemInfo.Num = addNum;

		itemInfo.PveStandby = 0;
		itemInfo.PvpStandby = 0;
		itemInfo.Order = 0;
		itemInfo.Artifact = 0;
		itemInfo.ArtifactLv = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `gameItemInfo` (`itemBase`, `itemNum`, `itemUser`) values(%llu, %llu, %llu)", itemInfo.Base, itemInfo.Num, itemInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameItemInfo);
		itemInfo.ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameItemInfo);
		m_data_cache.set_itemInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
		return true;
	}
	return false;
}


bool football_db_analyze_t::useEquip(strEquipInfo& itemInfo)
{
	if (itemInfo.Num > itemInfo.Equip)
	{
		itemInfo.Equip++;

		assert(itemInfo.Num >= itemInfo.Equip);

		char bufEquip[1024] = { 0 };
		sprintf(bufEquip, "update `gameEquipInfo` set `Equip` =  %llu where `ID` = %llu", itemInfo.Equip, itemInfo.ID);
		singleton_t<db_request_list>::instance().push_request_list(bufEquip, sql_update, DB_GameEquipInfo);
		m_data_cache.set_equipInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
		return true;
	}
	else
	{
		return false;
	}
}

bool football_db_analyze_t::useEquip(const uint64_t equipId, strEquipInfo& equipInfo)
{
	if (m_data_cache.get_equipInfo(NULL, equipId, NULL, equipInfo))
	{
		return useEquip(equipInfo);
	}
	return false;
}

bool football_db_analyze_t::useEquip(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo)
{
	if (m_data_cache.get_equipInfo(userId, NULL, equipBase, equipInfo))
	{
		return useEquip(equipInfo);
	}
	return false;
}

bool football_db_analyze_t::unuseEquip(strEquipInfo& itemInfo)
{
	itemInfo.Equip--;

	assert(itemInfo.Num >= itemInfo.Equip);

	char bufEquip[1024] = { 0 };
	sprintf(bufEquip, "update `gameEquipInfo` set `Equip` = %llu where `ID` = %llu", itemInfo.Equip, itemInfo.ID);
	singleton_t<db_request_list>::instance().push_request_list(bufEquip, sql_update, DB_GameEquipInfo);
	m_data_cache.set_equipInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
	return true;
}

bool football_db_analyze_t::unuseEquip(const uint64_t equipId, strEquipInfo& equipInfo)
{
	if (m_data_cache.get_equipInfo(NULL, equipId, NULL, equipInfo))
	{
		return unuseEquip(equipInfo);
	}
	return false;
}

bool football_db_analyze_t::unuseEquip(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo)
{
	if (m_data_cache.get_equipInfo(userId, NULL, equipBase, equipInfo))
	{
		return unuseEquip(equipInfo);
	}
	return false;
}

bool football_db_analyze_t::addEquipNumOrCreate(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo, const uint64_t addNum)
{
	if (m_data_cache.get_equipInfo(userId, NULL, equipBase, equipInfo))
	{
		equipInfo.Num += addNum;

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameEquipInfo` set `Num` = %llu where `ID` = %llu", equipInfo.Num, equipInfo.ID);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameEquipInfo);
		m_data_cache.set_equipInfo(equipInfo.userId, equipInfo.ID, equipInfo.Base, equipInfo);
	}
	else
	{
		equipInfo.userId = userId;
		equipInfo.Base = equipBase;
		equipInfo.Num = addNum;
		equipInfo.Equip = 0;
		equipInfo.roleId = 0;
		equipInfo.Lv = 1;
		equipInfo.Exp = 0;

		char buf[1024] = { 0 };
		sprintf(buf, "insert into `gameEquipInfo` (`Base`, `Num`, `userId`, `Lv`) values(%llu, %llu, %llu, 1)", equipBase, equipInfo.Num, userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_insert, DB_GameEquipInfo);
		equipInfo.ID = singleton_t<db_request_list>::instance().getLastInsertID(DB_GameEquipInfo);
		m_data_cache.set_equipInfo(equipInfo.userId, equipInfo.ID, equipInfo.Base, equipInfo);
		return true;
	}
	return false;
}

bool football_db_analyze_t::reduceEquip(strEquipInfo& itemInfo, const uint64_t needNum, const bool equiped)
{
	if (itemInfo.Num >= needNum)
	{
		if (equiped)
		{
			itemInfo.Num -= needNum;
			itemInfo.Equip -= needNum;

			assert(itemInfo.Num >= itemInfo.Equip);

			char bufEquip0[1024] = { 0 };
			sprintf(bufEquip0, "update `gameEquipInfo` set `Equip` = %llu, `Num` = %llu where `ID` = %llu;", itemInfo.Equip, itemInfo.Num, itemInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(bufEquip0, sql_update, DB_GameEquipInfo);
			m_data_cache.set_equipInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
		}
		else
		{
			itemInfo.Num -= needNum;

			assert(itemInfo.Num >= itemInfo.Equip);

			char buf[1024] = { 0 };
			sprintf(buf, "update `gameEquipInfo` set `Num` = %llu where `ID` = %llu;", itemInfo.Num, itemInfo.ID);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameEquipInfo);
			m_data_cache.set_equipInfo(itemInfo.userId, itemInfo.ID, itemInfo.Base, itemInfo);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool football_db_analyze_t::reduceEquip(const uint64_t equipId, strEquipInfo& equipInfo, const uint64_t needNum, const bool equiped)
{
	if (m_data_cache.get_equipInfo(NULL, equipId, NULL, equipInfo))
	{
		return reduceEquip(equipInfo, needNum, equiped);
	}
	return false;
}

bool football_db_analyze_t::reduceEquip(const uint64_t userId, const uint64_t equipBase, strEquipInfo& equipInfo, const uint64_t needNum, const bool equiped)
{
	if (m_data_cache.get_equipInfo(userId, NULL, equipBase, equipInfo))
	{
		return reduceEquip(equipInfo, needNum, equiped);
	}
	return false;
}

void football_db_analyze_t::gainItems(const std::vector<uint64_t> items, const std::vector<uint64_t> numbers, const uint64_t userId, std::vector<uint64_t>& newItemID)
{
	strUserInfo userInfo;
	if (m_data_cache.get_userInfo(userId, userInfo))
	{
		for (size_t i = 0; i < items.size(); i++)
		{
			if (items[i] == 40001)
			{
				userInfo.userStrength += numbers[i];
				newItemID.push_back(40001);

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu", userInfo.userStrength, userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

			}
			//gold
			else if (items[i] == CURRENCY_GOLD)
			{
				userInfo.userGold += numbers[i];
				newItemID.push_back(CURRENCY_GOLD);


				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userGold` = %llu where `userId` = %llu", userInfo.userGold, userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			}
			//gem
			else if (items[i] == CURRENCY_GEM)
			{
				userInfo.userGem += numbers[i];
				newItemID.push_back(CURRENCY_GEM);

				char buf[1024] = { 0 };
				sprintf(buf, "update `gameUserInfo` set `userGem` = %llu where `userId` = %llu", userInfo.userGem, userId);
				singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
			}
			//check equip or items
			else if (IsEquipItem(items[i]))
			{
				strEquipInfo equipInfo;
				this->addEquipNumOrCreate(userId, items[i], equipInfo, numbers[i]);
				newItemID.push_back(equipInfo.ID);
			}
			else
			{
				strItemInfo itemInfo;
				addItemNumOrCreate(userId, items[i], itemInfo, numbers[i]);
				newItemID.push_back(itemInfo.ID);
			}
		}
		m_data_cache.set_userInfo(userInfo.userId, userInfo);
	}
}

void football_db_analyze_t::gainItem(const uint64_t itemBase_, const uint64_t num, strUserInfo& userInfo, std::vector<uint64_t>& newItemID)
{
	if (itemBase_ == 40001)
	{
		userInfo.userStrength += num;
		newItemID.push_back(40001);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userStrength` = %llu where `userId` = %llu", num, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);

	}
	//gold
	else if (itemBase_ == CURRENCY_GOLD)
	{
		userInfo.userGold += num;
		newItemID.push_back(CURRENCY_GOLD);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGold` = %llu where `userId` = %llu", userInfo.userGold, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	}
	//gem
	else if (itemBase_ == CURRENCY_GEM)
	{
		userInfo.userGem += num;
		newItemID.push_back(CURRENCY_GEM);

		char buf[1024] = { 0 };
		sprintf(buf, "update `gameUserInfo` set `userGem` = `userGem` + %llu where `userId` = %llu", num, userInfo.userId);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	}
	//check equip or items
	else if (IsEquipItem(itemBase_))
	{
		strEquipInfo equipInfo;
		this->addEquipNumOrCreate(userInfo.userId, itemBase_, equipInfo, num);
		newItemID.push_back(equipInfo.ID);
	}
	else
	{
		strItemInfo itemInfo;
		addItemNumOrCreate(userInfo.userId, itemBase_, itemInfo, num);
		newItemID.push_back(itemInfo.ID);
	}
	m_data_cache.set_userInfo(userInfo.userId, userInfo);
}

void football_db_analyze_t::sendMail(uint64_t userId_, 
	const std::uint64_t mailId_, 
	const std::string& to_user_, 
	const std::string& title_,
	const std::string& content_, 
	const std::vector<uint64_t>& items_, 
	const std::vector<uint64_t>& nums_, 
	bool attach_)
{
	loginfo((LOG_SERVER, "send mail userId:%llu mailId:%llu to_user:%s title:%s content:%s item_count:%d", 
		userId_, mailId_, to_user_.c_str(), title_.c_str(), content_.c_str(), items_.size()));

	std::vector<strUserMailsInfo> mailInfo;
	if (!m_data_cache.get_mailInfo(userId_, mailInfo))
	{
		std::vector<strUserMailsInfo> vecUserMails;
		strUserMailsInfo info;
		info.userId = userId_;
		info.mailId = mailId_;
		info.title = title_;
		info.contents = content_;
		info.attach = attach_;
		info.flag = EMailState::Default;
		info.items = items_;
		info.itemsNumber = nums_;
		info.time = time(NULL);

		if (to_user_.size() > 0)
			info.sender = to_user_;
		else
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
			info.sender = "系统";
			std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Sender_General");
			if (iterLanguage != mapServerLanguage.end())
				info.sender = iterLanguage->second;
		}

		vecUserMails.push_back(info);
		m_db.hinitUserMailInfo(userId_, vecUserMails);
	}
	else
	{
		strUserMailsInfo info;
		info.userId = userId_;
		info.mailId = mailId_;
		info.title = title_;
		info.contents = content_;
		info.attach = attach_;
		info.flag = EMailState::Default;
		info.items = items_;
		info.itemsNumber = nums_;
		info.time = time(NULL);

		if (to_user_.size() > 0)
			info.sender = to_user_;
		else
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<std::string, std::string>& mapServerLanguage = config.get_serverLanguage_config();
			info.sender = "系统";
			std::map<std::string, std::string>::iterator iterLanguage = mapServerLanguage.find("Mail_Sender_General");
			if (iterLanguage != mapServerLanguage.end())
				info.sender = iterLanguage->second;
		}

		mailInfo.push_back(info);
		m_db.hupdateUserMailInfo(userId_, mailInfo);
	}
}

bool football_db_analyze_t::resetGameDataNewDay(uint64_t userId_)
{
	strUserInfo userInfo;
	if (!m_data_cache.get_userInfo(userId_, userInfo))
		return false;

#pragma region 重置用户信息
	userInfo.userBuyStrengthCount = 0;//!体力购买次数
	userInfo.userExchangeGoldNum = 0;//!炼金次数
	userInfo.questPoint = 0;
	userInfo.questStatus = 0;
	userInfo.guildFetched = 0;
	userInfo.guildProgress = 0;
	userInfo.guildDonateStatus = 0;

	char buf[1024] = { 0 };
	sprintf(buf, "update `gameUserInfo` set `userBuyStrengthCount` = 0, `userExchangeGoldNum` = 0, `questPoint` = 0, `questStatus` = 0, `guildFetched` = 0, `guildProgress` = 0, `guildDonateStatus` = 0 where `userId` = %llu;", userId_);
	singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);
#pragma endregion

#pragma region 充值信息
	strRechargeInfo rechargeInfo;
	if (m_data_cache.get_rechargeInfo(userId_, rechargeInfo))
	{
		rechargeInfo.dailyRecharge = 0;
		m_db.hupdateRechargeInfo(&rechargeInfo);
	}
#pragma endregion

#pragma region 重置PVE
	std::vector<boost::shared_ptr<strPveInfo>> vecPveInfo;
	m_data_cache.mget_pveInfo(userId_, vecPveInfo);
	for (size_t i = 0; i < vecPveInfo.size(); i++)
	{
		int remainCount = 0;

		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
		std::map<uint64_t, config_levelSetup_t*>::iterator iterSceneList = mapLevelSetUp.find(vecPveInfo[i]->pveType);
		if (iterSceneList != mapLevelSetUp.end())
		{
			remainCount = iterSceneList->second->m_CountLimit;
		}

		vecPveInfo[i]->remainCount = remainCount;
		vecPveInfo[i]->refreshCount = 0;
		m_data_cache.set_pveInfo(vecPveInfo[i]->userId, vecPveInfo[i]->pveType, *vecPveInfo[i].get());
	}
	MV_SAFE_RELEASE(vecPveInfo);

	game_resource_t& config = singleton_t<game_resource_t>::instance();
	std::map<uint64_t, config_levelSetup_t*>& mapLevelSetUp = config.get_levelSetup_config();
	std::map<uint64_t, config_levelSetup_t*>::iterator iterSceneList = mapLevelSetUp.begin();;
	for (; iterSceneList != mapLevelSetUp.end(); iterSceneList++)
	{
		char buf[1024] = { 0 };
		sprintf(buf, "update `userPveInfo` set `remainCount` = %d, `RefreshedCount` = 0 where `userId` = %llu and `pveType` = %llu", iterSceneList->second->m_CountLimit, userId_, iterSceneList->first);
		singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, "userPveInfo");
	}
#pragma endregion

#pragma region 重置抽卡
	strDrawCardInfo drawCardInfo;
	if (m_data_cache.get_drawCardInfo(userId_, drawCardInfo))
	{
		drawCardInfo.drawTime0 = 0;
		drawCardInfo.drawTime1 = 0;
		drawCardInfo.drawTimes0 = 0;
		drawCardInfo.drawTimes1 = 0;

		char bufCard[1024] = { 0 };
		sprintf(bufCard, "update `drawCardInfo` set `drawTime0` = 0, `drawTime1` = 0, `drawTimes0` = 0, `drawTimes1` = 0 where `userId` = %llu;", userId_);
		singleton_t<db_request_list>::instance().push_request_list(bufCard, sql_update, "drawCardInfo");
		m_data_cache.set_drawCardInfo(drawCardInfo.userId, drawCardInfo);
	}
#pragma endregion

#pragma region 重置英雄对战
	int remainCount = 0;
	iterSceneList = mapLevelSetUp.find(9000);
	if (iterSceneList != mapLevelSetUp.end())
		remainCount = iterSceneList->second->m_CountLimit;

	strPetPvpInfo petPvpInfo;
	if (m_data_cache.get_petPvpInfo(userId_, NULL, petPvpInfo))
	{
		if (petPvpInfo.userId >= 100000)
		{
			petPvpInfo.remainCount = remainCount;
			petPvpInfo.resetTimes = 0;

			char buf1[1024] = { 0 };
			sprintf(buf1, "update `userPetPvpInfo` set `remainCount` = %d, `resetTimes` = 0 where `userId` = %llu;", remainCount, userId_);
			singleton_t<db_request_list>::instance().push_request_list(buf1, sql_update, DB_UserPetPvpInfo);
			m_data_cache.set_petPvpInfo(petPvpInfo.userId, petPvpInfo.rank, petPvpInfo);
		}
	}
#pragma endregion

#pragma region 重置战队突袭
	//rolePvp
	remainCount = 0;
	iterSceneList = mapLevelSetUp.find(6000);
	if (iterSceneList != mapLevelSetUp.end())
		remainCount = iterSceneList->second->m_CountLimit;

	strRolePvpInfo rolePvpInfo;
	if (m_data_cache.get_rolePvpInfo(userId_, NULL, rolePvpInfo))
	{
		if (rolePvpInfo.userId >= 100000)
		{
			rolePvpInfo.remainCount = remainCount;
			rolePvpInfo.resetTimes = 0;

			char buf2[1024] = { 0 };
			sprintf(buf2, "update `userRolePvpInfo` set `remainCount` = %d, `resetTimes` = 0 where `userId` = %llu;", remainCount, userId_);
			singleton_t<db_request_list>::instance().push_request_list(buf2, sql_update, DB_UserRolePvpInfo);
			m_data_cache.set_rolePvpInfo(rolePvpInfo.userId, rolePvpInfo.rank, rolePvpInfo);
		}
	}

#pragma endregion

#pragma region 重置商店
	//shop info
	char buf4[1024] = { 0 };
	sprintf(buf4, "delete  from `shopInfo` where `userId` = %llu;", userId_);
	singleton_t<db_request_list>::instance().push_request_list(buf4, sql_update, DB_ShopInfo);
	m_data_cache.del_shopInfo(userId_);
#pragma endregion

#pragma region 重置好友信息
	strFriendInfo friendInfo;
	if (m_data_cache.get_friendInfo(userId_, friendInfo))
	{
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<std::string, std::string>& mapGlobal = config.get_global_config();
		std::map<std::string, std::string>::iterator iterGlobal = mapGlobal.find("FriendStaGetMaximum");
		if (iterGlobal != mapGlobal.end())
		{
			friendInfo.leftGetStrengthTimes = atol(iterGlobal->second.c_str());
		}
		iterGlobal = mapGlobal.find("FriendStaGainMaximum");
		if (iterGlobal != mapGlobal.end())
		{
			friendInfo.leftSendStrengthTimes = atol(iterGlobal->second.c_str());
		}

		friendInfo.sendGiftList.clear();
		friendInfo.getGiftList.clear();

		Json::Value sendGiftList = Json::arrayValue;
		Json::Value getGiftList = Json::arrayValue;
		Json::FastWriter write;

		std::string contents1 = write.write(sendGiftList);
		std::string contents2 = write.write(getGiftList);

		char buf3[1024] = { 0 };
		sprintf(buf3, "update `friendInfo` set `sendGiftList` = '%s', `getGiftList` = '%s', `leftGetStrengthTimes` = %llu, `leftSendStrengthTimes` = %llu where `userId` = %llu;",
			contents1.c_str(), contents2.c_str(), friendInfo.leftGetStrengthTimes, friendInfo.leftSendStrengthTimes, userId_);
		singleton_t<db_request_list>::instance().push_request_list(buf3, sql_update, DB_UserFriendInfo);
		m_data_cache.set_friendInfo(userId_, friendInfo);
	}
#pragma endregion

#pragma region 重置活动
	UserCelebrationInfoMap celebrationInfo;
	if (m_data_cache.get_celebrationInfo(userId_, celebrationInfo))
	{
		time_t now_time = time(NULL);
		game_resource_t& config = singleton_t<game_resource_t>::instance();
		std::map<uint64_t, config_celebrationList_t*>& mapCeleListByID = config.get_celebrationListByID_conifg();

		UserCelebrationInfoMap::iterator iterMapSec = celebrationInfo.begin();
		for (; iterMapSec != celebrationInfo.end();)
		{
			std::map<uint64_t, config_celebrationList_t*>::iterator iterCeleListByID = mapCeleListByID.find(iterMapSec->first);
			if (iterCeleListByID == mapCeleListByID.end())
			{
				iterMapSec++;
				continue;
			}

			if (iterCeleListByID->second->m_Type == CELEBRATION_TYPE_ONLINE_CUMULATE_TIME)
			{
				for (std::map<int64_t, uint64_t>::iterator iter = iterMapSec->second.begin(); iter != iterMapSec->second.end(); ++iter)
				{
					//上次纪录时间不能刷新
					if (iter->first == CELEBRATION_ONLINE_LAST_CUMULATE_TIME)
						continue;

					if (iter->first == CELEBRATION_START_TIME)
						continue;

					if (iter->first == CELEBRATION_LAST_REFRESH_TIME)
						continue;

					iter->second = 0;
				}
			}
			else if (iterCeleListByID->second->m_Type == CELEBRATION_TYPE_CHECK_DAY)
			{
				//上一条的奖励已领取，才可以领下一天的
				uint64_t index = 0;
				bool fetched_all = true;
				for (std::map<int64_t, uint64_t>::iterator iter = iterMapSec->second.begin(); iter != iterMapSec->second.end(); ++iter)
				{
					if (iter->first < 0)
						continue;

					//!已领取
					if (iter->second == 1)
					{
						index = iter->first;
					}
					else if (iter->second == 0 && index + 1 == iter->first)//!不可用
					{
						iter->second = 2;
						fetched_all = false;
						break;
					}
				}

				//if (fetched_all)
				//{
				//	for (std::map<int64_t, uint64_t>::iterator iter = iterMapSec->second.begin(); iter != iterMapSec->second.end(); ++iter)
				//	{
				//		if (iter->first < 0) continue;
				//		else if (iter->first == 1) iter->second = 2;
				//		else iter->second = 0;
				//	}
				//}
			}
			
			//!其他类型
			if (iterCeleListByID->second->m_Type != CELEBRATION_TYPE_ONLINE_CUMULATE_TIME)
			{
				//!每日刷新
				if (iterCeleListByID->second->m_RefreshTime == celebration_refresh_e::DAY)
				{
					celebrationInfo.erase(iterMapSec++);
					continue;
				}
				else if (iterCeleListByID->second->m_RefreshTime == celebration_refresh_e::WEEK)
				{
					std::map<int64_t, uint64_t>::iterator iterMap = iterMapSec->second.find(CELEBRATION_LAST_REFRESH_TIME);
					if (iterMap == iterMapSec->second.end())
					{
						iterMapSec->second.insert(make_pair(CELEBRATION_LAST_REFRESH_TIME, now_time));
					}
					else
					{
						bool sameWeek = IsSameWeek(iterMap->second, now_time);
						if (!sameWeek)
						{
							celebrationInfo.erase(iterMapSec++);
							continue;
						}
					}
				}
				else if (iterCeleListByID->second->m_RefreshTime == celebration_refresh_e::MONTH)
				{
					std::map<int64_t, uint64_t>::iterator iterMap = iterMapSec->second.find(CELEBRATION_LAST_REFRESH_TIME);
					if (iterMap == iterMapSec->second.end())
					{
						iterMapSec->second.insert(make_pair(CELEBRATION_LAST_REFRESH_TIME, now_time));
					}
					else
					{
						bool sameMonth = IsSameMonth(iterMap->second, now_time);
						if (!sameMonth)
						{
							celebrationInfo.erase(iterMapSec++);
							continue;
						}
					}
				}
			}

			iterMapSec++;
		}

		m_db.hUpdateCelebrationInfo(userId_, celebrationInfo);
	}
#pragma endregion

#pragma region 每日任务
	std::map<uint64_t, boost::shared_ptr<strQuestList>> questList;
	if (m_data_cache.get_questListByUserId(userId_, questList))
	{
		std::map<uint64_t, boost::shared_ptr<strQuestList>>::iterator iter = questList.begin();
		for (; iter != questList.end(); iter++)
		{
			game_resource_t& config = singleton_t<game_resource_t>::instance();
			std::map<uint64_t, config_questBase_t*>& mapQuestBase = config.get_questBase_config();
			std::map<uint64_t, config_questBase_t*>::iterator iterQuestBase = mapQuestBase.find(iter->second->questId);
			if (iterQuestBase == mapQuestBase.end())
				continue;

			if (iterQuestBase->second->m_Type == 3)
			{
				iter->second->progress = 0;
				iter->second->state = EQuestState::AlreadyAccept_Quest;
			}
		}

		m_db.hUpdateUserQuestList(userId_, questList);
	}
	MV_SAFE_RELEASE(questList);
#pragma endregion

#pragma region 重置委托任务
	std::map<uint64_t, config_delegateQuestPool_t*>& mapDQuestPool = config.get_delegateQuestPool_config();
	std::map<uint64_t, config_delegateQuestBase_t*>& mapDQuestBase = config.get_delegateQuestBase_config();

	std::vector<strDelegateQuest> delegateQuests;
	if (m_data_cache.get_delegateQuest(userInfo.userId, delegateQuests))
	{
		std::vector<uint64_t> itemsVec, itemsExist;
		for (size_t j = 0; j < delegateQuests.size(); j++)
		{
			itemsExist.push_back(delegateQuests[j].taskId);
		}

		std::map<uint64_t, config_delegateQuestPool_t*>::iterator iterDQPool = mapDQuestPool.find(102);
		if (iterDQPool != mapDQuestPool.end())
		{
			for (size_t j = 0; j < delegateQuests.size(); j++)
			{
				if (delegateQuests[j].status == NotAccept)
				{
					std::vector<uint64_t> newDQuest;
					std::map<uint64_t, config_delegateQuestBase_t*>::iterator iterDQuestBase = mapDQuestBase.find(delegateQuests[j].taskId);
					if (iterDQuestBase == mapDQuestBase.end())
					{
						continue;
					}

					getVectorValueByRate(iterDQPool->second->m_QuestID, iterDQPool->second->m_Weight, itemsExist, 1, newDQuest);

					if (newDQuest.size() >= 1)
					{
						itemsExist.push_back(newDQuest[0]);
						itemsVec.push_back(newDQuest[0]);
					}
				}
			}

			//刷新宠物状态
			std::vector<boost::shared_ptr<strPetInfo>> vecPetInfo;
			m_data_cache.mget_petInfoByUserId(userInfo.userId, vecPetInfo);
			for (size_t j = 0; j < delegateQuests.size(); j++)
			{
				if (delegateQuests[j].status > Accepted)
				{
					for (size_t k = 0; k < vecPetInfo.size(); k++)
					{
						if (vecPetInfo[k]->DelegateQuest == delegateQuests[j].taskId)
						{
							vecPetInfo[k]->DelegateQuest = 0;
							m_data_cache.set_petInfo(vecPetInfo[k]->userId, vecPetInfo[k]->ID, vecPetInfo[k]->base, *vecPetInfo[k].get());
						}
					}
				}
			}
			MV_SAFE_RELEASE(vecPetInfo);
			char buf[1024] = { 0 };
			sprintf(buf, "update `gamePetInfo` set `DelegateQuest` = 0 where `userId` = %llu;", userInfo.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf, sql_update, DB_GamePetInfo);

			std::vector<strDelegateQuest> vecUserDelegate;
			for (size_t j = 0; j < delegateQuests.size(); j++)
			{
				if (delegateQuests[j].status >= Accepted)
				{
					vecUserDelegate.push_back(delegateQuests[j]);
				}
			}

			for (size_t j = 0; j < itemsVec.size(); j++)
			{
				strDelegateQuest strDelegate;
				strDelegate.startTime = 0;
				strDelegate.status = NotAccept;
				strDelegate.userId = userInfo.userId;
				strDelegate.taskId = itemsVec[j];

				vecUserDelegate.push_back(strDelegate);
			}

			userInfo.DQRefreshTime = time(NULL);
			userInfo.DQRefreshTimes = userInfo.DQRefreshTimes % 100;//!委托任务重置次数 刷新次数不刷新

			char buf_user[1024] = { 0 };
			sprintf(buf_user, "update `gameUserInfo` set `DQRefreshTimes` = %llu, `DQRefreshTime` = %llu where `userId` = %llu", userInfo.DQRefreshTimes, userInfo.DQRefreshTime, userInfo.userId);
			singleton_t<db_request_list>::instance().push_request_list(buf_user, sql_update, DB_GameUserInfo);

			std::vector<strDelegateEvent> vecDelegateEvent;
			m_data_cache.get_delegateEvent(userInfo.userId, vecDelegateEvent);
			m_db.updateUserDelegateQuestList(userInfo.userId, delegateQuests, vecDelegateEvent);
		}
	}

#pragma endregion

	userInfo.lastRefreshNewDayTime = time(NULL);

	char bufuu[1024] = { 0 };
	sprintf(bufuu, "update `gameUserInfo` set `lastRefreshNewDayTime` = %llu where `userId` = %llu;", userInfo.lastRefreshNewDayTime, userId_);
	singleton_t<db_request_list>::instance().push_request_list(bufuu, sql_update, DB_GameUserInfo);
	m_data_cache.set_userInfo(userInfo.userId, userInfo);
	return true;
}


bool football_db_analyze_t::addNewsData(const uint64_t userId, const ENewsID id, const uint64_t time, const std::string& params)
{
	strNewsInfo newsInfo;
	m_data_cache.get_newsInfo(userId, newsInfo);

	int newsLimit = min(1, config_inst.getGlobalIntValue("NewsLimit", 10));
	if (newsInfo.infos.size() >= newsLimit)
	{
		std::sort(newsInfo.infos.begin(), newsInfo.infos.end(), [](strNewsData a, strNewsData b){ return a.time < b.time; });
		newsInfo.infos.resize(newsLimit - 1);
	}

	strNewsData newsData;
	newsData.id = id;
	newsData.time = time;
	newsData.params = params;
	newsInfo.infos.push_back(newsData);
	m_data_cache.set_newsInfo(userId, newsInfo);
	return true;
}


#pragma endregion
