cmake_minimum_required(VERSION 2.6.8)

set(CMAKE_CPP_FLAGS "-std=c++0x -DBOOST_THREAD_USE_LIB")
#set(wxWidgets_USE_DEBUG true)
find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(Boost COMPONENTS random date_time regex filesystem system signals serialization thread program_options REQUIRED)
find_package(Wt REQUIRED)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Threads)
if(WIN32)
	set(WIN_LIB  ws2_32 mswsock)
endif(WIN32)

include(${wxWidgets_USE_FILE})

add_executable(client 
	src/client/client.cpp 
	src/client/gui.cpp
	src/client/cmd.cpp
	src/client/run_vnc.cpp
	)
target_link_libraries(client ${wxWidgets_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${WIN_LIB})

add_executable(server
	src/server/server.cpp
	src/server/cmd.cpp
	src/server/config_vnc.cpp 
	src/server/d3des.c
	)
target_link_libraries(server ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${WIN_LIB})

add_subdirectory(src/webgui/)
