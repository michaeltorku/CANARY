#pragma once
#include <iostream>
#include "host.hpp"


class Path{
    public:
        
        Node lower_host;
        Node upper_host;
        Path(Node lower_host, Node upper_host):lower_host(lower_host), upper_host(upper_host){}
        double utilization = 0.3;
        void send_data(int data, bool reduce);
};