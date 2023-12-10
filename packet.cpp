#include "packet.hpp"

Packet::Packet(int id, int counter, int num_hosts, int data){
    this->id = id;
    this->counter = counter;
    this->num_hosts = num_hosts;
    this->data = data;
}

Packet& Packet::operator=(const Packet& rhs) {
    this->id = rhs.id;
    this->counter = rhs.counter;
    this->num_hosts = rhs.num_hosts;
    this->data = rhs.data;
    return *this;
}

Packet::Packet(){

}

Packet& Packet::operator+=(const Packet& rhs) {
        this->num_hosts +=rhs.num_hosts;
        this->counter += rhs.counter;
        this->data += rhs.data;

        // Return a reference to the current object
        return *this;
    }
Packet::Packet(const Packet& rhs){
    this->id = rhs.id;
    this->counter = rhs.counter;
    this->num_hosts = rhs.num_hosts;
    this->data = rhs.data;
}

Packet::Packet(Packet& rhs){
    this->id = rhs.id;
    this->counter = rhs.counter;
    this->num_hosts = rhs.num_hosts;
    this->data = rhs.data;
}

