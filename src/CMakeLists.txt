add_library(noitapatcher MODULE
    main.cpp
)

target_link_libraries(noitapatcher
    PRIVATE
        minhook
        dbghelp
)

install(
    TARGETS noitapatcher
    COMPONENT np
    LIBRARY DESTINATION .
)
