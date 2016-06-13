#/bin/bash
fb_max_svr_count=30
kg_max_svr_count=2
cy_max_svr_count=7

#function get_goods_count(){
#host=$1
#good_id=$2
#ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"select count(*) from t_horse_bag_info where goods_id = $good_id;select count(*) from t_horse_bag_info where goods_id = $good_id;\"' $host"
#}

function get_goods_count(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/get_card_chest.sh' $host"
}

function get_user_vip_pack(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select platform_gamesvr_id as account_id, event_id as vip_pack_level from t_user_event_info where event_type=400058;select platform_gamesvr_id as account_id, event_id as day_type from t_user_event_info where event_type=400061;\"' $host"
}

function get_user_investment_type(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select platform_gamesvr_id as account_id, event_id as investment_type from t_user_event_info where event_type=400054;\"' $host"
}


function clear_star_times(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"update t_task_limit_info set star_task_times=0;\"' $host"
}

function get_horse_dress_user_count(){
host=$1
good_id=$2
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"select platform_gamesvr_id from t_horse_bag_info where goods_id = $good_id;select platform_gamesvr_id from t_horse_dress_info where goods_id = $good_id;\"' $host"
}

function get_top_total_fighting(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"select a.platform_gamesvr_id, a.vip_level, a.total_fighting, c.level, from_unixtime(a.last_login_time), from_unixtime(a.last_logout_time) from t_account_info as a, t_character_info as c where a.uid = c.id order by a.total_fighting desc limit 10;\"' $host"
}

function clear_user_event(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"delimiter | create procedure test_cursor() begin DECLARE   plat char(30); DECLARE  e_type, e_id, no_more_departments INT; DECLARE   tmp_cur cursor for (select uu.platform_gamesvr_id,uu.event_type,uu.event_id from t_user_event_info as uu, (select * from t_user_event_info as buy_event, (select new_t.new_plat_id,new_t.new_type,new_t.new_id ,new_t.new_count from (select platform_gamesvr_id as new_plat_id, event_type as new_type, event_id as new_id , count as new_count from t_user_event_info where event_type = 400068) as new_t ) as new_t_1 where buy_event.platform_gamesvr_id = new_t_1.new_plat_id and buy_event.event_type = 400067 and buy_event.event_id = new_t_1.new_id and  new_t_1.new_count=7) tmp_new where (uu.event_type=tmp_new.event_type and uu.platform_gamesvr_id =tmp_new.platform_gamesvr_id and uu.event_id=tmp_new.event_id) or (uu.event_type=tmp_new.new_type and uu.platform_gamesvr_id =tmp_new.new_plat_id and uu.event_id=tmp_new.new_id)); DECLARE CONTINUE HANDLER FOR NOT FOUND SET no_more_departments=1;  SET no_more_departments=0;  open   tmp_cur; REPEAT fetch tmp_cur   into  plat, e_type, e_id; select plat, e_type, e_id; delete from t_user_event_info where platform_gamesvr_id = plat and event_type = e_type and event_id = e_id; UNTIL no_more_departments END REPEAT; close tmp_cur; end;| delimiter ; call test_cursor(); drop  procedure test_cursor;\"' $host"
}


function clear_user_event1(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"delimiter | create procedure test_cursor() begin DECLARE   plat char(30); DECLARE  e_type, e_id, no_more_departments INT; DECLARE   tmp_cur cursor for (select uu.platform_gamesvr_id , uu.event_type, uu.event_id from t_user_event_info as uu, (select u.platform_gamesvr_id , u.event_type, u.event_id,  tmp_new.t_platform, tmp_new.t_event_type, tmp_new.t_count from t_user_event_info as u, (select tmp.t_platform, tmp.t_event_type, tmp.t_count from (select platform_gamesvr_id as t_platform, event_type as t_event_type, count(*) as t_count from t_user_event_info where event_type >= 400055 and  event_type <=400057 group by platform_gamesvr_id,event_type) tmp ) as tmp_new where u.platform_gamesvr_id = tmp_new.t_platform and u.event_type=400054 and ((u.event_id = 1 and tmp_new.t_event_type = 400055 and tmp_new.t_count = 7) or (u.event_id = 2 and tmp_new.t_event_type = 400056 and tmp_new.t_count = 10) or (u.event_id = 3 and tmp_new.t_event_type = 400057 and tmp_new.t_count = 14))) tmp_new_1 where (uu.platform_gamesvr_id = tmp_new_1.platform_gamesvr_id and uu.event_type = tmp_new_1.event_type and uu.event_id = tmp_new_1.event_id) or (uu.platform_gamesvr_id = tmp_new_1.t_platform and uu.event_type = tmp_new_1.t_event_type) ); DECLARE CONTINUE HANDLER FOR NOT FOUND SET no_more_departments=1;  SET no_more_departments=0;  open   tmp_cur; REPEAT fetch tmp_cur   into  plat, e_type, e_id; select plat, e_type, e_id; delete from t_user_event_info where platform_gamesvr_id = plat and event_type = e_type and event_id = e_id; UNTIL no_more_departments END REPEAT; close tmp_cur; end;| delimiter ; call test_cursor(); drop  procedure test_cursor;\"' $host"
}


function get_player_level(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select c.platform_gamesvr_id, c.level as player_level, h.level as horse_level from t_character_info c, t_horse_info h where c.platform_gamesvr_id = h.platform_gamesvr_id and c.char_flag=0;\"' $host"
}



function delete_task(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" delete from t_task_info where task_id=613210;;\"' $host"
}

function lock_user(){
host=$1
svr_idx=$2
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" update t_account_info set lock_flag = 1 where platform_gamesvr_id='83929_s$svr_idx';\"' $host"
}


function change_horse_attr(){
host=$1
svr_idx=$2
ssh  -p10022 $host "su -  -c 'php ~/server_issue/server_api/timer_bash/change_horse_attr.php' $host"
}

function change_old_account_from_new_guide(){
host=$1
svr_idx=$2
ssh  -p10022 $host "su -  -c 'cd ~/server_issue/server_api/timer_bash/;php change_old_account_from_new_guide.php' $host"
}

function get_user_levels(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select level, count(level) from t_character_info where char_flag = 0 group by level;\"' $host"
}

function get_avg_level(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select * from t_global_server_info;\"' $host"
}

function clear_21_newbie_task(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"delete from t_task_info where task_id>=613221;\"' $host"
}

function get_trap_buff(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" select platform_gamesvr_id, in_using_buffs from t_buff_info where in_using_buffs like \'%,8;%\'\"' $host"
}

function get_guild_money(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"select guild_level, guild_money from t_guild_info where guild_level >= 4 order by guild_level desc;\"' $host"
}


#清理ams活动数据
function delete_user_event(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" delete from t_user_event_info where event_type in (401012,402016,401013,402017,100052,100053,401015,402025);\"' $host"
}

#清卡牌积分
function clear_card_score(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" update t_account_info set card_score=0;\"' $host"
}

#清卡牌兑换
function delete_card_exchange(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" delete from t_user_event_info where event_type = 400079 ;\"' $host"
}

#投资计划
function delete_touzi_exchange(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \" delete from t_user_event_info where event_type = 400087 ;\"' $host"
}

function get_charge_info(){
host=$1
ssh  -p10022 $host "su -  -c 'bash ~/server_issue/ssh_mysql.sh \"select * from t_charge_info where charge_time>=date_sub(now(),interval 4 day);\"' $host"
}

goods_id=132003

###########################################################
#fb
platform="cr.bbgpoint.com"
svr_idx="s1"
#get_goods_count $svr_idx"-"$platform $goods_id
#get_user_vip_pack $svr_idx"-"$platform
#clear_star_times $svr_idx"-"$platform
#get_horse_dress_user_count $svr_idx"-"$platform $goods_id
#get_top_total_fighting $svr_idx"-"$platform
#get_user_investment_type $svr_idx"-"$platform
#clear_user_event $svr_idx"-"$platform
#clear_user_event1 $svr_idx"-"$platform
#get_player_level $svr_idx"-"$platform
#delete_task $svr_idx"-"$platform
#get_charge_info $svr_idx"-"$platform
#lock_user $svr_idx"-"$platform 1
#change_horse_attr $svr_idx"-"$platform 
#get_avg_level $svr_idx"-"$platform
#change_old_account_from_new_guide $svr_idx"-"$platform
#clear_21_newbie_task $svr_idx"-"$platform
#get_trap_buff $svr_idx"-"$platform
#get_guild_money $svr_idx"-"$platform
delete_user_event $svr_idx"-"$platform
#clear_card_score $svr_idx"-"$platform
#delete_card_exchange $svr_idx"-"$platform
delete_touzi_exchange $svr_idx"-"$platform

#s2+
for ((a=30; a <= $fb_max_svr_count ; a++)) # 双圆括号, 并且"LIMIT"变量前边没有 "$".
do
svr_idx="s$a"
#s_pwd="z10MnPByWlpn8U86"
#get_goods_count $svr_idx"-"$platform $goods_id
#get_user_vip_pack $svr_idx"-"$platform
#clear_star_times $svr_idx"-"$platform
#get_horse_dress_user_count $svr_idx"-"$platform $goods_id
#get_top_total_fighting $svr_idx"-"$platform
#get_user_investment_type $svr_idx"-"$platform
#clear_user_event $svr_idx"-"$platform
#clear_user_event1 $svr_idx"-"$platform
#get_player_level $svr_idx"-"$platform
#delete_task $svr_idx"-"$platform
#get_charge_info $svr_idx"-"$platform
#lock_user $svr_idx"-"$platform $a
#change_horse_attr $svr_idx"-"$platform
#get_avg_level $svr_idx"-"$platform
#change_old_account_from_new_guide $svr_idx"-"$platform
#clear_21_newbie_task $svr_idx"-"$platform
#get_trap_buff $svr_idx"-"$platform
#get_guild_money $svr_idx"-"$platform
delete_user_event $svr_idx"-"$platform
#clear_card_score $svr_idx"-"$platform
#delete_card_exchange $svr_idx"-"$platform
delete_touzi_exchange $svr_idx"-"$platform
done

###########################################################
#kg
platform="cr.bbgpoint.com"
for ((a=3; a <= kg_max_svr_count ; a++)) # 双圆括号, 并且"LIMIT"变量前边没有 "$".
do
svr_idx="s$a""kg"
#get_goods_count $svr_idx"-"$platform $goods_id
#get_user_vip_pack $svr_idx"-"$platform
#clear_star_times $svr_idx"-"$platform
#get_horse_dress_user_count $svr_idx"-"$platform $goods_id
#get_top_total_fighting $svr_idx"-"$platform
#get_user_investment_tmp $svr_idx"-"$platform
#clear_user_event $svr_idx"-"$platform
#clear_user_event1 $svr_idx"-"$platform
#get_player_level $svr_idx"-"$platform
#delete_task $svr_idx"-"$platform
#get_charge_info $svr_idx"-"$platform
#lock_user $svr_idx"-"$platform $a
#change_horse_attr $svr_idx"-"$platform
#get_avg_level $svr_idx"-"$platform
#change_old_account_from_new_guide $svr_idx"-"$platform
#clear_21_newbie_task $svr_idx"-"$platform
#get_trap_buff $svr_idx"-"$platform
#get_guild_money $svr_idx"-"$platform
delete_user_event $svr_idx"-"$platform
#clear_card_score $svr_idx"-"$platform
#delete_card_exchange $svr_idx"-"$platform
delete_touzi_exchange $svr_idx"-"$platform
done

###########################################################
#cy
platform=".gamebox.com"
for ((a=4; a <= $cy_max_svr_count ; a++)) # 双圆括号, 并且"LIMIT"变量前边没有 "$".
do
svr_idx="crserver$a"
#get_goods_count $svr_idx$platform $goods_id
#get_user_vip_pack $svr_idx$platform
#clear_star_times $svr_idx$platform
#get_horse_dress_user_count $svr_idx$platform $goods_id
#get_top_total_fighting $svr_idx$platform
#get_user_investment_tmp $svr_idx$platform
#clear_user_event $svr_idx$platform
#clear_user_event1 $svr_idx$platform
#get_player_level $svr_idx""$platform
#delete_task $svr_idx""$platform
#get_charge_info $svr_idx""$platform
#lock_user $svr_idx""$platform $a
#change_horse_attr $svr_idx""$platform
#get_avg_level $svr_idx""$platform
#change_old_account_from_new_guide $svr_idx""$platform
#clear_21_newbie_task $svr_idx""$platform
#get_trap_buff $svr_idx""$platform
#get_guild_money $svr_idx""$platform
delete_user_event $svr_idx""$platform
#clear_card_score $svr_idx""$platform
#delete_card_exchange $svr_idx""$platform
delete_touzi_exchange $svr_idx"-"$platform
done



