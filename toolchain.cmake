# --- 1. DEFINICIÓN DEL SISTEMA ---
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# --- 2. ENGAÑAR A GEODE/CMAKE (La clave del éxito) ---
# Forzamos las variables que Geode revisa internamente
set(WIN32 1 CACHE BOOL "Force WIN32" FORCE)
set(MSVC 1 CACHE BOOL "Force MSVC" FORCE)
set(MSVC_VERSION 1930 CACHE STRING "Force MSVC Version (VS2022)" FORCE) # Simulamos VS2022
set(GEODE_TARGET_PLATFORM "win" CACHE STRING "Force Geode Platform" FORCE)

# --- 3. COMPILADORES (Clang-CL) ---
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_RC_COMPILER llvm-rc)
set(CMAKE_AR llvm-lib)
set(CMAKE_LINKER lld-link)

set(TRIPLE x86_64-pc-windows-msvc)
set(CMAKE_C_COMPILER_TARGET ${TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${TRIPLE})

# --- 4. RUTAS Y FLAGS ---
set(XWIN_DIR "$ENV{HOME}/.xwin")

# Flags de compilación (Sintaxis MSVC / clang-cl)
# /TP = Tratar como C++, /MD = Runtime DLL, /O2 = Optimizar
set(COMPILE_FLAGS "-fuse-ld=lld /MD /O2 /Z7 /EHsc /imsvc ${XWIN_DIR}/crt/include /imsvc ${XWIN_DIR}/sdk/include/ucrt /imsvc ${XWIN_DIR}/sdk/include/um /imsvc ${XWIN_DIR}/sdk/include/shared")

set(CMAKE_C_FLAGS "${COMPILE_FLAGS} /TC")
set(CMAKE_CXX_FLAGS "${COMPILE_FLAGS} /TP")

# Flags del Linker
# Nota: nodefaultlib:msvcrtd.lib es vital para evitar el error de Debug
set(LINK_FLAGS "-fuse-ld=lld -libpath:${XWIN_DIR}/crt/lib/x86_64 -libpath:${XWIN_DIR}/sdk/lib/ucrt/x86_64 -libpath:${XWIN_DIR}/sdk/lib/um/x86_64 -nodefaultlib:msvcrtd.lib -defaultlib:msvcrt.lib")

set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)
