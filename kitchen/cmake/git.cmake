if(${CMAKE_VERSION} VERSION_LESS 2.8.3)
  # The module defines the following variables:
  #   GIT_EXECUTABLE - path to git command line client
  #   GIT_FOUND - true if the command line client was found
  # Example usage:
  #   find_package(Git)
  #   if(GIT_FOUND)
  #     message("git found: ${GIT_EXECUTABLE}")
  #   endif()

  #=============================================================================
  # Copyright 2010 Kitware, Inc.
  #
  # Distributed under the OSI-approved BSD License (the "License");
  # see accompanying file Copyright.txt for details.
  #
  # This software is distributed WITHOUT ANY WARRANTY; without even the
  # implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  # See the License for more information.
  #=============================================================================
  # (To distribute this file outside of CMake, substitute the full
  #  License text for the above reference.)

  # Look for 'git' or 'eg' (easy git)
  #
  set(git_names git eg)

  # Prefer .cmd variants on Windows unless running in a Makefile
  # in the MSYS shell.
  #
  if(WIN32)
    if(NOT CMAKE_GENERATOR MATCHES "MSYS")
      set(git_names git.cmd git eg.cmd eg)
    endif()
  endif()

  find_program(GIT_EXECUTABLE
    NAMES ${git_names}
    DOC "git command line client"
    )
  mark_as_advanced(GIT_EXECUTABLE)

  # Handle the QUIETLY and REQUIRED arguments and set GIT_FOUND to TRUE if
  # all listed variables are TRUE

  find_package(PackageHandleStandardArgs)
  find_package_handle_standard_args(Git DEFAULT_MSG GIT_EXECUTABLE)

else()
  find_package(Git)
endif()

macro(git_status PROJECT)
  if (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -n1 --pretty=format:%H
      WORKING_DIRECTORY ${${PROJECT}_SOURCE_DIR}
      OUTPUT_VARIABLE ${PROJECT}_COMMITHASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE ${PROJECT}_COMMITHASH_STATUS
      )

    execute_process(COMMAND ${GIT_EXECUTABLE} log -n1 --pretty=format:%cD
      WORKING_DIRECTORY ${${PROJECT}_SOURCE_DIR}
      OUTPUT_VARIABLE ${PROJECT}_LAST_MODIFIED
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE ${PROJECT}_TREEHASH_STATUS
      )

    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
      WORKING_DIRECTORY ${${PROJECT}_SOURCE_DIR}
      OUTPUT_VARIABLE ${PROJECT}_GITTAG
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE ${PROJECT}_GITTAG_STATUS
      )

  else()
    set(${PROJECT}_COMMITHASH treehash_unavailable)
    set(${PROJECT}_LAST_MODIFIED lastmod_unavailable)
    set(${PROJECT}_GITTAG   tag_unavailable)
  endif()

  message(STATUS "${PROJECT} commit:       ${${PROJECT}_COMMITHASH}")
  message(STATUS "${PROJECT} tag:          ${${PROJECT}_GITTAG}")
  message(STATUS "${PROJECT} last_mod:     ${${PROJECT}_LAST_MODIFIED}")


endmacro()

