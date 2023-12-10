# README 

## Canary Project: Michael Torku & Andrew Cheng


This project is developed using C++20 and utilizes features specific to this version. To ensure successful compilation, please use a compiler that supports C++20 and set the appropriate version flags accordingly.


### To Compile The Program:
`g++ std=c++2a *.cpp -o exe`

### To Execute The Program:
`./exe`


### To Simulate Using Different Tree Topologies:
- Vary the ``const int TREE_NUM`` variable in `main.cpp`
- The different network topologies are detailed in the `select_tree_configurations` method also in `main.cpp`

### Notes
- The main branch focuses on the reduce phase - the analysis and experimentation conducted with regards to this project apply (without loss of generality) to the broadcast phase.
- The implementation of the broadcast phase can be found in the broadcast branch and includes approaches to simulate packet loss and the root switch waiting for retransmission signals from allreduce hosts.
