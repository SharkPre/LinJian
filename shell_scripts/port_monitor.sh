#!/bin/bash
#Check GameServer Port Using
#set -xv
LOGFILE=/data/logs/port_monitor.log
LOCAL_IP=`/sbin/ifconfig | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}' | grep -E "^10.|^192.|^172."`

echo "===================Checking Port is beginning at `date +%Y-%m-%d\ %H:%M:%S`===================" >> $LOGFILE
for list in `curl -s http://api.naruto.gametrees.com/remote_ip.php?host="$LOCAL_IP"`;do
    GMIP=$(echo $list | awk -F\| '{print $1}')
    AID=$(echo $list | awk -F\| '{print $2}')
    SID=$(echo $list | awk -F\| '{print $3}')
    GM_PORT=$(echo $list | awk -F\| '{print $4}')
    WEB_PORT=$(($GM_PORT+10200))
    GMNAME="GM_"$AID"_"$SID""
        
    GM_LIST=$(netstat -pant | grep -E "ESTABLISHED|LISTEN" | grep ":$GM_PORT")
    echo "$GM_LIST" | while read line;do
        GM_NAME=$(echo "$line" | awk -F\/ '{print $2}')
        GM_PID=$(echo "$line" | awk '{print $7}' | awk -F\/ '{print $1}' | sed s/[[:space:]]//g)
        if [ ! x"$GM_NAME" == x"$GMNAME" ] ;then
            echo "Error!!! $GM_PORT is using by $GM_NAME,pid is $GM_PID,should be using by $GMNAME, killing!" >> $LOGFILE
            kill -9 $GM_PID
        else
            echo "$GM_PORT is using by $GM_NAME" >> $LOGFILE
        fi
    done
    
    WEB_LIST=$(netstat -pant | grep -v grep | grep -v WAIT | grep "$WEB_PORT")
    echo "$WEB_LIST" | while read line1;do
        WEB_NAME=$(echo "$line1" | awk -F\/ '{print $2}')
        WEB_PID=$(echo "$line1" | awk '{print $7}' | awk -F\/ '{print $1}' | sed s/[[:space:]]//g)
        if [ ! x"$WEB_NAME" == x"$GMNAME" ] ;then
            echo "Error!!! $GM_PORT is using by $WEB_NAME,pid is $WEB_PID,should be using by $GMNAME, killing!" >> $LOGFILE
            kill -9 $WEB_PID
        else
            echo "$WEB_PORT is using by $WEB_NAME" >> $LOGFILE
        fi
    done
    echo "--------------------------------" >> $LOGFILE
done
echo "===================Checking Port is finished at `date +%Y-%m-%d\ %H:%M:%S`===================" >> $LOGFILE
