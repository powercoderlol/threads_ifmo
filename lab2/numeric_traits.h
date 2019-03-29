#pragma once
#include <cmath>
#include <type_traits>

inline double decimal_degree(int degree) {
    const static double pow_10[] = {1e-006, 1e-005, 0.0001, 0.001, 0.01,
                                    0.1,    1,      10,     100,   1000,
                                    10000,  100000, 1e+006};
    auto idx = static_cast<unsigned int>(degree + 6);
    return (idx < 13) ? pow_10[idx] : pow(10.0, degree);
}

inline double floor_round(double v, int precision) {
    double power = decimal_degree(precision);
    return ::floor(v * power) / power;
}

inline double math_round(double v, int precision) {
    double power = decimal_degree(precision);
    return ::floor((v * power) + 0.5) / power;
}

inline double math_ceil_round(double v, int precision) {
    v = math_round(v, precision + 1);
    double power = decimal_degree(precision);
    return ::ceil(v * power) / power;
}

inline double ceil_round(double v, int precision) {
    return math_ceil_round(v, precision);
}

enum class round_type_t { none, math, ceil, floor };
inline double round(double v, int precision, round_type_t rndt) {
    double power = decimal_degree(precision);
    switch(rndt) {
    case round_type_t::none:
        return v;
    case round_type_t::math:
        return ::floor((v * power) + 0.5) / power;
    case round_type_t::ceil:
        return ::ceil(v * power) / power;
    case round_type_t::floor:
        return ::floor(v * power) / power;
    default:
        return v;
    }
}
