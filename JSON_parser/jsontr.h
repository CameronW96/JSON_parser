#pragma once
#ifndef jsontr
#define jsontr

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

			public:
				JSON_KVP() {};
				//JSON_KVP(JSON_KVP& t_copy) {};

				/*JSON_KVP& operator=(const JSON_KVP& t_copy)
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
				}*/
			};

		private:
			// specifies whether this node is a JSON object
			// m_value should be initialized to a vector in this case
			bool m_is_object = false;
			std::string object_key;
			std::variant<JSON_KVP, std::vector<JSON_KVP>> m_kvp;

		public:
			// **** VALUE SETTERS ****

			void init(const std::string &t_key, const JSON_Value &t_value)
			{
				JSON_KVP init_kvp;
				init_kvp.m_key = t_key;
				init_kvp.m_value = t_value;

				m_kvp = init_kvp;
			}
			void init_int (const std::string &t_key, const int &t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_bool (const std::string &t_key, const bool &t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_fpoint (const std::string &t_key, const double &t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_string (const std::string &t_key, const std::string &t_value)
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_object (const std::string &t_key, const Node* &t_value) // use with unique_ptr
			{
				JSON_Value init_value;
				init_value.m_value_individual = std::move(t_value);

				init(t_key, init_value);
			}

			// **** STATE SETTERS ****

			// declares that this node contains an array and initializes the value as empty array
			// array must be populated with array_push_*type
			void init_array (const std::string &t_key)
			{
				JSON_KVP* temp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				temp_ptr->m_is_array = true;
				std::vector<JSON_Value> init_vector;
				temp_ptr->m_value = init_vector;
			}
			// declares that this node contains an object and initializes the node with an empty object
			// object must be populated with object_push
			void init_object (const std::string &t_key)
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

			// helper method that pushes a JSON_Value object into the current nodes array
			// returns false if node does not contain an array or true on success
			bool array_push(const JSON_Value &t_value)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				if (kvp_ptr == nullptr) { return false; } // check to ensure node contains the assumed data type (JSON_KVP vs. std::vector<JSON_KVP>)

				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				if (value_ptr == nullptr) { return false; } // check to ensure KVP contains the assumed data type (JSON_Value vs. std::vector<JSON_Value>)

				value_ptr->push_back(t_value);
				return true;
			}
			bool array_push_int (const int &t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				if (array_push(temp_value))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			bool array_push_bool (const bool &t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				if (array_push(temp_value))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			bool array_push_fpoint (const double &t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				if (array_push(temp_value))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			bool array_push_string (const std::string &t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				if (array_push(temp_value))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			bool array_push_object (const Node* &t_input)
			{
				JSON_Value temp_value;
				temp_value.m_value_individual = t_input;
				if (array_push(temp_value))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			// **** OBJECT MANIPULATION ****

			// pushes a previously constructed KVP
			bool object_push (const JSON_KVP &t_input)
			{
				std::vector<JSON_KVP>* obj_ptr = std::get_if<std::vector<JSON_KVP>>(&m_kvp);
				if (obj_ptr == nullptr) { return false; } // check to ensure KVP contains the assumed data type (std::vector<JSON_KVP> vs. JSON_KVP)

				obj_ptr->push_back(t_input);
				return true;
			}

			// **** STATE GETTERS ****

			// returns the state of this node
			// 1 = primitive KVP, 2 = object, 3 = array
			int get_state ()
			{}

			// returns the value type
			// 0 = VOID/NULL, 1 = int, 2 = bool, 3 = double, 4 = string, 5 = object
			int get_value_type ()
			{}
			
			std::string get_key ()
			{
				if (m_is_object == true)
				{ 
					return object_key;
				}
				else
				{
					JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
					return kvp_ptr->m_key;
				}
			}

			// **** VALUE GETTERS ****

			// returns value as type requested
			// throws "bad_type" if type does not match
			int get_value_as_int ()
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				JSON_Value* value_ptr = std::get_if<JSON_Value>(&kvp_ptr->m_value);

				if (std::holds_alternative<int>(value_ptr->m_value_individual))
				{
					return std::get<int>(value_ptr->m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			bool get_value_as_bool ()
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				JSON_Value* value_ptr = std::get_if<JSON_Value>(&kvp_ptr->m_value);

				if (std::holds_alternative<bool>(value_ptr->m_value_individual))
				{
					return std::get<bool>(value_ptr->m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			double get_value_as_fpoint ()
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				JSON_Value* value_ptr = std::get_if<JSON_Value>(&kvp_ptr->m_value);

				if (std::holds_alternative<double>(value_ptr->m_value_individual))
				{
					return std::get<double>(value_ptr->m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			std::string get_value_as_string ()
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				JSON_Value* value_ptr = std::get_if<JSON_Value>(&kvp_ptr->m_value);

				if (std::holds_alternative<std::string>(value_ptr->m_value_individual))
				{
					return std::get<std::string>(value_ptr->m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			Node* get_value_as_object ()
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				JSON_Value* value_ptr = std::get_if<JSON_Value>(&kvp_ptr->m_value);

				if (std::holds_alternative<Node*>(value_ptr->m_value_individual))
				{
					return std::get<Node*>(value_ptr->m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}

			int get_array_value_as_int (int t_index)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				std::vector<JSON_Value>& value_ref = *value_ptr;
				JSON_Value temp_value = value_ref[t_index];

				if (std::holds_alternative<int>(temp_value.m_value_individual))
				{
					return std::get<int>(temp_value.m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			bool get_array_value_as_bool (int t_index)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				std::vector<JSON_Value>& value_ref = *value_ptr;
				JSON_Value temp_value = value_ref[t_index];

				if (std::holds_alternative<bool>(temp_value.m_value_individual))
				{
					return std::get<bool>(temp_value.m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			double get_array_value_as_fpoint (int t_index)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				std::vector<JSON_Value>& value_ref = *value_ptr;
				JSON_Value temp_value = value_ref[t_index];

				if (std::holds_alternative<double>(temp_value.m_value_individual))
				{
					return std::get<double>(temp_value.m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			std::string get_array_value_as_string (int t_index)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				std::vector<JSON_Value>& value_ref = *value_ptr;
				JSON_Value temp_value = value_ref[t_index];

				if (std::holds_alternative<std::string>(temp_value.m_value_individual))
				{
					return std::get<std::string>(temp_value.m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
			Node* get_array_value_as_object (int t_index)
			{
				JSON_KVP* kvp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value>* value_ptr = std::get_if<std::vector<JSON_Value>>(&kvp_ptr->m_value);
				std::vector<JSON_Value>& value_ref = *value_ptr;
				JSON_Value temp_value = value_ref[t_index];

				if (std::holds_alternative<Node*>(temp_value.m_value_individual))
				{
					return std::get<Node*>(temp_value.m_value_individual);
				}
				else
				{
					throw "bad_type";
				}
			}
		};

	private:
		std::vector<Node> main_list;

	public:
		void init_node_int(std::string key, int value)
		{
			Node temp_node;
			temp_node.init_int(key, value);
			main_list.push_back(temp_node);
		}

		int return_node_value_int()
		{
			return main_list[0].get_value_as_int();
		}
		
		void operator[](int t_input)
		{

		}
		void operator[](bool t_input)
		{

		}
		void operator[](double t_input)
		{

		}
		void operator[](std::string t_input)
		{

		}
	};
}

#endif // !jsontr.h

