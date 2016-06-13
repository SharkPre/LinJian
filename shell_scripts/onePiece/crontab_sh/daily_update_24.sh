#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/db_user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`


#每天24点清理数据

#清理副本攻打购买次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
	    "delete from t_raid_stage_info ;"

#每天体力值上限（上限100点）
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set strength = 100;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set strength_update_flag = 0;"

#跨服竞技每日膜拜次数上限, 3
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set world_challenge_worship_count = 3;"


#每天星魂殿次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_astro_raid_info set today_left_enter_count = 1, today_left_buy_count = 2;"


#清理公会成员每日贡献
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_guild_member set member_today_contribution = 0;"


#清理公会水晶祭坛开启次数, 1
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_guild_info set crystal_altar_today_flag = 0 where crystal_altar_recharge_complete_time = 0;"

#清理公会水晶祭坛充能和购买增益
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_guild_member set crystal_altar_buy_plus_flag = 0, crystal_altar_recharge_flag = 0;"

#清理玩家每天征收次数,5
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_home_info set government_left_collect_count = 5;"


#单人竞技每日次数上限, 15
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set single_arena_today_left_challenge_count = 15;"


#多人竞技每日次数上限, 10
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set multi_arena_today_left_challenge_count = 10;"


#单人竞技每日连胜纪录
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set single_arena_today_continuous_win_count = 0, single_arena_today_last_challenge_rlt = 0, single_arena_tmp_continuous_win_count = 0;"


#每天免费占卜次数,1
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set star_product_free_count = 2;"


#每天召唤弗利嘉次数,5
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set star_divination_call_daily_count = 0;"


#多人副本每日进入次数,3
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set multi_raid_today_left_count = 3;"


#试炼塔每日重置（进入一次，免费复活两次）
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set palace_card_used = 0, palace_left_time_to_enter = 1, palace_sweep_complete_time = 0, palace_sweep_target_layer = 0, palace_cur_sweeping_layer = 0, palace_cur_obtained_exp = 0,palace_cur_layer_cleared_elements='',  palace_revive_count = 2, palace_dead_flag = 0 where palace_start_time = 0;"


#悬赏任务当天完成次数,悬赏任务当天免费刷新次数,公会任务当天完成次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_task_limit_info set reward_task_times = 0,  reward_task_free_fresh_times = 0,  guild_task_times = 0, star_task_times=0;"					


#公会商店今日剩余兑换次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_guild_member set guild_shop_today_left_exchange_count = 5;"			


#当天坐骑培养次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_horse_info set culture_times=0; "									


#今日玩摇钱树次数,每日活跃度,活跃度奖励信息
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set  money_tree_times=0, active_score=0, active_reward=0;"


#每日事件 
#mysql -u$db_user -p$db_password -h$db_host $db_name -e \
#	    "delete from t_user_event_info where event_type between 100000 and 199999;"

#每周事件
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
	    "delete from t_user_event_info where time < date_sub(now(),interval 7 day) and event_type between 200000 and 299999;"

#每月事件
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
	    "delete from t_user_event_info where time < date_sub(now(),interval 30 day) and event_type between 300000 and 399999;"

#每日护送军资次数 每日打劫军资车次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set left_convoy_times=3, left_rob_convoy_times=5;"

#每日购买单人竞技次数 vip复活次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set buy_arena_times=0, free_rivive_times=0;"

#每天获取的普通酒魂数 每天获取的高级酒魂数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set get_common_rune_daily_count=0, get_advanced_rune_daily_count=0;"

#每天占卜数量 每天购买体力药水次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set product_star_daily_count=0, buy_strength_daily_count=0;"


#每天挂机总时长
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set total_practise_time=0;"


#已读邮件保存一周
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
		  "delete from t_mail_info  where state = 1 and time < date_sub(now(),interval 7 day);"

#未读邮件保存30天
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
		  "delete from t_mail_info  where state = 0 and time < date_sub(now(),interval 30 day);"		  
		
#设置需要选举的公会
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
        "update t_guild_info set vote_flag = 1 where guild_id in (select guild_id from t_guild_member where  member_title = 3 and quit_flag = 0 and member_last_logout_time != 0 and member_last_logout_time < unix_timestamp(now()) - 15 * 86400);"

#生成候选人
mysql -u$db_user -p$db_password -h$db_host $db_name -e \
          "insert into t_guild_vote(guild_platform_gamesvr_id,guild_id,candidate_platform_gamesvr_id,candidate_name, candidate_title, candidate_id,candidate_contribution, candidate_level) select guild_platform_gamesvr_id,guild_id,member_platform_gamesvr_id,member_name,member_title, member_id,member_total_contribution, member_level from t_guild_member where guild_id in (select guild_id from t_guild_info where  vote_flag= 1) and member_title != 3 and quit_flag = 0 and (member_last_logout_time = 0 or member_last_logout_time > unix_timestamp(now())  - 15 * 86400) order by member_title desc, member_total_contribution desc, member_level desc;"		
		
		
#每天免费酒魂次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set comm_rune_free_count=1, advanced_rune_free_count = 1;"

#生成服务器活跃玩家平均等级
mysql -u$db_user -p$db_password -h$db_host $db_name -e "insert into t_global_server_info(avg_level) select avg(level) - 10 from (select level from t_character_info where char_flag = 0 and id > 10000 and id in (select uid from t_account_info where last_logout_time > unix_timestamp(now()) - 15 * 86400) order by level desc limit 10) tmp;"

#当前抽奖次数和免费刷新次数
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_chest_info set today_count = 0, free_refresh_count_left = 5;"

#删除上一次公会战报名信息
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_guild_battle_applied_info where weekday(now()) = 0 or weekday(now()) = 2 or weekday(now()) = 4;"

#删除上一次公会战配对信息
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_guild_battle_pair_info where weekday(now()) = 0 or weekday(now()) = 2 or weekday(now()) = 4;"

#删除上一次公会战得分信息
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_guild_battle_player_info where weekday(now()) = 0 or weekday(now()) = 2 or weekday(now()) = 4;"

#公会战冠军公会每日奖励
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set guild_battle_reward = 0, guild_battle_dead_complete_time = 0;"

#主城充能
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set crystal_recharge_time = 0;"

#捕鱼
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_yesterday_fish_score_ranking;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "insert into t_yesterday_fish_score_ranking(uid, char_name, score) select uid,char_name,today_fish_score from t_account_info  where today_fish_score>=300 order by today_fish_score desc limit 100;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set today_fish_score = 0;"

#秘境
mysql -u$db_user -p$db_password -h$db_host  $db_name -e "update t_account_info set mystery_daily_left_times= 2 where mystery_daily_left_times < 2;"
mysql -u$db_user -p$db_password -h$db_host  $db_name -e "update t_account_info set mystery_daily_already_buy_times= 0;"

#卡牌
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_yesterday_card_score_ranking;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "insert into t_yesterday_card_score_ranking(uid, char_name, score) select uid,char_name,today_card_score from t_account_info  where today_card_score>=100 order by today_card_score desc limit 100;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set today_card_score = 0;"


#公会每日充值
#mysql -u$db_user -p$db_password -h$db_host  $db_name -e "update t_guild_info set guild_daily_recharge_gold = 0;"

#魔兽扭蛋机每日免费次数
#mysql -u$db_user -p$db_password -h$db_host  $db_name -e "update t_gashapon_info set today_times = 0, refining_warcraft_crystal_times = 0;"

#今日魔兽喂养次数
#mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set feed_warcraft_infos=0;"

#商店每日限购
#mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_shop_limit_buy;"



