cmake_minimum_required( VERSION 3.5 )

project( Day22 )

find_package( Boost REQUIRED COMPONENTS context )

add_executable( d22 main.cpp graph.cpp )

set_target_properties( d22 PROPERTIES
  CXX_STANDARD 14
  COMPILE_OPTIONS "-Wall;-Werror"
  INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
  LINK_LIBRARIES "${Boost_LIBRARIES}"
)
