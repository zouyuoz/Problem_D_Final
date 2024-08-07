#include <iostream>
#include <cmath>
#include <iomanip>

// 使用絕對和相對誤差的組合
bool areAlmostEqual(double a, double b, double epsilon = 1e-9) {
    double diff = std::abs(a - b);
    if (diff <= epsilon)
        return true;
    
    return diff <= epsilon * std::max(std::abs(a), std::abs(b));
}

int main() {
    double a = 4773.999;
    double b = 4774.000;
    
    std::cout << std::setprecision(15);
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "abs(a - b) = " << std::abs(a - b) << std::endl;
    
    bool delta1 = std::abs(a - b) <= 0.001;
    bool delta2 = areAlmostEqual(a, b, 0.001);
    
    std::cout << "Using simple comparison: " << std::boolalpha << delta1 << std::endl;
    std::cout << "Using improved comparison: " << std::boolalpha << delta2 << std::endl;

    return 0;
}