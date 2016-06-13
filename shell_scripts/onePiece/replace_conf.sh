#!/bin/bash
#unzip xml.zip
cp xml/* server_api/battle_server/etc/conf
cp xml/* server_api/global_server/etc/conf
cp xml/* server_issue/server_api/battle_server/etc/conf
cp xml/* server_issue/server_api/global_server/etc/conf
cp xml/landMark.xml server_issue/server_api/world_guild_server/etc/conf
#./restart_server.sh
