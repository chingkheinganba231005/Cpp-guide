#include <iostream>
#include <string>

class Animal {
public:
    explicit Animal(std::string name) : name_(std::move(name)) {}
    const std::string& name() const { return name_; }
    void sleep() const { std::cout << name_ << " is sleeping\n"; }
private:
    std::string name_;
};

class Dog : public Animal {           // Dog IS-AN Animal: it gets name() and sleep()
public:
    explicit Dog(std::string name) : Animal(std::move(name)) {}   // construct the base part first
    void fetch() const { std::cout << name() << " fetches the ball\n"; }
};

int main() {
    Dog d("Rex");
    d.sleep();       // inherited
    d.fetch();       // its own
}
