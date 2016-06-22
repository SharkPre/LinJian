
#命令名字不区分大小写 project不是强制的 但是最好还是加上比较好 
PROJECT(MONGODB)

AUX_SOURCE_DIRECTORY(./src SRC_LIST)

# 设置变量值 
SET(LIBRARY_OUTPUT_PATH ../lib)
ADD_DEFINITIONS(-std=c++0x -g -W -MMD)

INCLUDE_DIRECTORIES(./ ./include /usr/local/include)

LINK_DIRECTORIES(/usr/local/lib)
#LINK_LIBRARIES()

#告诉工程生成一个库 
ADD_LIBRARY(mongodb ${SRC_LIST})
TARGET_LINK_LIBRARIES(mongodb mongoclient boost_thread boost_filesystem boost_program_options )

# set 命令用来设置变量
# 可以通过message来输出变量的值  message(${PROJECT_SOURCE_DIR})
# add_exectuable 告诉工程生成一个可执行文件 
参考网址:
 http://www.cnblogs.com/JohnShao/archive/2011/09/26/2191600.html