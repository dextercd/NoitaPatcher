add_library(noitapatcher MODULE
    main.cpp
)

target_link_libraries(noitapatcher
    PRIVATE
        minhook
        LuaJIT_library
)

install(
    TARGETS noitapatcher
    COMPONENT np
    LIBRARY DESTINATION .
)
