add_library(blocker
    src/Blocker.cpp
    src/BlockerSyn.cpp
    src/BlockerHan.cpp
    src/BlockerConfig.cpp
    src/EepBlockerDriver.cpp
    src/EepBlockConfBase.cpp
    src/EepBlockConf.cpp
    src/Message.cpp
)

target_include_directories(blocker PUBLIC include)

target_link_libraries(blocker PUBLIC common estd util async bsp)
