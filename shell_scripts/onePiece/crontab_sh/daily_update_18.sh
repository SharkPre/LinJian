#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/db_user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`

#每天18点清理数据


#每天体力值补充20点（上限100点）
#mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set strength = 100 where strength + 20 > 100;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set strength = strength + 20 where strength + 20 <= 100;"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "update t_account_info set strength_update_flag = 0;"


