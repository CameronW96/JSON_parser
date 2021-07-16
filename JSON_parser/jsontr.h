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
			using var_t = std::variant<int, bool, double, std::string, std::shared_ptr<Node>>;

		private:
			class JSON_Value
			{
			public:
				var_t m_value_individual = 0;
			};

			class JSON_KVP
			{
			public:
				bool m_is_array = false;
				std::string m_key;
				std::variant<JSON_Value, std::vector<JSON_Value>> m_value;

			public:
				static JSON_KVP make_kvp(std::string t_key, JSON_Value t_value)
				{
					JSON_KVP temp_kvp;
					temp_kvp.m_key = t_key;
					temp_kvp.m_value = t_value;
					return temp_kvp;
				}
			};

		private:
			// specifies whether this node is a JSON object
			// m_value should be initialized to a vector in this case
			bool m_is_object = false;
			std::string m_object_key;
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
			// declares that this node contains an object and initializes the node with an empty object  TODO: Update this documentation
			// object must be populated with object_push
			void init_object (const std::string &t_key, std::vector<JSON_KVP> t_value_object)
			{
				/*JSON_Value temp_value;

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

				m_kvp = init_kvp;*/

				m_is_object = true;
				m_object_key = t_key;
				m_kvp = t_value_object;
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
					return m_object_key;
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
		Node main_list;
	private:
		// return integer indicating the type held in the string argument
		// 0 = NAN, 1 = INTEGER, 2 = DOUBLE
		static int is_number(std::string t_string_input)
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

		/**
		* Determines type from string input.
		* @param t_string_input Input string to be evaluated.
		* @returns Integer indindicating the type held in the string argument:
		* 0 = NULL/SYNTAX ERROR, 1 = INTEGER, 2 = DOUBLE, 3 = BOOL, 4 = STRING, 5 = NODE, 6 = ARRAY
		*/
		static int check_type(std::string t_string_input)
		{
			std::string::iterator it = t_string_input.begin();
			if (*it == '"') // check for string
			{
				return 4;
			}
			else if (*it == '[')
			{
				return 6;
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

		static double convert_to_double(std::string t_integer_string)
		{
			// remove whitespace
			t_integer_string.erase(std::remove_if(t_integer_string.begin(), t_integer_string.end(), ::isspace), t_integer_string.end());

			return std::stod(t_integer_string);
		}

		static int convert_to_int(std::string t_integer_string)
		{
			// remove whitespace
			t_integer_string.erase(std::remove_if(t_integer_string.begin(), t_integer_string.end(), ::isspace), t_integer_string.end());

			return std::stoi(t_integer_string);
		}

		/**
		* Parses a JSON object from a std::string.
		* This function heap allocates a new node and returns a shared_ptr as a Node object is
		* self referential and can only hold a pointer to itself. Due to the dynamic nature of the
		* JSON structure, smart pointers are preferable over RAII in this instance. This function also 
		* calls get_value() which will result in a recursive loop if the value is another object.
		* The loop is resolved when it reaches that last node in the tree structure.
		* @param t_object_input The object value to be parsed.
		* @returns A shared pointer to a heap allocated Node object.
		* @see get_value()
		*/
		static std::vector<Node::JSON_KVP> read_object(std::string t_object_input)
		{
			std::string::iterator it = t_object_input.begin();
			std::vector<Node::JSON_KVP> temp_kvp_array;
			std::string key;
			std::string value;

			// skip any leading white space
			while (*it == ' ') { it++; }
			// return an empty kvp array if not reading a JSON object
			if (*it != '{') { return temp_kvp_array; }

			// loop through key value pairs
			while (*it != '}')
			{
				// skip white space in-between blocks
				if (*it == ' ')
				{
					it++;
					continue;
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
				while (*it != ',' & *it != '}')
				{
					value.push_back(*it);
					it++;
				}

				temp_kvp_array.push_back(Node::JSON_KVP::make_kvp(key, get_value(value)));

				if (*it == '}')
				{
					break;
				}
				else
				{
					it++;
				}
			}
			return temp_kvp_array;
		}

		static Node::JSON_KVP read_array(const std::string& t_input, std::string::iterator& t_iterator)
		{

		}

		/**
		* Removes leading and trailing white spaces from input.
		* @param t_string_input String to be formatted
		* @returns std::string
		*/
		static std::string format_value(std::string t_string_input)
		{
			int len_counter = 0;
			// count leading spaces if any
			while (t_string_input[len_counter] == ' ')
			{
				len_counter++;
			}
			// erase leading spaces if any found
			if (len_counter > 0)
			{
				t_string_input.erase(0, len_counter);
			}
			// count trailing spaces if any
			len_counter = 0;
			for (int i = t_string_input.size(); i > -1; i--)
			{
				if (t_string_input[i] == ' ')
				{
					len_counter++;
				}
				else
				{
					break;
				}
			}
			// erase trailing spaces if any found
			if (len_counter > 0)
			{
				t_string_input.erase((t_string_input.size() - len_counter), std::string::npos);
			}
			return t_string_input;
		}

		/**
		* Removes quotation marks from the first and last position in a string.
		* @param t_string_input String to be edited.
		* @attention Assumes no leading or trailing whitespace. Format with format_value()
		* first if needed.
		* @see format_value()
		*/
		static std::string remove_quotes(std::string t_string_input)
		{
			if (t_string_input[0] == '"')
			{
				t_string_input.erase(0, 1);
			}

			if (t_string_input[t_string_input.size()] == '"')
			{
				t_string_input.erase(t_string_input.size(), 1);
			}
			return t_string_input;
		}

		/**
		* Determines the type and extracts the value of string input.
		* This function also calls read_array() and read_object() which contain recursive calls to get_value().
		* This enables us to fully traverse the potential recursive tree structure contained in a Node.
		* @param t_string_input std::string input to extract the value from.
		* @returns A JSON_Value object.
		*/
		static Node::JSON_Value get_value(std::string t_value_input)
		{
			Node::JSON_Value temp_value;
			// format input
			std::string value = format_value(t_value_input);
			// get type of input
			int type = check_type(value);

			switch (type)
			{
			case 0: // invalid input
			{
				return temp_value;
				break;
			}
			case 1: // integer
			{
				temp_value.m_value_individual = convert_to_int(value);
				return temp_value;
				break;
			}
			case 2: // double
			{
				temp_value.m_value_individual = convert_to_double(value);
				return temp_value;
				break;
			}
			case 3: // bool
			{
				bool temp_bool;
				if (value == "true")
				{
					temp_bool = true;
				}
				else
				{
					temp_bool = false;
				}
				temp_value.m_value_individual = temp_bool;
				return temp_value;
				break;
			}
			case 4: // string
			{
				temp_value.m_value_individual = value;
				return temp_value;
				break;
			}
			case 5: // node
			{
				std::shared_ptr<Node> temp_node_object(new Node);
				temp_node_object->init_object("", read_object(value));
				temp_value.m_value_individual = temp_node_object;
				return temp_value;
				break;
			}
			case 6: // array
			{
				// must call read_array
				break;
			}
			}	
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
			temp_list.main_list.init_object("", read_object(t_json_input));
			return temp_list;			
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

