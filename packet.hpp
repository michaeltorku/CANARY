#include <iostream>


class Packet{
    public:
    int id; // identifier of the packet
    int counter; // number of reduced packets
    int num_hosts; // number of hosts in all reduce
    int children; // carries identifier of the port from which the packet was received 
    // when switch cannot store a packet because of collisions
    int bypass; // if set, the switch should not process the packet, just forward it
    int multicast; // if set, packet should be multicast to the children of the switch
    int switch_address; // carries switch address when switch cannot store a packet because of collisions
    int data; // data to be reduced
    int destination; // IP address or identifier of leader host

    //bypass and multicast are initialized to 0

    Packet(int id, int counter, int num_hosts, int data);
};