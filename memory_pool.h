#pragma once

#include <algorithm>
#include <math.h>

template <typename T>
class MemoryPool{
private:
    struct Node{
        Node* next;
    };
    Node* head;
    void* init_ptr;

public:
    MemoryPool(int size){
        int CHUNK_SIZE = std::max(sizeof(T), sizeof(T*));   // min. 8 bytes
        int total_chunks = size;

        init_ptr = ::operator new(total_chunks * CHUNK_SIZE);
        Node* curr_node_ptr = static_cast<Node*>(init_ptr);
        char* curr_byte_ptr = reinterpret_cast<char*>(curr_node_ptr);
        char* next_byte_ptr = nullptr;
        head = curr_node_ptr;
        Node* next_node_ptr = nullptr;

        for(int i=0; i<total_chunks; i++){
            next_byte_ptr = curr_byte_ptr + CHUNK_SIZE;
            next_node_ptr = reinterpret_cast<Node*>(next_byte_ptr);

            if(i < total_chunks-1){
                curr_node_ptr -> next = next_node_ptr;
                curr_byte_ptr = next_byte_ptr;
                curr_node_ptr = next_node_ptr;
            }
            else
                curr_node_ptr -> next = nullptr;
        }
    }

    ~MemoryPool(){
        ::operator delete(init_ptr);
    }

    void add(T* ptr){
        ptr -> ~T();
        Node* node_ptr = reinterpret_cast<Node*>(ptr);
        node_ptr -> next = head;
        head = node_ptr;
    }
    /*
    To-do: Add Slab Allocation for better
    handling in case of running out of chunks.
    */
    T* remove(){
        if (head == nullptr)
            return nullptr;
        T* T_ptr = reinterpret_cast<T*>(head);
        head = head -> next;
        return T_ptr;
    }
};