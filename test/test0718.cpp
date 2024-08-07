#include <iostream>
#include <vector>
#include <memory>


class MyClass {
public:
    MyClass(int v) : value(v) {}
    MyClass(const MyClass& other) : value(other.value) {
        std::cout << "Copy constructor called" << std::endl;
    }

	void setValue(int const &val) { value = val; }
    int display() const { return value; }
private:
    int value;
};

using namespace std;

int main() {
	vector<MyClass> b;
	{
    vector<MyClass> a = { MyClass(8), MyClass(9), MyClass(7), MyClass(2), MyClass(4) };
	b = std::move(a);
	}
	for (auto const &t : b) { cout << t.display(); }
	//for (auto const &t : b) { cout << t.display(); }

    return 0;
}
