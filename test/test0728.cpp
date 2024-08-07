#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

class MyClass {
public:
    int value;
    MyClass(int v) : value(v) {}
    void MyFunction(const MyClass& other) const {
        // 模擬一些操作
        volatile int result = value * other.value;
    }
};

void method1(const std::vector<MyClass>& vec) {
    for (const auto& m : vec) {
        for (const auto& n : vec) {
            m.MyFunction(n);
        }
    }
}

void method2(const std::vector<MyClass>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = 0; j < vec.size(); ++j) {
            vec[i].MyFunction(vec[j]);
        }
    }
}

void method3(const std::vector<MyClass>& vec) {
    std::for_each(vec.begin(), vec.end(), [&](const auto& m) {
        std::for_each(vec.begin(), vec.end(), [&](const auto& n) {
            m.MyFunction(n);
        });
    });
}

// 使用 C++17 結構化綁定的自定義迭代器
class PairIterator {
    const std::vector<MyClass>& vec;
    size_t i = 0, j = 0;
public:
    PairIterator(const std::vector<MyClass>& v) : vec(v) {}
    bool next(const MyClass*& m, const MyClass*& n) {
        if (i >= vec.size()) return false;
        m = &vec[i];
        n = &vec[j];
        if (++j >= vec.size()) {
            ++i;
            j = 0;
        }
        return true;
    }
};

void method4(const std::vector<MyClass>& vec) {
    PairIterator it(vec);
    const MyClass *m, *n;
    while (it.next(m, n)) {
        m->MyFunction(*n);
    }
}

int main() {
    std::vector<MyClass> vec;
    for (int i = 0; i < 1000; ++i) {
        vec.emplace_back(i);
    }

    auto time_method = [&](auto&& method, const char* name) {
        auto start = std::chrono::high_resolution_clock::now();
        method(vec);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff = end - start;
        std::cout << name << " took " << diff.count() << " ms\n";
    };

    time_method(method1, "Range-based for loops");
    time_method(method2, "Traditional for loops");
    time_method(method3, "Parallel std::for_each");
    time_method(method4, "Custom pair iterator");

    return 0;
}