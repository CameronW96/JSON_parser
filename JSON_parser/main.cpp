#include "jsontr.h"
#include <iostream>
#include <assert.h>

using namespace cjw;

int main()
{// TEST CREATE
		std::string primitive_test = "{ int : 1, double : 1.332, bool : false, string : \"Hello World!\" }";
		JSON_List list1 = JSON_List::parse(primitive_test);

		std::string array_test = "{ array : [1, 2, \"word\"] }";
		JSON_List list2 = JSON_List::parse(array_test);

		std::string object_test = "{ object : { nestedKey: \"nestedValue\", anotherArray : [1, true, 1.443] } }";
		JSON_List list3 = JSON_List::parse(object_test);

		std::string file_test = JSON_List::read_file("example.json");
		JSON_List list4 = JSON_List::parse(file_test);

// TEST READ
		int integer_return_test = JSON_List::r_int(list2.d_n("array").a_n(1));
		assert(integer_return_test == 2);

		double double_return_test = JSON_List::r_double(list1.d_n("double"));
		assert(double_return_test == 1.332);

		bool boolean_return_test = JSON_List::r_bool(list1.d_n("bool"));
		assert(boolean_return_test == false);

		std::string string_return_test = JSON_List::r_string(list2.d_n("array").a_n(2));
		assert(string_return_test == std::string("\"word\""));
	
		std::string file_parse_test1 = JSON_List::r_string(list4.d_n("glossary").d_n("title"));
		assert(file_parse_test1 == "\"example glossary\"");

		std::string file_parse_test2 = JSON_List::r_string(list4.d_n("glossary").d_n("GlossDiv").d_n("GlossList").d_n("GlossEntry").d_n("ID"));
		assert(file_parse_test2 == std::string("\"SGML\""));

// TEST UPDATE
		list2.update_value(42, list2.d_n("array").a_n(1));
		int update_int_test = JSON_List::r_int(list2.d_n("array").a_n(1));
		assert(update_int_test == 42);
		
		list1.update_value(14.8765, list1.d_n("double"));	
		double update_double_test = JSON_List::r_double(list1.d_n("double"));
		assert(update_double_test == 14.8765);

		list1.update_value(true, list1.d_n("bool"));
		bool update_bool_test = JSON_List::r_bool(list1.d_n("bool"));
		assert(update_bool_test == true);

		list2.update_value(std::string("string"), list2.d_n("array").a_n(2));
		std::string update_string_test = JSON_List::r_string(list2.d_n("array").a_n(2));
		assert(update_string_test == std::string("string"));

}