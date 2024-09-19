#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <memory>



class MyClass {
public:
    MyClass(int v) : value(v) {}
    MyClass(const MyClass& other) : value(other.value) {
        // std::cout << "Copy constructor called" << std::endl;
    }

	void setValue(int const &val) { value = val; }
    int display() const { return value; }
private:
    int value;
};

using namespace std;

int main() {

	vector<int> vec = {1, 3, 5, 7, 9};
	while (vec.size()) {
		int a = rand() % 10;
		for (auto it = vec.begin(); it != vec.end(); ++it){
			if (*it == a) {
				vec.erase(it);
				break;
			}
		}
		cout << "vec_size: " << vec.size() << ": ";
		for (size_t i = 0; i < vec.size(); ++i) {
			cout << vec[i] << " ";
		}
		cout << "\n";
	}

    return 0;
}
