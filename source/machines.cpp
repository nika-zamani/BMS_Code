#include "machines.h"

template<typename T>
machine<T>::machine(T s, void (*d)(void), void(*dt)(machine*), void* userdata){
    state = s;
    drive = d;
    drivethis = dt;
    data = userdata;
    on = 0;
    timer = 1; // so start() needs to be called
}

template<typename T>
void machine<T>::run(){
    if(!timer){
        if(drive) drive();
        if(drivethis) drivethis(this);
        timer = resetval;
    }
}

// need explicit definitions here so when machines.o links with bms.o
// templated methods have already been compiled
// other solution would be do define all machines in this file
template class machine<slavestates_t>;
template class machine<masterstates_t>;
template class machine<uint32_t*>;
