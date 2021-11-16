#include "fp_growth.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

DEFINE_string(file_path, "data/sample.txt", "the source file path");
DEFINE_uint32(min_support, 3, "the min_support degree");
DEFINE_string(root_name, "root", "the name of root node");
DEFINE_string(save_path, "data/result.txt", "the path to restore resutl");
DEFINE_uint32(keep_size, 1, "the minum size of freq set");
DEFINE_bool(display, true, "whether to show result");
// DEFINE_double();

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

void FpGrowth::UpdateHeadTable(std::map<std::string, HeadNode*>& head_table, const std::list<std::string>& trans,
                               uint32_t count) {
    for (auto tran = trans.begin(); tran != trans.end(); tran++) {
        if (head_table.find(*tran) != head_table.end()) {
            (head_table.at(*tran)->node_count) += count;
        } else {
            HeadNode* head_node = InitHeadNode(*tran, count);
            head_table.insert(std::pair<std::string, HeadNode*>(*tran, head_node));
        }
    }
}

std::map<std::string, FpGrowth::HeadNode*> FpGrowth::FilterHeadTable(std::map<std::string, HeadNode*> old_head_table) {
    LOG(INFO) << "filter the freq item of head table at " << &old_head_table;
    std::map<std::string, HeadNode*> head_talbe;
    for (auto item = old_head_table.begin(); item != old_head_table.end(); item++) {
        if ((item->second->node_count) < min_support) {
            continue;
        }
        head_talbe.insert(*item);
    }
    if (old_head_table.size() > head_talbe.size()) {
        LOG(INFO) << "filtered " << old_head_table.size() - head_talbe.size() << " items at " << &old_head_table;
    }
    //wwbbwws
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
                                                const std::list<std::string>& trans) {
    std::list<std::string> sorted_trans;
    std::vector<uint32_t> temp_vector;
    temp_vector.reserve(trans.size());
    for (auto item = trans.begin(); item != trans.end(); item++) {
        if (item_2_rank.find(*item) != item_2_rank.end()) {
            temp_vector.emplace_back(item_2_rank.at(*item));
        }
    }

    auto sort_func = [](uint32_t a, uint32_t b) -> bool { return a < b; };
    std::sort(temp_vector.begin(), temp_vector.end(), sort_func);
    for (uint32_t i = 0; i < temp_vector.size(); i++) {
        sorted_trans.emplace_back(rank_2_item.at(temp_vector[i]));
    }
    return sorted_trans;
}

void FpGrowth::UpdateHeadTableLink(HeadNode* head_node, TreeNode* tree_node) {
    if ((head_node->next) == nullptr) {
        head_node->next = tree_node;
    } else {
        TreeNode* temp_node = (head_node->next);
        while ((temp_node->next) != nullptr) {
            temp_node = temp_node->next;
        }
        temp_node->next = tree_node;
    }
}

void FpGrowth::UpdateFpTree(std::map<std::string, HeadNode*>& head_table, const std::list<std::string>& sorted_trans,
                            TreeNode* fp_tree, uint32_t count) {
    for (auto tran = sorted_trans.begin(); tran != sorted_trans.end(); tran++) {
        std::map<std::string, TreeNode*>& children = (fp_tree->children);
        if (children.find(*tran) != children.end()) {
            (children.at(*tran)->node_count) += count;
        } else {
            // create a new TreeNode
            LOG(INFO) << "create a new tree node for tran->" << *tran << "->" << count;
            TreeNode* new_node = InitTreeNode(*tran, fp_tree, count);
            // update head_table
            UpdateHeadTableLink(head_table.at(*tran), new_node);
            children.insert(std::pair<std::string, TreeNode*>(*tran, new_node));
        }
        // update fp_tree ptr
        fp_tree = children.at(*tran);
    }
}

// modify head_table,return fp_tree
void FpGrowth::CreateFpTree(std::list<std::list<std::string>>& prefix_paths, std::list<uint32_t>& leaf_counts,
                            std::map<std::string, HeadNode*>& head_table, TreeNode* fp_tree) {
    // construct sub_head_table
    LOG(INFO) << "update FpTree at" << fp_tree;
    auto count_ref = leaf_counts.begin();
    for (auto prefix_path = prefix_paths.begin(); prefix_path != prefix_paths.end(); prefix_path++) {
        UpdateHeadTable(head_table, *prefix_path, *count_ref);
        count_ref++;
    }

    head_table = FilterHeadTable(head_table);
    if (head_table.size() == 0) {
        return;
    }

    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    count_ref = leaf_counts.begin();
    for (auto prefix_path = prefix_paths.begin(); prefix_path != prefix_paths.end(); prefix_path++) {
        auto sorted_trans = SortSingleData(item_2_rank, rank_2_item, *prefix_path);
        UpdateFpTree(head_table, sorted_trans, fp_tree, *count_ref);
        count_ref++;
    }
}

void FpGrowth::FindPrefixPath(HeadNode* head_node, std::list<std::list<std::string>>& prefix_paths,
                              std::list<uint32_t>& leaf_counts) {
    TreeNode* parent_node;
    TreeNode* leaf_node = head_node->next;
    uint32_t count=0;
    while (leaf_node != nullptr) {
        // exlude self
        parent_node = leaf_node->parent;
        if ((parent_node->node_name) == ROOT_NAME) {
            leaf_node = leaf_node->next;
            continue;
        }
        count = leaf_node->node_count;
        std::list<std::string> prefix_path;
        while ((parent_node->node_name) != ROOT_NAME) {
            prefix_path.emplace_back(parent_node->node_name);
            parent_node = parent_node->parent;
        }
        if (prefix_path.size()==0){
            leaf_node=leaf_node->next;
            continue;
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
    LOG(INFO) << "free memory of FpTree at " << fp_tree;
}

void FpGrowth::FreeHeadTable(std::map<std::string, HeadNode*> head_table) {
    if (head_table.size() > 0) {
        LOG(INFO) << "free memory of HeadTable at " << &head_table;
        for (auto head_entry = head_table.begin(); head_entry != head_table.end(); head_entry++) {
            delete head_entry->second;
        }
    }
}

// condition if (fp_tree->node_name)==ROOT_NAME,break

void FpGrowth::RecurrentCreateFpTree(std::map<std::string, HeadNode*>& head_table, TreeNode* fp_tree,
                                     std::list<std::set<std::string>>& freq_itemset, std::set<std::string>& prefix) {
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    uint32_t head_size = rank_2_item.size();
    for (uint32_t i = head_size - 1; i >= 0; --i) {
        HeadNode* head_node = head_table.at(rank_2_item.at(i));

        // must use the copy
        std::set<std::string> new_prefix = prefix;
        new_prefix.insert(head_node->node_name);
        freq_itemset.emplace_back(new_prefix);
        std::list<std::list<std::string>> prefix_paths;
        std::list<uint32_t> leaf_counts;
        FindPrefixPath(head_node, prefix_paths, leaf_counts);
        std::map<std::string, HeadNode*> sub_head_table;
        TreeNode* sub_fp_tree = InitTreeNode(ROOT_NAME, nullptr, 1);
        CreateFpTree(prefix_paths, leaf_counts, sub_head_table, sub_fp_tree);
        if (sub_head_table.size() > 0) {
            RecurrentCreateFpTree(sub_head_table, sub_fp_tree, freq_itemset, new_prefix);
        } else {
            // means that we arrive the last item of head_table,now can free FpTree and HeadTable
            FreeFpTree(fp_tree);
            FreeHeadTable(head_table);

            // // but the sub fp tree is has a root node alloc on heap
            FreeFpTree(sub_fp_tree);
            return;
        }
    }
}

std::list<std::list<std::string>> FpGrowth::FilterFreqSet(std::list<std::set<std::string>>& freq_itemset,
                                                          uint32_t keep_size) {
    if (keep_size <= 0) {
        throw std::runtime_error("param keep_size should >0");
    }
    std::list<std::list<std::string>> filtered_freq_itemset;
    for (auto freq_item = freq_itemset.begin(); freq_item != freq_itemset.end(); freq_item++) {
        if (freq_item->size() >= keep_size) {
            std::list<std::string> append_item;
            for (auto item = freq_item->begin(); item != freq_item->end(); item++) {
                append_item.emplace_back(*item);
            }
            filtered_freq_itemset.emplace_back(append_item);
        }
    }
    return filtered_freq_itemset;
}

std::list<std::set<std::string>> FpGrowth::Run(const std::list<std::list<std::string>>& trans_data) {
    TreeNode* fp_tree = InitTreeNode(ROOT_NAME, nullptr, 1);
    std::map<std::string, HeadNode*> head_table;
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        UpdateHeadTable(head_table, *trans, 1);
    }
    std::list<std::set<std::string>> freq_itemset;
    head_table = FilterHeadTable(head_table);
    if (head_table.size() == 0) {
        LOG(WARNING) << "we got 0 head node at first stage,will return empty result,check the param min_support";
        return freq_itemset;
    }
    auto item_2_rank = ItemToRank(head_table);
    auto rank_2_item = RankToItem(item_2_rank);
    for (auto trans = trans_data.begin(); trans != trans_data.end(); trans++) {
        auto sorted_trans = SortSingleData(item_2_rank, rank_2_item, *trans);
        UpdateFpTree(head_table, sorted_trans, fp_tree, 1);
    }
    std::set<std::string> prefix;
    RecurrentCreateFpTree(head_table, fp_tree, freq_itemset, prefix);
    return freq_itemset;
}

std::list<std::list<std::string>> FpGrowth::ReadDataFromFile(const std::string& file_path, char split_char) {
    std::ifstream f_ptr(file_path);
    if (!f_ptr.is_open()) {
        throw std::runtime_error("can not open file");
    }
    std::string line;
    std::list<std::list<std::string>> trans_data;
    uint32_t rows = 0;
    while (std::getline(f_ptr, line)) {
        std::stringstream str_stream(line);
        std::string temp_str;
        std::set<std::string> trans;
        while (std::getline(str_stream, temp_str, split_char)) {
            trans.insert(temp_str);
        }
        std::list<std::string> list_trans;
        for(auto tran=trans.begin();tran!=trans.end();tran++){
            list_trans.emplace_back(std::move(*tran));
        }
        trans_data.emplace_back(list_trans);
        ++rows;
    }
    LOG(INFO) << "Read " << rows << " data from file->" << file_path << std::endl;
    // close ptr
    f_ptr.close();
    return trans_data;
}

void FpGrowth::SaveResult(std::list<std::list<std::string>>& freq_itemset, const std::string& dst_path) {
    std::ofstream writer(dst_path);
    for (auto itemset = freq_itemset.begin(); itemset != freq_itemset.end(); itemset++) {
        auto item = itemset->begin();
        for (; item != (--itemset->end()); item++) {
            writer << *item << ",";
        }
        writer << *item << std::endl;
    }
    writer.close();
}

int main(int argc, char** argv) {
    // parse cmd flag
    google::SetUsageMessage("Usage: [Options]");
    google::ParseCommandLineFlags(&argc, &argv, true);
    // test case of offical
    // std::list<std::list<std::string>> trans_data = {
    //     {"r", "z", "h", "j", "p"}, {"z", "y", "x", "w", "v", "u", "t", "s"}, {"z"},
    //     {"r", "x", "n", "o", "s"}, {"y", "r", "x", "z", "q", "t", "p"},      {"y", "z", "x", "e", "q", "s", "t",
    //     "m"}};

    // test case of liujianping
    // std::list<std::list<std::string>> trans_data = {{"E", "J"},
    //                                                 {"A", "B", "C", "E", "F"},
    //                                                 {"A", "C", "G"},
    //                                                 {"E", "I"},
    //                                                 {"A", "C", "D", "E", "G"},
    //                                                 {"A", "C", "E", "G", "L"},
    //                                                 {"A", "B", "C", "E", "F", "P"},
    //                                                 {"A", "C", "D"},
    //                                                 {"A", "C", "E", "G", "M"},
    //                                                 {"A", "C", "E", "G", "N"}};
    // std::string file_path = FLAGS_file_path;
    // uint32_t min_support = FLAGS_min_support;
    // std::string root_name = FLAGS_root_name;

    FpGrowth* fp = new FpGrowth(FLAGS_min_support, FLAGS_root_name);
    // run
    auto trans_data = fp->ReadDataFromFile(FLAGS_file_path, ',');
    auto freq_itemset = fp->Run(trans_data);
    auto filtered_freq_itemset = fp->FilterFreqSet(freq_itemset, FLAGS_keep_size);
    fp->SaveResult(filtered_freq_itemset, FLAGS_save_path);

    // display result
    if (FLAGS_display) {
        for (auto itemset = filtered_freq_itemset.begin(); itemset != filtered_freq_itemset.end(); itemset++) {
            std::cout << "{ ";
            for (auto item = (*itemset).begin(); item != (*itemset).end(); item++) {
                std::cout << *item << " ";
            }
            std::cout << "}" << std::endl;
        }
    }
    delete fp;
}