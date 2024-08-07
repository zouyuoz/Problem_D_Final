#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

struct Point {
    double x, y;
    Point(double _x, double _y) : x(_x), y(_y) {}
};

bool isInside(const std::vector<Point>& area, const Point& point) {
    bool isOdd = false; // if final intersect count is odd, than point is inside
    int n = area.size();
    
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((area[i].y <= point.y && point.y < area[j].y) || 
             (area[j].y <= point.y && point.y < area[i].y)) &&
            (point.x < (area[j].x - area[i].x) * (point.y - area[i].y) / (area[j].y - area[i].y) + area[i].x)) {
            isOdd = !isOdd;
        }
        
		// determine is on edge
		double fixed;
		Point ranged(0, 0);
		if (area[i].x == area[j].x) {
			fixed = area[i].x;
			ranged = Point(std::min(area[i].y, area[j].y), std::max(area[i].y, area[j].y));
			if (fixed == point.x && ranged.x <= point.y && ranged.y >= point.y) return false;
		}
		if (area[i].y == area[j].y) {
			fixed = area[i].y;
			ranged = Point(std::min(area[i].x, area[j].x), std::max(area[i].x, area[j].x));
			if (fixed == point.y && ranged.x <= point.x && ranged.y >= point.x) return false;
		}
    }
    return isOdd;
}

// 測試函數
void testIsInside(const std::vector<Point>& area, const Point& p) {
    bool result = isInside(area, p);
    std::cout << "Point(" << p.x << ", " << p.y << ") is inside: " << (result ? "true" : "false") << "\n";
}

void loop_map_items() {
	std::unordered_map<std::string, std::string> colors = {
		{"r", "#FF0000"},
		{"g", "#00FF00"},
		{"b", "#0000FF"}
	};
	for (const auto&[name, hex]: colors) {
		std::cout << "name: "<< name << ", hex: " << hex << "\n";
	}
}

int main() {
    std::vector<Point> area = {
        Point(0, 0), Point(3, 0), Point(3, 2), Point(2, 2),
        Point(2, 1), Point(1, 1), Point(1, 2), Point(0, 2)
    };

    loop_map_items();

    return 0;
}