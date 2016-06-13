db_user="root"
db_password="boyojoy.com"
db_host="127.0.0.1"

db_name="db_world_challenge_ranking"
delete_servername="BBGPoint-s1"

mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_challenge_final_last_ranking where platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_challenge_final_ranking where platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_challenge_last_ranking where platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_challenge_last_top_ranking where platform_name = '$delete_servername';"
mysql -u$db_user -p$db_password -h$db_host $db_name -e "delete from t_world_challenge_ranking where platform_name = '$delete_servername';"
