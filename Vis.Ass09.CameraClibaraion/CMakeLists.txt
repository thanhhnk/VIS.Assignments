cmake_minimum_required (VERSION 2.8)

# This is the name of the project
PROJECT( assignment9)

# This is for recursively searching for other directories.
# add_subdirectory (DirectoryName)

# sources to include
include_directories (.)
FIND_PACKAGE( OpenCV REQUIRED )
ADD_EXECUTABLE( assignment9  main.cpp)
TARGET_LINK_LIBRARIES( assignment9 ${OpenCV_LIBS} )


