cmake_minimum_required(VERSION 3.20)

project(HW-MusicPlayer-BackEnd VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/usr/lib/")

# 依赖库
find_package(Boost CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
find_package(jwt-cpp REQUIRED)

# mysql-connector-c++-8.0.33
set(LIB_MYSQL_CONNECTOR_INLCUDE /usr/local/mysql/connector-c++-8.0.33/include)
set(LIB_MYSQL_CONNECTOR_DIR /usr/local/mysql/connector-c++-8.0.33/lib64/debug)
find_library(LIB_MYSQL_CONNECTOR_CPP NAMES mysqlcppconn PATHS ${LIB_MYSQL_CONNECTOR_DIR} REQUIRED)

# 链接库
link_libraries(
    Boost::boost
    boost_url
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    fmt::fmt
    jwt-cpp::jwt-cpp
    spdlog::spdlog
    ${LIB_MYSQL_CONNECTOR_CPP}
)

include_directories(${LIB_MYSQL_CONNECTOR_INLCUDE})

file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
)

# test
file(GLOB TEST_FILES
    src/database/*.cpp
    src/models/*.cpp
    src/utils/*.cpp
)
add_executable(test-mysql ${TEST_FILES} test/test_mysql.cpp)
target_include_directories(test-mysql PRIVATE src)