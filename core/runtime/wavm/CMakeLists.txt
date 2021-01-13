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

add_library(wavm_module_provider
    module_provider.cpp
    )
target_link_libraries(wavm_module_provider
    libWAVM
    buffer
    blob
    logger
    scale
    wavm_wasm_memory
    )

add_library(wavm_runtime_api
    wavm_runtime_api.cpp
    )
target_link_libraries(wavm_runtime_api
    wavm_module_provider
    )

add_library(core_wavm
    core.cpp
    )
target_link_libraries(core_wavm
    wavm_runtime_api
    )

add_library(grandpa_api_wavm
    grandpa_api.cpp
    )
target_link_libraries(grandpa_api_wavm
    wavm_runtime_api
    )

add_library(wavm_wasm_memory
    wasm_memory_impl.cpp
    )
target_link_libraries(wavm_wasm_memory
    libWAVM
    buffer
    )
