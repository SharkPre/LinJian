rm -f cr.zip
#zip -r -9 cr.zip server_issue/server_api/global_server/bin/global_svr_1 
#zip -r -9 cr.zip server_issue/server_api/global_server/bin/libglobal.so 
#zip -r -9 cr.zip server_issue/server_api/DBSvr/bin/db_svr_1 
#zip -r -9 cr.zip server_issue/server_api/DBSvr/bin/libdb.so 
#zip -r -9 cr.zip server_issue/server_api/battle_server/bin/battle_svr_1 
#zip -r -9 cr.zip server_issue/server_api/battle_server/bin/libyy.so 
dos2unix  server_issue/server_api/timer_bash/*
#zip -r -9 cr.zip server_issue/server_api/timer_bash/* 
#zip -r -9 cr.zip server_issue/server_api/php_stat/php_stat_dailyStat.php
#zip -r -9 cr.zip server_issue/server_api/php_stat/php_stat_dailyStat_detail.php
zip -r -9 cr.zip root/api/php/yueyo_db1_proto.php
zip -r -9 cr.zip root/api_login.php
zip -r -9 cr.zip root/gm/*
#zip -r -9 cr.zip root/gm/api_generate_ams_activity_conf.php
#zip -r -9 cr.zip root/ams_activity.xml
cp cr.zip server_version/
cd server_version/
#cp ~/server_issue/server_api/DBSvr/etc/db_update.sql .
cp ../xml.zip .
echo ""
echo ""
echo ""
echo ""
echo "-------->  remember to send to source"
echo ""
echo ""
echo ""
echo ""
