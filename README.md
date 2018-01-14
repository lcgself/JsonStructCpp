# JsonStructCpp
实现结构体和json之间的自动转换

程序的目标是用一种“优雅”的方式实现结构体和json字符串之间的相互转换。
本质是实现了一个简化版的反射机制。
目前为止，支持编译器内置类型、stl的string和自定义的，基于
TBaseJsonStruct派生的结构体，以及他们的一维数组。

用法：
使用TBaseJsonStruct作为父结构体派生出自己的结构体，或者直接使用
JSONSTRUCT宏创建结构体，然后在结构体的构造函数中注册要转换到json中的
成员变量。示例如下：
JSONSTRUCT(testa)
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
    testa akey[5];
    int akeyNum;
    testb()
    {
        JSON_REGMEMBER(intkey);
        JSON_REGMEMBER(charkey);
        JSON_REGMEMBER(stringkey);
        JSON_REGMEMBER(akey);
        JSON_REGMEMBER(akeyNum);
        memset(charkey, 0, sizeof(charkey));
        akeyNum = 0;
        //intkey = {0};
    }
};

之后就可以在任何需要的地点使用FromJson从字符串取值或者
使用ToJson将结构体转化成json字符串。
代码示例：
int main()
{
    testb testbbb;
    std::string strJson1 = 
    "{\"intkey\":[5,10],\"charkey\":\"charvalue\",\"stringkey\":\"stringvalue\",\"dkey\":[{\"dkey1\":404,\"dkey2\":\"dkey2value\"}]}";
    cout << "src string:\n"<<strJson1 << endl;
    testbbb.FromJson(strJson1.c_str());
    cout << "fromJson result:"<< endl;
    cout << "\ttestbbb.intkey[0]:" <<testbbb.intkey[0]<< endl;
    cout << "\ttestbbb.intkey[1]:" <<testbbb.intkey[1]<< endl;
    cout << "\ttestbbb.charkey:" <<testbbb.charkey<< endl;
    cout << "\ttestbbb.stringkey:" <<testbbb.stringkey<< endl;
    cout << "\ttestbbb.dkey.dkey1:" <<testbbb.akey[0].dkey1<< endl;
    cout << "\ttestbbb.dkey.dkey2:" <<testbbb.akey[0].dkey2<< endl;
    cout << "\ttestbbb.dkeyNum:" <<testbbb.akeyNum<< endl;
    cout << endl;
    cout << "toJson result:"<< endl;
    cout <<testbbb.ToJson()<< endl;
    return 0;
}
运行结果：
src string:
{"intkey":[5,10],"charkey":"charvalue","stringkey":"stringvalue","dkey":[{"dkey1":404,"dkey2":"dkey2value"}]}
fromJson result:
        testbbb.intkey[0]:5
        testbbb.intkey[1]:10
        testbbb.charkey:charvalue
        testbbb.stringkey:stringvalue
        testbbb.dkey.dkey1:1991451420
        testbbb.dkey.dkey2:
        testbbb.dkeyNum:0

toJson result:
{"intkey":[5,10],"charkey":"charvalue","stringkey":"stringvalue","akeyNum":0}
