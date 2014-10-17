#include <iostream>
#include <string>

using namespace std;

class person
{
public:
    person(const string& name, unsigned int age);

    ~person();

    string get_person_name() const;

private:
    string m_name;
    unsigned int m_age;
};


//向外暴露这个接口，使用" extern "C" "来声明之，否则待会儿调用symbol来获取
//本函数的地址时就必须填写mingle之后的全名
extern "C" person create_person(const string& name, unsigned int age);
