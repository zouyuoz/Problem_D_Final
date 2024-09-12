#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <cmath>

struct Vertex {
    int id;
    int x, y;  // 假設我們在2D網格上工作
};

struct Edge {
    int to;
    int weight;
};

using Graph = std::unordered_map<int, std::vector<Edge>>;

// 用於優先隊列的比較函數
struct CompareVertex {
    bool operator()(const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    }
};

class GlobalRouter {
private:
    Graph graph;
    std::unordered_map<int, Vertex> vertices;
    std::unordered_set<int> terminals;

    double rectDist(const Vertex& a, const Vertex& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    std::vector<int> getMinSpanningTreeEdges(const std::unordered_set<int>& connectedVertices, 
                                             const std::unordered_set<int>& unconnectedTerminals) {
        // 這裡應該實現最小生成樹算法，為簡化，我們返回一個空vector
        return {};
    }

    double computeDeltaMin(int v, const std::unordered_map<int, double>& Lmin, 
                           const std::unordered_set<int>& unconnectedTerminals) {
        double deltaMin = std::numeric_limits<double>::max();
        for (int t : unconnectedTerminals) {
            deltaMin = std::min(deltaMin, rectDist(vertices[v], vertices[t]) - Lmin.at(t));
        }
        return deltaMin;
    }

public:
    std::vector<int> findSteinerTree(const std::unordered_set<int>& terminalSet) {
        terminals = terminalSet;
        std::unordered_set<int> connectedVertices;
        std::unordered_set<int> unconnectedTerminals = terminals;

        // 隨機選擇一個起始終端
        int start = *terminals.begin();
        connectedVertices.insert(start);
        unconnectedTerminals.erase(start);

        while (!unconnectedTerminals.empty()) {
            std::priority_queue<std::pair<int, double>, 
                                std::vector<std::pair<int, double>>, 
                                CompareVertex> openList;
            std::unordered_map<int, int> parent;
            std::unordered_map<int, double> gScore;

            for (int v : connectedVertices) {
                openList.push({v, 0});
                gScore[v] = 0;
            }

            while (!openList.empty()) {
                int current = openList.top().first;
                openList.pop();

                if (unconnectedTerminals.count(current)) {
                    // 找到一個未連接的終端
                    while (current != start && connectedVertices.count(current) == 0) {
                        connectedVertices.insert(current);
                        current = parent[current];
                    }
                    unconnectedTerminals.erase(current);
                    break;
                }

                for (const Edge& edge : graph[current]) {
                    int next = edge.to;
                    double tentativeGScore = gScore[current] + edge.weight;

                    if (gScore.count(next) == 0 || tentativeGScore < gScore[next]) {
                        parent[next] = current;
                        gScore[next] = tentativeGScore;

                        std::vector<int> mstEdges = getMinSpanningTreeEdges(connectedVertices, unconnectedTerminals);
                        std::unordered_map<int, double> Lmin;
                        // 計算Lmin...

                        double deltaMin = computeDeltaMin(next, Lmin, unconnectedTerminals);
                        double hScore = 2.0/3.0 * std::accumulate(Lmin.begin(), Lmin.end(), 0.0, 
                            [](double sum, const auto& p) { return sum + p.second; }) + deltaMin;

                        double fScore = gScore[next] + hScore;
                        openList.push({next, fScore});
                    }
                }
            }
        }

        // 構建並返回Steiner樹
        return {};
    }
};