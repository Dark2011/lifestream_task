cmake_minimum_required(VERSION 3.2)

set(BUILD_NUM 1)
if($ENV{TRAVIS_BUILD_NUMBER})
	set(BUILD_NUM $ENV{TRAVIS_BUILD_NUMBER})
endif()

message (STATUS "Current build number is: " ${BUILD_NUM})

project(udp_server VERSION 0.0.${BUILD_NUM})

find_package(Qt5 REQUIRED Core Network)

# Automatic handling of moc preprocessor
set(CMAKE_AUTOMOC ON)
# Automatic handling of uic preprocessor
set(CMAKE_AUTOUIC ON)
# Automatic handling of rcc preprocessor
set(CMAKE_AUTORCC ON)


set(HDR message.h fileprocessor.h driver.h server.h)
set(SRC fileprocessor.cpp driver.cpp server.cpp main.cpp)



add_executable(server ${HDR} ${SRC})

set_target_properties(server PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON 
)

#target_include_directories(server 
#    PUBLIC ${Boost_INCLUDE_DIR} 
#)

target_link_libraries(server PUBLIC Qt5::Core Qt5::Network)
target_compile_definitions(server PRIVATE SAVE_TO_FS)

install(TARGETS server RUNTIME DESTINATION bin)	

