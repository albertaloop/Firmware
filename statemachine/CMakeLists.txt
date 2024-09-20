cmake_minimum_required(VERSION 3.10)

# what is a project?
# https://stackoverflow.com/a/26882812
project(state_machine)

set(STATE_MACHINE_SRCS
    main.c
    statemachine.c
    udp.c
)

add_executable(state_machine
    ${STATE_MACHINE_SRCS}
)

target_link_libraries(state_machine
    PUBLIC
    pthread
)