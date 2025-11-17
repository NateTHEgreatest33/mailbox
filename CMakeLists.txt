# cmake_minimum_required(VERSION 3.5)

project(mailboxAPI CXX ASM)

set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_library( mailboxAPI
    mailbox.hpp
    mailbox.tpp

    mailbox_types.hpp
    mailbox_map_types.hpp

    test/mailbox_test.hpp
    test/mailbox_test.cpp
    )

target_link_libraries( mailboxAPI
  # pico_stdlib 

  messageAPI
  utilLib
  consoleAPI

)

target_include_directories( mailboxAPI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" )
