#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>

/**
 * @brief High-Performance Lock-Free Order Queue
 * Designed for Low-Latency-Trading-Engine
 */
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    LockFreeQueue() {
        Node* dummy = new Node(T());
        head.store(dummy);
        tail.store(dummy);
    }

    // 주문 생산자 (Order Producer)
    void enqueue(T val) {
        Node* newNode = new Node(val);
        Node* oldTail = tail.exchange(newNode);
        oldTail->next = newNode;
    }

    // 주문 소비자 (Matching Engine / Consumer)
    bool dequeue(T& res) {
        Node* oldHead = head.load();
        Node* nextNode = oldHead->next;
        if (nextNode) {
            res = nextNode->data;
            head.store(nextNode);
            delete oldHead;
            return true;
        }
        return false;
    }
};

struct Order {
    int id;
    double price;
    int quantity;
};

int main() {
    LockFreeQueue<Order> orderQueue;

    // Constructor Thread: Making Customer fastly
    std::thread producer([&]() {
        for (int i = 0; i < 100000; ++i) {
            orderQueue.enqueue({i, 50000.0 + i, 1});
        }
    });

    // Consumer Thread: Process Order without delay
    std::thread consumer([&]() {
        Order ord;
        int count = 0;
        while (count < 100000) {
            if (orderQueue.dequeue(ord)) {
                if (count % 10000 == 0) {
                    std::cout << "[Matching Engine] Processed Order ID: " << ord.id << "\n";
                }
                count++;
            }
        }
    });

    producer.join();
    consumer.join();

    std::cout << "Successfully processed 100,000+ orders with Zero-Lock architecture.\n";
    return 0;
}
