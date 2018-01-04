# Need pthreads.
find_package(Threads REQUIRED)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Found a threading library, but it is not pthreads.")
endif()

# Try to find atomic operations.
include(FindAtomics)

if ((NOT (GPOS_GCC_FETCH_ADD_32 AND GPOS_GCC_FETCH_ADD_64
          AND GPOS_GCC_CAS_32 AND GPOS_GCC_CAS_64))
    AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")))
  message(FATAL_ERROR "Could not find GCC-style atomic built-ins or Solaris "
                      "atomic headers. GPOS will fail to build. Please try "
                      "using a recent g++ or clang++ compiler.")
endif()

include_directories(include)

configure_file(config.h.in
        ${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h)

# for the generated config.h file.
include_directories(${PROJECT_BINARY_DIR}/libgpos/include/)

# Compile .cpp source files under libgpos/src into monolithic gpos library.
add_library(gpos
            src/_api.cpp
            src/common/CAutoTimer.cpp
            src/common/CBitSet.cpp
            src/common/CBitSetIter.cpp
            src/common/CBitVector.cpp
            src/common/CHeapObject.cpp
            src/common/CMainArgs.cpp
            src/common/CRandom.cpp
            src/common/CStackDescriptor.cpp
            src/common/CStackObject.cpp
            src/common/CTimerUser.cpp
            src/common/CWallClock.cpp
            src/common/clibwrapper.cpp
            src/common/pthreadwrapper.cpp
            src/common/syslibwrapper.cpp
            src/error/CAutoExceptionStack.cpp
            src/error/CAutoTrace.cpp
            src/error/CErrorContext.cpp
            src/error/CErrorHandlerStandard.cpp
            src/error/CException.cpp
            src/error/CFSimulator.cpp
            src/error/CLogger.cpp
            src/error/CLoggerStream.cpp
            src/error/CLoggerSyslog.cpp
            src/error/CMessage.cpp
            src/error/CMessageRepository.cpp
            src/error/CMessageTable.cpp
            src/error/CMiniDumper.cpp
            src/error/CSerializable.cpp
            src/error/ILogger.cpp
            src/io/CFileDescriptor.cpp
            src/io/CFileReader.cpp
            src/io/CFileWriter.cpp
            src/io/COstream.cpp
            src/io/COstreamBasic.cpp
            src/io/COstreamFile.cpp
            src/io/COstreamString.cpp
            src/io/ioutils.cpp
            src/memory/CAutoMemoryPool.cpp
            src/memory/CCacheFactory.cpp
            src/memory/CMemoryPool.cpp
            src/memory/CMemoryPoolInjectFault.cpp
            src/memory/CMemoryPoolManager.cpp
            src/memory/CMemoryPoolStack.cpp
            src/memory/CMemoryPoolTracker.cpp
            src/memory/CMemoryVisitorPrint.cpp
            src/memory/IMemoryPool.cpp
            src/string/CStringStatic.cpp
            src/string/CWString.cpp
            src/string/CWStringBase.cpp
            src/string/CWStringConst.cpp
            src/string/CWStringDynamic.cpp
            src/string/CWStringStatic.cpp
            src/sync/CAutoMutex.cpp
            src/sync/CAutoSpinlock.cpp
            src/sync/CEvent.cpp
            src/sync/atomic.cpp
            src/task/CAutoSuspendAbort.cpp
            src/task/CAutoTaskProxy.cpp
            src/task/CAutoTraceFlag.cpp
            src/task/CTask.cpp
            src/task/CTaskContext.cpp
            src/task/CTaskLocalStorage.cpp
            src/task/CTaskSchedulerFifo.cpp
            src/task/CThreadManager.cpp
            src/task/CWorker.cpp
            src/task/CWorkerId.cpp
            src/task/CWorkerPoolManager.cpp
            src/task/ITask.cpp
            src/task/IWorker.cpp
            src/test/CFSimulatorTestExt.cpp
            src/test/CTimeSliceTest.cpp
            src/test/CUnittest.cpp
            src/utils.cpp
        )
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

set_target_properties(gpos PROPERTIES
                      SOVERSION ${GPORCA_ABI_VERSION}
                      VERSION ${GPORCA_VERSION_STRING})

# Tests reside in the 'server' subdirectory.
add_subdirectory(server)

# Installation.
option(VERBOSE_INSTALL_PATH "Install in a subdirectory path that includes GPOS version, CPU architecture, and bit width" OFF)
if (VERBOSE_INSTALL_PATH)
  set(installpath "libgpos/${GPORCA_VERSION_VERSION}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}")
  string(TOLOWER ${installpath} installpath)
  get_filename_component(full_install_name_dir "${installpath}/lib" ABSOLUTE)
  install(TARGETS gpos DESTINATION "${installpath}/lib")
  install(DIRECTORY include/gpos DESTINATION "${installpath}/include")
  install(FILES "${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h" DESTINATION "${installpath}/include/gpos")
else()
  get_filename_component(full_install_name_dir "${CMAKE_INSTALL_PREFIX}/lib" ABSOLUTE)
  install(TARGETS gpos DESTINATION lib)
  install(DIRECTORY include/gpos DESTINATION include)
  install(FILES "${PROJECT_BINARY_DIR}/libgpos/include/gpos/config.h" DESTINATION include/gpos)
endif()

