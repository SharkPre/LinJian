#!/bin/bash
FILE=~/server_issue/server_api/php_stat/php_stat_common_define.php
db_host=`awk -F \" '/host/ {print $2;}' $FILE`
db_user=`awk -F \" '/db_user/ {print $2;}' $FILE`
db_password=`awk -F \" '/password/ {print $2;}' $FILE`
db_name=`awk -F \" '/db_name/ {print $2;}' $FILE`


FILE=server_issue/server_api/battle_server/etc/bench.conf
svr_idx=`awk '/server_idx/ {print $2;}' $FILE`

end_time=`date +%Y-%m-01`
start_time=`date -d last-month +%Y-%m-01`

rm -f ~/$svr_idx""_charge.txt

mysql -u$db_user -p$db_password -h$db_host $db_name -e "select '$svr_idx', user_name, charge_money, charge_time, order_id from t_charge_info where charge_time > '$start_time' and charge_time < '$end_time';" > ~/$svr_idx""_charge.txt

sed -i '1d' ~/$svr_idx""_charge.txt
