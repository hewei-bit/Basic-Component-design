#include "IMUser.h"

std::string int2string(uint32_t user_id)
{
    std::stringstream ss;
    ss << user_id;
    return ss.str();
}

bool insertUser(CDBConn *pDBConn, int id)
{
}