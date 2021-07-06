cmake_minimum_required(VERSION 3.0.0) # cmake最小版本
project(MowUI VERSION 0.1.0)# 工程基础信息
# cmake基础语法
#---
# set(<VAR_NAME> <VAR_VAL>) 其中VAR_NAME理解为变量名,VAR_VAL理解为变量值
# VAR_VAL可以是直接值,比如数字,文本？（暂无实践）,可以使用replacement代替
# replacement即为${<VAR_NAME>}语法
# ---
# add_executable(<${name}> [...source_list]) 添加一组可执行文件,name好像就是当前项目名称

set(CMAKE_CXX_STANDARD 14)# 设置项目编译std版本

include(CTest)
enable_testing()

find_package(Curses REQUIRED)# 查找对应的动态库，第二个参数是是否必须
include_directories(${CURSES_INCLUDE_DIR})# 引入对应库的地址

aux_source_directory(. source_list)# 扫描源代码路径
add_executable(MowUI ${source_list})# 添加cpp代码文件
target_link_libraries(MowUI ${CURSES_LIBRARIES})# 链接对应的库，必须处于add_executable后面

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
