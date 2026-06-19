cmake_minimum_required(VERSION 3.29)

project(
	NGP.emu
	DESCRIPTION "Neo Geo Pocket Color emulator"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

include(emuFrameworkUtils.cmake)
printConfigInfo()
configureAppTarget(ngpemu)
addMednafenFlags(ngpemu)
addPkgConfigDepMultiConfig(ngpemu emuframework)
addCxxModules(ngpemu emuex imagine)
evalPkgConfigFlags(ngpemu all)
add_subdirectory(src)