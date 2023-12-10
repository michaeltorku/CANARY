#include <iostream>
#include <set>
#include <random>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"
#include "packet.hpp"

std::unordered_map<std::string, std::vector<Path>> forward_paths;
std::unordered_map<std::string, std::vector<Path>> backward_paths;
std::vector<std::vector<int>> ongoing_allreduces;
std::vector<std::thread> data_threads;
std::vector<std::thread> broadcast_threads;

std::mutex switch_map_mutex;
std::mutex host_map_mutex;
std::mutex forward_paths_mutex;
std::mutex backward_paths_mutex;

std::unordered_map<int, Host> host_map;
std::unordered_map<int, Switch> switch_map;

std::unordered_map<int, std::mutex> switch_mutex_map;
std::unordered_map<int, std::mutex> host_mutex_map;

const std::chrono::seconds TIMEOUT(1);
std::atomic<bool> keep_running(true);

// WLOG Root Switch is Switch 0
int root_switch = 0;

std::vector<std::vector<std::string>> tree_types = {

    // std::vector<std::string> medium_sized_tree = 0
    {
        "H0:S3", 
        "H1:S3", 
        "H2:S4", 
        "H3:S4",
        "S3:S1",
        "S3:S2",
        "S4:S1",
        "S4:S2",
        "S1:S0",
        "S2:S0",
    },
    // std::vector<std::string> large_sized_tree = 1
    {
        "S1:S0",
        "S2:S0",
        "S3:S0",
        "S6:S0",
        "S7:S0",
        "S2:S1",
        "S3:S1",
        "S6:S1",
        "S7:S1",
        "S5:S2",
        "S4:S2",
        "S4:S3",
        "S5:S3",
        "S8:S6",
        "S9:S6",
        "S8:S7",
        "S9:S7",
        "H1:S4",
        "H0:S4",
        "H2:S5",
        "H3:S5",
        "H4:S8",
        "H5:S8",
        "H6:S9",
        "H7:S9"
        
    },
    // std::vector<std::string> small_sized_tree_congestion_unaware = 2
    {
        "H0:S3", 
        "H1:S3", 
        "H2:S4", 
        "H3:S4",
        "S3:S1",
        "S4:S2",
        "S1:S0",
        "S2:S0",
    },

    // std::vector<std::string> medium_sized_ring = 3
    {
        "S1:S2",
        "S2:S3",
        "S3:S4",
        "S4:S5",
        "S5:S0",
        "H0:S0",
        "H1:S1",
        "H2:S2",
        "H3:S3",
        "H4:S4",
        "H5:S5"
    },

    // std::vector<std::string> spine_leaf_topology = 4
    {
        "S1:S0",
        "S2:S0",
        "S3:S0",
        "S4:S0",
        "H0:S5",
        "H1:S6",
        "H2:S7",
        "H3:S8", 
        "S5:S1",
        "S5:S2", 
        "S5:S3", 
        "S5:S4", 
        "S6:S1", 
        "S6:S2", 
        "S6:S3", 
        "S6:S4", 
        "S7:S1", 
        "S7:S2", 
        "S7:S3", 
        "S7:S4", 
        "S8:S1", 
        "S8:S2", 
        "S8:S3", 
        "S8:S4",
    },
};

void select_tree_configurations(int & num_hosts, int & num_switches, std::vector<int>& allreduce_hosts, int config_num=0){
    switch (config_num)
    {
    case 0: //medium_sized tree
        num_hosts = 4;
        num_switches = 5;
        allreduce_hosts = {0, 1, 2, 3}; 
        break;
    case 1: // large_sized tree
        num_hosts = 8;
        num_switches = 10;
        allreduce_hosts = {0, 1, 2, 3, 4, 5, 6, 7};
        break;
    case 2: // small_sized tree
        num_hosts = 4;
        num_switches = 5;
        allreduce_hosts = {0, 1, 2, 3}; 
        break;
    case 3: // medium_sized ring
        num_hosts = 6;
        num_switches = 6;
        allreduce_hosts = {0, 1, 2, 3, 4, 5}; 
        break;
    case 4: // spine-leaf topology
        num_hosts = 4;
        num_switches = 9;
        allreduce_hosts = {0, 1, 2, 3}; 
        break;
    default:
        break;
    }
}
void network_setup(int number_of_hosts, int number_of_switches, int tree_num){
    std::vector<std::string> pairings = tree_types[tree_num];
    
    for (std::string & pair: pairings){
        std::istringstream iss(pair);
        std::string lower_node, upper_node;
        
        // Splitting the string by ':'
        std::getline(iss, lower_node, ':');
        std::getline(iss, upper_node);

        // Create a Path object and add it to the paths vector
        if (forward_paths.contains(lower_node)){
            std::vector<Path> & path_list = forward_paths[lower_node];
            path_list.emplace_back(lower_node, upper_node);
        }else{
            forward_paths[lower_node] = {Path(lower_node, upper_node)};
        }

        if (backward_paths.contains(upper_node)){
            std::vector<Path> & path_list = backward_paths[upper_node];
            path_list.emplace_back(upper_node, lower_node);
        }else{
            backward_paths[upper_node] = {Path(upper_node, lower_node)};
        }
        
    }
    
    // Switches and Hosts have paths not to be used (forward_paths is used)
    for (int host_id=0; host_id<number_of_hosts; host_id++){
        std::string host_rep = "H"+std::to_string(host_id);
        std::vector<Path> const_arg = forward_paths.contains(host_rep)? forward_paths[host_rep]:std::vector<Path>{};
        host_map.emplace(host_id, Host(host_id, 2, const_arg));
        host_mutex_map.emplace(std::piecewise_construct, 
                           std::forward_as_tuple(host_id), 
                           std::forward_as_tuple());
    }
    for (int switch_id=0; switch_id<number_of_switches; switch_id++){
        std::string switch_rep = "S"+std::to_string(switch_id);
        std::vector<Path> const_arg = forward_paths.contains(switch_rep)? forward_paths[switch_rep]:std::vector<Path>{};
        switch_map.emplace(switch_id, Switch(switch_id, const_arg));
        switch_mutex_map.emplace(std::piecewise_construct, 
                           std::forward_as_tuple(switch_id), 
                           std::forward_as_tuple());
    }

}
std::vector<Path>& getPaths(Host & h){
    std::string host_rep = "H"+std::to_string(h.id);
    return forward_paths[host_rep];

}

std::vector<Path>& getPaths(Switch & s){
    std::string switch_rep = "S"+std::to_string(s.id);
    return forward_paths[switch_rep];
    
}

int getBackwardPathIndex(Path& selected_path){
    backward_paths_mutex.lock();
    auto it = std::find_if(backward_paths[selected_path.upper_node].begin(), backward_paths[selected_path.upper_node].end(), [&selected_path](Path path){
                return path.upper_node == selected_path.lower_node;
            });
    int res =  std::distance(backward_paths[selected_path.upper_node].begin(), it);
    backward_paths_mutex.unlock();
    return res;
}
void send(Host& host, int reduce_id, Packet data, std::string reduce_ip_address){
    host_map_mutex.lock();
    switch_map_mutex.lock();
    forward_paths_mutex.lock();
    
    std::vector<Path>& host_paths = getPaths(host);
    int p_idx = load_balancer::balance(host_paths, reduce_ip_address);

    Path &selected_path = host_paths[p_idx];
    std::this_thread::sleep_for(std::chrono::seconds(selected_path.utilization));
    selected_path.utilization = std::min(selected_path.utilization+1, 7);
    std::thread(
        [p_idx, &host](){
            std::this_thread::sleep_for(std::chrono::seconds(8));
            forward_paths_mutex.lock();
            getPaths(host)[p_idx].utilization -= 1;
            forward_paths_mutex.unlock();

        }
    ).detach();

    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        target.descriptor_map[reduce_id].first += host.descriptor_map[reduce_id];
        int index = getBackwardPathIndex(selected_path);
        target.descriptor_map[reduce_id].second.push_back(index);
    }
    forward_paths_mutex.unlock();
    switch_map_mutex.unlock();
    host_map_mutex.unlock();
}


void forward_data(int reduce_id, int num_hosts, std::string reduce_ip_address) {
    bool loop_cond = true;
    while (loop_cond) {
        switch_map_mutex.lock();
        loop_cond = switch_map.at(root_switch).descriptor_map[reduce_id].first.counter != num_hosts;
        std::cout << "now root switch has: " << switch_map.at(root_switch).descriptor_map[reduce_id].first.counter << " bits of data."<< std::endl;
        for (auto & s: switch_map){
            Switch & toggle = s.second;
            if (toggle.paths.size() == 0){
                continue;
            }
            forward_paths_mutex.lock();
            int p_idx = load_balancer::balance(getPaths(toggle), reduce_ip_address);
            Path & selected_path = toggle.paths[p_idx]; 
            int target_node_id = selected_path.upper_node[1] - '0';
            Switch &target = switch_map.at(target_node_id);
            forward_paths_mutex.unlock(); 
            for (auto reduce_id__data_pair : toggle.descriptor_map){
                int reduce_id = reduce_id__data_pair.first; 
                Packet data = reduce_id__data_pair.second.first;
                target.descriptor_map[reduce_id].first += data;
                if (data.data){
                    int index = getBackwardPathIndex(selected_path);
                    target.descriptor_map[reduce_id].second.push_back(index);
                }
                std::cout << target.id << " received " << data.data << std::endl;
                toggle.descriptor_map[reduce_id].first = Packet();
            }  
                     
        }


        switch_map_mutex.unlock();
        std::this_thread::sleep_for(TIMEOUT);
    }
}

void broadcast_data(int reduce_id, int data, Switch & s){
    backward_paths_mutex.lock();
    switch_map_mutex.lock();
    
    std::vector<Path> & rev_paths = backward_paths["S"+std::to_string(s.id)];    
    for (int p_idx : s.descriptor_map[reduce_id].second){
        std::string target_str = rev_paths[p_idx].upper_node;
        if (target_str[0] == 'H'){
            int target_host_id = target_str[1] - '0';
            host_map_mutex.lock();
            host_map.at(target_host_id).result = data;
            host_map_mutex.unlock();
            
        }else{
            int target_switch_id = target_str[1] - '0';
            // broadcast_threads.emplace_back(broadcast_data, reduce_id, data, std::ref(switch_map.at(target_switch_id)));
            broadcast_threads.emplace_back(broadcast_data, reduce_id, data, std::ref(switch_map.at(target_switch_id))).detach();
            // detach is used to simulate packet loss

        }
    switch_map_mutex.unlock();    
    backward_paths_mutex.unlock();
    }
}

std::string generatePrivateIP() {
    int firstOctet = rand() % 3;
    std::ostringstream ip;

    if (firstOctet == 0) { // 10.x.x.x
        ip << "10." << (rand() % 256) << "." << (rand() % 256) << "." << (rand() % 256);
    } else if (firstOctet == 1) { // 172.16.x.x to 172.31.x.x
        ip << "172." << (16 + rand() % 16) << "." << (rand() % 256) << "." << (rand() % 256);
    } else { // 192.168.x.x
        ip << "192.168." << (rand() % 256) << "." << (rand() % 256);
    }
    return ip.str();
}



int main(){
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int> allreduce_hosts; int num_hosts; int num_switches;
    const int TREE_NUM = 4;
    select_tree_configurations(num_hosts, num_switches, allreduce_hosts, TREE_NUM);
    network_setup(num_hosts, num_switches, 1); 


    int expected = 0;
    std::random_device rd;  // get a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(100, 50000);

    int reduce_id = distr(gen);
    std::string reduce_ip_address = generatePrivateIP();

    for (int host: allreduce_hosts){
        auto num = distr(gen);
        expected += num;
        host_map.at(host).descriptor_map[reduce_id] = Packet(reduce_id, 1, allreduce_hosts.size(), num);
        data_threads.emplace_back([host, reduce_id, reduce_ip_address](){
            send(host_map.at(host), reduce_id, host_map.at(host).descriptor_map[0], reduce_ip_address);
            });
        
    }

    for (auto & th : data_threads){
        if (th.joinable()){
            th.join();
        }
    }

    std::thread timeoutThread(forward_data, reduce_id, num_hosts, reduce_ip_address);
    if (timeoutThread.joinable()){
        timeoutThread.join();
    }

    switch_map_mutex.lock();
    std::cout << "Root Switch Result: " << switch_map[root_switch].descriptor_map[reduce_id].first.data << " Expected: " << expected <<std::endl;
    // End timing + Print Profiling (Reduce Phase)
    auto end_reduce_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_reduce_time - start_time);
    std::cout << "Elapsed (Reduce) time: " << elapsed_time.count() << " milliseconds" << std::endl;

    std::cout << "----------------------------------------------------------------------------------" << std::endl;
    int reduce_result = switch_map[root_switch].descriptor_map[reduce_id].first.data;
    for (auto & s : switch_map){
        std::cout << s.second.id << " has size: " << s.second.descriptor_map[reduce_id].second.size() << std::endl;
        std::cout << "content of vector: ";
        for (int i: s.second.descriptor_map[reduce_id].second){
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
    // std::cout << reduce_result << " " << expected << std::endl;
    std::thread broadcastThread(broadcast_data, reduce_id, reduce_result, std::ref(switch_map.at(root_switch)));
    switch_map_mutex.unlock();

    if (broadcastThread.joinable()){
        broadcastThread.join();
    }
    for (auto & th : broadcast_threads){
        if (th.joinable()){
            th.join();
        }
    }
    // End timing + Print Profiling (Broadcast Phase)
    auto end_broadcast_time = std::chrono::high_resolution_clock::now();
    auto final_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_broadcast_time - start_time);
    auto elapsed_b_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_broadcast_time - end_reduce_time);

    std::this_thread::sleep_for(std::chrono::seconds(3)); // Used to simulate waiting for retransmission signal from hosts

    std::cout << "Elapsed (Broadcast) time: " << elapsed_b_time.count() << " milliseconds" << std::endl;
    std::cout << "Final time: " << final_time.count() << " milliseconds" << std::endl;

    host_map_mutex.lock();
    std::cout << "hosts size: " << allreduce_hosts.size() << std::endl;
    for (int h : allreduce_hosts){
        std::cout << "host " << h << " got: "<<  host_map.at(h).result << std::endl;
    }
    host_map_mutex.unlock();

    return 0;    
}