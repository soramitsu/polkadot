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

hunter_config(libp2p
    URL  https://github.com/libp2p/cpp-libp2p/archive/ceff681e0b3dbfab4792be3dabcb064fff14a8ff.tar.gz
    SHA1 78fb38f212fa49029aff24c669a39648d9b4e68b
    CMAKE_ARGS TESTING=OFF EXAMPLES=OFF
    )
