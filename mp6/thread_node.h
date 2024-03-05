//
// Created by Nachiket Naganure on 11/19/23.
//

//#ifndef MP6_THREAD_NODE_H
//#define MP6_THREAD_NODE_H
//
//#endif //MP6_THREAD_NODE_H
#include "thread.H"

class thread_node{
    Thread* thread;
    thread_node* next;


public:
    static thread_node* head_list;
    static thread_node* tail_list;
    thread_node(){
        thread = nullptr;
        next = nullptr;
    }

    thread_node(Thread *thread1){
        thread = thread1;
        next = nullptr;
    }

    void add_thread_node(Thread *thread1){
        thread_node* new_thread_node = new thread_node(thread1);
        if(head_list == nullptr){
            head_list = new_thread_node;
            tail_list = head_list;
        }
        else{
            tail_list->next = new_thread_node;
            tail_list = tail_list->next;
        }
    }

    void delete_thread_node(Thread *thread1){
        if(head_list == nullptr || tail_list == nullptr){
            return;
        }
        thread_node *current = head_list, *prev = head_list;
        while(current != nullptr){
            if(current->thread == thread1){
                prev->next = current->next;
                delete current;
                return;
            }
            else{
                if(current != head_list){
                    prev = current;
                }
                current = current->next;
            }
        }

    }

    bool empty(){
        if(head_list != nullptr) return false;
        return true;
    }

    Thread* get_front_thread(){
        thread_node *front_node = head_list;
        Thread* front = head_list->thread;
        head_list = head_list->next;
        delete front_node;
        return front;
    }

};
