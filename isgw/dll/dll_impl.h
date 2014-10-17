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


//���Ⱪ¶����ӿڣ�ʹ��" extern "C" "������֮��������������symbol����ȡ
//�������ĵ�ַʱ�ͱ�����дmingle֮���ȫ��
extern "C" person create_person(const string& name, unsigned int age);
