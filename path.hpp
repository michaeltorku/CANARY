#pragma once
#include <iostream>

class Path{
    public:    
        std::string lower_node; 
        std::string upper_node;
        Path(std::string lower_node, std::string upper_node);
        double utilization = 0.3;
        void send_data(int data, bool reduce);
};