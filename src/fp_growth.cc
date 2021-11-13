#include "fp_growth.h"
#include <stdexcept>
#include <map>
#include <algorithm>
#include <iostream>
#include <list>
#include <set>

bool SearchChild(std::set<std::uintptr_t> children,FpNode* insert_node){
    bool exist=false;
    std::uintptr_t node_hash=reinterpret_cast<std::uintptr_t>(insert_node);
    if (children.find(node_hash)!=children.end()){
        exist=true;
    }
    return exist;
}

HeadNode* InitHeadNode(std::string node_name,size_t node_count){
    HeadNode* hnode=new HeadNode;
    hnode->node_name=node_name;
    hnode->node_count=node_count;
}

//use linked list to restore sort result
std::vector<HeadNode*> ItemCount(std::vector<std::vector<std::string>>& data,float min_support_degree){
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
    //the min support degree
    size_t min_support=(size_t)(min_support_degree*data_size);
    std::list<HeadNode*> result;
    
    for(auto item=item_count_map.begin();item!=item_count_map.end();item++){
        if(item->second < min_support){
            continue;
        }
        result.push_back(InitHeadNode(item->first,item->second));
    }

    //we just want to compare the second value
    auto sort_func=[](HeadNode* n1,HeadNode* n2)->bool {
        return (n1->node_count>n2->node_count);
    };
    result.sort(sort_func);
    //convert list->vecotr
    std::vector<HeadNode*> result_vector;
    result_vector.reserve(result.size());
    for(auto item=result.begin();item!=result.end();item++){
        result_vector.emplace_back(*item);
    }
    return result_vector;
}


//we can not got the size of sorted data,use linked list to restore
std::list<std::vector<std::string>> DataSort(std::vector<std::vector<std::string>>& data,std::vector<HeadNode*>& head_table){
    size_t data_size=data.size();
    auto sort_func=[](size_t a,size_t b)->bool{
        return a<b;
    };

    std::map<std::string,size_t> item_2_index;
    std::map<size_t,std::string> index_2_item;
    for(size_t i=0;i<head_table.size();i++){
        item_2_index.insert(std::pair<std::string,size_t>(head_table[i]->node_name,i));
        index_2_item.insert(std::pair<size_t,std::string>(i,head_table[i]->node_name));
    }

    std::list<std::vector<std::string>> sorted_data;
    for(size_t i=0;i<data.size();i++){
        auto& sequence=data[i];
        size_t cnt=0;
        std::list<size_t> temp_vec;
        for(auto item=sequence.begin();item<sequence.end();item++){
            if (item_2_index.find(*item)!=item_2_index.end()){
                temp_vec.push_back(item_2_index.at(*item));
            }
            cnt++;
        }
        temp_vec.sort(sort_func);
        std::vector<std::string> sorted_sequence;
        sorted_sequence.reserve(cnt);
        for(auto item=temp_vec.begin();item!=temp_vec.end();item++){
            sorted_sequence.emplace_back(index_2_item.at(*item));
        }
        sorted_data.emplace_back(std::move(sorted_sequence));
    }
    return sorted_data;
}

FpNode* InitFpNode(std::string& tree_name,FpNode* parent_node){
    FpNode* FpTree=new FpNode;
    FpTree->node_name=tree_name;
    FpTree->parent=parent_node;
    FpTree->next=nullptr;
    FpTree->children=std::map<std::string,FpNode*>();
    FpTree->node_count=1;
    return FpTree;
}

//free memory recurrently
void DeleteFpTree(FpNode* FpTree){
    std::map<std::string,FpNode*>& children=FpTree->children;
    //leaf node
    if(children.size()==0){
        std::cout << FpTree->node_name << "->" << FpTree->node_count << std::endl;
        delete FpTree;
    }else{
        for(auto item=children.begin();item!=children.end();item++){
            DeleteFpTree(item->second);
        }
        std::cout << FpTree->node_name << "->" << FpTree->node_count << std::endl;
        delete FpTree;
    }
}

//update head table linked
void UpdateHeadTable(HeadNode* head_node,FpNode* node){
    if ((head_node->next)==nullptr){
        head_node->next=node;
    }else{
        FpNode* temp=head_node->next;
        while((temp->next)!=nullptr){
            temp=temp->next;
        }
        temp->next=node;
    }
}

void DeleteHeadTalbe(std::vector<HeadNode*> head_table){
    for(size_t i=0;i<head_table.size();i++){
        delete head_table[i];
    }
}


std::list<std::string> FindPrefixPath(FpNode* leaf,std::string& root_name){
    std::list<std::string> prefix_path;
    while((leaf->node_name)!=root_name){
        prefix_path.push_back(leaf->node_name);
        std::cout << leaf->node_name << " ";
        leaf=leaf->parent;
    }
    std::cout << std::endl;
    return prefix_path;
}


void UpdateFpTree(std::vector<HeadNode*> head_table,std::vector<std::string> single_data,FpNode* FpTree,std::map<std::string,size_t> head_map){
    for(auto item=single_data.begin();item!=single_data.end();item++){
        std::map<std::string,FpNode*>& children=FpTree->children;
        //if find the freq item in current node
        if(children.find(*item)!=children.end()){
            (children.at(*item)->node_count)++;
        }else{
            FpNode* node=InitFpNode(*item,FpTree);
            children.insert(std::pair<std::string,FpNode*>(*item,node));
            size_t index=head_map.at(*item);
            UpdateHeadTable(head_table[index],node);
        }
        FpTree = children.at(*item);
    }
}

int main(){
    std::vector<std::vector<std::string>> data = {
        {"A","B","C","E","F","O"},
        {"A","C","G"},
        {"E","I"},
        {"A","C","D","E","G"},
        {"A","C","E","G","L"},
        {"E","J"},
        {"A","B","C","E","F","P"},
        {"A","C","D"},
        {"A","C","E","G","M"},
        {"A","C","E","G","N"}
    };
    
    auto head_table=ItemCount(data,0.2);
    auto sorted_data=DataSort(data,head_table);
    std::cout <<  "--------------------------------" << std::endl;
    for (auto v1=sorted_data.begin();v1!=sorted_data.end();v1++){
        for (auto item=(*v1).begin();item<(*v1).end();item++){
            std::cout<< (*item) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
    std::string root_name("root");
    FpNode* FpTree=InitFpNode(root_name,nullptr);
    std::map<std::string,size_t> item_2_index;
    for(size_t i=0;i<head_table.size();i++){
       item_2_index.insert(std::pair<std::string,size_t>((head_table[i])->node_name,i)); 
    }
    for(auto single_item=sorted_data.begin();single_item!=sorted_data.end();single_item++){
        UpdateFpTree(head_table,*single_item,FpTree,item_2_index);
    }
    std::cout << "construct fp tree successfully....." << std::endl;
    for(auto item=head_table.begin();item!=head_table.end();item++){
        std::cout << (*item)->next->node_count << std::endl;
    }
    
    FpNode* test_node=head_table[head_table.size()-2]->next;
    while (test_node!=nullptr){
        FindPrefixPath(test_node,root_name);
        test_node=test_node->next;
    }
    // FindPrefixPath(head_table[head_table.size()-1]->next,root_name);
    // DeleteFpTree(FpTree);
    return 0;
}