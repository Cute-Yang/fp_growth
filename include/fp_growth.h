#ifndef _FP_GROWTH_H
#define _FP_GROWTH_H
#include <vector>
#include <string>
#include <map>
#include <list>
#include <cstdint>


// build fp tree
typedef struct TreeNode {
  std::string node_name;
  size_t node_count;
  struct TreeNode *parent;
  struct TreeNode *next;
  // cast the address of struct -> int,restore it in a map
  std::map<std::string, struct TreeNode*> children;
} FpNode;

// define tree node
typedef struct Node {
  std::string node_name;
  size_t node_count;
  FpNode *next;
} HeadNode;

//find the freq 1 item 
std::vector<HeadNode*> ItemCount(std::vector<std::vector<std::string>>&,float);

std::list<std::vector<std::string>> DataSort(std::vector<std::vector<std::string>>&,std::list<HeadNode*>&);

//search the next node whether in the children
bool SearchChild(std::map<std::string,FpNode*>,FpNode*);

void SingleDataSort();
void DeleteFpTree(FpNode*);
void DelteHeadTable(std::vector<HeadNode*>);
FpNode* InitFpNode(std::string&,FpNode*);
//build the FPTree
void UpdateFPTree(std::vector<HeadNode*>&,std::vector<std::string>,FpNode*,std::map<std::string,size_t>&);
void UpdateHeadTable(HeadNode*,FpNode*);
void FindFreqItemsets(std::vector<HeadNode*>,FpNode*);
HeadNode* InitHeadNode(std::string,size_t);
std::list<std::string> FindPrefixPath(FpNode*);
#endif