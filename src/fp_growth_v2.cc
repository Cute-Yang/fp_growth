#include "fp_growth_v2.h"
#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

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
    for (auto trans_item = trans.begin(); trans_item != trans.end(); trans_item++) {
        if (head_table.find(*trans_item) == head_table.end()) {
            HeadNode* head_node = InitHeadNode(*trans_item, count);
            head_table.insert(std::pair<std::string, HeadNode*>(*trans_item, head_node));
        } else {
            (head_table.at(*trans_item)->node_count) += count;
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
    return trans;
}

void FpGrowth::UpdateFpTree(std::map<std::string, HeadNode*>& head_table, std::list<std::string>& sorted_trans,
                            TreeNode* fp_tree, uint32_t count) {
    for (auto tran = sorted_trans.begin(); tran != sorted_trans.end(); tran++) {
        std::map<std::string, TreeNode*>& children = fp_tree->children;
        // if the item set found in fp_tree,add the count
        if (children.find(*tran) != children.end()) {
            (children.at(*tran)) += count;
        } else {
            // create a new TreeNode
            TreeNode* new_ndoe = InitTreeNode(*tran, fp_tree, count);
            fp_tree->children.insert(std::pair<std::string, TreeNode*>(*tran, new_ndoe));
            // update head_table
            UpdateHeadTableLink(head_table.at(*tran), new_ndoe);
        }
        // update fp_tree ptr
        fp_tree = (fp_tree->children).at(*tran);
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
    uint32_t count = leaf_node->node_count;
    while (leaf_node != nullptr) {
        // exlude self
        parent_node = leaf_node->parent->next;
        std::list<std::string> prefix_path;
        while (parent_node != nullptr) {
            prefix_path.emplace_back(parent_node->node_name);
            parent_node = parent_node->next;
        }
        prefix_paths.emplace_back(prefix_path);
        leaf_counts.emplace_back(count);
        leaf_node = leaf_node->next;
    }
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

void FpGrowth::RecurrentCreateFpTree(std::map<std::string, HeadNode*>& head_table, TreeNode* fp_tree,
                                     std::list<std::set<std::string>>& freq_itemset, std::set<std::string> prefix) {
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    size_t head_size = rank_2_item.size();
    for (size_t i = head_size - 1; i >= 0; i--) {
        HeadNode* head_node = head_table.at(rank_2_item.at(i));
        prefix.insert(head_node->node_name);
        freq_itemset.emplace_back(prefix);
        std::list<std::list<std::string>> prefix_paths;
        std::list<uint32_t> leaf_counts;
        FindPrefixPath(head_node, prefix_paths, leaf_counts);
        std::map<std::string, HeadNode*> sub_head_table;
        auto sub_fp_tree = CreateFpTree(prefix_paths, leaf_counts, sub_head_table);
        if (sub_head_table.size() > 0) {
            LOG(INFO) << "construct sub fp_tree...";
            RecurrentCreateFpTree(sub_head_table, sub_fp_tree, freq_itemset, prefix);
        }

        // finished
        LOG(INFO) << "free fp tree...";
        FreeFpTree(fp_tree);
        LOG(INFO) << "free head_table...";
        FreeHeadTable(head_table);
    }
}

std::list<std::set<std::string>> FpGrowth::Run(std::list<std::list<std::string>>& trans_data) {
    TreeNode* fp_tree = InitTreeNode(ROOT_NAME, nullptr, 1);
    std::map<std::string, HeadNode*> head_table;
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        UpdateHeadTable(head_table, *trans, 1);
    }
    head_table = FilterHeadTable(head_table);
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        auto sorted_trans = SortSingleData(item_2_rank, rank_2_item, *trans);
        UpdateFpTree(head_table, sorted_trans, fp_tree, 1);
    }

    std::set<std::string> prefix;
    std::list<std::set<std::string>> freq_itemset;
    RecurrentCreateFpTree(head_table, fp_tree, freq_itemset, prefix);
    return freq_itemset;
}

FpGrowth::~FpGrowth(){
    LOG(INFO) << "stop logging...";
    google::ShutdownGoogleLogging();  
}


int main(){
    std::list<std::list<std::string>> trans_data={
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
    FpGrowth* fp=new FpGrowth(3,"root");
    auto freq_itemset=fp->Run(trans_data);
}