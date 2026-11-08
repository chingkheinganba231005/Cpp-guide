#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include "nn.hpp"

int main() {
    std::mt19937 rng(7);
    std::normal_distribution<double> noise(0, 0.1);
    std::vector<std::vector<double>> X; std::vector<double> y;
    for (int i = 0; i < 100; ++i) {                               // two moons: labels +1 and -1
        double t = 3.14159 * i / 100;
        X.push_back({std::cos(t) + noise(rng), std::sin(t) + noise(rng)});              y.push_back(1);
        X.push_back({1 - std::cos(t) + noise(rng), 0.5 - std::sin(t) + noise(rng)});    y.push_back(-1);
    }
    MLP model({2, 16, 16, 1}, rng);
    auto params = model.params();
    std::cout << "parameters: " << params.size() << '\n';

    for (int epoch = 0; epoch <= 60; ++epoch) {
        Value loss(0.0);
        int correct = 0;
        for (std::size_t i = 0; i < X.size(); ++i) {
            Value score = model({Value(X[i][0]), Value(X[i][1])});
            Value margin = Value(1.0) - score * Value(y[i]);     // hinge loss: max(0, 1 - y*score)
            if (margin.data() > 0) loss = loss + margin;
            correct += (score.data() > 0) == (y[i] > 0);
        }
        Value data_loss = loss * Value(1.0 / X.size());
        for (auto& p : params) p.zero_grad();
        data_loss.backward();
        double lr = 1.0 - 0.9 * epoch / 60.0;                     // learning-rate decay
        for (auto& p : params) p.step(lr);
        if (epoch % 10 == 0) std::cout << "epoch " << epoch << "  loss " << data_loss.data() << "  accuracy " << correct / 2.0 << "%\n";
    }
}
