#!/bin/bash
cd `dirname $0`
if [ "$1" == "clean"  ];then
    cd `dirname $0`
    rm -rf  bak xml python php proto
    exit;
fi

bin_dir="./gen_proto_app/bin/"
cpp_proto="$bin_dir/cpp_proto"
py_proto="$bin_dir/py_proto"
php_proto="$bin_dir/php_proto"
php_one_proto="$bin_dir/php_one_proto"
#pb_proto="$bin_dir/pb_proto"
plugin_list="./gen_proto_app/pub_plugin/gen_db_bind_func.py,./plugin/gen_online.py"

#$1: 项目名 $2:cpp中结构体的前缀配置
function get_deal_xml  {
	log_date=$(date +%Y%m%d_%H%M%S )
	project_name=$1
	xml_name="./xml/$project_name.xml"
	#备份原xml文件
	mv $xml_name ./bak/${project_name}.xml.$log_date 

	wget  "http://bigpiece.gt.com/protocol/proto/download.php?project=$project_name" -O $xml_name 
	$cpp_proto  --struct_fix_name="$2" --plugin_name_list="$plugin_list" ./$xml_name -o ./proto/ 
	#$php_proto $xml_name -o ./php 
	$php_proto $xml_name -o ./php  
	#$php_one_proto $xml_name -o ./php  
	$py_proto $xml_name -o  ./python
	#$pb_proto $xml_name -A -o ./pb
}

#创建备份文件夹
[  -d  ./bak ] || mkdir ./bak
[  -d  ./proto ] || mkdir ./proto
[  -d  ./python ] || mkdir ./python
[  -d  ./php ] || mkdir ./php
[  -d  ./xml ] || mkdir ./xml

#-----------------------------------


#-----------------------------------


get_deal_xml yueyo_db1 
get_deal_xml yueyo_db
get_deal_xml yueyo_glob
get_deal_xml yueyo_world
#get_deal_xml seer2_db 
#get_deal_xml yueyo_db1

#get_deal_xml mole2_btl  mbl


# cp  to proto
#cplist=" \
#	yueyo_world.cpp\
#	yueyo_world.h\
#	yueyo.h\
#"

time_now=`date +%Y%m%d-%H:%M:%S`
#cp -v ../../inc/yueyo_db_enum.h ./bak/yueyo_db_enum.h_$time_now
#cp -v ../../inc/yueyo_db1_enum.h ./bak/yueyo_db1_enum.h_$time_now
#cp -v ../../inc/yueyo_glob_enum.h ./bak/yueyo_glob_enum.h_$time_now
#cp -v ../../inc/yueyo_world_enum.h ./bak/yueyo_world_enum.h_$time_now
#cp -v ../../inc/yueyo_db1.h ./bak/yueyo_db1.h_$time_now
#cp -v ../../inc/yueyo_glob.h ./bak/yueyo_glob.h_$time_now
#cp -v ../../inc/yueyo_world.h ./bak/yueyo_world.h_$time_now
#cp -v ../yueyo_db1.cpp ./bak/yueyo_db1.cpp_$time_now
#cp -v ../yueyo_glob.cpp ./bak/yueyo_glob.cpp_$time_now
#cp -v ../yueyo_world.cpp ./bak/yueyo_world.cpp_$time_now
#cp -v ../../inc/yueyo_db.h ./bak/yueyo_db.h_$time_now
#cp -v ../yueyo_db.cpp ./bak/yueyo_db.cpp_$time_now
#cp -v ../../inc/yueyo.h ./bak/yueyo.h_$time_now
#cp -v ../yueyo.cpp ./bak/yueyo.cpp_$time_now

function bcp {
	#src=$1
	#des=$2
	echo -ne "\xEF\xBB\xBF" > "$2"
	cat $1 >> $2
}

bcp ./proto/yueyo_db_enum.h  ../incProto/yueyo_db_enum.h
bcp ./proto/yueyo_db1_enum.h  ../incProto/yueyo_db1_enum.h
bcp ./proto/yueyo_glob_enum.h  ../incProto/yueyo_glob_enum.h
bcp ./proto/yueyo_world_enum.h  ../incProto/yueyo_world_enum.h
bcp ./proto/yueyo.h  ../incProto/yueyo.h
bcp ./proto/yueyo.cpp  ../incProto/yueyo.cpp
bcp ./proto/yueyo_db1.h  ../incProto/yueyo_db1.h
bcp ./proto/yueyo_db1.cpp  ../incProto/yueyo_db1.cpp
bcp ./proto/yueyo_db.h  ../incProto/yueyo_db.h
bcp ./proto/yueyo_db.cpp  ../incProto/yueyo_db.cpp
bcp ./proto/yueyo_glob.h  ../incProto/yueyo_glob.h
bcp ./proto/yueyo_glob.cpp  ../incProto/yueyo_glob.cpp
bcp ./proto/yueyo_world.h  ../incProto/yueyo_world.h
bcp ./proto/yueyo_world.cpp  ../incProto/yueyo_world.cpp
