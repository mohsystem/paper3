#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

struct Node {
    int value;
    std::vector<Node*> children;
    Node* parent; // used only during building on the original tree

    explicit Node(int v) : value(v), children(), parent(nullptr) {}
};

static void add_child(Node* parent, Node* child) {
    if (!parent || !child) return;
    parent->children.push_back(child);
}

static void assign_parents(Node* node, Node* parent) {
    if (!node) return;
    node->parent = parent;
    for (Node* c : node->children) {
        assign_parents(c, node);
    }
}

static Node* find(Node* node, int target) {
    if (!node) return nullptr;
    if (node->value == target) return node;
    for (Node* c : node->children) {
        Node* f = find(c, target);
        if (f) return f;
    }
    return nullptr;
}

static Node* build_reoriented(Node* node, Node* prev) {
    Node* clone = new (std::nothrow) Node(node->value);
    if (!clone) return nullptr;
    for (Node* c : node->children) {
        if (c != prev) {
            Node* cc = build_reoriented(c, node);
            if (!cc) { delete clone; return nullptr; }
            add_child(clone, cc);
        }
    }
    if (node->parent && node->parent != prev) {
        Node* pc = build_reoriented(node->parent, node);
        if (!pc) { delete clone; return nullptr; }
        add_child(clone, pc);
    }
    return clone;
}

static Node* reparent(Node* root, int target_value) {
    if (!root) return nullptr;
    assign_parents(root, nullptr);
    Node* target = find(root, target_value);
    if (!target) return nullptr;
    return build_reoriented(target, nullptr);
}

static std::string to_string_tree(const Node* node) {
    if (!node) return "null";
    std::string s = std::to_string(node->value);
    if (!node->children.empty()) {
        s.push_back('(');
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i > 0) s.push_back(',');
            s += to_string_tree(node->children[i]);
        }
        s.push_back(')');
    }
    return s;
}

static void free_tree(Node* node) {
    if (!node) return;
    for (Node* c : node->children) {
        free_tree(c);
    }
    delete node;
}

static Node* build_example_tree() {
    Node* n0 = new Node(0);
    Node* n1 = new Node(1);
    Node* n2 = new Node(2);
    Node* n3 = new Node(3);
    Node* n4 = new Node(4);
    Node* n5 = new Node(5);
    Node* n6 = new Node(6);
    Node* n7 = new Node(7);
    Node* n8 = new Node(8);
    Node* n9 = new Node(9);

    add_child(n0, n1); add_child(n0, n2); add_child(n0, n3);
    add_child(n1, n4); add_child(n1, n5);
    add_child(n2, n6); add_child(n2, n7);
    add_child(n3, n8); add_child(n3, n9);
    return n0;
}

static Node* build_single_node(int value) {
    return new Node(value);
}

int main() {
    // Test 1: Reparent on 6
    Node* root = build_example_tree();
    Node* r1 = reparent(root, 6);
    std::cout << (r1 ? to_string_tree(r1) : std::string("null")) << "\n";
    free_tree(r1);

    // Test 2: Reparent on 0
    Node* r2 = reparent(root, 0);
    std::cout << (r2 ? to_string_tree(r2) : std::string("null")) << "\n";
    free_tree(r2);

    // Test 3: Reparent on 9
    Node* r3 = reparent(root, 9);
    std::cout << (r3 ? to_string_tree(r3) : std::string("null")) << "\n";
    free_tree(r3);

    // Test 4: Single node tree
    Node* single = build_single_node(42);
    Node* r4 = reparent(single, 42);
    std::cout << (r4 ? to_string_tree(r4) : std::string("null")) << "\n";
    free_tree(r4);
    free_tree(single);

    // Test 5: Target not found
    Node* r5 = reparent(root, 100);
    std::cout << (r5 ? to_string_tree(r5) : std::string("null")) << "\n";
    free_tree(r5);

    free_tree(root);
    return 0;
}