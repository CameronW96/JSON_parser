#include "jsontr.h"
#include <iostream>






int main()
{
	std::string key = "mykey";
	int value = 100;

	JSONator::JSON_List my_list;
	my_list.init_node_int(key, value);
	int return_value = my_list.return_node_value_int();

	std::cout << "My value is: " << return_value << std::endl;






	return 0;
}