cmake_minimum_required(VERSION 3.3)

project(trade LANGUAGES C VERSION 0.1.0)

set(SOURCES
    main.c
    trade.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
