
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

class Task180 {
private:
    void buildReparentedTree(const map<int, vector<int>>& tree, int current, int parent,
                            set<int>& visited, map<int, vector<int>>& result) {
        visited.insert(current);
        if (result.find(current) == result.end()) {
            result[current] = vector<int>();
        }
        
        auto it = tree.find(current);
        if (it != tree.end()) {
            for (int neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    result[current].push_back(neighbor);
                    buildReparentedTree(tree, neighbor, current, visited, result);
                }
            }
        }
        
        if (parent != -1) {
            result[current].push_back(parent);
        }
    }

public:
    map<int, vector<int>> reparentTree(const map<int, vector<int>>& tree, int newRoot) {
        map<int, vector<int>> result;
        
        if (tree.empty() || tree.find(newRoot) == tree.end()) {
            return result;
        }
        
        set<int> visited;
        buildReparentedTree(tree, newRoot, -1, visited, result);
        
        return result;
    }
};

void printTree(const map<int, vector<int>>& tree) {
    cout << "{ ";
    for (const auto& pair : tree) {
        cout << pair.first << ": [";
        for (size_t i = 0; i < pair.second.size(); i++) {
            cout << pair.second[i];
            if (i < pair.second.size() - 1) cout << ", ";
        }
        cout << "] ";
    }
    cout << "}" << endl;
}

int main() {
    Task180 solution;
    
    // Test case 1: Simple tree from example
    map<int, vector<int>> tree1 = {
        {0, {1, 2, 3}},
        {1, {4, 5}},
        {2, {6, 7}},
        {3, {8, 9}}
    };
    cout << "Test 1 - Reparent from 6:" << endl;
    printTree(solution.reparentTree(tree1, 6));
    
    // Test case 2: Single node
    map<int, vector<int>> tree2 = {{0, {}}};
    cout << "\\nTest 2 - Single node:" << endl;
    printTree(solution.reparentTree(tree2, 0));
    
    // Test case 3: Linear tree
    map<int, vector<int>> tree3 = {
        {0, {1}},
        {1, {2}},
        {2, {3}}
    };
    cout << "\\nTest 3 - Linear tree, reparent from 2:" << endl;
    printTree(solution.reparentTree(tree3, 2));
    
    // Test case 4: Reparent at root
    map<int, vector<int>> tree4 = {
        {0, {1, 2}},
        {1, {3}}
    };
    cout << "\\nTest 4 - Reparent at root:" << endl;
    printTree(solution.reparentTree(tree4, 0));
    
    // Test case 5: Empty tree
    map<int, vector<int>> tree5;
    cout << "\\nTest 5 - Empty tree:" << endl;
    printTree(solution.reparentTree(tree5, 0));
    
    return 0;
}
