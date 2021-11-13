#include <vector>
#include <list>
#include <string>
#include <map>

class FpGrowth{
    public:
        typedef struct TreeNode{
            std::string node_name;
            uint32_t node_count;
            struct TreeNode *parent;
            struct TreeNode *next;
            std::map<std::string,struct TreeNode*> children;
        }TreeNode;

        typedef struct HeadNode{
            std::string node_name;
            uint32_t node_count;
            TreeNode* next;
        }HeadNode;

        FpGrowth(uint32_t min_support_value):min_support(min_support_value){
            
        }
        
        ~FpGrowth();
        
        //init a tree node with name and parent node
        TreeNode* InitTreeNode(std::string,TreeNode*,uint32_t);
        HeadNode* InitHeadNode(std::string,uint32_t);
        // void InitFpTree(std::string,uint32_t);
        void UpdateHeadTable(std::map<std::string,HeadNode*>&,std::list<std::string>&,uint32_t);
        std::map<std::string,HeadNode*> FilterHeadTable(std::map<std::string,HeadNode*>);
        std::map<std::string,uint32_t> ItemToRank(std::map<std::string,HeadNode*>&);
        std::map<uint32_t,std::string> RankToItem(std::map<std::string,uint32_t>&);
        
        std::list<std::string> SortSingleData(std::map<std::string,uint32_t>&,std::map<uint32_t,std::string>&,std::list<std::string>&);
        
        void UpdateFpTree(std::map<std::string,HeadNode*>,std::list<std::string>,TreeNode*);

        void UpdateHeadTableLink(HeadNode*,TreeNode*);
        
    private:
        uint32_t min_support;
        
};