db_user="root"
db_password="boyojoy.com"
db_host="127.0.0.1"

db_name="db_world_guild_battle_info"
delete_servername="BBGPoint-s1"

mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_guild_landmark_base_info where master_platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete t from t_world_guild_landmark_battle_seq_hero_info t, t_world_guild_landmark_battle_info a where a.master_platform_name = '$delete_servername' and t.landmark_id = a.landmark_id"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete t from t_world_guild_landmark_battle_seq_hero_info t, t_world_guild_landmark_battle_info a where a.challenger_platform_name = '$delete_servername' and t.landmark_id = a.landmark_id"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete t from t_world_guild_landmark_battle_seq_info t, t_world_guild_landmark_battle_info a where a.master_platform_name = '$delete_servername' and t.landmark_id = a.landmark_id"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete t from t_world_guild_landmark_battle_seq_info t, t_world_guild_landmark_battle_info a where a.challenger_platform_name = '$delete_servername' and t.landmark_id = a.landmark_id"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_guild_landmark_battle_info where master_platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_guild_landmark_battle_info where challenger_platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_guild_landmark_bid_info where bid_platform_name = '$delete_servername';"

