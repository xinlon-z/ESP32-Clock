cmake_minimum_required(VERSION 3.16)
project(esp32_clock_tests C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(MAIN "${CMAKE_CURRENT_SOURCE_DIR}/../main")
set(TEST_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"   # fake ESP-IDF / FreeRTOS headers
    "${MAIN}")                            # app/, platform/, user_config.h

function(clock_test name)
    add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE ${TEST_INCLUDES})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

clock_test(test_event_queue)
clock_test(test_event_bus)
clock_test(test_mqtt_service_music_parse)
clock_test(test_mqtt_service_cover_pump)
clock_test(test_music_player_icon_geometry)
clock_test(test_music_time_format)
clock_test(test_music_background)
clock_test(test_music_background_image)
clock_test(test_music_visualizer)
clock_test(test_music_model)

# Separate compilation: music_presenter.cpp and music_model.cpp both define
# kFramesPerSecond in anonymous namespaces, which collide in a single TU.
add_executable(test_music_presenter_eventbus
    test_music_presenter_eventbus.cpp
    ${MAIN}/app/features/music/music_presenter.cpp
    ${MAIN}/app/features/music/music_model.cpp
    ${MAIN}/app/core/event/event_bus.cpp
    ${MAIN}/app/services/mqtt_service.cpp
    ${MAIN}/app/services/cover_service.cpp)
target_include_directories(test_music_presenter_eventbus PRIVATE ${TEST_INCLUDES})
add_test(NAME test_music_presenter_eventbus COMMAND test_music_presenter_eventbus)
clock_test(test_clock_model)
clock_test(test_clock_presenter)
clock_test(test_power_mgr_voltage)
clock_test(test_power_mgr_idle_mode)
clock_test(test_power_service)
clock_test(test_time_service)
clock_test(test_screen_nav)

# Separate compilation for these two tests: including music_mqtt.cpp and
# mqtt_service.cpp in the same TU causes a redefinition of MusicMqtt::CoverImage.
# music_mqtt.cpp must remain inlined (the test calls its anonymous-namespace
# internals updateState/updateCover directly).
add_executable(test_music_mqtt_ingest_service_event
    test_music_mqtt_ingest_service_event.cpp
    ${MAIN}/app/services/mqtt_service.cpp
    ${MAIN}/app/services/cover_service.cpp
    ${MAIN}/app/core/event/event_bus.cpp)
target_include_directories(test_music_mqtt_ingest_service_event PRIVATE ${TEST_INCLUDES})
add_test(NAME test_music_mqtt_ingest_service_event COMMAND test_music_mqtt_ingest_service_event)
