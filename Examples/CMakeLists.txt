function(SimpleMod ModDirectory ComponentName)
    install(DIRECTORY ${ModDirectory}
        DESTINATION "."
        COMPONENT ${ComponentName}
    )
endfunction()

SimpleMod(Hamis          NP_Example)
SimpleMod(rainbow_cursor NSEW_Example)

add_subdirectory(server)
add_subdirectory(client)
