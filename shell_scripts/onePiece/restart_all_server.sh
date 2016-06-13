#启动db server
cd ~/server_issue/server_api/DBSvr/bin/
./run.sh

#启动glob server
cd ~/server_issue/server_api/global_server/bin/
./run.sh

usleep 2000000

#启动world server
cd ~/server_issue/server_api/world_server/bin/
./run.sh

usleep 2000000


#启动world guild server
cd ~/server_issue/server_api/world_guild_server/bin/
./run.sh

usleep 2000000

#启动battle server
cd ~/server_issue/server_api/battle_server/bin/
./run.sh
