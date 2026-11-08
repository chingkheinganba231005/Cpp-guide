#include <iostream>

int main() {
    char cmd = 'q';
    switch (cmd) {
        case 'w': std::cout << "up\n"; break;
        case 's': std::cout << "down\n"; break;
        case 'q':
        case 'Q': std::cout << "quit\n"; break;   // two cases share one action
        default:  std::cout << "unknown\n";
    }
}
