#pragma once

namespace dsa_lib {

// Simple Templated Linked-List Queue (FIFO) for BFS Hunter Pathfinding
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val, Node* n = nullptr) : data(val), next(n) {}
    };

    Node* headNode = nullptr;
    Node* tailNode = nullptr;
    int count = 0;

public:
    Queue() : headNode(nullptr), tailNode(nullptr), count(0) {}
    ~Queue() { clear(); }

    void push(T val) {
        Node* n = new Node(val);
        if (!tailNode) {
            headNode = tailNode = n;
        } else {
            tailNode->next = n;
            tailNode = n;
        }
        count++;
    }

    void pop() {
        if (empty()) return;
        Node* temp = headNode;
        headNode = headNode->next;
        if (!headNode) tailNode = nullptr;
        delete temp;
        count--;
    }

    T& front() {
        return headNode->data;
    }

    const T& front() const {
        return headNode->data;
    }

    bool empty() const {
        return headNode == nullptr;
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
