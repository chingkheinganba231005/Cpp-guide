PyTorch, TensorFlow and JAX are Python on the outside and C++ (plus CUDA) on the inside. In this project you build the heart of such a library: **automatic differentiation** (autograd), a **neural network** made of layers, and **gradient descent** training. Then you'll see how real libraries turn this into fast tensor code.

## The idea: derivatives by the chain rule, automatically

Training a network means adjusting its weights to make a **loss** smaller, and for that we need the derivative (gradient) of the loss with respect to every weight. Autograd computes all of them in one backward pass:

1. **Forward**: compute the result and record each operation in a graph (who was computed from what, and how).
2. **Backward**: start from the loss with gradient 1, walk the graph in reverse, and at each operation apply the chain rule to pass gradients to its inputs.

:::eli5
Think of a factory line where each machine slightly changes a product. If the final product is too heavy, the backward pass asks each machine, from the last to the first, “how much did your setting contribute to the extra weight?”. Each machine only needs to know its own local effect and the answer from the machine after it.
:::

For `c = a * b`, the local derivatives are ∂c/∂a = b and ∂c/∂b = a. So in the backward pass, `a.grad += b * c.grad` and `b.grad += a * c.grad`. Every operation needs only this kind of local rule.

## A scalar autograd engine

This design follows Andrej Karpathy's *micrograd*. Each `Value` holds a number, its gradient, its parents, and a small function that pushes its gradient back to them:

```cpp file=autograd.hpp
#pragma once
#include <cmath>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

struct Node {
    double data = 0, grad = 0;
    std::vector<std::shared_ptr<Node>> parents;
    std::function<void()> backward_fn = [] {};     // pushes this node's grad to its parents
};

class Value {                                       // a cheap handle; the graph is shared
public:
    Value(double x = 0) : n_(std::make_shared<Node>()) { n_->data = x; }
    double data() const { return n_->data; }
    double grad() const { return n_->grad; }
    void step(double lr) { n_->data -= lr * n_->grad; }   // gradient descent update
    void zero_grad() { n_->grad = 0; }

    friend Value operator+(const Value& a, const Value& b) {
        Value out(a.data() + b.data());
        out.n_->parents = {a.n_, b.n_};
        Node *pa = a.n_.get(), *pb = b.n_.get(), *po = out.n_.get();
        out.n_->backward_fn = [=] { pa->grad += po->grad; pb->grad += po->grad; };                 // d(a+b)/da = 1
        return out;
    }
    friend Value operator*(const Value& a, const Value& b) {
        Value out(a.data() * b.data());
        out.n_->parents = {a.n_, b.n_};
        Node *pa = a.n_.get(), *pb = b.n_.get(), *po = out.n_.get();
        out.n_->backward_fn = [=] { pa->grad += pb->data * po->grad; pb->grad += pa->data * po->grad; };   // d(ab)/da = b
        return out;
    }
    friend Value operator-(const Value& a, const Value& b) { return a + b * Value(-1.0); }
    Value tanh() const {
        double t = std::tanh(data());
        Value out(t);
        out.n_->parents = {n_};
        Node *pa = n_.get(), *po = out.n_.get();
        out.n_->backward_fn = [=] { pa->grad += (1 - t * t) * po->grad; };      // d tanh(x)/dx = 1 - tanh^2
        return out;
    }

    void backward() {                               // reverse-mode autodiff from this (scalar) output
        std::vector<Node*> order;
        std::unordered_set<Node*> seen;
        std::function<void(Node*)> visit = [&](Node* v) {      // topological sort: parents before children
            if (!seen.insert(v).second) return;
            for (auto& p : v->parents) visit(p.get());
            order.push_back(v);
        };
        visit(n_.get());
        n_->grad = 1.0;
        for (auto it = order.rbegin(); it != order.rend(); ++it) (*it)->backward_fn();
    }
private:
    std::shared_ptr<Node> n_;
};
```

A gradient check: compare autograd's answer with a numerical estimate from a tiny nudge. This is the standard test for any autograd implementation.

```cpp run with=autograd.hpp
#include <iostream>
#include "autograd.hpp"

double f(double a, double b) { return std::tanh(a * b + a) * b; }

int main() {
    Value a(0.7), b(-1.3);
    Value out = (a * b + a).tanh() * b;
    out.backward();
    double h = 1e-6;
    double da = (f(0.7 + h, -1.3) - f(0.7 - h, -1.3)) / (2 * h);    // central difference
    double db = (f(0.7, -1.3 + h) - f(0.7, -1.3 - h)) / (2 * h);
    std::cout << "f = " << out.data() << '\n';
    std::cout << "df/da autograd " << a.grad() << "  numeric " << da << '\n';
    std::cout << "df/db autograd " << b.grad() << "  numeric " << db << '\n';
}
```

## A neural network

A **neuron** computes tanh(w · x + b). A **layer** is several neurons, and a **multi-layer perceptron (MLP)** is layers in sequence. Train it with gradient descent: forward, compute the loss, zero the gradients, backward, then step each parameter against its gradient.

```cpp file=nn.hpp
#pragma once
#include <random>
#include <vector>
#include "autograd.hpp"

class Neuron {
public:
    Neuron(int nin, std::mt19937& rng, bool linear = false) : linear_(linear) {
        std::uniform_real_distribution<double> d(-1, 1);
        for (int i = 0; i < nin; ++i) w_.emplace_back(d(rng));
    }
    Value operator()(const std::vector<Value>& x) const {
        Value act = b_;
        for (std::size_t i = 0; i < w_.size(); ++i) act = act + w_[i] * x[i];
        return linear_ ? act : act.tanh();
    }
    void params(std::vector<Value>& out) { out.insert(out.end(), w_.begin(), w_.end()); out.push_back(b_); }
private:
    std::vector<Value> w_;
    Value b_{0.0};
    bool linear_;
};

class MLP {
public:
    MLP(std::vector<int> sizes, std::mt19937& rng) {
        for (std::size_t l = 0; l + 1 < sizes.size(); ++l) {
            layers_.emplace_back();
            for (int j = 0; j < sizes[l + 1]; ++j) layers_.back().emplace_back(sizes[l], rng, l + 2 == sizes.size());
        }
    }
    Value operator()(std::vector<Value> x) const {
        for (const auto& layer : layers_) {
            std::vector<Value> next;
            for (const auto& n : layer) next.push_back(n(x));
            x = std::move(next);
        }
        return x[0];
    }
    std::vector<Value> params() { std::vector<Value> p; for (auto& l : layers_) for (auto& n : l) n.params(p); return p; }
private:
    std::vector<std::vector<Neuron>> layers_;
};
```

Now train it to separate two interleaved half-moons, a dataset that a straight line cannot separate:

```cpp run with=autograd.hpp,nn.hpp
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
```

The loss falls and the accuracy climbs towards 100%: the network learns a curved boundary between the moons. You have written a real (if tiny) deep-learning framework.

## From scalars to tensors: how real libraries are fast

A scalar graph is perfect for learning but slow: every multiplication allocates a node. Real frameworks apply the same chain rule to **tensors**, multi-dimensional arrays with one graph node per *operation on a whole array*:

| Concept | In a real library |
|---|---|
| **Storage** | one contiguous buffer (a CPU vector or GPU memory) holding all the numbers |
| **Shape and strides** | how indices map to the buffer: `offset = i * stride0 + j * stride1`. Transposes and slices just change strides, without copying |
| **Operators** | kernels such as `matmul`, `add` and `relu`, each with a forward and a backward kernel |
| **Backward of matmul** | if Y = X W, then dX = dY Wᵀ and dW = Xᵀ dY: matrix multiplications again, which is why matmul speed is everything (chapter 31) |
| **Dispatch** | choose a CPU or CUDA kernel by device and data type |
| **Memory** | a caching allocator reuses buffers, as in chapter 27 |

A tensor's index mapping in a few lines:

```cpp run
#include <array>
#include <iostream>
#include <vector>

struct Tensor2D {
    std::vector<float> storage;
    std::array<std::size_t, 2> shape, stride;
    std::size_t offset = 0;
    float& operator()(std::size_t i, std::size_t j) { return storage[offset + i * stride[0] + j * stride[1]]; }
    Tensor2D transposed() const { return {storage, {shape[1], shape[0]}, {stride[1], stride[0]}, offset}; }   // no data moved
};

int main() {
    Tensor2D a{{1, 2, 3, 4, 5, 6}, {2, 3}, {3, 1}};      // a 2x3 matrix, row-major
    Tensor2D t = a.transposed();                          // 3x2: the same numbers, strides swapped
    for (std::size_t i = 0; i < t.shape[0]; ++i) { for (std::size_t j = 0; j < t.shape[1]; ++j) std::cout << t(i, j) << ' '; std::cout << '\n'; }
}
```

## Plugging into PyTorch

When you need a custom operation in production, you write it in C++ (and CUDA) and register it with PyTorch as a **C++ extension**:

```cpp norun
#include <torch/extension.h>

torch::Tensor fused_gelu(torch::Tensor x) {                 // your own operator, callable from Python
    return 0.5 * x * (1 + torch::tanh(0.79788456 * (x + 0.044715 * x * x * x)));
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("fused_gelu", &fused_gelu); }
```

`torch.utils.cpp_extension.load(name="ext", sources=["ext.cpp"])` compiles it, and then `ext.fused_gelu(t)` works on PyTorch tensors. The same pybind11 approach exposes any C++ library to Python.

:::exercise Extend the engine
1. Add `relu`, `exp`, division and powers, each with its backward rule, and check each one with a numerical gradient test.
2. Replace the hinge loss with binary cross-entropy using a sigmoid output.
3. Measure training time, then build a `Matrix`-based linear layer (forward Y = XW + b, and the backward formulas from the table) and compare its speed with the scalar engine.
---solution---
The backward rules: relu: `pa->grad += (pa->data > 0) * po->grad`; exp: `pa->grad += out_data * po->grad`; a / b: `pa->grad += po->grad / b`, `pb->grad -= po->grad * a / (b * b)`; pow(a, k): `pa->grad += k * pow(a, k - 1) * po->grad`. The matrix version is typically hundreds of times faster, because it does one graph node and one tight loop per layer instead of millions of small allocations.
:::

::quiz
? In reverse-mode autodiff, in what order are the backward functions called?
+ Reverse topological order: from the output back to the inputs
- Forward order
- Random order
> Each node needs its output's gradient to be final before passing it on.
? If Y = X W, what is the gradient of the loss with respect to W?
+ Xᵀ dY
- dY Wᵀ
- X dY
> And dX = dY Wᵀ. Both are matrix multiplications.
? How does a tensor library transpose a matrix without copying?
+ By swapping the shape and strides that map indices into the buffer
- It always copies
- With the GPU
> Views share storage and differ only in how they index it.
::
