#include <string.h>
#include <vector>
#include <map>
#include <iostream>
#include "../jsonstruct.h"

using namespace std;

JSONSTRUCT(testd)
{
    int dkey1;
    std::string dkey2;
    testd()
    {
        JSON_REGMEMBER(dkey1);
        JSON_REGMEMBER(dkey2);
        dkey1 = 0;
    }
};
struct testb :public TBaseJsonStruct
{
    int intkey[2];
    char charkey[200];
    std::string stringkey;
    testd dkey[5];
    int dkeyNum;
    testb()
    {
        JSON_REGMEMBER(intkey);
        JSON_REGMEMBER(charkey);
        JSON_REGMEMBER(stringkey);
        JSON_REGMEMBER(dkey);
        JSON_REGMEMBER(dkeyNum);
        memset(charkey, 0, sizeof(charkey));
        dkeyNum = 0;
        //intkey = {0};
    }
};

int main()
{
    testb testbbb;
    std::string strJson1 = "{\"intkey\":[5,10],\"charkey\":\"charvalue\",\"stringkey\":\"stringvalue\",\"dkey\":[{\"dkey1\":404,\"dkey2\":\"dkey2value\"}]}";
    cout << "src string:\n"<<strJson1 << endl;
    testbbb.FromJson(strJson1.c_str());
    cout << "fromJson result:"<< endl;
    cout << "\ttestbbb.intkey[0]:" <<testbbb.intkey[0]<< endl;
    cout << "\ttestbbb.intkey[1]:" <<testbbb.intkey[1]<< endl;
    cout << "\ttestbbb.charkey:" <<testbbb.charkey<< endl;
    cout << "\ttestbbb.stringkey:" <<testbbb.stringkey<< endl;
    cout << "\ttestbbb.dkey.dkey1:" <<testbbb.dkey[0].dkey1<< endl;
    cout << "\ttestbbb.dkey.dkey2:" <<testbbb.dkey[0].dkey2<< endl;
    cout << "\ttestbbb.dkeyNum:" <<testbbb.dkeyNum<< endl;
    cout << endl;
    cout << "toJson result:"<< endl;
    cout <<testbbb.ToJson()<< endl;
    return 0;
}
