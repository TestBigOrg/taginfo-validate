cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(taginfo-validate VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++14")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Boost REQUIRED COMPONENTS program_options system)
find_package(Threads REQUIRED)


# Vendored Libosmium
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libosmium-2.7.2/cmake")
set(OSMIUM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libosmium-2.7.2/include")
find_package(Osmium REQUIRED COMPONENTS io)

# Vendored RapidJSON
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/rapidjson-f51d7c9d4b1a9313bde4a5ccf94c4af7089661a8/include")


add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cc)
target_link_libraries(${PROJECT_NAME} ${OSMIUM_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${OSMIUM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIR})
