#include <iostream>
#include <thread>
#include <set>
#include <random>
#include <vector>
#include <map>
#include <sstream>
#include <mutex>
#include "host.hpp"
#include "switch.hpp"



std::vector<std::vector<int>> ongoing_allreduces;
std::vector<std::thread> executions;
std::mutex switch_map_mutex;
std::mutex host_map_mutex;
std::map<int, Host> host_map;
std::map<int, Switch> switch_map;

// Randomly select 3 hosts for allreduce TODO:USE
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

// Create paths, hosts, and switches
void network_setup(int number_of_hosts, int number_of_switches){
    std::map<std::string, std::vector<Path>> all_paths; // Map indexed by path src (holds all paths starting from src)
    std::vector<std::string> pairings = {"H0:S3", "H1:S3", "H2:S4", "H3:S4",
    "S3:S1","S3:S2","S4:S1","S4:S2","S1:S0","S2:S0",
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
            path_list.push_back(Path(lower_node, upper_node));
        }else{
            all_paths[lower_node] = {Path(lower_node, upper_node)};
        }
        
    }
    

    for (int i=0; i<number_of_hosts; i++){
        std::string host_rep = "H"+std::to_string(i);
        std::vector<Path> const_arg = all_paths.contains(host_rep)? all_paths[host_rep]:std::vector<Path>{};
        // host_map.emplace(i, Host(i, 2, const_arg)); // id, data, paths
        host_map.try_emplace(i, i, 2, const_arg);
    }
    for (int i=0; i<number_of_switches; i++){
        std::string switch_rep = "S"+std::to_string(i);
        std::vector<Path> const_arg = all_paths.contains(switch_rep)? all_paths[switch_rep]:std::vector<Path>{};
        // switch_map.emplace(i, Switch(i, const_arg)); // id, paths
        switch_map.try_emplace(i, i, const_arg);
    }

}

void threadSend(Host& host, std::map<int, Host>& host_map, std::map<int, Switch>& switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex) {
    host.send(0, host.all_reduce_descriptor[0], host_map, switch_map, host_map_mutex, switch_map_mutex);
}

int main(){
    // std::vector<int> host_ids = {0, 1, 2, 3, 4, 5, 6, 7};
    
    network_setup(4, 5);
    int num_hosts = 4;
    std::vector<int> allreduce_hosts = {0, 1, 2, 3}; // TODO:select hosts for allreduce

    int expected = 0;
    std::random_device rd;  // get a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(10, 1000);
    std::cout << host_map.size() << " and: " << switch_map.size() << std::endl;
    for (int host: allreduce_hosts){
        // auto num = distr(gen);
        // expected += num;
        host_map.at(host).all_reduce_descriptor[0] = 2;
        executions.emplace_back(threadSend, std::ref(host_map.at(host)), std::ref(host_map), std::ref(switch_map), std::ref(host_map_mutex), std::ref(switch_map_mutex));
        // Host & hostt = host_map.at(host);
        // hostt.send(0, hostt.all_reduce_descriptor[0], host_map, switch_map, host_map_mutex, switch_map_mutex);

    }
    
    // WLOG Root Switch is Switch 0
    // for (auto& item : switch_map) {
    //     for (auto& th : item.second.executions) {
    //         if (th.joinable()) {
    //             th.join();
    //         }
    //     }
    // }
    

    for (auto& th : executions) {
    if (th.joinable()) {
        th.join();
    }
    }
    std::cout << "All Reduce is Complete" << std::endl;
    std::cout << "Root Switch Result: " << switch_map[0].all_reduce_descriptor[0] << std::endl;
    return 0;    
}