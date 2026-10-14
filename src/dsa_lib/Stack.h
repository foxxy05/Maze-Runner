#pragma once

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
        return topNode->data;
    }

    const T& top() const {
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
