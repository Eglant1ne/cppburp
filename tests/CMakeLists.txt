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

# ─── HttpParser + normalizeRequest (custom runner, no QtTest) ─────────────────

set(TST_HTTPPARSER_SOURCES
    test_main.cpp

    ../src/httpparser.cpp
)

add_executable(tst_httpparser ${TST_HTTPPARSER_SOURCES})

target_include_directories(tst_httpparser PRIVATE
    ../src
)

target_link_libraries(tst_httpparser PRIVATE
    Qt6::Core
    Qt6::Network
)

set_target_properties(tst_httpparser PROPERTIES
    WIN32_EXECUTABLE FALSE
)

add_test(NAME HttpParserTests COMMAND tst_httpparser)
