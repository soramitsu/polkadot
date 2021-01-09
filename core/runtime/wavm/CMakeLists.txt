#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(wavm_external_interface
    wavm_external_interface.cpp
    )
target_link_libraries(wavm_external_interface
    libWAVM
    buffer
    logger
    )

add_library(core_wavm
    core.cpp
    )
target_link_libraries(core_wavm
    buffer
    libWAVM
    scale
    wavm_wasm_memory
    )

add_library(wavm_wasm_memory
    wasm_memory_impl.cpp
    )
target_link_libraries(wavm_wasm_memory
    libWAVM
    buffer
    )
