#include <iostream>

int main() {
    int* p = new int{42};          // ask the heap for space for one int; get its address
    std::cout << *p << " lives on the heap at " << p << '\n';
    *p += 1;
    delete p;                      // give it back. YOU must remember to do this.
    p = nullptr;                   // good habit: p no longer points anywhere valid

    int n = 5;                     // a run-time size
    double* arr = new double[n]{1, 2, 3, 4, 5};   // an array on the heap
    double sum = 0;
    for (int i = 0; i < n; ++i) sum += arr[i];
    std::cout << "sum " << sum << '\n';
    delete[] arr;                  // arrays need delete[] (not delete)
}
