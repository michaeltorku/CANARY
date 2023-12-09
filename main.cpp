#include <iostream>
#include <set>
#include <random>
#include <vector>
#include <map>
#include <sstream>
#include <chrono>
#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"


std::vector<std::vector<int>> ongoing_allreduces;
std::map<int, Host> host_map;
std::map<int, Switch> switch_map;

std::vector<int> select_hosts_for_allreduce(std::set<int> host_ids){
    if (host_ids.size() < 3){ //WLOG all reduces are run in batches of 3
        return std::vector<int>{};
    }
    std::cout << "Selecting hosts for allreduce" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::vector<int> allreduce_hosts;
    for (int i = 0; i<3; i++){
        std::uniform_int_distribution<> dis(0, host_ids.size() - 1);
        auto it = host_ids.begin();
        std::advance(it, dis(gen));
        int host1 = *it;
        host_ids.erase(it);
    }
    return allreduce_hosts;
    
}

std::vector<int> initiate_allreduce(std::vector<int> allreduce_hosts){
    std::cout << "Initiating allreduce" << std::endl;
    return std::vector<int>{1,2,3};
}

void network_setup(int number_of_hosts, int number_of_switches){
    std::map<std::string, std::vector<Path>> all_paths;
    std::vector<std::string> pairings = {"H0:S3", "H1:S3", "H2:S4", "H3:S4",
    "S3:S1",
    "S3:S2","S4:S1",
    "S4:S2","S1:S0","S2:S0",
    };
    
    for (std::string & pair: pairings){
        std::istringstream iss(pair);
        std::string lower_node, upper_node;
        
        // Splitting the string by ':'
        std::getline(iss, lower_node, ':');
        std::getline(iss, upper_node);

        // Create a Path object and add it to the paths vector
        if (all_paths.contains(lower_node)){
            std::vector<Path> & path_list = all_paths[lower_node];
            path_list.emplace_back(lower_node, upper_node);
        }else{
            all_paths[lower_node] = {Path(lower_node, upper_node)};
        }
        
    }
    

    for (int i=0; i<number_of_hosts; i++){
        std::string host_rep = "H"+std::to_string(i);
        std::vector<Path> const_arg = all_paths.contains(host_rep)? all_paths[host_rep]:std::vector<Path>{};
        host_map.emplace(i, Host(i, 2, const_arg));
    }
    for (int i=0; i<number_of_switches; i++){
        std::string switch_rep = "S"+std::to_string(i);
        std::vector<Path> const_arg = all_paths.contains(switch_rep)? all_paths[switch_rep]:std::vector<Path>{};
        switch_map.emplace(i, Switch(i, const_arg));
    }

}

void send(Host& host, int reduce_id, int data){
    int p_idx = load_balancer::balance(host.paths);
    Path &selected_path = host.paths[p_idx];

    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        receive(target, reduce_id, host.all_reduce_map[reduce_id]);
    }else{
        Host &target = host_map.at(target_node_id);
        receive(target, reduce_id, host.all_reduce_map[reduce_id]);
    }
}

void receive(Host& host, int reduce_id, int data){
    host.all_reduce_map[reduce_id] += data;
    send(host, reduce_id, data);
}

void send(Switch& toggle, int reduce_id, int data){
    if (toggle.paths.size() == 0){
        return;
    }
    int p_idx = load_balancer::balance(toggle.paths);
    Path & selected_path = toggle.paths[p_idx];
    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        receive(target, reduce_id, data);
    }else{
        Host &target = host_map.at(target_node_id);
        receive(target, reduce_id, data);
    }
}

void receive(Switch& toggle, int reduce_id, int data){
    std::cout << toggle.id << " received " << data << std::endl;
    toggle.all_reduce_map[reduce_id] += data;
    send(toggle, reduce_id, data);
}


int main(){
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int> host_ids = {0, 1, 2, 3, 4, 5, 6, 7};
    network_setup(4, 5);
    
    int num_hosts = 4;
    std::vector<int> allreduce_hosts = {0, 1, 2, 3}; // select hosts for allreduce
    
    for (int host: allreduce_hosts){
        host_map.at(host).all_reduce_map[0] = 2;
        send(host_map.at(host), 0, 2);
        // host_map.at(host).send(0, host_map.at(host).all_reduce_map[0],);
    }
    std::cout << "Root Switch Result: " << switch_map[0].all_reduce_map[0] << std::endl;
    // WLOG Root Switch is Switch 0

    // End timing + Print Profiling
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Elapsed time: " << elapsed_time.count() << " microseconds" << std::endl;
    return 0;    
}