#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`

#每月24点清理数据

#清理每月签到 每月签到领奖
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set sign_in=0, sign_in_reward=0;"
