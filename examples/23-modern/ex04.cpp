#include <iostream>
#include <string>
#include <variant>
#include <vector>

struct NewOrder { int id; double price; int qty; };
struct Cancel   { int id; };
struct Modify   { int id; int new_qty; };
using Message = std::variant<NewOrder, Cancel, Modify>;   // e.g. messages arriving from an exchange

template <class... Fs> struct overloaded : Fs... { using Fs::operator()...; };   // combine lambdas into one visitor

int main() {
    std::vector<Message> feed{NewOrder{1, 101.5, 10}, Modify{1, 7}, Cancel{1}};
    for (const auto& m : feed) {
        std::visit(overloaded{
            [](const NewOrder& o) { std::cout << "new #" << o.id << " " << o.qty << " @ " << o.price << '\n'; },
            [](const Cancel& c)   { std::cout << "cancel #" << c.id << '\n'; },
            [](const Modify& m)   { std::cout << "modify #" << m.id << " -> qty " << m.new_qty << '\n'; },
        }, m);
    }
    std::cout << "sizeof(Message) = " << sizeof(Message) << " bytes (largest alternative + a tag)\n";
}
