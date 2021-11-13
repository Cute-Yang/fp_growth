#include "fp_growth_v2.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>

FpGrowth::TreeNode* FpGrowth::InitTreeNode(std::string node_name,TreeNode* parent,uint32_t node_count){
    TreeNode* node=new TreeNode;
    node->node_name=node_name;
    node->node_count=node_count;
    node->parent=parent;
    node->next=nullptr;
    node->children=std::map<std::string,TreeNode*>();
    return node;
}

FpGrowth::HeadNode* FpGrowth::InitHeadNode(std::string node_name,uint32_t node_count){
    HeadNode* node=new HeadNode;
    node->node_name=node_name;
    node->node_count=node_count;
    node->next=nullptr;
    return node;
}

void FpGrowth::UpdateHeadTable(std::map<std::string,HeadNode*>& head_table,std::list<std::string>& trans,uint32_t count){
    for(auto trans_item=trans.begin();trans_item!=trans.end();trans_item++){
        if(head_table.find(*trans_item)==head_table.end()){
            HeadNode* head_node=InitHeadNode(*trans_item,count);
            head_table.insert(std::pair<std::string,HeadNode*>(*trans_item,head_node));
        }else{
            (head_table.at(*trans_item)->node_count)++;
        }
    }
}

std::map<std::string,FpGrowth::HeadNode*> FpGrowth::FilterHeadTable(std::map<std::string,HeadNode*> old_head_table){
    std::map<std::string,HeadNode*> head_talbe;
    for(auto item=old_head_table.begin();item!=old_head_table.end();item++){
        if((item->second->node_count)<min_support){
            continue;
        }
        head_talbe.insert(std::move(*item));
    }
    return head_talbe;
}

std::map<std::string,uint32_t> FpGrowth::ItemToRank(std::map<std::string,HeadNode*>& head_table){
    std::map<std::string,uint32_t> result;
    std::list<HeadNode*> temp_list;
    for(auto item=head_table.begin();item!=head_table.end();item++){
        temp_list.emplace_back(item->second);
    }
    auto sort_func=[](HeadNode* n1,HeadNode* n2)->bool {
        return ((n1->node_count)>(n2->node_count));
    };
    temp_list.sort(sort_func);
    uint32_t index=0;
    for(auto item=temp_list.begin();item!=temp_list.end();item++){
        result.insert(std::pair<std::string,uint32_t>((*item)->node_name,index));
        index++;
    }
    return result;
}

std::map<uint32_t,std::string> FpGrowth::RankToItem(std::map<std::string,uint32_t>& item_to_rank){
    std::map<uint32_t,std::string> result;
    for(auto item=item_to_rank.begin();item!=item_to_rank.end();item++){
        result.insert(std::pair<uint32_t,std::string>(item->second,item->first));
    }
    return result;
}


std::list<std::string> FpGrowth::SortSingleData(std::map<std::string,uint32_t>& item_2_rank,std::map<uint32_t,std::string>& rank_2_item,std::list<std::string>& trans){
    std::list<std::string> sorted_trans;
    std::vector<uint32_t> temp_vector;
    temp_vector.reserve(trans.size());
    for(auto item=trans.begin();item!=trans.end();item++){
        temp_vector.emplace_back(item_2_rank.at(*item));
    }
    
    auto sort_func=[](uint32_t a,uint32_t b)->bool{
        return a<b;
    };
    std::sort(temp_vector.begin(),temp_vector.end(),sort_func);
    for(size_t i=0;i<temp_vector.size();i++){
        sorted_trans.emplace_back(rank_2_item.at(temp_vector[i]));
    }
    return trans;
}