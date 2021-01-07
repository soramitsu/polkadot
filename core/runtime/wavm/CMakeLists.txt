#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(core_wavm
    core.cpp
    )
target_link_libraries(core_wavm
    buffer
    libWAVM
    )

add_library(wavm_wasm_memory
    wasm_memory_impl.cpp
    )
target_link_libraries(wavm_wasm_memory
    libWAVM
    buffer
    )
