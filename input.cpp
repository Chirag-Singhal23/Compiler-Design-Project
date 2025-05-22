// Sample C++ file with optimization opportunities
#include <iostream>

int main() {
    // Redundant conditions
    int x = 5;
    int y = 5;
    
    // This can be optimized (x == x is always true)
    if (x == x) {
        std::cout << "x equals x" << std::endl;
    }
    
    // Constant folding opportunity
    int a = 10;
    int b = 20;
    int c = a + b;  // Can be optimized to c = 30
    
    // Always true condition
    if (true || x > 3) {
        std::cout << "This is always executed" << std::endl;
    }
    
    // Always false condition
    if (false && y < 10) {
        std::cout << "This is never executed" << std::endl;
    }
    
    // More constant folding opportunities
    int result = 5 * 10 + 20 / 4;  // Can be optimized to result = 55
    
    return 0;
}