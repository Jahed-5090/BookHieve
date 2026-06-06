#pragma once
#include "globals.h"
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <algorithm>

// ══════════════════════════════════════════════════════════════════════════════
//  GenreGraph  –  Directed Acyclic Graph of genre → related genres
//  BFS : breadth-first genre recommendation
//  DFS : deep-dive related genre exploration
//  Topological Sort : genre dependency ordering (prerequisite relationships)
//
//  Nodes = genre names, Edges = "related to / prerequisite"
// ══════════════════════════════════════════════════════════════════════════════
class GenreGraph {
    map<string, vector<string>> adj;   // adjacency list
    vector<string> nodes;

    bool hasNode(const string& g) const {
        return find(nodes.begin(), nodes.end(), g) != nodes.end();
    }

public:
    void addNode(const string& genre) {
        if (!hasNode(genre)) {
            nodes.push_back(genre);
            adj[genre]; // ensure key exists
        }
    }

    void addEdge(const string& from, const string& to) {
        addNode(from); addNode(to);
        adj[from].push_back(to);
    }

    // ─── BFS from a start genre ───────────────────────────────────────────
    void bfs(const string& start) const {
        if (adj.find(start) == adj.end()) {
            cout << RED << "  Genre not in graph.\n" << RESET; return;
        }
        set<string>    visited;
        queue<string>  q;
        q.push(start);
        visited.insert(start);
        cout << GREEN << "  BFS from [" << start << "]: " << RESET;
        while (!q.empty()) {
            string cur = q.front(); q.pop();
            cout << CYAN << cur << RESET;
            for (auto& nb : adj.at(cur)) {
                if (!visited.count(nb)) {
                    visited.insert(nb);
                    q.push(nb);
                    cout << " → ";
                }
            }
        }
        cout << "\n";
    }

    // ─── DFS from a start genre ───────────────────────────────────────────
    void dfsHelper(const string& node, set<string>& visited) const {
        visited.insert(node);
        cout << MAGENTA << node << RESET;
        for (auto& nb : adj.at(node)) {
            if (!visited.count(nb)) {
                cout << " → ";
                dfsHelper(nb, visited);
            }
        }
    }

    void dfs(const string& start) const {
        if (adj.find(start) == adj.end()) {
            cout << RED << "  Genre not in graph.\n" << RESET; return;
        }
        set<string> visited;
        cout << GREEN << "  DFS from [" << start << "]: " << RESET;
        dfsHelper(start, visited);
        cout << "\n";
    }

    // ─── Topological Sort (Kahn's algorithm) ─────────────────────────────
    void topoSort() const {
        map<string,int> indegree;
        for (auto& n : nodes) indegree[n] = 0;
        for (auto& p : adj) {
            auto& vec = p.second;
            for (auto& v : vec) indegree[v]++;
        }

        queue<string> q;
        for (auto& n : nodes)
            if (indegree[n] == 0) q.push(n);

        cout << GREEN << "  Topological Order: " << RESET;
        bool first = true;
        while (!q.empty()) {
            string cur = q.front(); q.pop();
            if (!first) cout << " → ";
            cout << CYAN << cur << RESET;
            first = false;
            for (auto& nb : adj.at(cur)) {
                if (--indegree[nb] == 0) q.push(nb);
            }
        }
        cout << "\n";
    }

    void printGraph() const {
        if (nodes.empty()) { cout << RED << "  Graph is empty.\n" << RESET; return; }
        cout << BOLD << "  Genre Relationship Graph:\n" << RESET;
        for (auto& n : nodes) {
            cout << "  " << CYAN << left << setw(18) << n << RESET << " → ";
            for (size_t i=0; i<adj.at(n).size(); i++) {
                if (i) cout << ", ";
                cout << YELLOW << adj.at(n)[i] << RESET;
            }
            if (adj.at(n).empty()) cout << MAGENTA << "(leaf)" << RESET;
            cout << "\n";
        }
    }

    // Seed a default genre graph for the library
    void seedDefault() {
        addEdge("Fiction",        "Science Fiction");
        addEdge("Fiction",        "Fantasy");
        addEdge("Fiction",        "Mystery");
        addEdge("Science Fiction","Dystopian");
        addEdge("Science Fiction","Space Opera");
        addEdge("Fantasy",        "Epic Fantasy");
        addEdge("Fantasy",        "Dark Fantasy");
        addEdge("Mystery",        "Thriller");
        addEdge("Non-Fiction",    "Biography");
        addEdge("Non-Fiction",    "History");
        addEdge("Non-Fiction",    "Science");
        addEdge("Science",        "Technology");
        addEdge("History",        "Politics");
        addEdge("Technology",     "Programming");
    }
};
