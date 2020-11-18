cmake_minimum_required(VERSION 3.2)

set(BUILD_NUM 1)
if($ENV{TRAVIS_BUILD_NUMBER})
	set(BUILD_NUM $ENV{TRAVIS_BUILD_NUMBER})
endif()

message (STATUS "Current build number is: " ${BUILD_NUM})

project(udp_client VERSION 0.0.${BUILD_NUM})

if (WIN32)
	set(Boost_USE_STATIC_LIBS ON)
else()
	add_definitions( -DBOOST_ALL_NO_LIB )
	add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED date_time system iostreams thread log)

set(HDR message.h fileprocessor.h driver.h transport.h)
set(SRC fileprocessor.cpp driver.cpp transport.cpp main.cpp)



add_executable(client ${HDR} ${SRC})

set_target_properties(client PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON 
)

target_include_directories(client 
    PUBLIC ${Boost_INCLUDE_DIR} 
)

target_link_libraries(client PUBLIC Boost::system Boost::date_time Boost::iostreams Boost::thread Boost::log)

target_compile_definitions(client PRIVATE MULTITHREAD)
if (WIN32)
	target_compile_definitions(client PRIVATE _WIN32_WINNT=0x0601)
endif()


install(TARGETS client RUNTIME DESTINATION bin)	

