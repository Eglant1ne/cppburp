set(TEST_SOURCES
    tst_proxyserver.cpp

    ../src/proxyserver.cpp
    ../src/proxyconnection.cpp
    ../src/httpparser.cpp
)

add_executable(tst_proxyserver ${TEST_SOURCES})

target_include_directories(tst_proxyserver PRIVATE
    ../src
)

target_link_libraries(tst_proxyserver PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

set_target_properties(tst_proxyserver PROPERTIES
    WIN32_EXECUTABLE FALSE
)

add_test(NAME ProxyServerTests COMMAND tst_proxyserver)