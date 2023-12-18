add_executable(nsew_server
    main.cpp
)

target_link_libraries(nsew_server
    PRIVATE
        Boost::system
        absl::hash absl::flat_hash_map
        nsew_common
)

if (WIN32)
    # Workaround for linker errors when cross compiling from Linux to Windows
    target_link_libraries(nsew_server PRIVATE ws2_32 mswsock)
endif()

install(TARGETS nsew_server
    DESTINATION "server"
    COMPONENT NSEW_Server
)
