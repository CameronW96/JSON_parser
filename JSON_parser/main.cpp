#include <iostream>
#include <sstream>
#include <variant>
#include <type_traits>

using var_t = std::variant<int, bool, double, std::string>;

void print(var_t t_arg)
{
	std::visit(
		[](const auto & arg)
		{ 
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, int>)
			{
				std::cout << arg << std::endl;
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				if (arg)
				{
					std::cout << "True" << std::endl;
				}
				else
				{
					std::cout << "False" << std::endl;
				}
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				std::cout << arg << std::endl;
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				std::cout << arg << std::endl;
			}
			else
			{
				static_assert(!std::is_same<T, T>::value, "Unsupported data type");
			}
		},
		t_arg);
}

int main() {
	var_t w = 1;
	print(w);
	w = false;
	print(w);
	w = 1.066;
	print(w);
	w = std::string( "Hello World!");
	print(w);
}