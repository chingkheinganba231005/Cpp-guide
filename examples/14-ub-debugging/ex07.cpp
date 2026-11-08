int scale(int value, double factor) {
    int value2 = value * factor;      // -Wconversion: double to int loses information
    {
        int value = 3;                // -Wshadow: hides the parameter named value
        return value2 + value;
    }
}

int main() { return scale(2, 1.5); }
