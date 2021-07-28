# Need pthreads.
find_package(Threads REQUIRED)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Found a threading library, but it is not pthreads.")
endif()

# Try to find atomic operations(原子操作).
include(FindAtomics)

if ((NOT (GPOS_GCC_FETCH_ADD_32 AND GPOS_GCC_FETCH_ADD_64
          AND GPOS_GCC_CAS_32 AND GPOS_GCC_CAS_64))
    AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")))
  message(FATAL_ERROR "Could not find GCC-style atomic built-ins or Solaris "
                      "atomic headers. GPOS will fail to build. Please try "
                      "using a recent g++ or clang++ compiler.")
endif()

include_directories(include)

include_directories(${PROJECT_SOURCE_DIR}/libgpopt/include/)
include_directories(${PROJECT_SOURCE_DIR}/libnaucrates/include/)

# PROJECT_BINARY_DIR 是 build 目录
configure_file(config.h.in
        ${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h)

# for the generated config.h file.
# file 代表文件操作
include_directories(${PROJECT_BINARY_DIR}/libgpos/include/)

file(GLOB_RECURSE hdrs ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Add headers to make them visible in some IDEs (Clion, VS, Xcode)
# list 是列表操作，例如：这里就是向列表 srcs 中添加新的元素
list(APPEND srcs ${hdrs})

# 增加一个 库 构建对象，此处是构建库 gpos
add_library(gpos ${srcs})

# 将库链接到目标上
target_link_libraries(gpos ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

# Extra system libs needed on Solaris.
if (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  find_library(RT_LIBRARY NAMES rt librt)
  if (${RT_LIBRARY-NOTFOUND})
    message(WARNING "rt library not found. Linking may fail.")
  else()
    target_link_libraries(gpos ${RT_LIBRARY})
  endif()

  find_library(SOCKET_LIBRARY NAMES socket libsocket)
  if (${SOCKET_LIBRARY-NOTFOUND})
    message(WARNING "socket library not found. Linking may fail.")
  else()
    target_link_libraries(gpos ${SOCKET_LIBRARY})
  endif()
endif()

# 设置目标的属性，这里是设置库 gpos 的属性
set_target_properties(gpos PROPERTIES
                      SOVERSION ${GPORCA_ABI_VERSION}
                      VERSION ${GPORCA_VERSION_STRING})

# Tests reside(住，居住) in the 'server' subdirectory.
add_subdirectory(server)

# Installation.
# 提供供用户选择的配置信息
option(VERBOSE_INSTALL_PATH "Install in a subdirectory path that includes GPOS version, CPU architecture, and bit width" OFF)
if (VERBOSE_INSTALL_PATH)
  set(installpath "libgpos/${GPORCA_VERSION_VERSION}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}")
  string(TOLOWER ${installpath} installpath)
  get_filename_component(full_install_name_dir "${installpath}/lib" ABSOLUTE)
  install(TARGETS gpos DESTINATION "${installpath}/lib")
  install(DIRECTORY include/gpos DESTINATION "${installpath}/include")
  install(FILES "${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h" DESTINATION "${installpath}/include/gpos")
else()  # 执行这个步骤 OFF
  get_filename_component(full_install_name_dir "${CMAKE_INSTALL_PREFIX}/lib" ABSOLUTE)  # 取得绝对路径
  install(TARGETS gpos DESTINATION lib)
  install(DIRECTORY include/gpos DESTINATION include)
  install(FILES "${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h" DESTINATION include/gpos)
endif()

