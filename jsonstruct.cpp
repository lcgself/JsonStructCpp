#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "jsonstruct.h"
#include "cJSON.h"

#if defined(__GNUC__)
#include <memory>
#include <cxxabi.h>
#include <stdlib.h>
#endif
#define JSON_STRUCT_DEBUG 0
#define JSONSUPPORT_POINT 0

#if JSON_STRUCT_DEBUG
#include <stdio.h>
#endif // JSON_STRUCT_DEBUG
enum EMJsonStructValueType
{
    BOOL_TYPE = 1,
    CHAR_TYPE,
    WCHAR_TYPE,
    SHORT_TYPE,
    INT_TYPE,
    LONG_TYPE,
    LONGLONG_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    LONGDOUBLE_TYPE,
    //stl type
    STRING_TYPE,
    STL_SET,
    STL_PAIR,
    STL_MAP,
    STL_VECTOR,
    STL_LIST,
    STL_DEQUE,
    STL_CONTAINER,      //vector list queue,dequeue,stack ...
    //cuntom type
    JSON_TYPE,
};
struct TJsonStructMemberType
{
    EMJsonStructValueType type;
    unsigned int typeSize;
    bool isArray;
    bool isPointer;
    bool isConst;
    std::size_t arraySize;
    TJsonStructMemberType* childType1;
    TJsonStructMemberType* childType2;
    TJsonStructMemberType():childType1(NULL),
        childType2(NULL),
        typeSize(0),
        isArray(false),
        isPointer(false),
        isConst(false),
        arraySize(0)
    {
    }
    ~TJsonStructMemberType()
    {
        if(NULL != childType1)
        {
            delete childType1;
        }
        if(NULL != childType2)
        {
            delete childType2;
        }
    }
};
struct TJsonStructDeriveInfo
{
    unsigned int typeSize;
};
struct TJsonStructBaseMember
{
    bool support;
    TJsonStructMemberType type;
    std::string typeName;
    std::string name;
    void*   pAddr;
};
struct TBaseJsonStruct_private
{
    static std::map<std::string,TJsonStructDeriveInfo> derivesName;
    std::vector<TJsonStructBaseMember> members;
};
//通过find_if查找数组大小成员变量的 判断函数
struct member_array_size_finder
{
	member_array_size_finder(TJsonStructBaseMember &member)
	{
	    name_num = member.name + "_num";
	    nameNum = member.name + "Num";
	    name_size = member.name + "_size";
	    nameSize = member.name + "Size";
	    name_count = member.name + "_count";
	    nameCount = member.name + "Count";
	}
	member_array_size_finder(const std::string &array_name)
	{
	    name_num = array_name + "_num";
	    nameNum = array_name + "Num";
	    name_size = array_name + "_size";
	    nameSize = array_name + "Size";
	    name_count = array_name + "_count";
	    nameCount = array_name + "Count";
	}
	bool operator()(const TJsonStructBaseMember &p_member)
	{
		return (name_num == p_member.name)
		 || (nameNum == p_member.name)
		 || (name_size == p_member.name)
		 || (nameSize == p_member.name)
		 || (name_count == p_member.name)
		 || (nameCount == p_member.name);
	}
	std::string name_num;
	std::string nameNum;
	std::string name_size;
	std::string nameSize;
	std::string nameCount;
	std::string name_count;
};

std::map<std::string,TJsonStructDeriveInfo> TBaseJsonStruct_private::derivesName = std::map<std::string,TJsonStructDeriveInfo>();
static bool CheckJsonStructMemberType(std::string& name, TJsonStructMemberType& type)
{
#if defined(__GNUC__)
    /**
    gcc:数组：A+大小+_
        指针：P
        const: K
        static不能被识别
        引用不能被识别
        std::string : Ss
        stl容器：St+容器名称
    例: const std::string* pstr[20] = A20_PKSs
    **/
#if JSONSUPPORT_POINT == 0
    std::size_t ppos = name.find('P');
    if(std::string::npos != ppos)
    {
        /**do not support pointer**/
        type.isPointer = true;
        return false;
    }
    ppos = name.find('K');
    if(std::string::npos != ppos)
    {
        /**do not support const**/
        type.isConst = true;
        return false;
    }
#endif
    const char* pchName = name.c_str();
    if('A' == pchName[0])
    {
        type.isArray = true;
        type.arraySize = atoi(pchName + 1);
        do
        {
            pchName+=1;
        }while(pchName[0] != '_');
        pchName += 1;
    }
    //begin acture type check
    switch(pchName[0])
    {
    case 'b':
        type.type = BOOL_TYPE;
        break;
    case 'c':
        type.type = CHAR_TYPE;
        break;
    case 'w':
        type.type = WCHAR_TYPE;
        break;
    case 's':
        type.type = SHORT_TYPE;
        break;
    case 'i':
        type.type = INT_TYPE;
        break;
    case 'l':
        type.type = LONG_TYPE;
        break;
    case 'x':
        type.type = LONGLONG_TYPE;
        break;
    case 'f':
        type.type = FLOAT_TYPE;
        break;
    case 'd':
        type.type = DOUBLE_TYPE;
        break;
    case 'e':
        type.type = LONGDOUBLE_TYPE;
        break;
    case 'S':
        {
            //stl type
            if('s' == pchName[1])
            {
                type.type = STRING_TYPE;
            }else if('t' == pchName[1])
            {
                if('s' == pchName[3] && 'e' == pchName[4])
                {//std set
                    type.type = STL_SET;
                }else if('m' == pchName[3] && 'a' == pchName[4])
                {//std map
                    type.type = STL_MAP;
                }else if('p' == pchName[3] && 'a' == pchName[4])
                {//std map
                    type.type = STL_PAIR;
                }else if(('v' == pchName[3] && 'e' == pchName[4])
                         || ('l' == pchName[3] && 'i' == pchName[4])
                         ||('d' == pchName[3] && 'e' == pchName[4]))
                {//std map
                    type.type = STL_CONTAINER;
                }else
                {
                    //unsupported type
                    return false;
                }
            }
        }
        break;
    default:
        {
            std::string typestring(name);
            ppos = name.find("_");
            if(std::string::npos != ppos)
            {
                typestring = name.substr(ppos + 1, std::string::npos);
            }
            std::map<std::string,TJsonStructDeriveInfo>::iterator it = TBaseJsonStruct_private::derivesName.begin();
            it = TBaseJsonStruct_private::derivesName.find(typestring);
            if(it != TBaseJsonStruct_private::derivesName.end())
            {
                type.type = JSON_TYPE;
                type.typeSize = it->second.typeSize;
            }
            else
            {
                //格式未识别
                return false;
            }
        }
    }
#else
    /**
    for win32 std::string = class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
    struct= struct testd [5]
    **/
    std::size_t ppos = std::string::npos;
#if JSONSUPPORT_POINT == 0
    ppos = name.find('*');
    if(std::string::npos != ppos)
    {
        /**do not support pointer**/
        type.isPointer = true;
        return false;
    }
    ppos = name.find("const");
    if(std::string::npos != ppos)
    {
        /**do not support const**/
        type.isConst = true;
        return false;
    }
#endif
    //begin acture type check
    std::string typestring("");
    ppos = name.find(' ');
    if(std::string::npos != ppos)
    {
        typestring = name.substr(0,ppos);
    }
    if(!name.find("bool"))
    {
        type.type = BOOL_TYPE;
    }else if(!name.find("char"))
    {
        type.type = CHAR_TYPE;
    }
    else if(!name.find("wchar_t"))
    {
        type.type = WCHAR_TYPE;
    }
    else if(!name.find("short"))
    {
        type.type = SHORT_TYPE;
    }
    else if(!name.find("int"))
    {
        type.type = INT_TYPE;
    }
    else if(!name.find("__int64"))
    {//compare long long first
        type.type = LONGLONG_TYPE;
    }
    else if(!name.find("long double"))
    {
        type.type = LONGDOUBLE_TYPE;
    }
    else if(!name.find("long"))
    {
        type.type = LONG_TYPE;
    }
    else if(!name.find("float"))
    {
        type.type = FLOAT_TYPE;
    }
    else if(!name.find("double"))
    {
        type.type = DOUBLE_TYPE;
    }
    else if(!name.find("class") || !name.find("struct"))
    {//stl type begin with class, jsonstruct maybe struct or class
        if(0 == typestring.length())
        {
            return false;
        }
        std::string checkstring;
        ppos = name.find(' ', ppos + 1);
        if (std::string::npos != ppos)
        {
            checkstring = name.substr(0, ppos);
        }else
        {
            checkstring = name;
        }
        if(!checkstring.find("class std::basic_string"))
        {//std string
            type.type = STRING_TYPE;
        }else if( !checkstring.find("class std::set"))
        {//std set
            type.type = STL_SET;
            //TODO 暂不支持容器
            return false;
        }else if(!checkstring.find("class std::map"))
        {//std map
            type.type = STL_MAP;
            //TODO 暂不支持容器
            return false;
        }else if(!checkstring.find("struct std::pair"))
        {//std map
            type.type = STL_PAIR;
            //TODO 暂不支持容器
            return false;
        }else if(!checkstring.find("class std::vector")
                    || !checkstring.find("class std::list")
                    || !checkstring.find("class std::deque"))
        {//std map
            type.type = STL_CONTAINER;
            //TODO 暂不支持容器
            return false;
        }else
        {
            std::map<std::string,TJsonStructDeriveInfo>::iterator it = TBaseJsonStruct_private::derivesName.begin();
            it = TBaseJsonStruct_private::derivesName.find(checkstring);
            if(it != TBaseJsonStruct_private::derivesName.end())
            {
                type.type = JSON_TYPE;
                type.typeSize = it->second.typeSize;
            }
            else
            {
                //格式未识别
                return false;
            }
        }
    }
    else
    {
        //格式未识别
        return false;
    }
    if(typestring != name)
    {
        ppos = name.find('[');
        if(std::string::npos != ppos)
        {
            type.isArray = true;
            typestring = name.substr(ppos + 1, std::string::npos);
            type.arraySize = atoi(typestring.c_str());
        }
    }
#endif
    return true;
}
bool SetJsonStructNumberValue(TJsonStructBaseMember& member, int intvalue, double doublevalue)
{
    switch(member.type.type)
    {
    case BOOL_TYPE:
        *(bool*)member.pAddr = intvalue;
        break;
    case CHAR_TYPE:
        *(char*)member.pAddr = intvalue;
        break;
    case WCHAR_TYPE:
        *(wchar_t*)member.pAddr = intvalue;
        break;
    case SHORT_TYPE:
        *(short*)member.pAddr = intvalue;
        break;
    case INT_TYPE:
        *(int*)member.pAddr = intvalue;
        break;
    case LONG_TYPE:
        *(long*)member.pAddr = intvalue;
        break;
    case LONGLONG_TYPE:
        *(long long*)member.pAddr = intvalue;
        break;
    case FLOAT_TYPE:
        *(float*)member.pAddr = doublevalue;
        break;
    case DOUBLE_TYPE:
        *(double*)member.pAddr = doublevalue;
        break;
    case LONGDOUBLE_TYPE:
        *(long double*)member.pAddr = doublevalue;
        break;
    default:
        return false;
    }
    return true;
}
int GetJsonStructIntValue(TJsonStructBaseMember& member)
{
    int nRet = 0;
    switch(member.type.type)
    {
    case BOOL_TYPE:
        nRet = *(bool*)member.pAddr;
        break;
    case CHAR_TYPE:
        nRet = *(char*)member.pAddr;
        break;
    case WCHAR_TYPE:
        nRet = *(wchar_t*)member.pAddr;
        break;
    case SHORT_TYPE:
        nRet = *(short*)member.pAddr;
        break;
    case INT_TYPE:
        nRet = *(int*)member.pAddr;
        break;
    case LONG_TYPE:
        nRet = *(long*)member.pAddr;
        break;
    case LONGLONG_TYPE:
        nRet = *(long long*)member.pAddr;
        break;
    case FLOAT_TYPE:
        nRet = *(float*)member.pAddr;
        break;
    case DOUBLE_TYPE:
        nRet = *(double*)member.pAddr;
        break;
    case LONGDOUBLE_TYPE:
        nRet = *(long double*)member.pAddr;
        break;
    }
    return nRet;
}
TBaseJsonStruct::TBaseJsonStruct()
{
    privateValue = new TBaseJsonStruct_private();
}
TBaseJsonStruct::~TBaseJsonStruct()
{
    delete privateValue;
}
void TBaseJsonStruct::RegMember(const char* szStructName, unsigned int structSize, const char* szTypeName, const char* szName, void* pAddr)
{
    TJsonStructBaseMember member ;
    member.typeName = szTypeName;
    TJsonStructDeriveInfo detiveInfo;
    detiveInfo.typeSize = structSize;
    TBaseJsonStruct_private::derivesName[szStructName] = detiveInfo;
    member.name = szName;
    member.pAddr = pAddr;
    member.support = CheckJsonStructMemberType(member.typeName, member.type);
#if JSON_STRUCT_DEBUG
    printf("member name:%s\ttype:%s\tsupport:%d",
           szName, szTypeName, member.support);
    if(member.support)
    {
        printf("\tanaType:%d,isArray:%d", member.type.type, member.type.isArray);
        if(member.type.isArray)
        {
            printf("\t\tarraySize:%d", member.type.arraySize);
        }
    }
    printf("\n");
#endif // JSON_STRUCT_DEBUG
    privateValue->members.push_back(member);
}

std::string TBaseJsonStruct::ToJson()
{
    std::string strRet;
    cJSON* root = ToJsonNode();
    if(NULL != root)
    {
        char* pszJson = cJSON_PrintUnformatted(root);
        strRet = pszJson;
        free(pszJson);
        cJSON_Delete(root);
    }
    return strRet;
}
cJSON* TBaseJsonStruct::ToJsonNode()
{
    cJSON *root = cJSON_CreateObject();
    if(!root) return NULL;
    int arraySize = 0;
    std::vector<TJsonStructBaseMember>::iterator iteMember = privateValue->members.begin();
    for(; iteMember != privateValue->members.end(); ++iteMember)
    {
        //名称不存在返回空指针
        arraySize = 0;
        if(iteMember->support)
        {
            if(iteMember->type.isArray)
            {
                std::vector<TJsonStructBaseMember>::iterator iteArraySize =
                std::find_if(privateValue->members.begin(), privateValue->members.end(),
                             member_array_size_finder(iteMember->name));
                if(iteArraySize != privateValue->members.end())
                {
                    arraySize =GetJsonStructIntValue(*iteArraySize);
                }
                if(!arraySize)
                {//set size to array size
                    arraySize = iteMember->type.arraySize;
                }
            }
            switch(iteMember->type.type)
            {
                case BOOL_TYPE:
                    if(!arraySize)
                    {//not array
                        if(*(bool*)iteMember->pAddr)
                        {
                            cJSON_AddTrueToObject(root, iteMember->name.c_str());
                        }else
                        {
                            cJSON_AddFalseToObject(root, iteMember->name.c_str());
                        }
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            if(*(((bool*)iteMember->pAddr) + arrIdx))
                            {
                                cJSON_AddItemToArray(node, cJSON_CreateTrue());
                            }else
                            {
                                cJSON_AddItemToArray(node, cJSON_CreateFalse());
                            }
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case CHAR_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(char*)iteMember->pAddr);
                    }else
                    {
                        cJSON_AddStringToObject(root, iteMember->name.c_str(), ((char*)iteMember->pAddr));
                    }
                    break;
                case WCHAR_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(wchar_t*)iteMember->pAddr);
                    }else
                    {
                        //" ERROR:do not support wchar_t now"
                    }
                    break;
                case SHORT_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(short*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((short*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case INT_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(int*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((int*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case LONG_TYPE:
                    //TODO cjson只支持int
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(long*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((long*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case LONGLONG_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(long long*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((long long*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case FLOAT_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(float*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((float*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case DOUBLE_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(double*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((double*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case LONGDOUBLE_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddNumberToObject(root, iteMember->name.c_str(), *(long double*)iteMember->pAddr);
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateNumber(*(((long double*)iteMember->pAddr) + arrIdx)));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case STRING_TYPE:
                    if(!arraySize)
                    {//not array
                        cJSON_AddStringToObject(root, iteMember->name.c_str(), ((std::string*)iteMember->pAddr)->c_str());
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON_AddItemToArray(node, cJSON_CreateString((((std::string*)iteMember->pAddr) + arrIdx)->c_str()));
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                case JSON_TYPE:
                    //TBaseJsonStruct的派生结构体
                    if(!arraySize)
                    {//not array
#if JSON_STRUCT_DEBUG
#endif // JSON_STRUCT_DEBUG
                        cJSON *node = ((TBaseJsonStruct*)iteMember->pAddr)->ToJsonNode();
                        if(node)
                        {
                            cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                        }
                    }else
                    {
                        cJSON *node = cJSON_CreateArray();
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            void* pCurAddr = ((char*)iteMember->pAddr) + (arrIdx * iteMember->type.typeSize);
                            cJSON_AddItemToArray(node, ((TBaseJsonStruct*)pCurAddr)->ToJsonNode());
                        }
                        cJSON_AddItemToObject(root, iteMember->name.c_str(), node);
                    }
                    break;
                default:
                    /*
                    //stl type
                    STL_SET,
                    STL_PAIR,
                    STL_MAP,
                    STL_VECTOR,
                    STL_LIST,
                    STL_DEQUE,
                    STL_CONTAINER,
                    */;
            }
        }else
        {
        }
    }
    if(cJSON_IsNull(root) || cJSON_IsInvalid(root))
    {
        return NULL;
    }
    return root;
}
bool TBaseJsonStruct::FromJson(const char* pszJson)
{
    bool bRet = false;
    //转换失败返回空指针
    cJSON *root = cJSON_Parse(pszJson);
    bRet = FromJsonNode(root);
    cJSON_Delete(root);
    return bRet;
}
bool TBaseJsonStruct::FromJsonNode(cJSON *root)
{
    if(0 == root) return false;
    int arraySize = 0;
    std::vector<TJsonStructBaseMember>::iterator iteMember = privateValue->members.begin();
    for(; iteMember != privateValue->members.end(); ++iteMember)
    {
        arraySize = 0;
        //名称不存在返回空指针
        cJSON *node = cJSON_GetObjectItem(root,iteMember->name.c_str());
        if(0 == node) continue;
        if(iteMember->support)
        {
            if(iteMember->type.isArray)
            {
                if(cJSON_IsArray(node))
                {
                    arraySize = cJSON_GetArraySize(node);
                    std::vector<TJsonStructBaseMember>::iterator iteArraySize =
                    std::find_if(privateValue->members.begin(), privateValue->members.end(),
                                 member_array_size_finder(iteMember->name));
                    if(iteArraySize != privateValue->members.end())
                    {
                        SetJsonStructNumberValue(*iteArraySize, arraySize, arraySize);
                    }
                }
            }
            switch(iteMember->type.type)
            {
                case BOOL_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(bool*)iteMember->pAddr = true;
                            break;
                        case cJSON_False:
                            *(bool*)iteMember->pAddr = false;
                            break;
                        case cJSON_Number:
                            *(bool*)iteMember->pAddr = node->valueint;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((bool*)iteMember->pAddr) + arrIdx) = true;
                                break;
                            case cJSON_False:
                                *(((bool*)iteMember->pAddr) + arrIdx) = false;
                                break;
                            case cJSON_Number:
                                *(((bool*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            }
                        }
                    }
                    break;
                case CHAR_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(char*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(char*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(char*)iteMember->pAddr = node->valueint;
                            break;
                        case cJSON_String:
                            if(iteMember->type.isArray)
                            {
                                strncpy((char*)iteMember->pAddr, node->valuestring, iteMember->type.arraySize);
                            }else
                            {
                                *(char*)iteMember->pAddr = node->valuestring[0];
                            }
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((char*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((char*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((char*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            case cJSON_String:
                                *(((char*)iteMember->pAddr) + arrIdx) = arrNode->valuestring[0];
                                break;
                            }
                        }
                    }
                    break;
                case WCHAR_TYPE:
                    //" ERROR:do not support wchar_t now"
                    break;
                case SHORT_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(short*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(short*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(short*)iteMember->pAddr = node->valueint;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((short*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((short*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((short*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            }
                        }
                    }
                    break;
                case INT_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(int*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(int*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(int*)iteMember->pAddr = node->valueint;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((int*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((int*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((int*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            }
                        }
                    }
                    break;
                case LONG_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(long*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(long*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(long*)iteMember->pAddr = node->valueint;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((long*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((long*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((long*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            }
                        }
                    }
                    break;
                case LONGLONG_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(long long*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(long long*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(long long*)iteMember->pAddr = node->valueint;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((long long*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((long long*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((long long*)iteMember->pAddr) + arrIdx) = arrNode->valueint;
                                break;
                            }
                        }
                    }
                    break;
                case FLOAT_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(float*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(float*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(float*)iteMember->pAddr = node->valuedouble;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((float*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((float*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((float*)iteMember->pAddr) + arrIdx) = arrNode->valuedouble;
                                break;
                            }
                        }
                    }
                    break;
                case DOUBLE_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(double*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(double*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(double*)iteMember->pAddr = node->valuedouble;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((double*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((double*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((double*)iteMember->pAddr) + arrIdx) = arrNode->valuedouble;
                                break;
                            }
                        }
                    }
                    break;
                case LONGDOUBLE_TYPE:
                    if(!arraySize)
                    {//not array
                        switch(node->type)
                        {
                        case cJSON_True:
                            *(long double*)iteMember->pAddr = 1;
                            break;
                        case cJSON_False:
                            *(long double*)iteMember->pAddr = 0;
                            break;
                        case cJSON_Number:
                            *(long double*)iteMember->pAddr = node->valuedouble;
                            break;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            switch(arrNode->type)
                            {
                            case cJSON_True:
                                *(((long double*)iteMember->pAddr) + arrIdx) = 1;
                                break;
                            case cJSON_False:
                                *(((long double*)iteMember->pAddr) + arrIdx) = 0;
                                break;
                            case cJSON_Number:
                                *(((long double*)iteMember->pAddr) + arrIdx) = arrNode->valuedouble;
                                break;
                            }
                        }
                    }
                    break;
                case STRING_TYPE:
                    if(!arraySize)
                    {//not array
                        if(node->type == cJSON_String)
                        {
                            *(std::string*)iteMember->pAddr = node->valuestring;
                        }
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            if(arrNode->type == cJSON_String)
                            {
                                *(((std::string*)iteMember->pAddr) + arrIdx) = arrNode->valuestring;
                            }
                        }
                    }
                    break;
                case JSON_TYPE:
                    //TBaseJsonStruct的派生结构体
                    if(!arraySize)
                    {//not array
#if JSON_STRUCT_DEBUG
#endif // JSON_STRUCT_DEBUG
                        ((TBaseJsonStruct*)iteMember->pAddr)->FromJsonNode(node);
                    }else
                    {
                        for(int arrIdx = 0; arrIdx < arraySize; ++arrIdx)
                        {
                            void* pCurAddr = ((char*)iteMember->pAddr) + (arrIdx * iteMember->type.typeSize);
                            cJSON* arrNode = cJSON_GetArrayItem(node, arrIdx);
                            ((TBaseJsonStruct*)pCurAddr)->FromJsonNode(arrNode);
                        }
                    }
                    break;
                default:
                    /*
                    //stl type
                    STL_SET,
                    STL_PAIR,
                    STL_MAP,
                    STL_VECTOR,
                    STL_LIST,
                    STL_DEQUE,
                    STL_CONTAINER,
                    */;
            }
        }else
        {
        }
    }
    return true;
}
