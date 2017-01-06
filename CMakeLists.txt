cmake_minimum_required( VERSION 3.5 )

project( Day22 )

find_package( Boost REQUIRED COMPONENTS context )

add_executable( d22 main.cpp )
target_link_libraries( d22 ${Boost_LIBRARIES} )
  CXX_STANDARD 14
  COMPILE_OPTIONS "-Wall;-Werror"
  INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
)
