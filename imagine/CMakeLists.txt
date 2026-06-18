cmake_minimum_required(VERSION 4.1)\n\nproject(\n	Imagine\n	VERSION 1.5.85\n	DESCRIPTION "Game/Multimedia Engine"\n	HOMEPAGE_URL "https://www.explusalpha.com/"\n)\n\nset(CMAKE_INSTALL_PREFIX "${IMAGINE_SDK_PLATFORM_PATH}" CACHE PATH "Installation directory" FORCE)\n\nif(APPLE_PLATFORM)\n	enable_language(OBJCXX)\nendif()\n\nprintConfigInfo()\nadd_library(imagine STATIC)\n# Correcao Android NDK\n
# --- CORREÇÃO FORÇADA PARA ANDROID NDK LIBC++ ---
if(ANDROID)
    set_target_properties(imagine PROPERTIES 
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_include_directories(imagine SYSTEM BEFORE PRIVATE
        "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1"
    )
    target_link_libraries(imagine PRIVATE c++)
endif()
\nif(NOT ANDROID AND TARGET "__CMAKE::CXX26")\n    set_target_properties(imagine PROPERTIES CXX_MODULE_STD ON)\nelse()\n    message(STATUS "Ignorando CXX_MODULE_STD para imagine no Android")\nendif()\nset_target_properties(imagine PROPERTIES OUTPUT_NAME "imagine${GEN_TARGET_EXT}")\nset_property(TARGET imagine PROPERTY C_STANDARD 23)\ntarget_include_directories(imagine PRIVATE include)\ntarget_compile_options(imagine PRIVATE -Werror)\ninstall(TARGETS imagine DESTINATION lib)\ninstall(DIRECTORY "${CMAKE_SOURCE_DIR}/include/imagine" DESTINATION include)\nadd_subdirectory(src)\n\nif(SUBENV STREQUAL pandora)\n	addConfigEnable(imagine CONFIG_MACHINE_PANDORA)\nendif()\n\ngenerateConfigHeader(imagine)\nevalPkgConfigCFlags(imagine)\nwritePkgConfigFiles(imagine)