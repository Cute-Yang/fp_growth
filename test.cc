#include <iostream>
#include <cstdint>
#include <set>

int main(){
    int a=12;
    std::uintptr_t b=reinterpret_cast<std::uintptr_t>(&a);
    int c=12;
    std::set<std::uintptr_t> address_saver;
    address_saver.insert(b);
    address_saver.insert(reinterpret_cast<std::uintptr_t>(&c));
    
    if(address_saver.find(b)!=address_saver.end()){
        std::cout << "we find the ptr of a in our set" << std::endl;
    }
    return 0;
}