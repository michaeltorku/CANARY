#include <iostream>
#include <set>
#include <random>
#include <vector>
#include <map>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"
#include "packet.hpp"


std::vector<std::vector<int>> ongoing_allreduces;
std::vector<std::thread> executions;
std::map<int, Host> host_map;
std::map<int, Switch> switch_map;

std::map<int, std::mutex> switch_mutex_map;
std::map<int, std::mutex> host_mutex_map;

const std::chrono::seconds TIMEOUT(1);
std::atomic<bool> keep_running(true);

// WLOG Root Switch is Switch 0
int root_switch = 0;

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
    

    for (int host_id=0; host_id<number_of_hosts; host_id++){
        std::string host_rep = "H"+std::to_string(host_id);
        std::vector<Path> const_arg = all_paths.contains(host_rep)? all_paths[host_rep]:std::vector<Path>{};
        host_map.emplace(host_id, Host(host_id, 2, const_arg));
        host_mutex_map.emplace(std::piecewise_construct, 
                           std::forward_as_tuple(host_id), 
                           std::forward_as_tuple());
    }
    for (int switch_id=0; switch_id<number_of_switches; switch_id++){
        std::string switch_rep = "S"+std::to_string(switch_id);
        std::vector<Path> const_arg = all_paths.contains(switch_rep)? all_paths[switch_rep]:std::vector<Path>{};
        switch_map.emplace(switch_id, Switch(switch_id, const_arg));
        switch_mutex_map.emplace(std::piecewise_construct, 
                           std::forward_as_tuple(switch_id), 
                           std::forward_as_tuple());
    }

}

void send(Host& host, int reduce_id, int data){
    int p_idx = load_balancer::balance(host.paths);
    Path &selected_path = host.paths[p_idx];

    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        target.descriptor_map[reduce_id] += host.descriptor_map[reduce_id];
        // receive(target, reduce_id, host.descriptor_map[reduce_id]); // send host initial data
    }
    // else{
    //     Host &target = host_map.at(target_node_id);
    //     receive(target, reduce_id, host.descriptor_map[reduce_id]); // send host initial data
    // }
}

// void receive(Host& host, int reduce_id, int data){
//     host.descriptor_map[reduce_id] += data;
//     send(host, reduce_id, data);
// }

// void send(Switch& toggle, int reduce_id, int data){
//     if (toggle.paths.size() == 0){
//         return;
//     }
//     int p_idx = load_balancer::balance(toggle.paths);
//     Path & selected_path = toggle.paths[p_idx];
//     int target_node_id = selected_path.upper_node[1] - '0';
//     if (selected_path.upper_node[0] == 'S'){
//         Switch &target = switch_map.at(target_node_id);
//         receive(target, reduce_id, data); // switch only sends data received
//     }
//     // else{
//     //     Host &target = host_map.at(target_node_id);
//     //     receive(target, reduce_id, data); // switch only sends data received (should agg and send data on timeout)
//     // }
// }

void receive(Switch& toggle, int reduce_id, int data){
    std::cout << toggle.id << " received " << data << std::endl;
    toggle.descriptor_map[reduce_id] += data;
    // send(toggle, reduce_id, data);
}


void forward_data() {
    while (keep_running) {
        // Perform the task
        for (auto & s: switch_map){
            std::cout << "switch " << s.second.id << std::endl;
            Switch & toggle = s.second;
            if (toggle.paths.size() == 0){
                continue;
            }
            int p_idx = load_balancer::balance(toggle.paths);
            Path & selected_path = toggle.paths[p_idx];
            int target_node_id = selected_path.upper_node[1] - '0';
            Switch &target = switch_map.at(target_node_id);
            for (auto reduce_id__data_pair : toggle.descriptor_map){
                int reduce_id = reduce_id__data_pair.first; int data = reduce_id__data_pair.second;
                target.descriptor_map[reduce_id] += data;
                std::cout << target.id << " received " << data << std::endl;
                toggle.descriptor_map[reduce_id] = 0;
            }            
        }

        // Sleep for x seconds
        std::this_thread::sleep_for(TIMEOUT);
    }
}

int main(){
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int> host_ids = {0, 1, 2, 3, 4, 5, 6, 7};
    network_setup(4, 5);
    
    int num_hosts = 4;
    std::vector<int> allreduce_hosts = {0, 1, 2, 3}; // select hosts for allreduce

    int expected = 0;
    std::random_device rd;  // get a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(100, 50000);
    int reduce_id = distr(gen);

    for (int host: allreduce_hosts){
        auto num = distr(gen);
        expected += num;
        host_map.at(host).descriptor_map[reduce_id] = Packet(reduce_id, 1, allreduce_hosts.size(), num);
        // host_map.at(host).descriptor_map[0] = num;
        executions.emplace_back([host](){
            send(host_map.at(host), reduce_id, host_map.at(host).descriptor_map[0]);
            });
        // host_map.at(host).send(0, host_map.at(host).descriptor_map[0],);
    }

    std::thread timeoutThread(forward_data);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    keep_running = false;
    if (timeoutThread.joinable()){
        timeoutThread.join();
    }
    for (auto & th : executions){
        if (th.joinable()){
            th.join();
        }
    }
    std::cout << "Root Switch Result: " << switch_map[0].descriptor_map[0] << " Expected: " << expected <<std::endl;

    // End timing + Print Profiling
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    std::cout << "Elapsed time: " << elapsed_time.count() << " seconds" << std::endl;
    return 0;    
}

// MAKE PACKET MORE ROBUST
// PROTECT RESOURCES
// ADD LOAD-BALANCING
// ADD DIFFERENT STRUCTURES