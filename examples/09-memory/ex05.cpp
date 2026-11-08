#include <iostream>
#include <vector>

int global_counter = 0;                       // static data

int main() {
    int local = 1;                            // stack
    std::vector<int> v{1, 2, 3};              // the vector object itself: stack; its elements: heap
    static int calls = 0;                     // static data
    std::cout << "code   (main)        " << reinterpret_cast<void*>(&main) << '\n'
              << "static (global)      " << &global_counter << '\n'
              << "static (calls)       " << &calls << '\n'
              << "heap   (v's data)    " << v.data() << '\n'
              << "stack  (local)       " << &local << '\n';
}
