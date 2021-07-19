## Template for a custom hunter configuration
# Useful when there is a need for a non-default version or arguments of a dependency,
# or when a project not registered in soramistu-hunter should be added.
#
# hunter_config(
#     package-name
#     VERSION 0.0.0-package-version
#     CMAKE_ARGS "CMAKE_VARIABLE=value"
# )
#
# hunter_config(
#     package-name
#     URL https://repo/archive.zip
#     SHA1 1234567890abcdef1234567890abcdef12345678
#     CMAKE_ARGS "CMAKE_VARIABLE=value"
# )

hunter_config(
    soralog
    URL https://github.com/soramitsu/soralog/archive/v0.0.9.tar.gz
    SHA1 a5df392c969136e9cb2891d7cc14e3e6d34107d6
    CMAKE_ARGS TESTING=OFF EXAMPLES=OFF EXPOSE_MOCKS=ON
    KEEP_PACKAGE_SOURCES
)

hunter_config(
    libp2p
    URL https://github.com/libp2p/cpp-libp2p/archive/9e3d90f3fe43c23ba8980e3498584a7b15507c5d.tar.gz
    SHA1 ca417cbe83acf056a70445cf521b5b9421cc9644
    CMAKE_ARGS TESTING=OFF EXAMPLES=OFF EXPOSE_MOCKS=ON
    KEEP_PACKAGE_SOURCES
)

# hunter_config(
#     binaryen
#     VERSION release-101
#     URL "https://github.com/WebAssembly/binaryen/archive/refs/tags/version_101.tar.gz"
#     SHA1 3817856cf47413af916d6da1548ec2a0b11c6b56
#     CMAKE_ARGS
#         BUILD_LLVM_DWARF=OFF
#         BUILD_STATIC_LIB=ON
#         ENABLE_WERROR=OFF
#   )
