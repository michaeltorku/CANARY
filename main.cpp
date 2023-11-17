#include <iostream>
#include <set>
#include <random>
#include <vector>
#include "host.hpp"

std::vector<std::vector<int>> ongoing_allreduces;

//allreduces are run in batches of 3
std::vector<int> select_hosts_for_allreduce(std::set<int> host_ids){
    if (host_ids.size() < 3){
        std::cout << "Not enough hosts for allreduce" << std::endl;
        return;
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
    //FIX Set up host and switch network here
    // for (int i = 0; i<allreduce_hosts.size(); i++){
    //     Host(i, 2, std::vector<Path>{S()});
    //     // FIX SET UP PATHS
    // }

}

int main(){
    std::set<int> host_ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> allreduce_hosts = select_hosts_for_allreduce(host_ids);

    return 0;    
}