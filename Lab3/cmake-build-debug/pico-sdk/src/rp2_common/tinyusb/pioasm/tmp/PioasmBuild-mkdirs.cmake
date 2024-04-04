# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "H:/Metropolia/IOT/Pico/Pico-SDK-v1.5.0/pico-sdk/tools/pioasm"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pioasm"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "H:/Metropolia/IOT/Embedded/Lab3/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
