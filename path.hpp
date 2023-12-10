#pragma once
#include <iostream>

class Path{
    public:    
        std::string lower_node; 
        std::string upper_node;
        Path(std::string lower_node, std::string upper_node);
        int utilization = 1;
        void send_data(int data, bool reduce);
};