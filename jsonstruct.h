
#ifndef _JSON_STRUCT_CPP_H_
#define _JSON_STRUCT_CPP_H_
#include <string>
#include <typeinfo>

#define JSONSTRUCT(x) struct x :public TBaseJsonStruct
#define JSON_REGMEMBER(x) {RegMember(typeid(*this).name(),sizeof(*this),\
                                     ((char*)((TBaseJsonStruct*)this)) - ((char*)this),\
                                     typeid(x).name(), #x, &x);}

struct TBaseJsonStruct_private;
struct cJSON;
struct TBaseJsonStruct
{
    TBaseJsonStruct();
    ~TBaseJsonStruct();
private:
    TBaseJsonStruct_private* privateValue;
    bool FromJsonNode(cJSON* root);
    cJSON* ToJsonNode();
protected:
    void RegMember(const char* szStructName, unsigned int structSize,unsigned int thisOffset, const char* szTypeName, const char* szName, void* pAddr);
public:
    //公共接口
    std::string ToJson();
    bool FromJson(const char* pszJson);
};
#endif // _JSON_STRUCT_CPP_H_
