#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`

#每天20点清理数据


#多人竞技每日连胜纪录
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set multi_arena_today_continuous_win_count = 0, multi_arena_today_last_challenge_rlt = 0;"


#单人竞技排行宝箱领取
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set single_arena_today_reward_flag = 0;"



#单人竞技8点排行
mysql -u$db_user -p$db_password -h$db_host $db_name -e  "update t_account_info as a, t_arena_challenge_ranking as r set a.single_arena_yesterday_ranking = r.ranking where a.platform_gamesvr_id = r.platform_gamesvr_id;"