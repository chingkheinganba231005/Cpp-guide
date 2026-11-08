#include <string>

const std::string& make_greeting() {
    std::string s = "hello";
    return s;           // s is destroyed when the function returns: the reference dangles
}

int main() {
    const std::string& g = make_greeting();
    return static_cast<int>(g.size());
}
