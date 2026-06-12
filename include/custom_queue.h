#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Custom Queue Data Structure                                        ║
// ║  FIFO (First In First Out) implementation using linked list nodes   ║
// ║  Replaces queue to avoid STL dependency                        ║
// ╚══════════════════════════════════════════════════════════════════════╝

#ifndef CUSTOM_QUEUE_H
#define CUSTOM_QUEUE_H

#include <iostream>
#include <stdexcept>

/**
 * @class Queue
 * @brief A generic FIFO queue implementation using linked list nodes
 * @tparam T The data type stored in the queue
 */
template <typename T>
class Queue {
private:
    /**
     * @struct Node
     * @brief Represents a single node in the queue linked list
     */
    struct Node {
        T data;           // Data stored in this node
        Node* next;       // Pointer to the next node
        
        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* frontNode;      // Pointer to the front of the queue
    Node* backNode;       // Pointer to the back of the queue
    int queueSize;        // Current number of elements in the queue

public:
    /**
     * @brief Constructor - initializes an empty queue
     */
    Queue() : frontNode(nullptr), backNode(nullptr), queueSize(0) {}

    /**
     * @brief Destructor - deallocates all nodes
     */
    ~Queue() {
        while (!empty()) {
            pop();
        }
    }

    /**
     * @brief Adds an element to the back of the queue
     * @param value The value to add
     */
    void push(const T& value) {
        Node* newNode = new Node(value);
        
        if (empty()) {
            // Queue is empty, new node is both front and back
            frontNode = backNode = newNode;
        } else {
            // Add to the back and update backNode pointer
            backNode->next = newNode;
            backNode = newNode;
        }
        queueSize++;
    }

    /**
     * @brief Removes and discards the front element
     * @throws runtime_error if queue is empty
     */
    void pop() {
        if (empty()) {
            throw runtime_error("Cannot pop from empty queue");
        }
        
        Node* temp = frontNode;
        frontNode = frontNode->next;
        queueSize--;
        
        if (empty()) {
            backNode = nullptr;
        }
        
        delete temp;
    }

    /**
     * @brief Returns a reference to the front element without removing it
     * @return Reference to the front element
     * @throws runtime_error if queue is empty
     */
    T& front() {
        if (empty()) {
            throw runtime_error("Cannot access front of empty queue");
        }
        return frontNode->data;
    }

    /**
     * @brief Returns a const reference to the front element without removing it
     * @return Const reference to the front element
     * @throws runtime_error if queue is empty
     */
    const T& front() const {
        if (empty()) {
            throw runtime_error("Cannot access front of empty queue");
        }
        return frontNode->data;
    }

    /**
     * @brief Returns a reference to the back element without removing it
     * @return Reference to the back element
     * @throws runtime_error if queue is empty
     */
    T& back() {
        if (empty()) {
            throw runtime_error("Cannot access back of empty queue");
        }
        return backNode->data;
    }

    /**
     * @brief Returns a const reference to the back element without removing it
     * @return Const reference to the back element
     * @throws runtime_error if queue is empty
     */
    const T& back() const {
        if (empty()) {
            throw runtime_error("Cannot access back of empty queue");
        }
        return backNode->data;
    }

    /**
     * @brief Checks if the queue is empty
     * @return true if queue is empty, false otherwise
     */
    bool empty() const {
        return queueSize == 0;
    }

    /**
     * @brief Returns the number of elements in the queue
     * @return The size of the queue
     */
    int size() const {
        return queueSize;
    }

    /**
     * @brief Removes all elements from the queue
     */
    void clear() {
        while (!empty()) {
            pop();
        }
    }

    /**
     * @brief Copy constructor - creates a deep copy of another queue
     */
    Queue(const Queue& other) : frontNode(nullptr), backNode(nullptr), queueSize(0) {
        if (other.empty()) return;
        Node* current = other.frontNode;
        while (current) {
            push(current->data);
            current = current->next;
        }
    }

    /**
     * @brief Copy assignment operator
     */
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            clear();
            if (other.empty()) return *this;
            Node* current = other.frontNode;
            while (current) {
                push(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Allow move operations if needed
    Queue(Queue&& other) noexcept 
        : frontNode(other.frontNode), backNode(other.backNode), queueSize(other.queueSize) {
        other.frontNode = nullptr;
        other.backNode = nullptr;
        other.queueSize = 0;
    }

    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            clear();
            frontNode = other.frontNode;
            backNode = other.backNode;
            queueSize = other.queueSize;
            other.frontNode = nullptr;
            other.backNode = nullptr;
            other.queueSize = 0;
        }
        return *this;
    }
};

#endif // CUSTOM_QUEUE_H
