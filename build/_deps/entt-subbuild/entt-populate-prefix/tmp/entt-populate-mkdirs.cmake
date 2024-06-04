# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-src"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-build"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/tmp"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/src/entt-populate-stamp"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/src"
  "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/src/entt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/src/entt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/steve/CLionProjects/BG_Raylib/build/_deps/entt-subbuild/entt-populate-prefix/src/entt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
