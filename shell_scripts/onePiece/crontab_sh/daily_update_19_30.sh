#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/db_user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`

#每天19点30清理数据

#清理魔族入侵数据
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set invade_inspiration = 0, invade_god_bless_flag = 0, invade_dead_complete_time = 0;"


