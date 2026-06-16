cmake_minimum_required(VERSION 4.1)

project(
	Snes9xEXPlus
	DESCRIPTION "SNES/Super Famicom emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(s9xp)
addPkgConfigDepMultiConfig(s9xp emuframework)
addCxxModules(s9xp emuex imagine)
evalPkgConfigFlags(s9xp all)
link_directories($ENV{HOME}/imagine-sdk/android-libs)
target_include_directories(s9xp PRIVATE
	$ENV{HOME}/imagine-sdk/include
	src/snes9x
	src/apu/bapu
)
target_compile_definitions(s9xp PRIVATE HAVE_STRINGS_H HAVE_STDINT_H RIGHTSHIFT_IS_SAR ZLIB USE_CURL)
target_compile_options(s9xp PRIVATE -Wno-register -Wno-implicit-fallthrough)

# Suporte a libcurl (conforme resumo técnico)
find_package(CURL)
if(CURL_FOUND)
    target_link_libraries(s9xp PRIVATE CURL::libcurl)
else()
    # Fallback para busca manual se find_package falhar no NDK
    target_link_libraries(s9xp PRIVATE curl crypto ssl)
endif()

add_subdirectory(src)