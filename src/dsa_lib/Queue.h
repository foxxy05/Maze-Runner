#pragma once
#include <cassert>
#include <utility>

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

    // Rule of 5: Copy Constructor
    Queue(const Queue& other) : headNode(nullptr), tailNode(nullptr), count(0) {
        Node* src = other.headNode;
        while (src) {
            push(src->data);
            src = src->next;
        }
    }

    // Rule of 5: Move Constructor
    Queue(Queue&& other) noexcept : headNode(other.headNode), tailNode(other.tailNode), count(other.count) {
        other.headNode = nullptr;
        other.tailNode = nullptr;
        other.count = 0;
    }

    // Rule of 5: Copy Assignment
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            Queue temp(other);
            std::swap(headNode, temp.headNode);
            std::swap(tailNode, temp.tailNode);
            std::swap(count, temp.count);
        }
        return *this;
    }

    // Rule of 5: Move Assignment
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            clear();
            headNode = other.headNode;
            tailNode = other.tailNode;
            count = other.count;
            other.headNode = nullptr;
            other.tailNode = nullptr;
            other.count = 0;
        }
        return *this;
    }

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
        assert(!empty() && "Queue::front() called on empty queue");
        return headNode->data;
    }

    const T& front() const {
        assert(!empty() && "Queue::front() const called on empty queue");
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
