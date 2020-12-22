#
# This module finds if Boost is installed and determines where the
# executables are. 
#
# You should have installed:
#
#  - cmake version 3.11 or greater
#  - phyton 2.7 or greater
#  - phyton-dev / python-devel (needed on linux)
#
# It sets the following variables:
#
#  Boost_FOUND : boolean                - system has Boost
#  Boost_INCLUDE_DIR : list(path)       - the Boost include directories
#  PYTHONLIBS_FOUND : boolean           - system has python
#  PYTHON_VERSION : string              - python version 
#
# If Boost is not found, this module downloads it according to the
# following variables:
#
#  BOOST_REQUESTED_VERSION : string     - the Boost version to be downloaded
#
# You can also specify its components:
#
#  find_package(Boost COMPONENTS program_options system)
#
# which are stored in Boost_ADD_COMPONENTS : list(string)
#
# You can add them with:
#  
#  list(APPEND Boost_ADD_COMPONENTS system)

cmake_minimum_required(VERSION 3.11)

# set requested version of boost to check/download
set(BOOST_REQUESTED_VERSION 1.67.0)

# set variable with underscores instead of dots
string(REPLACE "." "_" BOOST_REQUESTED_VERSION_UNDERSCORE ${BOOST_REQUESTED_VERSION})

# add boost components which you would like to install
list(APPEND Boost_ADD_COMPONENTS system)
list(APPEND Boost_ADD_COMPONENTS filesystem)
list(APPEND Boost_ADD_COMPONENTS log)
list(APPEND Boost_ADD_COMPONENTS program_options)
list(APPEND Boost_ADD_COMPONENTS regex)
list(APPEND Boost_ADD_COMPONENTS python)
list(APPEND Boost_ADD_COMPONENTS test)


function(bash_delete_old_boost)
  
  # set default boost libraries directory
  set (DEFAULT_LIB_DIR /usr/local/lib/)
  
  # add all boost libraries from default directory to list
  file(GLOB LIST_OF_FILES "${DEFAULT_LIB_DIR}*boost*")
  
  # remove old boost libs
  execute_process(
    COMMAND rm -rf ${LIST_OF_FILES}
    WORKING_DIRECTORY ${DEFAULT_LIB_DIR}
  )
  
  # remove old boost files
  execute_process(
    COMMAND rm -rf boost/
    WORKING_DIRECTORY ${DEFAULT_LIB_DIR}../include/
  )
endfunction()

function(powershell_delete_old_boost)
  
  # set default old boost directory
  set (DEFAULT_BOOST_DIR C:\\Boost)
  
  # if folder Boost with old boost files exists 
  if(EXISTS ${DEFAULT_BOOST_DIR})
    
    message(STATUS "Trying to delete old version of boost from: ${DEFAULT_BOOST_DIR}")
      
    # remove old boost
    execute_process(
      COMMAND powershell Remove-Item -Recurse -Force ${DEFAULT_BOOST_DIR}
    )
  endif()
endfunction()

# download boost on linux
function(bash_download_extract PATH)  
  
  # check if boost is properly installed and set ${Boost_INCLUDE_DIR} - REQUIRED parameter - stops build if failed
  find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${Boost_FIND_COMPONENTS})
  
  # return if boost is not found and flag BOOST is not set   
  if((NOT (${BOOST}) AND (NOT ${Boost_FOUND})))
    message(STATUS "Boost not found and flag BOOST not set!")
    message(STATUS "If you want the boost to automatically download use command: ")
    message(STATUS "cmake .. -DBOOST=TRUE")
    return()
  endif()
  
  # if there is no boost installed - download and install it
  if (NOT ${Boost_FOUND})
    
    message(STATUS "Boost not found - I'm trying to download it!")
    # delete old boost files and libs
    bash_delete_old_boost()
    
    # set boost temproary directory
    set(BOOST_TMP_DIR ${PATH}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE})
    
    # execute download file on linux from URL to selected PATH
    set(BOOST_TAR_DIR ${BOOST_TMP_DIR}.tar.gz)

    # clean the target dir
    file(REMOVE_RECURSE ${BOOST_TMP_DIR})

    message(STATUS "Download started - please wait...")
    # download requested version of boost
    file(DOWNLOAD
      ${BOOST_URL}
      ${BOOST_TAR_DIR}
      SHOW_PROGRESS
    )

    message(STATUS "Extract started - please wait...")
    # extract src folder from archive
    execute_process(
      COMMAND tar -C ${PATH} -xf ${BOOST_TAR_DIR}
    )
    
    # prepare boost for installation
    execute_process(
      COMMAND ./bootstrap.sh
      WORKING_DIRECTORY ${BOOST_TMP_DIR}
    )

    # install boost
    execute_process(
      COMMAND ./b2 ${BOOST_COMPONENTS_FOR_BUILD} install
      WORKING_DIRECTORY ${BOOST_TMP_DIR}
    )

    # remove the downloaded package
    file(REMOVE ${BOOST_TAR_DIR})
    
    # check if boost is properly installed and set ${Boost_INCLUDE_DIR} - REQUIRED parameter - stops build if failed
    find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${Boost_FIND_COMPONENTS} REQUIRED)
  else ()
    message(STATUS "Boost found!!")
  endif()
  
  # add boost to include directories
  include_directories(${Boost_INCLUDE_DIR})
endfunction()

# download boost on windows
function(powershell_download_extract PATH)

  # check if boost is properly installed and set ${Boost_INCLUDE_DIR} - REQUIRED parameter - stops build if failed
  find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${Boost_FIND_COMPONENTS})
  
  # return if boost is not found and flag BOOST is not set   
  if((NOT (${BOOST}) AND (NOT ${Boost_FOUND})))
    message(STATUS "Boost not found and flag BOOST not set!")
    message(STATUS "If you want the boost to automatically download use command: ")
    message(STATUS "cmake -G \"Visual Studio 14 2015 Win64\" .. -DBOOST=TRUE")
    return()
  endif()
  
  # if there is no boost installed - download and install it
  if (NOT ${Boost_FOUND})
    message(STATUS "Boost not found - I'm trying to download it!")
    message(STATUS "Operation might take even 3 hours!!!")
    
    # delete old boost files and libs
    powershell_delete_old_boost()
    
    # set boost temproary directory
    set(BOOST_TMP_DIR ${PATH}\\boost_${BOOST_REQUESTED_VERSION_UNDERSCORE})

    # set download zip directory
    set(BOOST_ZIP_DIR ${BOOST_TMP_DIR}.zip)
    
    # clean the target dir
    file(REMOVE_RECURSE ${BOOST_TMP_DIR})
    
    message(STATUS "Download started - please wait...")
    # download requested version of boost
    execute_process(
      COMMAND powershell -Command "(New-Object Net.WebClient).Downloadfile('${BOOST_URL}', '${BOOST_ZIP_DIR}')"
    )    
    
    message(STATUS "Extract started - please wait...")
    # extract src folder from archive
    execute_process(
      COMMAND powershell Expand-Archive -Path ${BOOST_ZIP_DIR} -DestinationPath ${PATH})

    # prepare boost for installation
    execute_process(
      COMMAND .\\bootstrap.bat
      WORKING_DIRECTORY ${BOOST_TMP_DIR}
    )

    # install boost
    execute_process(
      COMMAND .\\b2 ${BOOST_COMPONENTS_FOR_BUILD} install
      WORKING_DIRECTORY ${BOOST_TMP_DIR}
    )

    # remove the downloaded package
    file(REMOVE ${BOOST_ZIP_DIR})
    
    # check if boost is properly installed and set ${Boost_INCLUDE_DIR} - REQUIRED parameter - stops build if failed
    # need to invoke this function to update Boost_INCLUDE_DIR variable before deletePrefixLibFromBoostLibs function
    find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${Boost_FIND_COMPONENTS})
    
    # delete "lib" prefix form each boost library and search libs again with find_package function
    deletePrefixLibFromBoostLibs(${Boost_INCLUDE_DIR}\\..\\..\\lib)
    
    # check if boost is properly installed and set ${Boost_INCLUDE_DIR} - REQUIRED parameter - stops build if failed
    find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${Boost_FIND_COMPONENTS})
    
  else ()
    message(STATUS "Boost found!!")
  endif()
  
  # add boost to include directories
  include_directories(${Boost_INCLUDE_DIR})
endfunction()

# if python is added to boost build - function finds python libraries if not, shows message
function(find_python)
  # first need to find interpreter
  find_package(PythonInterp)
  # then libraries are able to find
  find_package(PythonLibs)
  
  # if python is not installed - stop cmake and print error
  if(NOT PYTHONLIBS_FOUND)
    message(FATAL_ERROR "Phyton libraries are not installed - install it first!!!")
  else()
    # show found version
    message(STATUS "PYTHONLIBS_VERSION_STRING = ${PYTHONLIBS_VERSION_STRING}")
    
    # remove dots from PYTHONLIBS_VERSION_STRING
    string(REPLACE "." "" PYTHON_VER ${PYTHONLIBS_VERSION_STRING})
    
    # get only 2 numbers form python version
    string(SUBSTRING ${PYTHON_VER} 0 2 PYTHON_VERSION_CUTED)
    
    # set global PYTHON_VERSION variable
    set(PYTHON_VERSION ${PYTHON_VERSION_CUTED} PARENT_SCOPE)
  endif()
endfunction()

# rename all libraries that starts with "lib" because cmake automaticly adds prefix to libraries and than windows can't find them and boost don't work
function(deletePrefixLibFromBoostLibs PATH)
  # get all files from pointed directory that starts with "lib" and add them to list
  file(GLOB LIST_OF_FILES "${PATH}//lib*")

  foreach(COMPONENT ${LIST_OF_FILES})
    # find last "/" sign and get name of library after it
    string(REGEX MATCH "[^/]+$" LIB_NAME ${COMPONENT})
    
    # get length of library name
    string(LENGTH ${LIB_NAME} LIB_LENGTH)
    
    # delete first 3 letters "lib" from name of library
    string(SUBSTRING ${LIB_NAME} 3 ${LIB_LENGTH} CUTED_NAME)
    
    # rename library that starts with "lib" because cmake automaticly adds 
    file(RENAME ${COMPONENT} ${COMPONENT}\\..\\${CUTED_NAME})
  endforeach()
endfunction()

# sets BOOST_URL variable depending on system (win/linux) and adding proper extention to url
function(setURL extention)
  # if zip/tar.gz file exists, do not download it
  if(EXISTS ${CMAKE_SOURCE_DIR}\\boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.${extention})
    # set BOOST_URL variable to zip/tar.gz file destination (PARENT_SCOPE - to make BOOST_URL variable global)
    set(BOOST_URL ${CMAKE_SOURCE_DIR}\\boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.${extention} PARENT_SCOPE)
  else()
    # if zip/tar.gz file does not exist, set BOOST_URL to selected version (PARENT_SCOPE - to make BOOST_URL variable global)
    set(BOOST_URL https://netix.dl.sourceforge.net/project/boost/boost/${BOOST_REQUESTED_VERSION}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.${extention} PARENT_SCOPE)
  endif()
endfunction()


# main starts here
# if boost components are not provided - print message and install all boost library
if(NOT Boost_ADD_COMPONENTS)
    message("No COMPONENTS specified for Boost - installing all boost!!") 
endif()

# if boost version is not provided - stop cmake and print error
if(NOT BOOST_REQUESTED_VERSION)
  message(FATAL_ERROR "BOOST_REQUESTED_VERSION is not defined.")
endif()

# Create a list(string) for the build command (e.g. --with-program_options;--with-system) and assigns it to BOOST_COMPONENTS_FOR_BUILD also make list of components to find during function find_package()
foreach(component ${Boost_ADD_COMPONENTS})
  # special cases are python and tests, because names of their libraries are different and than function find_package cant find those libs - we do not add them to BOOST_COMPONENTS_FOR_BUILD
  if(${component} STREQUAL "python")
    # finds if python is installed and sets PYTHON_VERSION variable
    find_python()
    
    # sets list of  to add python to build separately
    list(APPEND Boost_FIND_COMPONENTS ${component}${PYTHON_VERSION})
  endif()
  
  # add library to list of libraries to build
  list(APPEND BOOST_COMPONENTS_FOR_BUILD --with-${component})
  
  # dont add libs "python" or "test" to find list - find_package cant find them
  if(NOT ((${component} STREQUAL "python") OR (${component} STREQUAL "test")))
    # add library to list of libraries to search for
    list(APPEND Boost_FIND_COMPONENTS ${component})
  endif()
endforeach()

if (WIN32)
  # set link to download boost for windows
  setURL("zip")
  
  # download and install boost for windwos
  powershell_download_extract(${CMAKE_SOURCE_DIR})
else()
  # set link to download boost for linux
  setURL("tar.gz")
  
  # download and install boost for linux
  bash_download_extract(${CMAKE_SOURCE_DIR})
endif ()