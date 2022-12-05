cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0079 NEW)
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    project(keymaster)
endif()
cmake_minimum_required(VERSION 3.18)

add_library(
    keymaster_messages
    SHARED
    android_keymaster/android_keymaster_messages.cpp
    android_keymaster/android_keymaster_utils.cpp
    android_keymaster/authorization_set.cpp
    android_keymaster/keymaster_tags.cpp
    android_keymaster/logger.cpp
    android_keymaster/serializable.cpp
)
target_include_directories(
    keymaster_messages
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    keymaster_messages
    PRIVATE
    hardware_headers
)
target_compile_definitions(keymaster_messages PRIVATE -DKEYMASTER_NAME_TAGS)