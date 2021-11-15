#include "fp_growth_v2.h"
#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <vector>

void show_set(std::set<std::string>& data){
    for (auto item=data.begin();item!=data.end();item++){
        std::cout << *item <<" ";
    }
    std::cout << std::endl;
}

void show_list(std::list<std::string>& data){
    for (auto item=data.begin();item!=data.end();item++){
        std::cout << *item <<" ";
    }
    std::cout << std::endl;
}


void show_children_keys(std::map<std::string,FpGrowth::TreeNode*>& children){
    for(auto item=children.begin();item!=children.end();item++){
        std::cout << item->first << " ";
    }
    std::cout << std::endl;
}
FpGrowth::TreeNode* FpGrowth::InitTreeNode(std::string node_name, TreeNode* parent, uint32_t node_count) {
    TreeNode* node = new TreeNode;
    node->node_name = node_name;
    node->node_count = node_count;
    node->parent = parent;
    node->next = nullptr;
    node->children = std::map<std::string, TreeNode*>();
    return node;
}

FpGrowth::HeadNode* FpGrowth::InitHeadNode(std::string node_name, uint32_t node_count) {
    HeadNode* node = new HeadNode;
    node->node_name = node_name;
    node->node_count = node_count;
    node->next = nullptr;
    return node;
}

void FpGrowth::UpdateHeadTable(std::map<std::string, HeadNode*>& head_table, std::list<std::string>& trans,
                               uint32_t count) {
    for (auto tran = trans.begin(); tran != trans.end(); tran++) {
        if (head_table.find(*tran)!=head_table.end()) {
            (head_table.at(*tran)->node_count) += count;
        } else {
            HeadNode* head_node = InitHeadNode(*tran, count);
            head_table.insert(std::pair<std::string, HeadNode*>(*tran, head_node));
        }
    }
}

std::map<std::string, FpGrowth::HeadNode*> FpGrowth::FilterHeadTable(std::map<std::string, HeadNode*> old_head_table) {
    std::map<std::string, HeadNode*> head_talbe;
    for (auto item = old_head_table.begin(); item != old_head_table.end(); item++) {
        if ((item->second->node_count) < min_support) {
            continue;
        }
        head_talbe.insert(*item);
    }
    return head_talbe;
}

std::map<std::string, uint32_t> FpGrowth::ItemToRank(std::map<std::string, HeadNode*>& head_table) {
    std::map<std::string, uint32_t> result;
    std::list<HeadNode*> temp_list;
    for (auto item = head_table.begin(); item != head_table.end(); item++) {
        temp_list.emplace_back(item->second);
    }
    auto sort_func = [](HeadNode* n1, HeadNode* n2) -> bool { return ((n1->node_count) > (n2->node_count)); };
    temp_list.sort(sort_func);
    uint32_t index = 0;
    for (auto item = temp_list.begin(); item != temp_list.end(); item++) {
        result.insert(std::pair<std::string, uint32_t>((*item)->node_name, index));
        index++;
    }
    return result;
}

std::map<uint32_t, std::string> FpGrowth::RankToItem(std::map<std::string, uint32_t>& item_to_rank) {
    std::map<uint32_t, std::string> result;
    for (auto item = item_to_rank.begin(); item != item_to_rank.end(); item++) {
        result.insert(std::pair<uint32_t, std::string>(item->second, item->first));
    }
    return result;
}

// sort the trans by HeadTable rank
std::list<std::string> FpGrowth::SortSingleData(std::map<std::string, uint32_t>& item_2_rank,
                                                std::map<uint32_t, std::string>& rank_2_item,
                                                std::list<std::string>& trans) {
    std::list<std::string> sorted_trans;
    std::vector<uint32_t> temp_vector;
    temp_vector.reserve(trans.size());
    for (auto item = trans.begin(); item != trans.end(); item++) {
        if(item_2_rank.find(*item)!=item_2_rank.end()){
            temp_vector.emplace_back(item_2_rank.at(*item));
        }
    }

    auto sort_func = [](uint32_t a, uint32_t b) -> bool { return a < b; };
    std::sort(temp_vector.begin(), temp_vector.end(), sort_func);
    for (size_t i = 0; i < temp_vector.size(); i++) {
        sorted_trans.emplace_back(rank_2_item.at(temp_vector[i]));
    }
    return sorted_trans;
}

void FpGrowth::UpdateHeadTableLink(HeadNode* head_node,TreeNode* tree_node){
    if((head_node->next)==nullptr){
        head_node->next=tree_node;
    }else{
        TreeNode* temp_node=(head_node->next);
        while((temp_node->next)!=nullptr){
            temp_node=temp_node->next;
        }
        temp_node->next=tree_node;
    }
}

void FpGrowth::UpdateFpTree(std::map<std::string, HeadNode*>& head_table, std::list<std::string>& sorted_trans,
                            TreeNode* fp_tree, uint32_t count) {
    for (auto tran = sorted_trans.begin(); tran != sorted_trans.end(); tran++) {
        std::map<std::string, TreeNode*>& children = (fp_tree->children);
        // show_children_keys(children);
        if (children.find(*tran) != children.end()) {
            (children.at(*tran)->node_count) += count;
        } else {
            // create a new TreeNode
            TreeNode* new_node = InitTreeNode(*tran, fp_tree, count);
            // update head_table
            UpdateHeadTableLink(head_table.at(*tran), new_node);
            children.insert(std::pair<std::string,TreeNode*>(*tran,new_node));
        }
        // update fp_tree ptr 
        fp_tree = children.at(*tran);
    }
}

// modify head_table,return fp_tree
FpGrowth::TreeNode* FpGrowth::CreateFpTree(std::list<std::list<std::string>>& prefix_paths,
                                           std::list<uint32_t>& leaf_counts,
                                           std::map<std::string, HeadNode*>& head_table) {
    // construct sub_head_table
    auto count_ref = leaf_counts.begin();
    for (auto prefix_path = prefix_paths.begin(); prefix_path != prefix_paths.end(); prefix_path++) {
        UpdateHeadTable(head_table, *prefix_path, *count_ref);
        count_ref++;
    }

    head_table = FilterHeadTable(head_table);
    TreeNode* fp_tree = InitTreeNode(ROOT_NAME, nullptr, 1);
    if (head_table.size() == 0) {
        return fp_tree;
    }

    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    count_ref = leaf_counts.begin();
    for (auto prefix_path = prefix_paths.begin(); prefix_path != prefix_paths.end(); prefix_path++) {
        auto sorted_trans = SortSingleData(item_2_rank, rank_2_item, *prefix_path);
        UpdateFpTree(head_table, sorted_trans, fp_tree, *count_ref);
    }
    return fp_tree;
}

void FpGrowth::FindPrefixPath(HeadNode* head_node, std::list<std::list<std::string>>& prefix_paths,
                              std::list<uint32_t>& leaf_counts) {
    TreeNode* parent_node;
    TreeNode* leaf_node = head_node->next;
    uint32_t count;
    while (leaf_node != nullptr) {
        // exlude self
        parent_node = leaf_node->parent;
        if ((parent_node->node_name)==ROOT_NAME){
            leaf_node=leaf_node->next;
            continue;
        }
        count=leaf_node->node_count;
        std::list<std::string> prefix_path;
        while ((parent_node->node_name)!=ROOT_NAME) {
            prefix_path.emplace_back(parent_node->node_name);
            parent_node = parent_node->parent;
        }
        prefix_paths.emplace_back(prefix_path);
        leaf_counts.emplace_back(count);
        leaf_node = leaf_node->next;
    }
    // std::cout << "find_prefix_path core" << std::endl;
}

void FpGrowth::FreeFpTree(TreeNode* fp_tree) {
    std::map<std::string, TreeNode*>& children = fp_tree->children;
    // delete children first
    for (auto child = children.begin(); child != children.end(); child++) {
        FreeFpTree(child->second);
    }
    // then delete self
    delete fp_tree;
}

void FpGrowth::FreeHeadTable(std::map<std::string, HeadNode*> head_table) {
    for (auto head_entry = head_table.begin(); head_entry != head_table.end(); head_entry++) {
        delete head_entry->second;
    }
}

// condition if (fp_tree->node_name)==ROOT_NAME,break

void FpGrowth::RecurrentCreateFpTree(std::map<std::string, HeadNode*> head_table, TreeNode* fp_tree,
                                     std::list<std::set<std::string>>& freq_itemset, std::set<std::string> prefix) {
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    uint32_t head_size = rank_2_item.size();
    for (uint32_t i = head_size - 1; i >= 0; --i) {
        HeadNode* head_node = head_table.at(rank_2_item.at(i));
        std::cout << head_node->node_name << std::endl;
        show_set(prefix);
        prefix.insert(head_node->node_name);
        show_set(prefix);
        std::cout << "-----------------" << std::endl;
        freq_itemset.emplace_back(prefix);
        std::list<std::list<std::string>> prefix_paths;
        std::list<uint32_t> leaf_counts;
        FindPrefixPath(head_node, prefix_paths, leaf_counts);
        std::map<std::string, HeadNode*> sub_head_table;
        auto sub_fp_tree = CreateFpTree(prefix_paths, leaf_counts, sub_head_table);
        if (sub_head_table.size()>0) {
            RecurrentCreateFpTree(sub_head_table, sub_fp_tree, freq_itemset, prefix);
        }else{
            freq_itemset.emplace_back(std::set<std::string>({"---------------"}));
            prefix.clear();
            return;
        }
        // finished
        // LOG(INFO) << "free fp tree...";
        // FreeFpTree(fp_tree);
        // LOG(INFO) << "free head_table...";
        // FreeHeadTable(head_table);
    }
}

std::list<std::set<std::string>> FpGrowth::Run(std::list<std::list<std::string>>& trans_data) {
    TreeNode* fp_tree = InitTreeNode(ROOT_NAME, nullptr, 1);
    std::map<std::string, HeadNode*> head_table;
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        UpdateHeadTable(head_table, *trans, 1);
    }
    head_table = FilterHeadTable(head_table);
    for(auto item=head_table.begin();item!=head_table.end();item++){
        std::cout << item->first << "->" << item->second->node_count << std::endl;
    }
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        auto sorted_trans = SortSingleData(item_2_rank, rank_2_item, *trans);
        UpdateFpTree(head_table, sorted_trans, fp_tree, 1);
    }
    std::cout << "..............." << std::endl;
    std::set<std::string> prefix;
    std::list<std::set<std::string>> freq_itemset;
    RecurrentCreateFpTree(head_table, fp_tree, freq_itemset, prefix);
    return freq_itemset;
}

FpGrowth::~FpGrowth(){
    // LOG(INFO) << "stop logging...";
    // google::ShutdownGoogleLogging();  
}


int main(){
    std::list<std::list<std::string>> trans_data={
        {"r", "z", "h", "j", "p"},
        {"z", "y", "x", "w", "v", "u", "t", "s"},
        {"z"},
        {"r", "x", "n", "o", "s"},
        {"y", "r", "x", "z", "q", "t", "p"},
        {"y", "z", "x", "e", "q", "s", "t", "m"}
    };
    FpGrowth* fp=new FpGrowth(3,"root");
    auto freq_itemset=fp->Run(trans_data);
    for(auto itemset=freq_itemset.begin();itemset!=freq_itemset.end();itemset++){
        std::cout << "{ ";
        for(auto item=(*itemset).begin();item!=(*itemset).end();item++){
            std::cout << *item << " ";
        }
        std::cout << "}" << std::endl;
    }
}