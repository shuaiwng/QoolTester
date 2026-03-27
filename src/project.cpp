#include "tinyxml2/tinyxml2.h"
#include "project.h"
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>


using namespace tinyxml2;


Project::Project()
{

}
Project::~Project()
{

}


bool Project::openProject(const char * proj_path)
{
    XMLDocument xml_doc;
    if (xml_doc.LoadFile(proj_path) != tinyxml2::XML_SUCCESS){
        return false;
    }
    XMLElement* dataElm = xml_doc.FirstChildElement("Root");

    int max_level = 0;
    for (XMLElement* nodeElm = dataElm->FirstChildElement("Node");
         nodeElm != NULL; nodeElm = nodeElm->NextSiblingElement())
    {
        Node_data_t pNodeData;
        XMLElement* infoElm = nodeElm->FirstChildElement("Info");
        pNodeData.name = infoElm->Attribute("Name");
        pNodeData.level = std::stoi(std::string(infoElm->Attribute("Level")));
        if (std::string(infoElm->Attribute("isTestCase")) == "1"){
            pNodeData.isTestCase = true;
        } else {
            pNodeData.isTestCase = false;
        }
        pNodeData.testType = infoElm->Attribute("TestType");
        pNodeData.testResult = infoElm->Attribute("TestResult");
        pNodeData.comment = infoElm->Attribute("Comment");
        pNodeData.uid = std::stoi(std::string(infoElm->Attribute("UID")));
        pNodeData.full_name = std::to_string(pNodeData.uid) + ": " + pNodeData.name;

        XMLElement* testcase = nodeElm->FirstChildElement("TestCase");
        if(testcase){
            for (XMLElement* step = testcase->FirstChildElement("StepIn");
                 step != NULL; step = step->NextSiblingElement("StepIn"))
            {
                std::string stepText = (step->GetText()!=nullptr) ? step->GetText() : "";   // Bug from tinyxml2, not returning empty string.
                std::string stepSiblText = (step->NextSiblingElement()->GetText()!=nullptr) ? step->NextSiblingElement()->GetText() : "";
                pNodeData.testdata.push_back(std::make_pair(stepText, stepSiblText));
            }
        }
        m_project_data.node_data.push_back(pNodeData);

        // find max level
        if (max_level < pNodeData.level){
            max_level = pNodeData.level;
        }
    }
    m_project_data.max_level = max_level;
    xml_doc.Clear();

    return true;
}


void Project::saveProject(const char *proj_path)
{
    XMLDocument xmlDoc;
    XMLNode * pRoot = xmlDoc.NewElement("Root");
    xmlDoc.InsertFirstChild(pRoot);

    int node_vec_sz = m_project_data.node_data.size();
    for (int i = 0; i < node_vec_sz; i++)
    {
        XMLElement * pNodeElement = xmlDoc.NewElement("Node");
        XMLElement * pInfoElement = xmlDoc.NewElement("Info");
        pInfoElement->SetAttribute("UID", std::to_string(m_project_data.node_data[i].uid).c_str());
        pInfoElement->SetAttribute("Name", m_project_data.node_data[i].name.c_str());
        pInfoElement->SetAttribute("Level", std::to_string(m_project_data.node_data[i].level).c_str());
        pInfoElement->SetAttribute("isTestCase", std::to_string(int(m_project_data.node_data[i].isTestCase)).c_str());
        pInfoElement->SetAttribute("TestType", m_project_data.node_data[i].testType.c_str());
        pInfoElement->SetAttribute("TestResult", m_project_data.node_data[i].testResult.c_str());
        pInfoElement->SetAttribute("Comment", m_project_data.node_data[i].comment.c_str());

        pNodeElement->InsertEndChild(pInfoElement);

        if(!m_project_data.node_data[i].isTestCase){
            pRoot->InsertEndChild(pNodeElement);
            continue;
        }

        XMLElement * pTestElm = xmlDoc.NewElement("TestCase");
        int steps_sz = m_project_data.node_data[i].testdata.size();
        for (int i_test = 0; i_test < steps_sz; i_test++)
        {
            XMLElement * pStepInElm = xmlDoc.NewElement("StepIn");
            XMLElement * pStepOutElm = xmlDoc.NewElement("StepOut");
            pStepInElm->SetText(std::get<0>(m_project_data.node_data[i].testdata[i_test]).c_str());
            pStepOutElm->SetText(std::get<1>(m_project_data.node_data[i].testdata[i_test]).c_str());
            pTestElm->InsertEndChild(pStepInElm);
            pTestElm->InsertEndChild(pStepOutElm);
        }
        pNodeElement->InsertEndChild(pTestElm);

        pRoot->InsertEndChild(pNodeElement);
    }
    xmlDoc.SaveFile(proj_path);
    xmlDoc.Clear();
}

bool Project::saveTestCase(int uid_select, Node_data_t mod_data)
{
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select;
    if(!getVecIndex(uid_select, idx_select)){
        return false;
    }
    m_project_data.node_data[idx_select] = mod_data;

    return true;
}

std::vector<Node_data_t> Project::getSubNodeList(int idx_node)
{
    // determine note list, which includes child elements, if any
    std::vector<Node_data_t> node_mv_list;
    for (std::vector<Node_data_t>::iterator it=m_project_data.node_data.begin() + idx_node;
         it != m_project_data.node_data.end(); ++it)
    {
        if (node_mv_list.size() >= 1){
            if (it->level > node_mv_list[0].level){
                node_mv_list.push_back(*it);
            } else {
                break;
            }
        } else {
            node_mv_list.push_back(*it);
        }
    }
    return node_mv_list;
}


bool Project::followNode(int uid_select, int uid_target, bool b_asChild)
{
    if (!doesUIDExist(uid_target)){
        return false;
    }

    int idx_select, idx_target, idx_select_orig, idx_target_orig;
    if(!getVecIndex(uid_select, idx_select_orig) || !getVecIndex(uid_target, idx_target_orig)){
        return false;
    }

    // uids shall not follow each other, if they are on the same branch
    std::vector<Node_data_t> node_mv_list = getSubNodeList(idx_select_orig);
    std::vector<int> uid_mv_list;
    for (auto & it : node_mv_list){
        uid_mv_list.push_back(it.uid);
    }
    if (std::count(uid_mv_list.begin(), uid_mv_list.end(), uid_target)){    // parent -> child
        return false;
    }
    std::vector<Node_data_t> node_tag_list = getSubNodeList(idx_target_orig);
    std::vector<int> uid_tag_list;
    for (auto & it : node_tag_list){
        uid_tag_list.push_back(it.uid);
    }
    if (std::count(uid_tag_list.begin(), uid_tag_list.end(), uid_select)){  // child -> parent
        return false;
    }

    // handle the target children, if any
    int child_count = 0;
    for (std::vector<Node_data_t>::iterator it=m_project_data.node_data.begin()+idx_target_orig+1;
         it != m_project_data.node_data.end(); ++it)
    {
        if(it->level > m_project_data.node_data[idx_target_orig].level){
            child_count += 1;
        } else {
            break;
        }
    }

    // adjust the level info to the target node
    int diff_level = m_project_data.node_data[idx_target_orig].level -
                     m_project_data.node_data[idx_select_orig].level;

    if (b_asChild){
        diff_level += 1;
    }
    for (int i=0; i<node_mv_list.size(); i++){
        m_project_data.node_data[idx_select_orig+i].level += diff_level;
    }
    updateMaxLevel();

    // move note list
    bool b_leftRot = (idx_target_orig >= idx_select_orig) ? true : false;
    if(b_leftRot)
    {
        idx_select = idx_select_orig;
        idx_target = idx_target_orig;
        if (idx_target == m_project_data.node_data.size()-1){   // the last element as target
            std::rotate(m_project_data.node_data.begin()+idx_select,
                        m_project_data.node_data.begin()+idx_select+node_mv_list.size(),
                        m_project_data.node_data.end());
        } else {
            std::rotate(m_project_data.node_data.begin()+idx_select,
                        m_project_data.node_data.begin()+idx_select+node_mv_list.size(),
                        m_project_data.node_data.begin()+idx_target+child_count+1); // itr_last + 1 != vec.end()
        }
    }
    else
    {
        idx_select = m_project_data.node_data.size() - idx_select_orig - node_mv_list.size();
        idx_target = m_project_data.node_data.size() - idx_target_orig - 1;
        std::rotate(m_project_data.node_data.rbegin()+idx_select,
                    m_project_data.node_data.rbegin()+idx_select+node_mv_list.size(),
                    m_project_data.node_data.rbegin()+idx_target-child_count);
    }
    return true;
}

bool Project::addNode(int uid_target, NodeType eNodeType, bool b_asChild)
{
    if (!doesUIDExist(uid_target)){
        return false;
    }
    int new_uid;
    if (!findNextAvailableUID(new_uid)){
        return false;
    }
    int idx_target_orig;
    if(!getVecIndex(uid_target, idx_target_orig)){
        return false;
    }
    std::vector<Node_data_t> node_tag_list = getSubNodeList(idx_target_orig);

    bool is_testcase = (eNodeType == NodeType::eNodeTestCase) ? true : false;
    Node_data_t node_add {new_uid, "", std::to_string(new_uid)+": ", node_tag_list[0].level, is_testcase, "Acceptance Test", "Neutral", "", {{"", ""}}};
    if (b_asChild){
        node_add.level += 1;
    }
    m_project_data.node_data.insert(m_project_data.node_data.begin()+idx_target_orig+node_tag_list.size(), node_add);
    updateMaxLevel();

    return true;
}

bool Project::deleteNode(int uid_selected)
{
    if (!doesUIDExist(uid_selected)){
        return false;
    }
    int idx_selected_orig;
    if(!getVecIndex(uid_selected, idx_selected_orig)){
        return false;
    }
    std::vector<Node_data_t> node_del_list = getSubNodeList(idx_selected_orig);


    m_project_data.node_data.erase(m_project_data.node_data.begin()+idx_selected_orig,
                                   m_project_data.node_data.begin()+idx_selected_orig+node_del_list.size());
    return true;
}

bool Project::addCaseStep(int uid_select)
{
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select;
    if(!getVecIndex(uid_select, idx_select)){
        return false;
    }
    m_project_data.node_data[idx_select].testdata.push_back(std::make_tuple("",""));

    return true;
}

bool Project::deleteCaseStep(int uid_select, int idx_step)
{
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select;
    if(!getVecIndex(uid_select, idx_select)){
        return false;
    }
    if (m_project_data.node_data[idx_select].testdata.size() < 2){   // leave at least one test step
        return false;
    }
    m_project_data.node_data[idx_select].testdata.erase(
        m_project_data.node_data[idx_select].testdata.begin()+idx_step);

    return true;
}

bool Project::moveUpCaseStep(int uid_select, int idx_step)
{
    if (idx_step < 1){
        return false;
    }
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select;
    if(!getVecIndex(uid_select, idx_select)){
        return false;
    }
    std::swap(m_project_data.node_data[idx_select].testdata[idx_step-1], m_project_data.node_data[idx_select].testdata[idx_step]);
    return true;
}

bool Project::moveDownCaseStep(int uid_select, int current_row_count, int idx_step)
{
    if (idx_step == current_row_count-1){
        return false;
    }
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select;
    if(!getVecIndex(uid_select, idx_select)){
        return false;
    }
    std::swap(m_project_data.node_data[idx_select].testdata[idx_step], m_project_data.node_data[idx_select].testdata[idx_step+1]);
    return true;
}

bool Project::getVecIndex(int uid, int & idx_got)
{
    int idx = 0;
    for (auto & it : m_project_data.node_data){
        if(it.uid == uid){
            idx_got = idx;
            return true;
        } else {
            idx++;
        }
    }
    return false;
}


bool Project::doesUIDExist(int uid){
    std::vector<int> vec_uid;
    for (auto & it : m_project_data.node_data)
    {
        vec_uid.push_back(it.uid);
    }
    if (std::count(vec_uid.begin(), vec_uid.end(), uid)){
        return true;
    }
    return false;
}


void Project::updateMaxLevel()
{
    std::vector<int> vec_maxLevel;
    for (auto & it : m_project_data.node_data)
    {
        vec_maxLevel.push_back(it.level);
    }
    m_project_data.max_level = *std::max_element(vec_maxLevel.begin(), vec_maxLevel.end());
}

bool Project::findNextAvailableUID(int & uid_find)
{
    std::vector<int> vec_uid;
    for (auto & it : m_project_data.node_data)
    {
        vec_uid.push_back(it.uid);
    }
    for (int uid_search = 0; uid_search < std::numeric_limits<int>::max(); uid_search++){
        if(!std::count(vec_uid.begin(), vec_uid.end(), uid_search)){
            uid_find = uid_search;
            return true;
        } else {
            continue;
        }
    }
    return false;
}

bool Project::compareSubMainEqual(int uid_select, bool & b_equal)
{
    if (!doesUIDExist(uid_select)){
        return false;
    }
    int idx_select_orig;
    if(!getVecIndex(uid_select, idx_select_orig)){
        return false;
    }
    std::vector<Node_data_t> node_sub_list = getSubNodeList(idx_select_orig);
    if (m_project_data.node_data.size() != node_sub_list.size()){
        b_equal = false;
        return true;
    } else {
        b_equal = true;
        return true;
    }
    return false;
}


Project_data_t * Project::data()
{
    return & m_project_data;
}
