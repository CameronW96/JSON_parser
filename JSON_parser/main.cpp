#include "jsontr.h"
#include <iostream>



int main()
{
		std::string primitive_test = "{ int : 1, double : 1.332, bool : false, string : \"Hello World!\" }";
		cjw::JSON_List list1 = cjw::JSON_List::parse(primitive_test);

		std::string array_test = "{ array : [1, 2, \"word\"] }";
		cjw::JSON_List list2 = cjw::JSON_List::parse(array_test);

		std::string object_test = "{ object : { nestedKey: \"nestedValue\", anotherArray : [1, true, 1.443] } }";
		cjw::JSON_List list3 = cjw::JSON_List::parse(object_test);
}