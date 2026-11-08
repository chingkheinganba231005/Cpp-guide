#include <cstddef>
#include <iostream>
#include <vector>

class Matrix {
public:
    Matrix(std::size_t r, std::size_t c) : rows_(r), cols_(c), data_(r * c, 0.0) {}
    double& operator()(std::size_t r, std::size_t c) { return data_[r * cols_ + c]; }              // m(r, c) = ...
    double operator()(std::size_t r, std::size_t c) const { return data_[r * cols_ + c]; }         // read-only version
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
private:
    std::size_t rows_, cols_;
    std::vector<double> data_;     // row-major, as in chapter 7
};

Matrix operator*(const Matrix& a, const Matrix& b) {
    Matrix c(a.rows(), b.cols());
    for (std::size_t i = 0; i < a.rows(); ++i)
        for (std::size_t k = 0; k < a.cols(); ++k)
            for (std::size_t j = 0; j < b.cols(); ++j)
                c(i, j) += a(i, k) * b(k, j);
    return c;
}

int main() {
    Matrix a(2, 2), b(2, 2);
    a(0, 0) = 1; a(0, 1) = 2; a(1, 0) = 3; a(1, 1) = 4;
    b(0, 0) = 0; b(0, 1) = 1; b(1, 0) = 1; b(1, 1) = 0;
    Matrix c = a * b;
    std::cout << c(0, 0) << ' ' << c(0, 1) << '\n' << c(1, 0) << ' ' << c(1, 1) << '\n';
}
