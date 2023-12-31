cmake_minimum_required(VERSION 3.0.0...3.5.0)
project(cityscape VERSION 1.0.0)

# Find OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

# Set Assimp build flags
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library." FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable Assimp's export functionality." FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "If the supplementary tools for Assimp are built in addition to the library." FORCE)

add_definitions(-DASSIMP_BUILD_NO_X_IMPORTER 
                -DASSIMP_BUILD_NO_AMF_IMPORTER 
                -DASSIMP_BUILD_NO_3DS_IMPORTER
                -DASSIMP_BUILD_NO_MD3_IMPORTER
                -DASSIMP_BUILD_NO_MDL_IMPORTER
                -DASSIMP_BUILD_NO_MD2_IMPORTER
                -DASSIMP_BUILD_NO_PLY_IMPORTER
                -DASSIMP_BUILD_NO_ASE_IMPORTER
                #-DASSIMP_BUILD_NO_OBJ_IMPORTER
                -DASSIMP_BUILD_NO_HMP_IMPORTER
                -DASSIMP_BUILD_NO_SMD_IMPORTER
                -DASSIMP_BUILD_NO_MDC_IMPORTER
                -DASSIMP_BUILD_NO_MD5_IMPORTER
                -DASSIMP_BUILD_NO_STL_IMPORTER
                -DASSIMP_BUILD_NO_LWO_IMPORTER
                -DASSIMP_BUILD_NO_DXF_IMPORTER
                -DASSIMP_BUILD_NO_NFF_IMPORTER
                -DASSIMP_BUILD_NO_RAW_IMPORTER
                -DASSIMP_BUILD_NO_SIB_IMPORTER
                -DASSIMP_BUILD_NO_OFF_IMPORTER
                -DASSIMP_BUILD_NO_AC_IMPORTER
                -DASSIMP_BUILD_NO_BVH_IMPORTER
                -DASSIMP_BUILD_NO_IRRMESH_IMPORTER
                -DASSIMP_BUILD_NO_IRR_IMPORTER
                -DASSIMP_BUILD_NO_Q3D_IMPORTER
                -DASSIMP_BUILD_NO_B3D_IMPORTER
                -DASSIMP_BUILD_NO_COLLADA_IMPORTER
                -DASSIMP_BUILD_NO_TERRAGEN_IMPORTER
                -DASSIMP_BUILD_NO_CSM_IMPORTER
                -DASSIMP_BUILD_NO_3D_IMPORTER
                -DASSIMP_BUILD_NO_LWS_IMPORTER
                -DASSIMP_BUILD_NO_OGRE_IMPORTER
                -DASSIMP_BUILD_NO_OPENGEX_IMPORTER
                -DASSIMP_BUILD_NO_MS3D_IMPORTER
                -DASSIMP_BUILD_NO_COB_IMPORTER
                -DASSIMP_BUILD_NO_Q3BSP_IMPORTER
                -DASSIMP_BUILD_NO_NDO_IMPORTER
                -DASSIMP_BUILD_NO_IFC_IMPORTER
                -DASSIMP_BUILD_NO_XGL_IMPORTER
                -DASSIMP_BUILD_NO_ASSBIN_IMPORTER
                -DASSIMP_BUILD_NO_C4D_IMPORTER
                -DASSIMP_BUILD_NO_3MF_IMPORTER
                -DASSIMP_BUILD_NO_X3D_IMPORTER
                -DASSIMP_BUILD_NO_MMD_IMPORTER
                -DASSIMP_BUILD_NO_M3D_IMPORTER
                -DASSIMP_BUILD_NO_STEP_IMPORTER
                -DASSIMP_BUILD_NO_BLEND_IMPORTER)

# Add cmake project folders
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/glew/build/cmake)

# Add all project-wide includes
include_directories(
    ${CMAKE_SOURCE_DIR}
    thirdparty
    thirdparty/glfw/include
    thirdparty/glm
    thirdparty/glew/include
    thirdparty/assimp/include
    ${OPENGL_INCLUDE_DIRS})

# Glob all source / headers for project
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp)

# Glob all source / headers for Phi
file(GLOB_RECURSE PHI_SOURCE
    ${CMAKE_SOURCE_DIR}/phi/*.cpp)

file(GLOB_RECURSE PHI_HEADERS
    ${CMAKE_SOURCE_DIR}/phi/*.hpp)

# Add ImGui
set(IMGUI_PATH  ${CMAKE_SOURCE_DIR}/thirdparty/imgui/)
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
add_library(ImGui STATIC ${IMGUI_SOURCES})

# Final executable
add_executable(
    cityscape 
    ${PHI_SOURCE}
    ${PHI_HEADERS}
    ${IMGUI_SOURCES}
    ${SOURCE_FILES}
    ${HEADER_FILES})

target_link_libraries(cityscape assimp glfw glew ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
