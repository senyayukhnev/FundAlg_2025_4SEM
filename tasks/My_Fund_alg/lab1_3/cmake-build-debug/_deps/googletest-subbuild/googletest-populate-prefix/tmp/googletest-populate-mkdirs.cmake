# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-src"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-build"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/c/Users/senya/CLionProjects/fund_alg_4sem/lab1_3/cmake-build-debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
