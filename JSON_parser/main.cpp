#include "jsontr.h"
#include <iostream>
#include <assert.h>

using namespace cjw;

int main()
{
		std::string primitive_test = "{ int : 1, double : 1.332, bool : false, string : \"Hello World!\" }";
		JSON_List list1 = cjw::JSON_List::parse(primitive_test);

		std::string array_test = "{ array : [1, 2, \"word\"] }";
		JSON_List list2 = cjw::JSON_List::parse(array_test);

		std::string object_test = "{ object : { nestedKey: \"nestedValue\", anotherArray : [1, true, 1.443] } }";
		JSON_List list3 = cjw::JSON_List::parse(object_test);

		int integer_return_test = JSON_List::r_int(list2.d_n("array").a_n(1));
		assert(integer_return_test == 2);

		double double_return_test = JSON_List::r_double(list1.d_n("double"));
		assert(double_return_test == 1.332);

		bool boolean_return_test = JSON_List::r_bool(list1.d_n("bool"));
		assert(boolean_return_test == false);

		std::string string_return_test = JSON_List::r_string(list2.d_n("array").a_n(2));
		assert(string_return_test == std::string("word"));

		std::string file_test = JSON_List::read_file("example.json");
}