#include <fstream>
#include <iostream>
#include <vector>

struct Point {
    double x, y;
};

void outputToCSV(const std::vector<std::vector<Point>>& polygons, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file << "group,x,y\n";
    for (size_t i = 0; i < polygons.size(); ++i) {
        for (const auto& point : polygons[i]) {
            file << i+1 << "," << point.x << "," << point.y << "\n";
        }
    }

    file.close();
    std::cout << "Data written to " << filename << std::endl;
}

int main() {
    std::vector<std::vector<Point>> polygons = {
        {{12904.000, 5554.000}, {12904.000, 6304.000}, {13019.000, 6304.000}, {13019.000, 5554.000}},
        {{12408.904, 7555.116}, {12408.904, 7771.672}, {12375.112, 7771.672}, {12375.112, 8510.688},
         {12335.032, 8510.688}, {12335.032, 9091.672}, {12369.112, 9091.672}, {12369.112, 9370.072},
         {13019.112, 9370.072}, {13019.112, 7555.116}}
        // 添加更多多邊形...
    };

    outputToCSV(polygons, "polygons.csv");
    return 0;
}