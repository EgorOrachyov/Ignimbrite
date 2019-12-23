/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_PLATFORM_H
#define IGNIMBRITELIBRARY_PLATFORM_H

#if defined(__APPLE__)
    #define PLATFORM_MACOS
#elif defined(_WIN32)
#   define PLATFORM_WIN
#   include <windows.h>
#elif defined(__linux__)
#   define PLATFORM_LINUX
#else
#   error "Unsupported target platform"
#endif

#endif //IGNIMBRITELIBRARY_PLATFORM_H
