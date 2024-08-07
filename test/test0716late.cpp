#include <iostream>
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

int main() {
    // 创建一个 unique_ptr 管理 MyClass 对象
    std::unique_ptr<MyClass> b = std::make_unique<MyClass>(20);

    // 方法 1: 使用指针
    MyClass* b_ptr = b.get();
	b_ptr->setValue(30);
    std::cout << "b_ptr: " << b_ptr->display() << ", b: " << b->display() << std::endl;

    // 方法 2: 使用引用
    MyClass& b_ref = *b;
	b_ref.setValue(40);
    std::cout << "b_ref: " << b_ref.display() << ", b: " << b->display() << std::endl;

    // 方法 3: 通过拷贝构造函数
    MyClass b_copy = *b; // 拷贝构造函数被调用
	b_copy.setValue(50);
    std::cout << "b_copy: " << b_copy.display() << ", b: " << b->display() << std::endl;

    return 0;
}
