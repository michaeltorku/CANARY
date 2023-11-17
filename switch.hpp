#include <iostream>


class Switch:public Node{


public:
    // Switch();
    ~Switch();
    void Receive(const Switch& other, int data); //change to packet
    void Send(const Host& other);
    void request();
};