/**
* \mainpage JSONator
* 
* Author: Cameron White
* 
* This library provides a basic JSON data structure.
* 
* This is intended for use with raw text input such as the type received from web API requests or json files.
*/

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
#include <algorithm>

namespace cjw
{

	class JSON_List
	{
	private:
		class Node
		{
			friend class JSON_List;
			using var_t = std::variant<int, bool, double, std::string, Node*>;

		private:
			class JSON_Value
			{
			public:
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
			void init_double (const std::string &t_key, const double &t_value)
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
	private:
		// return integer indicating the type held in the string argument
		// 0 = NAN, 1 = INTEGER, 2 = DOUBLE
		int is_number(std::string t_string_input)
		{
			// remove whitespace from string
			t_string_input.erase(std::remove_if(t_string_input.begin(), t_string_input.end(), ::isspace), t_string_input.end());

			int decimal_counter = 0;
			for (int i = 0; i < t_string_input.size(); i++)
			{
				if (std::isdigit(static_cast<unsigned char>(t_string_input[i])))
				{
					continue;
				}
				else if (t_string_input[i] == '.')
				{
					decimal_counter++;
					continue;
				}
				else
				{
					return 0;
				}
			}
			if (decimal_counter == 0)
			{
				return 1;
			}
			else if (decimal_counter == 1)
			{
				return 2;
			}
			else
			{
				return 0;
			}
		}

		// return integer indicating the type held in the string argument
		// 0 = NULL/SYNTAX ERROR, 1 = INTEGER, 2 = DOUBLE, 3 = BOOL, 4 = STRING, 5 = NODE
		int check_type(std::string t_string_input)
		{
			std::string::iterator it = t_string_input.begin();
			if (*it == ' ') // skip preceeding whitespace
			{
				it++;
			}

			if (*it == '"') // check for string
			{
				return 4;
			}
			else if (*it == '{') // check for object
			{
				return 5;
			}
			else if (*it == 'f' || *it == 't') // check for bool
			{
				std::string temp_buf;
				for (int i = 0; i < 5; i++)
				{
					if (it == t_string_input.end())
					{
						break;
					}
					else
					{
						temp_buf.push_back(*it);
						it++;
					}
				}
				if (temp_buf == "false")
				{
					return 3;
				}
				else
				{
					temp_buf.pop_back();
					if (temp_buf == "true")
					{
						return 3;
					}
				}
			}
			else
			{
				return is_number(t_string_input);
			}


		}

		double convert_to_double(std::string t_integer_string)
		{
			// remove whitespace
			t_integer_string.erase(std::remove_if(t_integer_string.begin(), t_integer_string.end(), ::isspace), t_integer_string.end());

			return std::stod(t_integer_string);
		}

		int convert_to_int(std::string t_integer_string)
		{
			// remove whitespace
			t_integer_string.erase(std::remove_if(t_integer_string.begin(), t_integer_string.end(), ::isspace), t_integer_string.end());

			return std::stoi(t_integer_string);
		}

		Node read_object(const std::string &t_object_key, const std::string& t_input, std::string::iterator& t_iterator)
		{
			
		}

		Node::JSON_KVP read_array(const std::string& t_input, std::string::iterator& t_iterator)
		{

		}

		/// <summary>
		/// obtains the value from a string input 
 		/// </summary>
		/// <param name="t_input"></param>
		/// <param name="t_iterator"></param>
		/// <returns></returns>
		Node get_value()
		{
			// take in value to check

			// determine value
			int type = check_type(value);

			switch (type)
			{
			case 0: // invalid input
			{
				Node empty_object;
				empty_object.init_object(t_object_key);
				return empty_object; // return empty object
				break;
			}
			case 1: // integer
			{
				Node temp_integer_node;
				temp_integer_node.init_int(key, convert_to_int(value));
				node_return_object.object_push(temp_integer_node); // node or KVP??
				break;
			}
			case 2: // double
			{
				Node temp_double_node;
				temp_double_node.init_double(key, convert_to_double(value));
				break;
			}
			}
			// call appropriate read function

				// read function recursively calls get_value() if it encounters an array or object
				
		}

	public:
		/**
		* Parses c-string style input using recursion to traverse the JSON_List structure.
		* @param t_json_input JSON formatted text input.
		* @returns A JSON_List object that can be used to initialize another JSON_List object.
		* @warning If your JSON object is held in a char[] buffer
		* it must first be converted to std::string.
		* @see get_value(), read_array(), read_object()
		*/
		static JSON_List parse(std::string t_json_input)
		{
			JSON_List temp_list;
			std::string::iterator it = t_json_input.begin();
			
			while (*it == ' ') { it++; } // skip leading white space
			if (*it != '{') { return temp_list; } // return an empty object if not reading a JSON object

			std::string key;
			std::string value;

			while (*it != '}')
			{
				if (*it == ' ')
				{
					it++;
				}

				// read key
				if (*it == '"')
				{
					it++;
					while (*it != '"')
					{
						key.push_back(*it);
						it++;
					}
				}
				else
				{
					while (*it != ':')
					{
						key.push_back(*it);
						it++;
					}
				}

				// read value
					// get value string
				while (*it != ',')
				{
					value.push_back(*it);
					it++;
				}

			}

			// loop through first level of structure

				// obtain block of data

				// create temp node

				// separate key from block and store in temp node

				// separate value from block

				// call get_value and pass value from block
					
					// 

		}

		/**
		* Serializes the contents of the curent JSON_List structure and returns an std::string.
		* @warning May need to be converted to char[] depending on your use case.
		*/
		std::string serialize()
		{

		}
		
	};
}

#endif // !jsontr.h

