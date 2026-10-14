#pragma once
#include <functional>
#include <cassert>
#include <utility>

namespace dsa_lib {

// ============================================================================
// DSA CORE: BINARY SEARCH TREE (BST) WITH CUSTOM COMPARATOR & TRAVERSALS
// ============================================================================
template <typename T, typename Compare = std::less<T>>
class BST {
public:
    struct Node {
        T data;
        Node* left;
        Node* right;
        explicit Node(const T& val) : data(val), left(nullptr), right(nullptr) {}
    };

private:
    Node* root = nullptr;
    int count = 0;
    Compare cmp;

    void destroyRecursive(Node* node) {
        if (!node) return;
        destroyRecursive(node->left);
        destroyRecursive(node->right);
        delete node;
    }

    Node* copyRecursive(Node* src) {
        if (!src) return nullptr;
        Node* n = new Node(src->data);
        n->left = copyRecursive(src->left);
        n->right = copyRecursive(src->right);
        return n;
    }

    Node* insertRecursive(Node* node, const T& val) {
        if (!node) {
            count++;
            return new Node(val);
        }
        if (cmp(val, node->data)) {
            node->left = insertRecursive(node->left, val);
        } else if (cmp(node->data, val)) {
            node->right = insertRecursive(node->right, val);
        } else {
            // Equivalent by comparator: update data
            node->data = val;
        }
        return node;
    }

    Node* removeRecursive(Node* node, const T& val, bool& removed) {
        if (!node) return nullptr;

        if (cmp(val, node->data)) {
            node->left = removeRecursive(node->left, val, removed);
        } else if (cmp(node->data, val)) {
            node->right = removeRecursive(node->right, val, removed);
        } else {
            removed = true;
            count--;

            // Case 1: Leaf node (no children)
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            }
            // Case 2a: Only right child
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                return temp;
            }
            // Case 2b: Only left child
            if (!node->right) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            // Case 3: Two children
            // Find in-order successor (minimum node in right subtree)
            Node* succ = node->right;
            while (succ->left) {
                succ = succ->left;
            }
            node->data = succ->data;
            // Delete successor from right subtree
            count++; // Compensate because recursive call will decrement count
            bool dummy = false;
            node->right = removeRecursive(node->right, succ->data, dummy);
        }
        return node;
    }

    int heightRecursive(Node* node) const {
        if (!node) return 0;
        int lh = heightRecursive(node->left);
        int rh = heightRecursive(node->right);
        return 1 + (lh > rh ? lh : rh);
    }

    template <typename Callback>
    void inorderRecursive(Node* node, Callback&& cb) const {
        if (!node) return;
        inorderRecursive(node->left, cb);
        cb(node->data);
        inorderRecursive(node->right, cb);
    }

    template <typename Callback>
    void preorderRecursive(Node* node, Callback&& cb) const {
        if (!node) return;
        cb(node->data);
        preorderRecursive(node->left, cb);
        preorderRecursive(node->right, cb);
    }

    template <typename Callback>
    void postorderRecursive(Node* node, Callback&& cb) const {
        if (!node) return;
        postorderRecursive(node->left, cb);
        postorderRecursive(node->right, cb);
        cb(node->data);
    }

    template <typename Callback>
    void reverseInorderRecursive(Node* node, Callback&& cb) const {
        if (!node) return;
        reverseInorderRecursive(node->right, cb);
        cb(node->data);
        reverseInorderRecursive(node->left, cb);
    }

public:
    BST() : root(nullptr), count(0), cmp() {}
    explicit BST(Compare comparator) : root(nullptr), count(0), cmp(comparator) {}

    ~BST() {
        clear();
    }

    // Rule of 5: Copy Constructor
    BST(const BST& other) : root(nullptr), count(0), cmp(other.cmp) {
        root = copyRecursive(other.root);
        count = other.count;
    }

    // Rule of 5: Move Constructor
    BST(BST&& other) noexcept : root(other.root), count(other.count), cmp(std::move(other.cmp)) {
        other.root = nullptr;
        other.count = 0;
    }

    // Rule of 5: Copy Assignment
    BST& operator=(const BST& other) {
        if (this != &other) {
            BST temp(other);
            std::swap(root, temp.root);
            std::swap(count, temp.count);
            std::swap(cmp, temp.cmp);
        }
        return *this;
    }

    // Rule of 5: Move Assignment
    BST& operator=(BST&& other) noexcept {
        if (this != &other) {
            clear();
            root = other.root;
            count = other.count;
            cmp = std::move(other.cmp);
            other.root = nullptr;
            other.count = 0;
        }
        return *this;
    }

    void insert(const T& val) {
        root = insertRecursive(root, val);
    }

    bool remove(const T& val) {
        bool removed = false;
        root = removeRecursive(root, val, removed);
        return removed;
    }

    bool contains(const T& val) const {
        return search(val) != nullptr;
    }

    const T* search(const T& val) const {
        Node* cur = root;
        while (cur) {
            if (cmp(val, cur->data)) {
                cur = cur->left;
            } else if (cmp(cur->data, val)) {
                cur = cur->right;
            } else {
                return &(cur->data);
            }
        }
        return nullptr;
    }

    int size() const {
        return count;
    }

    bool empty() const {
        return root == nullptr;
    }

    int height() const {
        return heightRecursive(root);
    }

    void clear() {
        destroyRecursive(root);
        root = nullptr;
        count = 0;
    }

    const T& min() const {
        assert(root != nullptr && "BST::min() called on empty tree");
        Node* cur = root;
        while (cur->left) cur = cur->left;
        return cur->data;
    }

    const T& max() const {
        assert(root != nullptr && "BST::max() called on empty tree");
        Node* cur = root;
        while (cur->right) cur = cur->right;
        return cur->data;
    }

    template <typename Callback>
    void inorder(Callback&& cb) const {
        inorderRecursive(root, std::forward<Callback>(cb));
    }

    template <typename Callback>
    void preorder(Callback&& cb) const {
        preorderRecursive(root, std::forward<Callback>(cb));
    }

    template <typename Callback>
    void postorder(Callback&& cb) const {
        postorderRecursive(root, std::forward<Callback>(cb));
    }

    template <typename Callback>
    void reverseInorder(Callback&& cb) const {
        reverseInorderRecursive(root, std::forward<Callback>(cb));
    }

    // Inspect root for tree visualization
    const Node* getRoot() const {
        return root;
    }
};

} // namespace dsa_lib
