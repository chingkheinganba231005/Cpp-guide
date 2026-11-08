#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct Test { std::string name; std::function<void()> fn; };
std::vector<Test>& registry() { static std::vector<Test> r; return r; }
struct Register { Register(std::string n, std::function<void()> f) { registry().push_back({std::move(n), std::move(f)}); } };
struct Failure { std::string what; };

#define TEST(name) void name(); static Register reg_##name(#name, name); void name()
#define CHECK(cond) do { if (!(cond)) throw Failure{std::string(#cond) + " (line " + std::to_string(__LINE__) + ")"}; } while (0)

// --- the code under test ---
long long gcd(long long a, long long b) { return b == 0 ? (a < 0 ? -a : a) : gcd(b, a % b); }
double mean(const std::vector<double>& v) { double s = 0; for (double x : v) s += x; return v.empty() ? 0 : s / v.size(); }

// --- the tests ---
TEST(gcd_basic) { CHECK(gcd(12, 18) == 6); CHECK(gcd(17, 5) == 1); }
TEST(gcd_edge_cases) { CHECK(gcd(0, 5) == 5); CHECK(gcd(-12, 18) == 6); }
TEST(mean_values) { CHECK(std::abs(mean({1, 2, 3, 4}) - 2.5) < 1e-12); CHECK(mean({}) == 0); }
TEST(deliberately_failing) { CHECK(gcd(10, 4) == 3); }          // shows what a failure looks like

int main() {
    int failed = 0;
    for (const auto& t : registry()) {
        try { t.fn(); std::cout << "[ PASS ] " << t.name << '\n'; }
        catch (const Failure& f) { ++failed; std::cout << "[ FAIL ] " << t.name << ": " << f.what << '\n'; }
    }
    std::cout << registry().size() - failed << " passed, " << failed << " failed\n";
}
