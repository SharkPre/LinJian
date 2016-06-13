#!/bin/bash

db_user="william"
db_password="william"
db_host="192.168.2.113"

db_name_dest="db_yueyou_game_s2"
db_name_src="db_yueyou_game_s6"


mysqldump -u$db_user  -p$db_password -h$db_host  $db_name_dest t_character_info t_account_info > back_tables_s2.sql
mysqldump -u$db_user  -p$db_password -h$db_host  $db_name_src t_character_info t_account_info > back_tables_s6.sql
