#include "utils.h"
#include <algorithm>
#include <sstream>


std::string getFileWithExtention(std::string f, std::string ext)
{
    std::string test = f.substr(0, f.find('.'));
    if (f.back() == '.')
    {
        f = f + ext;
    }
    else if (f.find(".")!=std::string::npos && getExt(f, '.') == ext)
    {
        return f;
    }
    else
    {
        f = f + "." + ext;
    }
    return f;
}


std::string getExt(std::string s, char del)
{
    std::string str_name;
    std::replace(s.begin(), s.end(), del, ' ');
    std::stringstream s_tmp(s);

    while (s_tmp.rdbuf()->in_avail() != 0)
    {
        s_tmp>>str_name;
    }

    return str_name;
}
