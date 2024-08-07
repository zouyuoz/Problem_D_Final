#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

class Band {
public:
	int xl, xh, yl, yh;
	bool shouldBeDelete = 0;
	Band(int a, int b, int c, int d)
		: xl(a), xh(b), yl(c), yh(d) {}

	bool com(std::shared_ptr<Band>& other) {
		bool determine = *this <= other;
		if (determine) {
			this->shouldBeDelete = 1;
			return 1;
		}
		return 0;
	}

	bool operator <=(const std::shared_ptr<Band> &other) const {
		return xl >= other->xl && xh <= other->xh && yl >= other->yl && yh <= other->yh;
	}
};

// 方法1：使用索引，從後向前迭代
void method1(std::vector<std::shared_ptr<Band>>& vec) {
    for (int i = vec.size() - 1; i >= 0; --i) {
        for (int j = vec.size() - 1; j >= 0; --j) {
			if (i == j) continue;
            vec[i]->com(vec[j]);
        }
    }
    // 在迭代結束後刪除被標記的元素
    vec.erase(std::remove_if(vec.begin(), vec.end(), [](const std::shared_ptr<Band>& m) { return m->shouldBeDelete; }), vec.end());
}

int main() {
	std::vector<int> a = {8,0,0,9,0,11,1,0,0,3,0,2,0,0,0,4,6,5,0,7,10,12,0,15,0,33};
	for (const auto &thing : a) {
		for (size_t i = 0; i < a.size(); ++i) {
			if (a[i] > 5) {
				a.erase(a.begin() + i); 
				break;
			}
		}
	}
	for (auto const &e: a) std::cout << e << " ";
	return 0;
    std::vector<std::shared_ptr<Band>> vec = {
		std::make_shared<Band>(0,8,0,1),
		std::make_shared<Band>(5,6,0,1),
		std::make_shared<Band>(6,7,0,1),
		std::make_shared<Band>(7,8,0,1),
		std::make_shared<Band>(5,6,0,8),
		std::make_shared<Band>(6,7,0,9),
		std::make_shared<Band>(7,8,0,10),
		std::make_shared<Band>(5,11,7,8),
		std::make_shared<Band>(6,11,8,9),
		std::make_shared<Band>(7,11,9,10),
		std::make_shared<Band>(10,11,7,8),
		std::make_shared<Band>(10,11,8,9),
		std::make_shared<Band>(10,11,9,10),
		std::make_shared<Band>(10,11,7,14),
	};
    for (const auto& m : vec) std::cout << "[" << m->xl << " " << m->xh << " " << m->yl << " " << m->yh << "]\n";
	
    method1(vec);
    std::cout << "\nAfter method1:\n";
    for (const auto& m : vec) std::cout << "[" << m->xl << " " << m->xh << " " << m->yl << " " << m->yh << "]\n";
    std::cout << std::endl;

    return 0;
}