int* make_number() {
    int n = 42;
    return &n;       // returning the address of a local variable: it dies right here
}

int main() {
    int* p = make_number();
    return *p;
}
