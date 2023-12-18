FetchContent_Declare(LuaSocket
    GIT_REPOSITORY https://github.com/lunarmodules/luasocket.git
    GIT_TAG 88c8a85cb6528232288ec590910b7a966aace52b # v3.0.0
)

FetchContent_MakeAvailable(LuaSocket)

add_library(luasocket-socket MODULE
    ${luasocket_SOURCE_DIR}/src/luasocket.c
    ${luasocket_SOURCE_DIR}/src/timeout.c
    ${luasocket_SOURCE_DIR}/src/buffer.c
    ${luasocket_SOURCE_DIR}/src/io.c
    ${luasocket_SOURCE_DIR}/src/auxiliar.c
    ${luasocket_SOURCE_DIR}/src/compat.c
    ${luasocket_SOURCE_DIR}/src/options.c
    ${luasocket_SOURCE_DIR}/src/inet.c
    ${luasocket_SOURCE_DIR}/src/wsocket.c
    ${luasocket_SOURCE_DIR}/src/except.c
    ${luasocket_SOURCE_DIR}/src/select.c
    ${luasocket_SOURCE_DIR}/src/tcp.c
    ${luasocket_SOURCE_DIR}/src/udp.c
)

set_target_properties(luasocket-socket
    PROPERTIES
    OUTPUT_NAME core
    LIBRARY_OUTPUT_DIRECTORY socket
)

target_include_directories(luasocket-socket
    PRIVATE ${LUA_INCLUDE_DIR}
)

target_link_libraries(luasocket-socket
    PRIVATE ${LUA_LIBRARIES} ws2_32
)

install(
    TARGETS luasocket-socket
    DESTINATION "nsew_client/cdir/socket"
    COMPONENT NSEW_Client
)

install(
    FILES
        ${luasocket_SOURCE_DIR}/src/socket.lua
        ${luasocket_SOURCE_DIR}/src/ltn12.lua
    DESTINATION "nsew_client/ldir"
    COMPONENT NSEW_Client
)

install(
    FILES
        ${luasocket_SOURCE_DIR}/src/http.lua
        ${luasocket_SOURCE_DIR}/src/tp.lua
        ${luasocket_SOURCE_DIR}/src/ftp.lua
        ${luasocket_SOURCE_DIR}/src/smtp.lua
        ${luasocket_SOURCE_DIR}/src/url.lua
    DESTINATION "nsew_client/ldir/socket"
    COMPONENT NSEW_Client
)

install(DIRECTORY nsew_client
    DESTINATION "."
    COMPONENT NSEW_Client
)
