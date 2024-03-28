# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/PicoVittuSaatana/Pico-SDK-v1.5.0/pico-sdk/tools/pioasm"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pioasm"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Metropolia/Ohjelmointi/Embedded/Lab1/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
