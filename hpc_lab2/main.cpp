#include <omp.h>

#include <math.h>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using chasiki = std::chrono::high_resolution_clock;

omp_lock_t writelock;

int get_ioprec(int idx) {
    int ioprec[7] = {13, 11, 13, 14, 13, 11, 12};
    return ioprec[idx];
}

double get_prec(int idx) {
    double prec[7] = {2.77e-11, 1.9e-10, 2.05e-11, 2.22e-12,
                      8.67e-11, 6e-11,   6.3e-11};
    return prec[idx];
}

enum class function_ver {
    REDUCTION,
    LOCK,
    LOCK_CHUNK,
    CIRITCAL,
    CRITICAL_CHUNK,
    ATOMIC,
    ATOMIC_CHUNK
};

void get_enum_name(function_ver mode_, std::string& name) {
    std::string names[7] = {"REDUCTION",   "LOCK",           "LOCK_CHUNK",
                            "CRITICAL",    "CRITICAL_CHUNK", "ATOMIC",
                            "ATOMIC_CHUNK"};
    name = names[static_cast<int>(mode_)];
}

// function
inline double func(double x) {
    return (1 / (x * x)) * sin(1 / x) * sin(1 / x);
}

double trapezoidal_rule_reduction(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel for firstprivate(pnum) private(i) schedule(dynamic, chunk) \
    reduction(+: result)
    for(i = 1; i <= pnum; ++i) {
        result += func(a + step * i);
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_lock(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel for firstprivate(pnum) private(i) schedule(dynamic, chunk)
    for(i = 1; i <= pnum; ++i) {
        omp_set_lock(&writelock);
        result += func(a + step * i);
        omp_unset_lock(&writelock);
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_lock_chunk(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel shared(result) num_threads(omp_get_max_threads())
    {
        double local_result_ = 0;
#pragma omp for schedule(dynamic, chunk) nowait
        for(i = 1; i < pnum; ++i)
            local_result_ += func(a + step * i);
        omp_set_lock(&writelock);
        result += local_result_;
        omp_unset_lock(&writelock);
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_critical(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel for firstprivate(pnum) private(i) schedule(dynamic, chunk)
    for(i = 1; i <= pnum; ++i) {
#pragma omp critical
        result += func(a + step * i);
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_critical_chunk(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel shared(result) num_threads(omp_get_max_threads())
    {
        double local_result_ = 0;
#pragma omp for schedule(dynamic, chunk) nowait
        for(i = 1; i < pnum; ++i)
            local_result_ += func(a + step * i);
#pragma omp critical
        result += local_result_;
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_atomic(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel for firstprivate(pnum) private(i) schedule(dynamic, chunk)
    for(i = 1; i <= pnum; ++i) {
#pragma omp atomic
        result += func(a + step * i);
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

double trapezoidal_rule_atomic_chunk(int pnum, double a, double b) {
    double result = 0, step = (b - a) / pnum;
    int i = 0;
    pnum -= 1;
    int chunk = pnum / omp_get_max_threads();
#pragma omp parallel shared(result) num_threads(omp_get_max_threads())
    {
        double local_result_ = 0;
#pragma omp for schedule(dynamic, chunk) nowait
        for(i = 1; i < pnum; ++i)
            local_result_ += func(a + step * i);
#pragma omp atomic
        result += local_result_;
    }

    return (b - a) / (pnum + 1) * ((func(a) + func(b)) / 2 + result);
}

int main(int argc, char* argv[]) {
    int threads_num, pnum, prec_idx, iteration_counter = 0;
    double a, b, result = 0, result_ = 0, precision = 0;
    std::string mode_;

    if(argc < 7) {
        return 0;
    }

    omp_init_lock(&writelock);

    threads_num = std::stoi(argv[1]);
    a = std::stod(argv[2]);
    b = std::stod(argv[3]);
    pnum = std::stoi(argv[4]);
    prec_idx = std::stoi(argv[5]);
    auto parallel_mode_ = static_cast<function_ver>(std::stoi(argv[6]));

    std::function<double(int, double, double)> trapezoidal_rule =
        trapezoidal_rule_reduction;

    switch(parallel_mode_) {
    case function_ver::LOCK:
        trapezoidal_rule = trapezoidal_rule_lock;
        break;
    case function_ver::LOCK_CHUNK:
        trapezoidal_rule = trapezoidal_rule_lock_chunk;
        break;
    case function_ver::CIRITCAL:
        trapezoidal_rule = trapezoidal_rule_critical;
        break;
    case function_ver::CRITICAL_CHUNK:
        trapezoidal_rule = trapezoidal_rule_critical_chunk;
        break;
    case function_ver::ATOMIC:
        trapezoidal_rule = trapezoidal_rule_atomic;
        break;
    case function_ver::ATOMIC_CHUNK:
        trapezoidal_rule = trapezoidal_rule_atomic_chunk;
        break;
    case function_ver::REDUCTION:
    default:
        parallel_mode_ = function_ver::REDUCTION;
        break;
    }

    precision = get_prec(prec_idx);

    omp_set_dynamic(0);
    omp_set_num_threads(threads_num);

    result = trapezoidal_rule(pnum, a, b);
    result_ = 0;

    auto t1 = chasiki::now();
    auto t2 = t1;

    for(;;) {
        ++iteration_counter;
        pnum *= 2;
        t1 = chasiki::now();
        result_ = trapezoidal_rule(pnum, a, b);
        t2 = chasiki::now();
        if(abs(result - result_) / abs(result_) <= precision) {
            result = result_;
            break;
        }
        std::cout << ". . . patience . . ." << std::endl;
        result = result_;
    }

    std::cout << "threads num: " << omp_get_max_threads() << std::endl;
    std::cout << "iteration passed: " << iteration_counter << std::endl;
    std::cout << "final points number: " << pnum << std::endl;
    get_enum_name(parallel_mode_, mode_);
    std::cout << "parallel mode: " << mode_ << std::endl;
    std::cout << "result: " << std::setprecision(get_ioprec(prec_idx))
              << std::fixed << result << std::endl;
    auto duration = t2 - t1;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout << "Execution time: " << seconds << " sec (" << milliseconds
              << " ms)" << std::endl;

    return 0;
}
