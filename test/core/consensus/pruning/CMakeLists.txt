addtest(sync_test sync_test.cpp)

target_link_libraries(sync_test
    app_config_impl
    babe_synchronizer
    kagome_router
    sync_protocol
    Boost::Boost.DI
    p2p::p2p
    p2p::p2p_ping)

addtest(inj_test inj_test.cpp)
target_link_libraries(inj_test
    Boost::Boost.DI)
