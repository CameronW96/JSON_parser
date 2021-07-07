#pragma once
#ifndef jsontr.h
#define jsontr.h

#include <utility>
#include <string>
#include <vector>
#include <list>
#include <variant>
#include <type_traits>
#include <sstream>

namespace JSONator
{
	class JSON_List
	{
	private:
		class Node
		{
		private:
			using var_t = std::variant<int, bool, double, std::string, Node*>;

			class JSON_Value
			{
			public:

				// m_type represents the type held in m_value
				// 0 = VOID/NULL, 1 = int, 2 = bool, 3 = double, 4 = string, 5 = object
				// int m_type = 0; !not needed
				var_t m_value_individual = 0;
			public:

				JSON_Value() {};
				JSON_Value(const JSON_Value& t_copy)
				{
					m_value_individual = t_copy.m_value_individual;
				};
			};

			class JSON_KVP
			{
			public:

				// m_type represents the type held in m_value
				// 0 = VOID/NULL, 1 = KVP, 2 = array
				bool m_is_array = false;
				std::string m_key;
				std::variant<JSON_Value, std::vector<JSON_Value>> m_value;
				std::vector<JSON_Value> value2;

			public:
				JSON_KVP() {};
				JSON_KVP(JSON_KVP& t_copy) {};

				JSON_KVP& operator=(const JSON_KVP& t_copy)
				{
					m_is_array = t_copy.m_is_array;
					m_key = t_copy.m_key;
					if (std::holds_alternative<JSON_Value>(t_copy.m_value))
					{
						JSON_Value temp = std::get<JSON_Value>(t_copy.m_value);
						m_value = temp;
					}
					else
					{
						std::vector<JSON_Value> temp = std::get<std::vector<JSON_Value>>(t_copy.m_value);
						m_value = temp;
					}
				}
			};

			// specifies whether this node is a JSON object
			// m_value should be initialized to a vector in this case
			bool m_is_object = false;
			std::variant<JSON_KVP, std::vector<JSON_KVP>> m_kvp;

		public:
			// **** VALUE SETTERS ****

			void init(std::string t_key, JSON_Value t_value)
			{
				JSON_KVP init_kvp;
				init_kvp.m_key = t_key;
				init_kvp.m_value = t_value;

				m_kvp = init_kvp;
			}
			void init_int (std::string t_key, int t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_bool (std::string t_key, bool t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_fpoint (std::string t_key, double t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_string (std::string t_key, std::string t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_object (std::string t_key, Node* t_value) // use with unique_ptr
			{
				JSON_Value init_value;
				init_value.m_value_individual = std::move(t_value);

				init(t_key, init_value);
			}

			// **** STATE SETTERS ****

			// declares that this node contains an array and initializes the value as empty array
			// array must be populated with array_push_*type
			void init_array (std::string t_key)
			{
				JSON_KVP* temp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				temp_ptr->m_is_array = true;
				std::vector<JSON_Value> init_vector;
				temp_ptr->m_value = init_vector;
			}
			// declares that this node contains an object and initializes the node with an empty object
			// object must be populated with object_push
			void init_object (std::string t_key)
			{
				JSON_Value temp_value;

				JSON_KVP temp_kvp;
				temp_kvp.m_value = temp_value;

				std::unique_ptr<Node> temp_node(new Node);
				temp_node->m_is_object = true;
				temp_node->m_kvp = temp_kvp;

				JSON_Value init_value;
				init_value.m_value_individual = std::move(&temp_node);

				JSON_KVP init_kvp;
				init_kvp.m_key = t_key;
				init_kvp.m_value = init_value;

				m_kvp = init_kvp;
			}

			// **** ARRAY MANIPULATION ****
			void array_push(JSON_Value t_value)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value)->push_back(t_value);
			}
			void array_push_int (int t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				array_push(temp_value);
			}
			void array_push_bool (bool t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				array_push(temp_value);
			}
			void array_push_fpoint (double t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				array_push(temp_value);
			}
			void array_push_string (std::string t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				array_push(temp_value);
			}
			void array_push_object (Node* t_input) 
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				array_push(temp_value);
			}

			// **** OBJECT MANIPULATION ****

			// pushes a previously constructed Node object
			// expects a unique_ptr to be passed as the argument
			void object_push (Node& t_input)
			{}

			// **** STATE GETTERS ****

			// returns the state of this node
			// 1 = primitive KVP, 2 = object, 3 = array
			int get_state ()
			{}

			// returns the value type
			// 0 = VOID/NULL, 1 = int, 2 = bool, 3 = double, 4 = string, 5 = object
			int get_value_type ()
			{}

			// **** VALUE GETTERS ****

			// returns value as type requested
			// returns nullptr if type requested does not match type contained
			std::string get_key ()
			{}

			int get_value_as_int ()
			{}
			bool get_value_as_bool ()
			{}
			double get_value_as_fpoint ()
			{}
			std::string get_value_as_string ()
			{}
			Node* get_value_as_object () // returns unique_ptr and must be moved via move semantics
			{}

			int get_array_value_as_int (int t_index)
			{}
			bool get_array_value_as_bool (int t_index)
			{}
			double get_array_value_as_fpoint (int t_index)
			{}
			std::string get_array_value_as_string (int t_index)
			{}
			Node* get_array_value_as_object (int t_index) // returns unique_ptr and must be moved via move semantics
			{}
		};

	private:
		std::vector<Node> main_list;

	public:

	};
}

#endif // !jsontr.h

