#pragma once
// let's make qsort great again
// there is simple fp-like implementation of qsort algorithm:
// get list by value and return by value not like std::sort()
#include <algorithm>
#include <list>
#include <vector>

namespace algo {

template<class T>
std::list<T> qsort(std::list<T> input) {
    if(input.empty())
        return input;
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& mediana = *result.begin();
    auto divider = std::partition(
        input.begin(), input.end(),
        [&](T const& val) { return val < mediana; });

    std::list<T> less;
    less.splice(less.end(), input, input.begin(), divider);

    auto new_less(qsort(std::move(less)));
    auto more(qsort(std::move(input)));

    result.splice(result.end(), more);
    result.splice(result.begin(), new_less);
    return result;
}


} // namespace algo
