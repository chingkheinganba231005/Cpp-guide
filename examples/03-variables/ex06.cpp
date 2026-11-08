int main() {
    double price = 9.99;
    int a = price;    // compiles: silently drops the .99
    int b{price};     // error: narrowing conversion
}
