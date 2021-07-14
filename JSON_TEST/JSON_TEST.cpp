#include "pch.h"
#include "CppUnitTest.h"
#include "jsontr.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework; 

namespace JSONTEST
{
	TEST_CLASS(JSONTEST)
	{
	public:
		
		TEST_METHOD(convert_to_int_test1)
		{
			cjw::JSON_List json_list;
			std::string str = "1234";
			int converted_value = json_list.convert_to_int(str);
			Assert::AreEqual(converted_value, 1234);
		}
		TEST_METHOD(convert_to_int_test2)
		{
			cjw::JSON_List json_list;
			std::string str = "1 2 3 4";
			int converted_value = json_list.convert_to_int(str);
			Assert::AreEqual(converted_value, 1234);
		}
		TEST_METHOD(convert_to_int_test3)
		{
			cjw::JSON_List json_list;
			std::string str = " 1 2 3 4 ";
			int converted_value = json_list.convert_to_int(str);
			Assert::AreEqual(converted_value, 1234);
		}
	};
}
