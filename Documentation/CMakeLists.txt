find_package(Python3 REQUIRED)

set(DOC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/_build)

add_custom_target(noitapatcher_docs ALL
    COMMAND Python3::Interpreter -m sphinx.cmd.build
        ${CMAKE_CURRENT_SOURCE_DIR} ${DOC_BUILD_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(DIRECTORY ${DOC_BUILD_DIR}/
    DESTINATION Documentation
    COMPONENT NoitaPatcher_Docs
    PATTERN ".*" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/noitapatcher.lua
    DESTINATION Modules
    COMPONENT NoitaPatcher_Docs
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/MODULE_README.txt
    DESTINATION Modules
    COMPONENT NoitaPatcher_Docs
    RENAME README.txt
)
