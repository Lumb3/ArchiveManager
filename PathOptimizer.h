#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
#include <filesystem>

struct FileNode {
    std::string path;
    size_t size;
    int compressionType; // 0=text, 1=image, 2=video, 3=binary, etc.

    FileNode(const std::string& p, size_t s) : path(p), size(s) {
        // Determine compression type based on extension
        std::string ext = path.substr(path.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == "txt" || ext == "log" || ext == "xml" || ext == "json" || ext == "csv") {
            compressionType = 0; // Text files - high compression ratio
        } else if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif") {
            compressionType = 1; // Images - already compressed
        } else if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mp3") {
            compressionType = 2; // Media - already compressed
        } else {
            compressionType = 3; // Binary/other
        }
    }
};

struct CompressionEdge {
    size_t to;
    double weight;
    double compressionBenefit;

    CompressionEdge(size_t t, double w, double cb) : to(t), weight(w), compressionBenefit(cb) {}
};

class PathOptimizer {
private:
    std::vector<FileNode> nodes;
    std::vector<std::vector<CompressionEdge>> graph;

    // Calculate compression benefit when files are adjacent in zip
    double CalculateCompressionBenefit(const FileNode& current, const FileNode& next) {
        // Same type files benefit from being grouped together
        double typeBonus = (current.compressionType == next.compressionType) ? 0.3 : 0.0;

        // Size similarity helps with compression dictionary
        double sizeFactor = 1.0 - std::abs((double)current.size - (double)next.size) /
                           std::max((double)current.size, (double)next.size);
        sizeFactor *= 0.2; // Weight this factor

        // Text files benefit most from being grouped
        double compressionMultiplier = 1.0;
        if (current.compressionType == 0 && next.compressionType == 0) {
            compressionMultiplier = 1.5; // Text files compress better when grouped
        } else if (current.compressionType == 1 || next.compressionType == 1) {
            compressionMultiplier = 0.5; // Images don't compress much more
        }

        return (typeBonus + sizeFactor) * compressionMultiplier;
    }

    // Weight represents the "cost" of having these files adjacent (lower = better)
    double CalculateTransitionWeight(const FileNode& from, const FileNode& to) {
        double benefit = CalculateCompressionBenefit(from, to);
        // Convert benefit to cost (higher benefit = lower cost)
        return 1.0 - benefit;
    }

public:
    void AddFile(const std::string& path, size_t size) {
        nodes.emplace_back(path, size);
    }

    void Clear() {
        nodes.clear();
        graph.clear();
    }

    void BuildCompressionGraph() {
        size_t n = nodes.size();
        if (n == 0) return;

        graph.clear();
        graph.resize(n);

        // Build complete graph where each node connects to every other node
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    double weight = CalculateTransitionWeight(nodes[i], nodes[j]);
                    double benefit = CalculateCompressionBenefit(nodes[i], nodes[j]);
                    graph[i].emplace_back(j, weight, benefit);
                }
            }
        }
    }

    // Use Dijkstra to find optimal file ordering for compression
    std::vector<size_t> FindOptimalCompressionOrder() {
        size_t n = nodes.size();
        if (n == 0) return {};
        if (n == 1) return {0};

        // Find best starting node (largest text file or largest file overall)
        size_t startNode = 0;
        size_t maxTextSize = 0;
        size_t maxOverallSize = 0;

        for (size_t i = 0; i < n; ++i) {
            if (nodes[i].compressionType == 0 && nodes[i].size > maxTextSize) {
                maxTextSize = nodes[i].size;
                startNode = i;
            }
            if (nodes[i].size > maxOverallSize) {
                maxOverallSize = nodes[i].size;
                if (maxTextSize == 0) startNode = i; // Fallback to largest file
            }
        }

        std::vector<size_t> order;
        std::vector<bool> visited(n, false);

        size_t current = startNode;
        order.push_back(current);
        visited[current] = true;

        // Use modified Dijkstra approach: greedy selection of next best node
        while (order.size() < n) {
            double bestWeight = std::numeric_limits<double>::infinity();
            size_t bestNext = SIZE_MAX;

            // Find unvisited node with minimum weight from current node
            for (const auto& edge : graph[current]) {
                if (!visited[edge.to] && edge.weight < bestWeight) {
                    bestWeight = edge.weight;
                    bestNext = edge.to;
                }
            }

            if (bestNext != SIZE_MAX) {
                order.push_back(bestNext);
                visited[bestNext] = true;
                current = bestNext;
            } else {
                // Find any remaining unvisited node
                for (size_t i = 0; i < n; ++i) {
                    if (!visited[i]) {
                        order.push_back(i);
                        visited[i] = true;
                        current = i;
                        break;
                    }
                }
            }
        }

        return order;
    }

    // Alternative: Use full Dijkstra's algorithm to find minimum spanning tree approach
    std::vector<size_t> FindOptimalOrderDijkstra() {
        size_t n = nodes.size();
        if (n <= 1) return n == 1 ? std::vector<size_t>{0} : std::vector<size_t>{};

        // Use Dijkstra to build minimum spanning tree for file ordering
        std::vector<double> dist(n, std::numeric_limits<double>::infinity());
        std::vector<size_t> parent(n, SIZE_MAX);
        std::vector<bool> inMST(n, false);

        // Priority queue: {weight, node}
        std::priority_queue<std::pair<double, size_t>,
                          std::vector<std::pair<double, size_t>>,
                          std::greater<std::pair<double, size_t>>> pq;

        // Start with largest text file or largest file
        size_t start = 0;
        for (size_t i = 0; i < n; ++i) {
            if (nodes[i].compressionType == 0 && nodes[i].size > nodes[start].size) {
                start = i;
            }
        }

        dist[start] = 0.0;
        pq.push({0.0, start});

        std::vector<std::pair<size_t, size_t>> mstEdges; // {from, to}

        while (!pq.empty()) {
            double d = pq.top().first;
            size_t u = pq.top().second;
            pq.pop();

            if (inMST[u]) continue;
            inMST[u] = true;

            if (parent[u] != SIZE_MAX) {
                mstEdges.push_back({parent[u], u});
            }

            // Update distances to adjacent nodes
            for (const auto& edge : graph[u]) {
                size_t v = edge.to;
                if (!inMST[v] && edge.weight < dist[v]) {
                    dist[v] = edge.weight;
                    parent[v] = u;
                    pq.push({edge.weight, v});
                }
            }
        }

        // Convert MST to linear ordering using DFS
        std::vector<size_t> order;
        std::vector<bool> visited(n, false);
        std::vector<std::vector<size_t>> adjList(n);

        // Build adjacency list from MST edges
        for (const auto& edge : mstEdges) {
            adjList[edge.first].push_back(edge.second);
            adjList[edge.second].push_back(edge.first);
        }

        // DFS to create linear order
        std::function<void(size_t)> dfs = [&](size_t node) {
            visited[node] = true;
            order.push_back(node);

            for (size_t neighbor : adjList[node]) {
                if (!visited[neighbor]) {
                    dfs(neighbor);
                }
            }
        };

        dfs(start);

        return order;
    }

    std::vector<FileNode> GetOptimizedFileOrder() {
        if (nodes.empty()) return {};

        BuildCompressionGraph();

        // Try both approaches and pick the one with better estimated compression
        auto order1 = FindOptimalCompressionOrder();
        auto order2 = FindOptimalOrderDijkstra();

        // Use the simpler greedy approach for now
        auto bestOrder = order1;

        std::vector<FileNode> result;
        result.reserve(bestOrder.size());

        for (size_t idx : bestOrder) {
            if (idx < nodes.size()) {
                result.push_back(nodes[idx]);
            }
        }

        return result;
    }

    // Calculate estimated compression ratio for current order
    double EstimateCompressionRatio(const std::vector<size_t>& order) {
        if (order.size() <= 1) return 1.0;

        double totalBenefit = 0.0;
        for (size_t i = 0; i < order.size() - 1; ++i) {
            totalBenefit += CalculateCompressionBenefit(nodes[order[i]], nodes[order[i + 1]]);
        }

        return 1.0 + totalBenefit / order.size(); // Higher is better
    }
};