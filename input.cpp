#include <iostream>
int main() {
    int x ;
    cin>>x;
    int y = 10;
    cout<<x;
    // This for loop will be eliminated (false condition)
    for (int i = 0; false; i++) {
        std::cout << "This will never execute" << std::endl;
    }

    // Normal for loop
    for (int i = 0; i < 10; i++) {
        std::cout << "Loop iteration: " << i << std::endl;
    }

    // While loop with false condition (will be eliminated)
    while (false) {
        std::cout << "Dead code" << std::endl;
    }

    // While loop with constant condition
    int counter = 0;
    while (counter < 5) {
        std::cout << "Counter: " << counter << std::endl;
        counter++;
    }

    // Do-while with false condition (body executes once)
    do {
        std::cout << "Execute once" << std::endl;
    } while (false);

    // Constant folding in expressions
    int a = 5 + 3;
    int b = a * 2;

    // Redundant condition
    if (x == x) {
        std::cout << "Always true" << std::endl;
    }

    return 0;
}