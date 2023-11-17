#include <iostream>
#include <vector>
#include "host.hpp"
#include "loadBalance.cpp"
#include "path.hpp"



Host::Host(int id, int data, int all_reduce_id, std::vector<Path> paths){
    this->id = id;
    this->data = data;
    this->all_reduce_id = all_reduce_id; //FIX COULD REMOVE 
    this->paths = paths;
}
// Receive(const Host& other, int data); //change to packet
void Host::send(){
    int p_idx = load_balancer::balance(data, this->paths);
    Path selected_path = paths[p_idx];
    selected_path.send_data(data, false);// FIX PHASE SELECTION

}
void request();