#pragma once
#include <functional>
#include <math.h>

double integrate(
    const std::function<double(double)>& func, const double a, const double b,
    const int n) {
    double step = (b - a) / n;

    double result = (func(a) + func(b)) * step / 2;
    for(int i = 0; i < n; ++i) {
        result += func(a + i * step) * step;
    }

    return result;
}


// execution function
double liner(double x) {
    return x;
}

double func(double x) {

}

double integrate(const double a, const double b, const int n) {
    double step = (b - a) / n;

    double result = (liner(a) + liner(b)) * step / 2;
    for(int i = 0; i < n; ++i) {
        result += liner(a + i * step) * step;
    }

    return result;
}
