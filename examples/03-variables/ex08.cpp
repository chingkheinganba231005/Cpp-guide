int main() {
    const double pi = 3.14159;       // cannot be changed after initialisation
    constexpr int days_per_week = 7; // known at compile time
    pi = 3.0;                        // error: assignment of read-only variable
}
