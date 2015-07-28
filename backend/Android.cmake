SET(ANDROID_NDK "${PROJECT_SOURCE_DIR}/toolchains/android/ndk")
SET(ANDROID_BINS "${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/")

INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME Android)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(${ANDROID_BINS}/arm-linux-androideabi-gcc AndroidABI)
CMAKE_FORCE_CXX_COMPILER(${ANDROID_BINS}/arm-linux-androideabi-g++ AndroidABI)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH "${ANDROID_NDK}/toolchains/android/ndk/platforms/android-21/arch-arm" "${ANDROID_BINS}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
