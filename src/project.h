#pragma once

#include <iostream>
#include <vector>
#include <tuple>


struct Node_data_t{
    int uid;
    std::string name;
    std::string full_name;
    int level;
    bool isTestCase;
    std::string testType;
    std::string testResult;
    std::string comment;
    std::vector<std::tuple<std::string, std::string>> testdata;
};

struct Project_data_t{
    std::vector<Node_data_t> node_data;
    int max_level;
};

enum class NodeType {eNodeSubFolder, eNodeTestCase};

class Project
{
public:
    Project();
    ~Project();

    bool openProject(const char *);
    void saveProject(const char *);
    bool saveTestCase(int uid_select, Node_data_t mod_data);

    bool followNode(int uid_select, int uid_target, bool b_asChild);
    bool addNode(int uid_target, NodeType eNodeType, bool b_asChild);
    bool deleteNode(int uid_target);
    bool findNextAvailableUID(int & uid_find);
    bool compareSubMainEqual(int uid_select, bool & b_equal);

    bool addCaseStep(int uid_select);
    bool deleteCaseStep(int uid_select, int idx_step);
    bool moveUpCaseStep(int uid_select, int idx_step);
    bool moveDownCaseStep(int uid_select, int current_row_count, int idx_step);

    std::vector<Node_data_t> getSubNodeList(int idx_node);
    bool getVecIndex(int uid, int & idx_got);
    void updateMaxLevel();
    bool doesUIDExist(int uid);

    Project_data_t * data();


private:
    Project_data_t m_project_data;

};
