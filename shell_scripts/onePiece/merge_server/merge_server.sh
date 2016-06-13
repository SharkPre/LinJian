#!/bin/bash

db_user="root"
db_password="boyojoy.com"
db_host="127.0.0.1"

db_name_dest="db_yueyou_game_s12"
db_name_src="db_yueyou_game_shf"
randmin=1001
randmax=2000
itemidadd=200000000

#back up db

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_tmp_all_duplicate_name_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_tmp_duplicate_name_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_tmp_duplicate_guild_name_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_tmp_all_duplicate_guild_name_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_src_server_old_uid_info;"

#mysql -u"db_user_0" -p"yueyou123!" -h"127.0.0.1" db_yueyou_game_s2 < back_tables_s2.sql
#mysql -u"db_user_0" -p"yueyou123!" -h"127.0.0.1"  db_yueyou_game_s6 < back_tables_s6.sql

#保存源服老uid,英雄训练场用
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_src_server_old_uid_info ( id int(10) unsigned not null auto_increment comment '自增id', platform_gamesvr_id char(30) not null comment '平台号+服号可以唯一确定一个游戏账户', char_flag int(10) unsigned NOT NULL COMMENT '角色标签：0，英雄；具体编号，武将', uid int(10) unsigned NOT NULL COMMENT '作为游戏内玩家角色标识编号', new_uid int(10) unsigned NOT NULL COMMENT '新uid',primary key (id), KEY platform_id_key(platform_gamesvr_id, uid)) engine=innodb auto_increment=107029 default charset=utf8 comment='源服uid信息';"

mysql -u$db_user -p$db_password -h$db_host  -e "insert into $db_name_dest.t_src_server_old_uid_info(platform_gamesvr_id, char_flag, uid) select platform_gamesvr_id,char_flag,id from $db_name_src.t_character_info;"

#重名
mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_dest.t_character_info drop index char_name_uniq;"

#加索引 等下要连表查询
mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_dest.t_character_info add index name_index(char_name);"

#---------------------------------------------------------------------------- 
#一些表需要增加字段 
mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_dest.t_final_world_challenge_daily_info add column platform_gamesvr_id char(30) NOT NULL COMMENT '平台号+服号可以唯一确定一个游戏账户';"
mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_src.t_final_world_challenge_daily_info add column platform_gamesvr_id char(30) NOT NULL COMMENT '平台号+服号可以唯一确定一个游戏账户';"


#---------------------------------------------------------------------------- 
#合并表
#t_character_info
character_columns="platform_gamesvr_id,species,char_name,level,char_flag,exp,power,intelligence,agility,physique,phy_attack,phy_defense,magic_attack,magic_defanse,hp,crit,parry,speed,fighting,in_use,create_time,plant_id,culture_id,culture_exp,source,grade,antibroken,critdamage,learned_skills,skill_shortcuts"

mysql -u$db_user -p$db_password -h$db_host  -e "insert into $db_name_dest.t_character_info($character_columns) select $character_columns from $db_name_src.t_character_info where platform_gamesvr_id not like '%Theokole%';"

#t_account_info
account_columns="uid,platform_gamesvr_id,guild_platform_gamesvr_id,guild_id,char_name,vip_level,vip_exp,coin,money,cash,crystal,equipment_bag_item_count,prop_bag_item_count,task_bag_item_count,warehouse_item_count,form_in_use,skill_points,learned_skills,skill_shortcuts,learned_guild_skills,last_map_type,last_map_id,last_raid_id,map_cleared_elements,last_raid_info,last_raid_rewards,last_pos_x,last_pos_y,single_arena_ranking,single_arena_continuous_win_count_record,single_arena_last_challenge_rlt,single_arena_tmp_continuous_win_count,single_arena_today_continuous_win_count,single_arena_today_last_challenge_rlt,single_arena_today_left_challenge_count,single_arena_today_reward_flag,arena_badge_count,fighting,total_fighting,multi_arena_ranking,multi_arena_today_continuous_win_count,multi_arena_today_last_challenge_rlt,multi_arena_today_left_challenge_count,arena_honor_exp,arena_honor_level,multi_raid_today_left_count,last_login_time,last_logout_time,comm_rune_points,advanced_rune_points,orange_rune_points,cur_rune_items,comm_rune_free_count,advanced_rune_free_count,country,cur_star_items,cur_divination_id,star_sell_min_quality,star_score,star_product_free_count,star_divination_call_daily_count,money_tree_times,palace_badge_count,palace_challenge_top,palace_ranking,palace_card_used,palace_left_time_to_enter,palace_sweep_complete_time,palace_sweep_target_layer,palace_cur_sweeping_layer,palace_cur_obtained_exp,palace_cur_layer_cleared_elements,palace_start_time,palace_revive_count,palace_dead_flag,palace_top_time,achieve_points,title_id,auto_battle_flag,title_count,sign_in,sign_in_reward,active_score,active_reward,cur_convoy_quality,convoy_insurance_type,left_convoy_times,left_rob_convoy_times,total_practise_time,strength,single_raid_fight_state,single_raid_sweep_sell_min_quality,single_raid_sweep_complete_time,single_raid_sweep_target_count,single_raid_cur_sweeping_idx,single_raid_cur_obtained_exp,single_raid_war_key_used,single_raid_cur_obtained_silver,single_raid_sweep_raid_id,rob_convoy_cd_complete_time,invade_inspiration,invade_god_bless_flag,invade_total_harm,invade_dead_complete_time,vip_upgrade_reward,buy_arena_times,free_rivive_times,product_star_daily_count,get_common_rune_daily_count,get_advanced_rune_daily_count,buy_strength_daily_count,lock_flag,create_char_ip,last_offline_ip,garrison_inspiration,garrison_god_bless_flag,garrison_total_harm,garrison_dead_complete_time,single_arena_yesterday_ranking,inviter_uid,invite_count,invite_reward_flag,world_boss_inspiration,world_boss_total_harm,world_boss_dead_complete_time,single_raid_sweep_start_time,guild_battle_reward,guild_battle_dead_complete_time,gannicus_flag,thor_flag,gannicus_first_time_flag,thor_first_time_flag,strength_update_flag,star_rune,astro_raid_sweep_start_time,astro_raid_sweep_complete_time,astro_raid_sweep_target_count,astro_raid_cur_sweeping_idx,astro_raid_cur_obtained_star_rune,astro_raid_sweep_list,astro_raid_obtained_total_star_rune,crystal_recharge_time,world_challenge_arena_badge_count,fish_level,fish_exp,fish_score,fish_bullet,today_fish_score,world_challenge_worship_count,card_points,mystery_daily_left_times,mystery_unlock_chapter,mystery_cur_in_chapter,mystery_daily_already_buy_times,mystery_complete_time,mystery_cur_grid_index,mystery_left_steps,mystery_left_hp,mystery_left_change_times,mystery_left_skip_times,mystery_score,mystery_reward_count,mystery_reward_info,card_score,today_card_score,feed_warcraft_infos,warcraft_skills,warcraft_protective_info"

mysql -u$db_user -p$db_password -h$db_host  -e "insert into $db_name_dest.t_account_info($account_columns) select $account_columns from $db_name_src.t_account_info where platform_gamesvr_id not like '%Theokole%';"

#一些背包的item_id要增加避免合过来之后重复
mysql -u$db_user -p$db_password -h$db_host $db_name_src -e "update t_dress_info set item_id=item_id+$itemidadd;"
mysql -u$db_user -p$db_password -h$db_host $db_name_src -e "update t_horse_dress_info set item_id=item_id+$itemidadd;"
mysql -u$db_user -p$db_password -h$db_host $db_name_src -e "update t_star_dress_info set item_id=item_id+$itemidadd;"
mysql -u$db_user -p$db_password -h$db_host $db_name_src -e "update t_warehouse_info set item_id=item_id+$itemidadd;"

#合并各表
while read table_name columns
do
	if [ -n "$table_name" ] ; then
		#echo "table_name:	"$table_name" columns:	"$columns
		echo "table_name:	"$table_name
		if [ "$table_name" == t_buff_info ];then
			mysql -u$db_user -p$db_password -h$db_host  -e "delete from $db_name_src.$table_name where platform_gamesvr_id='';"
			mysql -u$db_user -p$db_password -h$db_host  -e "delete from $db_name_dest.$table_name where platform_gamesvr_id='';"
		fi

		$mysql -u$db_user -p$db_password -h$db_host  -e "delete from $db_name_dest.$table_name where platform_gamesvr_id like '%Theokole%';"
		mysql -u$db_user -p$db_password -h$db_host  -e "delete from $db_name_src.$table_name where platform_gamesvr_id like '%Theokole%';"
		mysql -u$db_user -p$db_password -h$db_host  -e "insert into $db_name_dest.$table_name($columns) select $columns from $db_name_src.$table_name;"
	fi
done < table_info

#---------------------------------------------------------------------------- 
#角色重名表
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_tmp_duplicate_name_info ( id int(10) unsigned not null auto_increment comment '自增id', platform_gamesvr_id char(30) not null comment '平台号+服号可以唯一确定一个游戏账户', char_name varchar(30) not null comment '角色名字，单服唯一；合服之后也要唯一', primary key (id), KEY name_index (char_name)) engine=innodb auto_increment=107029 default charset=utf8 comment='重名角色信息';"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_tmp_all_duplicate_name_info ( id int(10) unsigned not null auto_increment comment '自增id', platform_gamesvr_id char(30) not null comment '平台号+服号可以唯一确定一个游戏账户', char_name varchar(30) not null comment '角色名字，单服唯一；合服之后也要唯一', new_name varchar(30) not null comment '新名字', primary key (id), unique key platform_gamesvr_key(platform_gamesvr_id)) engine=innodb auto_increment=107029 default charset=utf8 comment='重名角色信息';"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "insert into t_tmp_duplicate_name_info (platform_gamesvr_id, char_name) select platform_gamesvr_id, char_name from t_character_info where char_flag=0 group by char_name having count(char_name)>1;"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "insert into t_tmp_all_duplicate_name_info (platform_gamesvr_id, char_name) select c.platform_gamesvr_id,c.char_name from t_character_info c, t_tmp_duplicate_name_info t where c.char_name=t.char_name and c.char_flag=0;"

#改名字
#concat("hello", floor(rand()*10000));
#m到n之间的随机数 floor(m+rand()*(n-m+1) );
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_tmp_all_duplicate_name_info set new_name=concat(char_name, floor($randmin+rand()*($randmax-$randmin+1)));"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_character_info a, t_tmp_all_duplicate_name_info t set a.char_name=t.new_name where a.platform_gamesvr_id=t.platform_gamesvr_id and a.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_account_info a, t_character_info t set a.char_name=t.char_name where a.platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_global_beat_boss_info a, t_character_info t set a.char_name=t.char_name where a.platform_gamesvr_id=t.platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_member a, t_character_info t set a.member_name=t.char_name where a.member_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_news a, t_character_info t set a.operator_name=t.char_name where a.operator_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_news a, t_character_info t set a.character_name=t.char_name where a.character_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_today_contribution a, t_character_info t set a.contributor_name=t.char_name where a.contributor_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_vote a, t_character_info t set a.candidate_name=t.char_name where a.candidate_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_arena_challenge_record a, t_character_info t set a.challenger_name=t.char_name where a.challenger_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_arena_challenge_record a, t_character_info t set a.challenged_name=t.char_name where a.challenged_platform_gamesvr_id=t.platform_gamesvr_id and t.char_flag=0;"

#恢复唯一索引
mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_dest.t_character_info drop index name_index;"

mysql -u$db_user -p$db_password -h$db_host  -e "alter table $db_name_dest.t_character_info add unique char_name_uniq(char_name);"

#---------------------------------------------------------------------------- 
#update uid
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_account_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_astro_raid_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_beat_boss_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update  t_buff_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_chest_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_global_beat_boss_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_raid_area_score a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_time_buff a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_title_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_info a, t_character_info c set a.chairman_id=c.id where a.chairman_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_member a, t_character_info c set a.member_id=c.id where a.member_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_news a, t_character_info c set a.operator_id=c.id where a.operator_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_news a, t_character_info c set a.character_id=c.id where a.character_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_vote a, t_character_info c set a.candidate_id=c.id where a.candidate_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_arena_challenge_record a, t_character_info c set a.challenger_uid=c.id where a.challenger_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_arena_challenge_record a, t_character_info c set a.challenged_uid=c.id where a.challenged_platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_final_world_challenge_daily_info a, t_character_info c set a.uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and c.char_flag=0;"

#更新英雄训练场里的训练中的英雄id
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_src_server_old_uid_info a, t_character_info c set a.new_uid=c.id where a.platform_gamesvr_id=c.platform_gamesvr_id and a.char_flag=c.char_flag;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_1=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_1=c.uid;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_2=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_2=c.uid;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_3=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_3=c.uid;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_4=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_4=c.uid;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_5=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_5=c.uid;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_home_info a, t_src_server_old_uid_info c set a.training_center_pos_flag_6=c.new_uid where a.platform_gamesvr_id=c.platform_gamesvr_id and a.training_center_pos_flag_6=c.uid;"

#---------------------------------------------------------------------------- 
#公会重名表
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_tmp_duplicate_guild_name_info ( id int(10) unsigned not null auto_increment comment '自增id', guild_platform_gamesvr_id char(30) not null comment '公会标识, 平台号+服号可以唯一确定一个公会', guild_name varchar(30) not null comment '公会名字，单服唯一；合服之后也要唯一', primary key (id), KEY name_index (guild_name)) engine=innodb auto_increment=107029 default charset=utf8 comment='重名公会信息';"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_tmp_all_duplicate_guild_name_info ( id int(10) unsigned not null auto_increment comment '自增id', guild_platform_gamesvr_id char(30) not null comment '公会标识, 平台号+服号可以唯一确定一个公会', guild_name varchar(30) not null comment '公会名字，单服唯一；合服之后也要唯一', new_guild_name varchar(30) not null comment '新名字', primary key (id)) engine=innodb auto_increment=107029 default charset=utf8 comment='重名公会信息';"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "insert into t_tmp_duplicate_guild_name_info (guild_platform_gamesvr_id, guild_name) select guild_platform_gamesvr_id, guild_name from t_guild_info group by guild_name having count(guild_name)>1;"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "insert into t_tmp_all_duplicate_guild_name_info (guild_platform_gamesvr_id, guild_name) select c.guild_platform_gamesvr_id,c.guild_name from t_guild_info c, t_tmp_duplicate_guild_name_info t where c.guild_name=t.guild_name;"

#改公会名
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_tmp_all_duplicate_guild_name_info set new_guild_name=concat(guild_name, floor($randmin+rand()*($randmax-$randmin+1)));"

mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_info a, t_tmp_all_duplicate_guild_name_info t set a.guild_name=t.new_guild_name where a.guild_platform_gamesvr_id=t.guild_platform_gamesvr_id;"

#---------------------------------------------------------------------------- 
#update guild id
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_member a, t_guild_info c set a.guild_id=c.guild_id where a.guild_platform_gamesvr_id=c.guild_platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_news a, t_guild_info c set a.guild_id=c.guild_id where a.guild_platform_gamesvr_id=c.guild_platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_today_contribution a, t_guild_info c set a.guild_id=c.guild_id where a.guild_platform_gamesvr_id=c.guild_platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_guild_vote a, t_guild_info c set a.guild_id=c.guild_id where a.guild_platform_gamesvr_id=c.guild_platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_account_info a, t_guild_info c set a.guild_id=c.guild_id where a.guild_platform_gamesvr_id=c.guild_platform_gamesvr_id;"


mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_arena_challenge_ranking;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "update t_account_info set single_arena_ranking = 1001; SET @counter=0; insert into t_arena_challenge_ranking(ranking, uid, species, level, char_name, platform_gamesvr_id, fighting) select @counter:=@counter+1 as rank, c.id, c.species, c.level, c.char_name, a.platform_gamesvr_id, a.fighting from t_character_info as c, ( select platform_gamesvr_id, fighting from t_account_info ) as a where c.platform_gamesvr_id = a.platform_gamesvr_id and c.char_flag = 0 and c.platform_gamesvr_id like 'Theokole%' order by a.fighting desc; update t_account_info as a , t_arena_challenge_ranking as r set a.single_arena_ranking = r.ranking where a.platform_gamesvr_id = r.platform_gamesvr_id;"
#---------------------------------------------------------------------------- 
#删除一些信息
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_battle_applied_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_battle_champion_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_battle_pair_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_battle_player_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_battle_applied_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guild_application;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_world_challenge_daily_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_world_challenge_targets;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_guard_soldier_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_worldcup_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_zhanshen_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_yesterday_card_score_ranking;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_yesterday_fish_score_ranking;"
#清除公会战翅膀 
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_dress_info where goods_id=100577;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_equipment_bag_info where goods_id=100577;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_warehouse_info where goods_id=100577;"
#删除 一些账号
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete from t_tmp_all_delete_name_info;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "create table t_tmp_all_delete_name_info ( id int(10) unsigned not null auto_increment comment '自增id', platform_gamesvr_id char(30) not null comment '平台号+服号可以唯一确定一个游戏账户', primary key (id), KEY platform_id_key(platform_gamesvr_id)) engine=innodb auto_increment=107029 default charset=utf8 comment='源服uid信息';"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "insert into t_tmp_all_delete_name_info(platform_gamesvr_id) select t.platform_gamesvr_id from t_account_info t, t_character_info a where t.last_logout_time < unix_timestamp(now()) - 30 * 86400 and t.platform_gamesvr_id = a.platform_gamesvr_id and a.char_flag = 0 and t.platform_gamesvr_id not like '%Theokole%' and a.level < 20;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete t  from t_tmp_all_delete_name_info t, t_charge_info a where t.platform_gamesvr_id = a.platform_gamesvr_id;"
#合并各表
while read table_name columns
do
        if [ -n "$table_name" ] ; then
		echo "table_name:       "$table_name
                mysql -u$db_user -p$db_password -h$db_host $db_name_dest  -e "delete t  from $table_name t, t_tmp_all_delete_name_info a  where t.platform_gamesvr_id = a.platform_gamesvr_id"
        fi
done < table_info
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete t  from t_account_info t, t_tmp_all_delete_name_info a  where t.platform_gamesvr_id = a.platform_gamesvr_id;"
mysql -u$db_user -p$db_password -h$db_host $db_name_dest -e "delete t  from t_character_info t, t_tmp_all_delete_name_info a  where t.platform_gamesvr_id = a.platform_gamesvr_id;"


