consteval int checked_port(int p) {
    if (p < 1 || p > 65535) throw "port out of range";   // reaching a throw at compile time = compile error
    return p;
}

int main() {
    constexpr int ok = checked_port(8080);
    constexpr int bad = checked_port(70000);   // rejected while compiling
    return ok + bad;
}
