#include "fp_growth.h"
#include <stdexcept>
#include <map>
#include <algorithm>

std::vector<std::pair<std::string,size_t>> ItemCount(std::vector<std::vector<std::string>> data,float min_support){
    size_t data_size=data.size();
    if (data_size==0){
        throw std::runtime_error("the data is empty,please check!");
    }
    std::map<std::string,size_t> item_count_map;
    for(size_t i=0;i<data_size;i++){
        std::vector<std::string>& item_seq=data[i];
        for(auto item=item_seq.begin();item<item_seq.end();item++){
            auto flag=item_count_map.find(*item);
            if(flag==item_count_map.end()){
                item_count_map.insert(std::pair<std::string,int>(*item,1));
            }else{
                item_count_map[*item]++;
            }
        }
    }
    size_t min_support=(int)(min_support*data_size);
    std::vector<std::pair<std::string,size_t>> result;
    result.reserve(item_count_map.size());
    for(auto item=item_count_map.begin();item!=item_count_map.end();item++){
        if(item->second < min_support){
            continue;
        }
        result.push_back(*item);
    }
    
}