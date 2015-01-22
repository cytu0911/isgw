NDS库支持 .ini 段键格式的配置，也支持 .xml 格式的配置文件，
类名分别为 UNIX_config 和 XML_config 

一 非重复主键支持，例如，配置内容如下：（注：zzzz、xxxx需要加上引号），该配置支持多层次
<A c="zzzz">
<a b="xxxx">yyyy</a>
<d e="uuuu">vvvv</d>
</A>

1 打开配置文件(生成配置类对象)
XML_config<NDS_single_config_node> Cfg_App(CONFIG_FILE);  //读取配置

2 访问配置值
如何获取zzzz：
Cfg_App["A"]("c"), 返回一个string，存放着zzzz
如何获取xxxx：
Cfg_App["A"]["a"]("b"), 返回一个string，存放着xxxx 
如何获取yyyy：
Cfg_App["A"]["a"](), 返回一个string，存放着yyyy
错误处理：
配置打开，配置项访问都可能出现异常，需要接收NDS_exception异常，错误信息
可以通过调用NDS_excption中的what()函数获取，这个函数不需要参数，返回一个const char*

二 重复主键支持，例如，配置内容如下：（注：zzzz、xxxx需要加上引号），该配置支持多层次
<A c="zzzz">
<a b="xxxx">aaaa</a>
<a b="bbbb">aaaa</a>
<b m="1" m="2">bbbb</b>
</A>

获取节点的数目：
Cfg_App["A"].get_tree().count("a") 获取到 a 的列数目为2
Cfg_App["A"]["b"].get_node().count("m") 获取到 b 节点的m属性的目为2

复杂配置打开方式：
 	XML_config<NDS_multi_config_node> Cfg_App(CONFIG_FILE);

	typedef multimap< string, string > node_type;
	typedef multimap< string, NDS_multi_config_node* > tree_type;
	typedef NDS_multi_config_node& tree_ret_type;
	typedef pair<node_type::iterator,node_type::iterator> node_range_ret_type;
	typedef pair<tree_type::iterator,tree_type::iterator> tree_range_ret_type;

	tree_range_ret_type treesub;
	node_range_ret_type nodesub;
	try{

		//重复的tree(a)
		treesub=Cfg_App["A"].equal_range_tree("a");
		for(tree_type::iterator i = treesub.first; i != treesub.second; ++i)
		cout << "a->" << (*i->second)() << endl;	
		//重复的node(m)
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
