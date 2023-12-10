#pragma once
#include <iostream>
#include <limits>
#include "path.hpp"
#include <vector>

class load_balancer{
    public:
    
    static int balance(std::vector<Path> paths, std::string clientIP, int algorithm=0);
};