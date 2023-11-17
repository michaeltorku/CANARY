#include <iostream>
#include <vector>
#include <memory>
#include "path.hpp"


class Node {
    

public:
    Node(){};
    std::vector<Path> paths; // Using smart pointers to Node
    Node(const std::vector<Path>& paths) : paths(paths) {
    }

    void addPath(const Path path) {
        paths.push_back(path);
    }
};