# 3.10 introduces C++ 17 support
cmake_minimum_required (VERSION 3.10)
project (example_coordinator)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable extensions to keep compatible with standards
set(CMAKE_CXX_EXTENSIONS OFF)

## find_package(FCL REQUIRED)
######## Find CCD the hard way
find_package(PkgConfig)
if (PKGCONFIG_FOUND)
    pkg_check_modules(FCL fcl>=0.6)
    pkg_check_modules(CCD ccd>=2.0)
else()
    find_package(FCL REQUIRED)
    find_package(CCD CONFIG)
endif()
#########

find_package(OpenMP REQUIRED)
find_package(Assimp REQUIRED)
find_package(Threads)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS s3 lambda)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_AWS_SDK")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_AWS_SDK")


set(GDP_LIBRARIES "gdp")

include_directories(include)
add_executable(example_client src/example_client.cpp)
target_link_libraries(example_client ${GDP_LIBRARIES})

add_executable(example_coordinator src/example_coordinator.cpp)
target_link_libraries(example_coordinator AWS::aws-lambda-runtime ${AWSSDK_LINK_LIBRARIES})
