#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        return std::hash<double>()(p.x) ^ std::hash<double>()(p.y);
    }
};

double euclidean_distance(const Point& a, const Point& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

std::vector<Point> get_neighbors(const Point& p, double step) {
    std::vector<Point> neighbors;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            neighbors.emplace_back(p.x + dx * step, p.y + dy * step);
        }
    }
    return neighbors;
}

std::vector<Point> astar(const Point& start, const Point& goal, double step) {
    auto compare = [](const std::pair<double, Point>& a, const std::pair<double, Point>& b) {
        return a.first > b.first;
    };
    std::priority_queue<std::pair<double, Point>, std::vector<std::pair<double, Point>>, decltype(compare)> open_set(compare);
    
    std::unordered_map<Point, Point, PointHash> came_from;
    std::unordered_map<Point, double, PointHash> g_score;
    std::unordered_map<Point, double, PointHash> f_score;
    
    open_set.emplace(0, start);
    g_score[start] = 0;
    f_score[start] = euclidean_distance(start, goal);
    
    while (!open_set.empty()) {
        Point current = open_set.top().second;
        open_set.pop();
        
        if (current == goal) {
            std::vector<Point> path;
            while (current == start == false) {
                path.push_back(current);
                current = came_from[current];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        for (const auto& neighbor : get_neighbors(current, step)) {
            double tentative_g_score = g_score[current] + euclidean_distance(current, neighbor);
            
            if (g_score.find(neighbor) == g_score.end() || tentative_g_score < g_score[neighbor]) {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g_score;
                f_score[neighbor] = g_score[neighbor] + euclidean_distance(neighbor, goal);
                open_set.emplace(f_score[neighbor], neighbor);
            }
        }
    }
    
    return {}; // No path found
}

int main() {
    Point start(0, 0);
    Point goal(10, 10);
    double step = 1.0; // 步長，可以調整
    
    auto path = astar(start, goal, step);
    
    if (path.empty()) {
        std::cout << "No path found" << std::endl;
    } else {
        std::cout << "Path found:" << std::endl;
        for (const auto& p : path) {
            std::cout << "(" << p.x << ", " << p.y << ") ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}