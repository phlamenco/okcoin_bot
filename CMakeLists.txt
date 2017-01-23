cmake_minimum_required(VERSION 3.4)
project(okcoin_bot)

# for boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.45.0 REQUIRED COMPONENTS system filesystem regex)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "found boost include ${Boost_INCLUDE_DIRS}")
    message(STATUS "found boost lib ${Boost_LIBRARY_DIRS}")
endif()

# for openssl
find_package(OPENSSL REQUIRED)
if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "found openssl root ${OPENSSL_ROOT_DIR}")
    message(STATUS "found openssl include ${OPENSSL_INCLUDE_DIRS}")
    message(STATUS "found openssl lib ${OPENSSL_LIBRARIES}")
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
endif()

# for rapidjson
find_package(RapidJSON REQUIRED)
if ( RAPIDJSON_FOUND )
    set(RAPIDJSON_USE_SSE2 ON)
    include_directories(RAPIDJSON_INCLUDE_DIRS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RAPIDJSON_CXX_FLAGS}")
endif()

# for curl
find_package(CURL REQUIRED)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    message(STATUS "found curl include ${CURL_INCLUDE_DIRS}")
    message(STATUS "found curl lib ${CURL_LIBRARIES}")
endif()

#for toml
add_subdirectory(lib/cpptoml-master-20170107)
include_directories(${cpptoml_SOURCE_DIR}/include)

# for log
add_subdirectory(lib/spdlog-master-20170108)
include_directories(${spdlog_SOURCE_DIR}/include)

include_directories(".")
include_directories(okcoin_client)
include_directories(util)
include_directories(rest_api/net/http/curl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES
        okcoin_client/md5.cpp
        okcoin_client/okcoinwebsocketapi.cpp
        okcoin_client/parameter.cpp
        util/json_util.cpp util/singleton.h log.h trade.h dump.h
        util/calc_util.h util/Timer.h util/queue.h strategy/asset_freeze.h
        strategy/tech_index.h util/time_window.h util/time_util.h
        util/numeric_util.h strategy/roc_strategy.h strategy/account_status.h
        pipeline.h configure.h
        util/window.h
        rest_api/net/http/curl/curl.cpp rest_api/net/http/curl/curlprotocol.cpp
        rest_api/net/http/curl/okcoinapi.cpp rest_api/net/http/curl/uri.cpp
        rest_api/net/http/curl/str.cpp)

add_library(okcoin_lib ${SOURCE_FILES})
add_executable(okcoin_bot okbot.cpp rest_api/rest_client.h)

target_link_libraries(okcoin_bot okcoin_lib)

if(Boost_FOUND)
    target_link_libraries(okcoin_bot ${Boost_LIBRARIES})
endif()
if(OPENSSL_FOUND)
    target_link_libraries(okcoin_bot ${OPENSSL_LIBRARIES})
endif()
if(CURL_FOUND)
    target_link_libraries(okcoin_bot ${CURL_LIBRARIES})
endif()

# Options. Turn on with 'cmake -Dmyvarname=ON'.
#option(test "Build all tests." OFF) # Makes boolean 'test' available.
################################
# Testing
################################

#if (test)
    # This adds another subdirectory, which has 'project(gtest)'.

add_subdirectory(lib/gtest-1.8.0)
enable_testing()
    # Include the gtest library. gtest_SOURCE_DIR is available due to
    # 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    ##############
    # Unit Tests
    ##############
add_executable(jsonTests test/rapidjson/test_json.cpp util/json_util.cpp)
add_executable(emailTests test/email/test_email.cpp)
add_executable(restApiTests test/okapi_test/ok_rest_api_test.cpp)
    # Standard linking to gtest stuff.
target_link_libraries(jsonTests gtest gtest_main ${Boost_LIBRARIES})
target_link_libraries(emailTests gtest gtest_main ${Boost_LIBRARIES})
target_link_libraries(restApiTests gtest gtest_main okcoin_lib ${CURL_LIBRARIES})
    # Extra linking for the project.
    #target_link_libraries(runUnitTests project1_lib)

    # This is so you can do 'make test' to see all your tests run, instead of
    # manually running the executable runUnitTests to see those specific tests.
    #add_test(NAME that-test-I-made COMMAND runUnitTests)

    # You can also omit NAME and COMMAND. The second argument could be some other
    # test executable.
add_test(test_json jsonTests)
add_test(test_email emailTests)
#endif()