
#include <iostream>
#include <memory>
#include <stdexcept>
#include <limits>

// Binary Search Tree implementation with security considerations
class BST {
private:
    struct Node {
        int value;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        
        explicit Node(int val) : value(val), left(nullptr), right(nullptr) {}
    };
    
    std::unique_ptr<Node> root;
    size_t node_count;
    static constexpr size_t MAX_NODES = 1000000; // Prevent excessive memory usage
    
    // Helper function to insert recursively with bounds checking
    Node* insertHelper(std::unique_ptr<Node>& node, int value) {
        if (!node) {
            // Check node count limit to prevent resource exhaustion
            if (node_count >= MAX_NODES) {
                throw std::runtime_error("Maximum tree size exceeded");
            }
            node = std::make_unique<Node>(value);
            node_count++;
            return node.get();
        }
        
        if (value < node->value) {
            return insertHelper(node->left, value);
        } else if (value > node->value) {
            return insertHelper(node->right, value);
        }
        
        // Value already exists, return existing node
        return node.get();
    }
    
    // Helper function to search recursively
    bool searchHelper(const Node* node, int value) const {
        if (!node) {
            return false;
        }
        
        if (value == node->value) {
            return true;
        } else if (value < node->value) {
            return searchHelper(node->left.get(), value);
        } else {
            return searchHelper(node->right.get(), value);
        }
    }
    
    // Find minimum value node in subtree
    Node* findMin(Node* node) const {
        if (!node) {
            return nullptr;
        }
        while (node->left) {
            node = node->left.get();
        }
        return node;
    }
    
    // Helper function to delete recursively
    std::unique_ptr<Node> deleteHelper(std::unique_ptr<Node> node, int value, bool& deleted) {
        if (!node) {
            deleted = false;
            return nullptr;
        }
        
        if (value < node->value) {
            node->left = deleteHelper(std::move(node->left), value, deleted);
        } else if (value > node->value) {
            node->right = deleteHelper(std::move(node->right), value, deleted);
        } else {
            // Node found, perform deletion
            deleted = true;
            node_count--;
            
            // Case 1: No children or only right child
            if (!node->left) {
                return std::move(node->right);
            }
            // Case 2: Only left child
            if (!node->right) {
                return std::move(node->left);
            }
            
            // Case 3: Two children
            // Find inorder successor (minimum in right subtree)
            Node* successor = findMin(node->right.get());
            if (successor) {
                node->value = successor->value;
                bool temp_deleted = false;
                node->right = deleteHelper(std::move(node->right), successor->value, temp_deleted);
            }
        }
        
        return node;
    }
    
public:
    BST() : root(nullptr), node_count(0) {}
    
    // Insert value into BST, returns true if inserted, false if already exists
    bool insert(int value) {
        try {
            Node* inserted = insertHelper(root, value);
            return inserted != nullptr;
        } catch (const std::exception& e) {
            std::cerr << "Insert error: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Search for value in BST
    bool search(int value) const {
        return searchHelper(root.get(), value);
    }
    
    // Delete value from BST, returns true if deleted, false if not found
    bool remove(int value) {
        bool deleted = false;
        root = deleteHelper(std::move(root), value, deleted);
        return deleted;
    }
    
    // Get current node count
    size_t size() const {
        return node_count;
    }
};

int main() {
    BST bst;
    
    // Test case 1: Basic insertions and search
    std::cout << "Test 1: Basic insertions" << std::endl;
    bst.insert(50);
    bst.insert(30);
    bst.insert(70);
    bst.insert(20);
    bst.insert(40);
    std::cout << "Search 30: " << (bst.search(30) ? "Found" : "Not found") << std::endl;
    std::cout << "Search 100: " << (bst.search(100) ? "Found" : "Not found") << std::endl;
    
    // Test case 2: Delete leaf node
    std::cout << "\\nTest 2: Delete leaf node (20)" << std::endl;
    bst.remove(20);
    std::cout << "Search 20 after delete: " << (bst.search(20) ? "Found" : "Not found") << std::endl;
    
    // Test case 3: Delete node with one child
    std::cout << "\\nTest 3: Delete node with one child (30)" << std::endl;
    bst.remove(30);
    std::cout << "Search 30 after delete: " << (bst.search(30) ? "Found" : "Not found") << std::endl;
    std::cout << "Search 40 (child of deleted): " << (bst.search(40) ? "Found" : "Not found") << std::endl;
    
    // Test case 4: Delete node with two children
    std::cout << "\\nTest 4: Delete root node with two children (50)" << std::endl;
    bst.remove(50);
    std::cout << "Search 50 after delete: " << (bst.search(50) ? "Found" : "Not found") << std::endl;
    std::cout << "Tree still maintains structure - search 70: " << (bst.search(70) ? "Found" : "Not found") << std::endl;
    
    // Test case 5: Insert duplicate and edge values
    std::cout << "\\nTest 5: Duplicate insertion and edge values" << std::endl;
    bst.insert(70); // Duplicate
    bst.insert(std::numeric_limits<int>::max());
    bst.insert(std::numeric_limits<int>::min());
    std::cout << "Search max int: " << (bst.search(std::numeric_limits<int>::max()) ? "Found" : "Not found") << std::endl;
    std::cout << "Search min int: " << (bst.search(std::numeric_limits<int>::min()) ? "Found" : "Not found") << std::endl;
    std::cout << "Tree size: " << bst.size() << std::endl;
    
    return 0;
}
