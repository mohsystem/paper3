
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

class Task180 {
private:
    map<int, vector<int>> adjacencyList;
    
    void buildTree(int current, int parent, map<int, vector<int>>& newTree, set<int>& visited) {
        visited.insert(current);
        
        if (adjacencyList.find(current) != adjacencyList.end()) {
            for (int neighbor : adjacencyList[current]) {
                if (visited.find(neighbor) == visited.end()) {
                    newTree[current].push_back(neighbor);
                    buildTree(neighbor, current, newTree, visited);
                }
            }
        }
    }
    
public:
    Task180(map<int, vector<int>> adjList) : adjacencyList(adjList) {}
    
    map<int, vector<int>> reparent(int newRoot) {
        map<int, vector<int>> newTree;
        set<int> visited;
        
        for (const auto& pair : adjacencyList) {
            newTree[pair.first] = vector<int>();
        }
        
        buildTree(newRoot, -1, newTree, visited);
        
        return newTree;
    }
};

void printTree(const map<int, vector<int>>& tree) {
    cout << "{";
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        if (it != tree.begin()) cout << ", ";
        cout << it->first << ": [";
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (i > 0) cout << ", ";
            cout << it->second[i];
        }
        cout << "]";
    }
    cout << "}" << endl;
}

int main() {
    // Test Case 1: Example from problem description
    map<int, vector<int>> tree1;
    tree1[0] = {1, 2, 3};
    tree1[1] = {0, 4, 5};
    tree1[2] = {0, 6, 7};
    tree1[3] = {0, 8, 9};
    tree1[4] = {1};
    tree1[5] = {1};
    tree1[6] = {2};
    tree1[7] = {2};
    tree1[8] = {3};
    tree1[9] = {3};
    
    Task180 t1(tree1);
    map<int, vector<int>> result1 = t1.reparent(6);
    cout << "Test 1 - Reparent on node 6: ";
    printTree(result1);
    
    // Test Case 2: Simple tree
    map<int, vector<int>> tree2;
    tree2[0] = {1, 2};
    tree2[1] = {0};
    tree2[2] = {0};
    
    Task180 t2(tree2);
    map<int, vector<int>> result2 = t2.reparent(1);
    cout << "Test 2 - Reparent on node 1: ";
    printTree(result2);
    
    // Test Case 3: Linear tree
    map<int, vector<int>> tree3;
    tree3[0] = {1};
    tree3[1] = {0, 2};
    tree3[2] = {1, 3};
    tree3[3] = {2};
    
    Task180 t3(tree3);
    map<int, vector<int>> result3 = t3.reparent(3);
    cout << "Test 3 - Reparent on node 3: ";
    printTree(result3);
    
    // Test Case 4: Single node
    map<int, vector<int>> tree4;
    tree4[0] = {};
    
    Task180 t4(tree4);
    map<int, vector<int>> result4 = t4.reparent(0);
    cout << "Test 4 - Single node: ";
    printTree(result4);
    
    // Test Case 5: Three nodes
    map<int, vector<int>> tree5;
    tree5[0] = {1};
    tree5[1] = {0, 2};
    tree5[2] = {1};
    
    Task180 t5(tree5);
    map<int, vector<int>> result5 = t5.reparent(2);
    cout << "Test 5 - Reparent on node 2: ";
    printTree(result5);
    
    return 0;
}
