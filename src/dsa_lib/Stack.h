#pragma once
#include <cassert>
#include <utility>

namespace dsa_lib {

// Simple Templated Linked-List Stack (LIFO) for DFS Maze Generation
template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val, Node* n = nullptr) : data(val), next(n) {}
    };

    Node* topNode = nullptr;
    int count = 0;

public:
    Stack() : topNode(nullptr), count(0) {}
    ~Stack() { clear(); }

    // Rule of 5: Copy Constructor
    Stack(const Stack& other) : topNode(nullptr), count(0) {
        if (!other.topNode) return;
        Node* src = other.topNode;
        topNode = new Node(src->data);
        Node* dst = topNode;
        src = src->next;
        while (src) {
            dst->next = new Node(src->data);
            dst = dst->next;
            src = src->next;
        }
        count = other.count;
    }

    // Rule of 5: Move Constructor
    Stack(Stack&& other) noexcept : topNode(other.topNode), count(other.count) {
        other.topNode = nullptr;
        other.count = 0;
    }

    // Rule of 5: Copy Assignment
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            Stack temp(other);
            std::swap(topNode, temp.topNode);
            std::swap(count, temp.count);
        }
        return *this;
    }

    // Rule of 5: Move Assignment
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            clear();
            topNode = other.topNode;
            count = other.count;
            other.topNode = nullptr;
            other.count = 0;
        }
        return *this;
    }

    void push(T val) {
        topNode = new Node(val, topNode);
        count++;
    }

    void pop() {
        if (empty()) return;
        Node* temp = topNode;
        topNode = topNode->next;
        delete temp;
        count--;
    }

    T& top() {
        assert(!empty() && "Stack::top() called on empty stack");
        return topNode->data;
    }

    const T& top() const {
        assert(!empty() && "Stack::top() const called on empty stack");
        return topNode->data;
    }

    bool empty() const {
        return topNode == nullptr;
    }

    int size() const {
        return count;
    }

    void clear() {
        while (!empty()) {
            pop();
        }
    }
};

} // namespace dsa_lib
