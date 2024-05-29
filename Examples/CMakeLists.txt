function(SimpleExample ModDirectory ComponentName)
    install(DIRECTORY ${ModDirectory}
        DESTINATION "."
        COMPONENT ${ComponentName}
    )
endfunction()

SimpleExample(Hamis          NP_Example)
SimpleExample(rainbow_cursor NSEW_Example)
SimpleExample(deadapple      NSEW_Example)

add_subdirectory(server)
add_subdirectory(client)
