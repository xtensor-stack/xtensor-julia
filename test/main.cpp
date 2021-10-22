/***************************************************************************
* Copyright (c) Johan Mabille and Sylvain Corlay                           *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

// Workaround for https://github.com/JuliaLang/julia/issues/34201#issuecomment-817081705.
#ifdef _MSC_VER
    #define NOMINMAX
    #include <uv.h>
    #include <windows.h>

    template<typename T>
    static inline T jl_atomic_load_relaxed(volatile T *obj)
    {
        return jl_atomic_load_acquire(obj);
    }
#endif

#include <julia.h>

#include "gtest/gtest.h"

#include <iostream>

#include <jlcxx/module.hpp>

int main(int argc, char* argv[])
{
    jlcxx::cxxwrap_init();

    ::testing::InitGoogleTest(&argc, argv);

    jlcxx::register_core_types();

    // Run test suite
    int ret = RUN_ALL_TESTS();

    // Closure of the Julia interpreter
    jl_atexit_hook(0);
    return ret;
}

