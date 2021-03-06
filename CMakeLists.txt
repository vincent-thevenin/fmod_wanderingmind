cmake_minimum_required(VERSION 3.10)
# set the project name
project(MAIN)

include(${CMAKE_CURRENT_SOURCE_DIR}/vendors/socket.io-client-cpp/CMakeLists.txt)

# add the executable
add_executable(TEST ${VENDORS_HEADERS} main.cpp)
add_executable(JSON ${VENDORS_HEADERS} json_test.cpp)

# set_property(TEST test PROPERTY CXX_STANDARD 11)
# set_property(TEST test PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(TEST sioclient)
target_link_libraries(TEST pthread )
message(STATUS ${Boost_INCLUDE_DIRS} )


# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# configure_file(test.h.in test)

file(GLOB_RECURSE VENDORS_HEADERS vendors/*.hpp)


include_directories(
    "fmodstudioapi20201linux/api/core/inc"
    "vendors"
)

# link_directories(
#     "api/core/lib/x86_64"
# )

target_link_libraries(
    TEST
    "${CMAKE_SOURCE_DIR}/fmodstudioapi20201linux/api/core/lib/x86_64/libfmod.so"
    "${CMAKE_SOURCE_DIR}/fmodstudioapi20201linux/api/core/lib/x86_64/libfmodL.so"
    
)

# target_include_directories(TEST PUBLIC
#     "${CMAKE_SOURCE_DIR}/api/core/lib/x86_64"
# )
