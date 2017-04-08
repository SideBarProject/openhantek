#pragma once

#include <vector>
#include <algorithm>

template <class T>
std::vector<T>& operator<<(std::vector<T>& v, T x) {
    v.push_back(x);
    return v;
}

template <class T>
size_t indexOf(const std::vector<T>& vec, const T& v) {
    return std::distance(vec.begin(), find(vec.begin(),vec.end(),v));
}

