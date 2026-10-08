#pragma once
#include <cstdio>
#include <cstdlib>
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            std::exit(1);                                                        \
        }                                                                        \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                                       \
    do {                                                                            \
        double va_ = (a);                                                           \
        double vb_ = (b);                                                           \
        double vt_ = (tol);                                                         \
        if (!((va_ - vb_) <= vt_ && (vb_ - va_) <= vt_)) {                          \
            std::fprintf(stderr, "FAIL %s:%d: |%s - %s| = |%g - %g| > %g\n",        \
                         __FILE__, __LINE__, #a, #b, va_, vb_, vt_);                \
            std::exit(1);                                                           \
        }                                                                           \
    } while (0)
