# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-src"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-build"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/tmp"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/src/json-populate-stamp"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/src"
  "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/src/json-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/src/json-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/steve/CLionProjects/BG_Raylib/vendor/json-subbuild/json-populate-prefix/src/json-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
