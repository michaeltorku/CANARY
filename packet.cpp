#include "packet.hpp"

Packet::Packet(int id, int counter, int num_hosts, int data){
    this->id = id;
    this->counter = counter;
    this->num_hosts = num_hosts;
    this->data = data;
}