#include <iostream>
#include <set>
#include <random>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"
#include "packet.hpp"

std::unordered_map<std::string, std::vector<Path>> all_paths;
std::vector<std::vector<int>> ongoing_allreduces;
std::vector<std::thread> data_threads;

std::mutex switch_map_mutex;
std::mutex host_map_mutex;
std::mutex all_paths_mutex;

std::unordered_map<int, Host> host_map;
std::unordered_map<int, Switch> switch_map;

std::unordered_map<int, std::mutex> switch_mutex_map;
std::unordered_map<int, std::mutex> host_mutex_map;

const std::chrono::seconds TIMEOUT(1);
std::atomic<bool> keep_running(true);

// WLOG Root Switch is Switch 0
int root_switch = 0;

void network_setup(int number_of_hosts, int number_of_switches){
    
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
    
    // Switches and Hosts have paths not to be used (all_paths is used)
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
std::vector<Path>& getPaths(Host & h){
    std::string host_rep = "H"+std::to_string(h.id);
    return all_paths[host_rep];

}
std::vector<Path>& getPaths(Switch & s){
    std::string switch_rep = "S"+std::to_string(s.id);
    return all_paths[switch_rep];
    
}
void send(Host& host, int reduce_id, Packet data){
    host_map_mutex.lock();
    switch_map_mutex.lock();
    all_paths_mutex.lock();
    
    std::vector<Path>& host_paths = getPaths(host);
    int p_idx = load_balancer::balance(host_paths);

    Path &selected_path = host_paths[p_idx];
    std::this_thread::sleep_for(std::chrono::seconds(selected_path.utilization));
    selected_path.utilization = std::min(selected_path.utilization+1, 7);
    std::thread(
        [p_idx, &host](){
            std::this_thread::sleep_for(std::chrono::seconds(8));
            all_paths_mutex.lock();
            getPaths(host)[p_idx].utilization -= 1;
            all_paths_mutex.unlock();

        }
    ).detach();

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
    all_paths_mutex.unlock();
    switch_map_mutex.unlock();
    host_map_mutex.unlock();
}


void forward_data(int reduce_id, int num_hosts) {
    bool loop_cond = true;
    while (loop_cond) {
        switch_map_mutex.lock();
        loop_cond = switch_map.at(root_switch).descriptor_map[reduce_id].counter != num_hosts;
        std::cout << "now: " << switch_map.at(root_switch).descriptor_map[reduce_id].counter << std::endl;
        for (auto & s: switch_map){
            std::cout << "switch " << s.second.id << std::endl;
            Switch & toggle = s.second;
            if (toggle.paths.size() == 0){ // ONE
                continue;
            }
            all_paths_mutex.lock();
            int p_idx = load_balancer::balance(getPaths(toggle)); //ONE
            Path & selected_path = toggle.paths[p_idx]; // ONE
            int target_node_id = selected_path.upper_node[1] - '0';
            Switch &target = switch_map.at(target_node_id);
            all_paths_mutex.unlock(); 
            for (auto reduce_id__data_pair : toggle.descriptor_map){
                int reduce_id = reduce_id__data_pair.first; 
                Packet data = reduce_id__data_pair.second;
                target.descriptor_map[reduce_id] += data;
                std::cout << target.id << " received " << data.data << std::endl;
                toggle.descriptor_map[reduce_id] = Packet();
            }  
                     
        }


        switch_map_mutex.unlock();
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
        data_threads.emplace_back([host, reduce_id](){
            send(host_map.at(host), reduce_id, host_map.at(host).descriptor_map[0]);
            });
        // host_map.at(host).send(0, host_map.at(host).descriptor_map[0],);
    }

    std::thread timeoutThread(forward_data, reduce_id, num_hosts);

    // std::this_thread::sleep_for(std::chrono::seconds(5));

    if (timeoutThread.joinable()){
        timeoutThread.join();
    }

    std::cout << "Root Switch Result: " << switch_map[root_switch].descriptor_map[reduce_id].data << " Expected: " << expected <<std::endl;
    for (auto & th : data_threads){
        if (th.joinable()){
            th.join();
        }
    }
    

    // End timing + Print Profiling
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Elapsed time: " << elapsed_time.count() << " milliseconds" << std::endl;

    return 0;    
}

// ADD LOAD-BALANCING
// ADD DIFFERENT STRUCTURES