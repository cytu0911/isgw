NDS��֧�� .ini �μ���ʽ�����ã�Ҳ֧�� .xml ��ʽ�������ļ���
�����ֱ�Ϊ UNIX_config �� XML_config 

һ ���ظ�����֧�֣����磬�����������£���ע��zzzz��xxxx��Ҫ�������ţ���������֧�ֶ���
<A c="zzzz">
<a b="xxxx">yyyy</a>
<d e="uuuu">vvvv</d>
</A>

1 �������ļ�(�������������)
XML_config<NDS_single_config_node> Cfg_App(CONFIG_FILE);  //��ȡ����

2 ��������ֵ
��λ�ȡzzzz��
Cfg_App["A"]("c"), ����һ��string�������zzzz
��λ�ȡxxxx��
Cfg_App["A"]["a"]("b"), ����һ��string�������xxxx 
��λ�ȡyyyy��
Cfg_App["A"]["a"](), ����һ��string�������yyyy
������
���ô򿪣���������ʶ����ܳ����쳣����Ҫ����NDS_exception�쳣��������Ϣ
����ͨ������NDS_excption�е�what()������ȡ�������������Ҫ����������һ��const char*

�� �ظ�����֧�֣����磬�����������£���ע��zzzz��xxxx��Ҫ�������ţ���������֧�ֶ���
<A c="zzzz">
<a b="xxxx">aaaa</a>
<a b="bbbb">aaaa</a>
<b m="1" m="2">bbbb</b>
</A>

��ȡ�ڵ����Ŀ��
Cfg_App["A"].get_tree().count("a") ��ȡ�� a ������ĿΪ2
Cfg_App["A"]["b"].get_node().count("m") ��ȡ�� b �ڵ��m���Ե�ĿΪ2

�������ô򿪷�ʽ��
 	XML_config<NDS_multi_config_node> Cfg_App(CONFIG_FILE);

	typedef multimap< string, string > node_type;
	typedef multimap< string, NDS_multi_config_node* > tree_type;
	typedef NDS_multi_config_node& tree_ret_type;
	typedef pair<node_type::iterator,node_type::iterator> node_range_ret_type;
	typedef pair<tree_type::iterator,tree_type::iterator> tree_range_ret_type;

	tree_range_ret_type treesub;
	node_range_ret_type nodesub;
	try{

		//�ظ���tree(a)
		treesub=Cfg_App["A"].equal_range_tree("a");
		for(tree_type::iterator i = treesub.first; i != treesub.second; ++i)
		cout << "a->" << (*i->second)() << endl;	
		//�ظ���node(m)
		nodesub = Cfg_App["A"]["b"].equal_range_node("m");
		for(node_type::iterator j = nodesub.first; j != nodesub.second; ++j)
		{
			cout << "m=" << j->second << endl;
		}
	}
	catch (NDS_exception e)
	{
		cout << e.what() << endl;
	}
