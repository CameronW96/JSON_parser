/**
* Author: Cameron White
* 
* This library provides a JSON data structure with CRUD capabilities. It supports multi-type arrays and objects while maintaining type safety.
* C++ 17 is required for variant and type traits.
* 
* More info: https://cameronw96.github.io/JSONator
* Github repo: https://github.com/CameronW96/JSONator
*
* LICENSE:
*
* JSONator
* Copyright (c) 2022 Cameron White
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
* 
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#ifndef jsonator
#define jsonator

#include <utility>
#include <string>
#include <vector>
#include <list>
#include <variant>
#include <type_traits>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace JSONator
{

	class JSON
	{
	private:
		class Node
		{
			friend class JSON;
		private:
			class JSON_KVP; // forward declaration
			/*
			* Class that contains a variant to serve as a typesafe union.
			* Contains two methods to support array notation and dot notation syntax when accessing a JSON.
			* Exists to allow multi-type array's and JSON objects.
			*/
			class JSON_Value
			{ 
				using var_t = std::variant<int, bool, double, std::string, std::shared_ptr<Node>, std::shared_ptr<std::vector<JSON_Value>>>;
			public:
				var_t m_value_individual = 0;
				bool m_error_state = 0;

			public:
				/*
				* Method that returns a reference to the JSON_Value object located at a specific index in an array.
				* A version of this exists in three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
				* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
				* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
				* primarily to provide a specific array index to the return, update, or delete methods.
				* @returns JSON_Value&
				*/
				JSON_Value& an(const int t_index)
				{
					std::shared_ptr<std::vector<JSON_Value>>* temp_value_array = std::get_if<std::shared_ptr<std::vector<JSON_Value>>>(&m_value_individual);
					if (temp_value_array == nullptr || t_index > (*temp_value_array)->size())
					{
						JSON_Value* error_value = heap_allocate_error_value();
						return *error_value;
					}
					else
					{
						std::vector<JSON_Value> temp_array = *(*temp_value_array);
						JSON_Value& temp_value = temp_array[t_index];
						return temp_value;
					}
				}

				/*
				* Method that returns a reference to a JSON_KVP object located inside of a JSON object. A version of this
				* method exists across three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
				* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
				* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
				* primarily to provide a specific object to the return, update, or delete methods.
				* @returns JSON_KVP&
				*/
				JSON_KVP& dn(const std::string t_key)
				{
					// get pointer to m_value_individual
					std::shared_ptr<Node>* temp_node = std::get_if<std::shared_ptr<Node>>(&m_value_individual);
					if (temp_node == nullptr)
					{
						JSON_KVP* error_kvp = heap_allocate_error_kvp();
						return *error_kvp;
					}

					JSON_KVP& temp_kvp = (*temp_node)->find_by_key(t_key);

					return temp_kvp;
				}

			};
			/*
			* Class that represents the Key-Value pair structure.
			* m_value can be either an individual object or a vector of objects. In the former case m_value holds either
			* a primitive type or a pointer to a heap allocated Node object. In the latter case m_value represents an array.
			*/
			class JSON_KVP
			{
			public:
				std::string m_key;
				std::variant<JSON_Value, std::vector<JSON_Value>> m_value;
				bool m_error_state = false;

			public:
				const static JSON_KVP make_kvp(const std::string& t_key, const JSON_Value& t_value) noexcept
				{
					JSON_KVP temp_kvp;
					temp_kvp.m_key = t_key;
					temp_kvp.m_value = t_value;
					return temp_kvp;
				}

				const static JSON_KVP make_kvp_array(const std::string &t_key, const std::vector<JSON_Value>& t_value) noexcept
				{
					JSON_KVP temp_kvp;
					temp_kvp.m_key = t_key;
					temp_kvp.m_value = t_value;
					return temp_kvp;
				}

				const static JSON_KVP make_error_kvp() noexcept // do not call this except to create a kvp used for 'key not found' errors
				{
					std::string key = "NULL";
					JSON_Value value;
					value.m_value_individual= -111;
					return make_kvp(key, value);
				}

				/*
				* Method that returns a reference to the JSON_Value object located at a specific index in an array.
				* A version of this exists in three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
				* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
				* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
				* primarily to provide a specific array index to the return, update, or delete methods.
				* @returns JSON_Value&
				*/
				JSON_Value& an(const int t_index)
				{
					std::vector<JSON_Value>* temp_value_array = std::get_if<std::vector<JSON_Value>>(&m_value);
					if (temp_value_array == nullptr || t_index > temp_value_array->size())
					{
						JSON_Value* error_value = heap_allocate_error_value();
						return *error_value;
					}
					else
					{
						Node::JSON_Value& temp_value = (*temp_value_array)[t_index];
						return temp_value;
					}
				}

				/*
				* Method that returns a reference to a JSON_KVP object located inside of a JSON object. A version of this
				* method exists across three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
				* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
				* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
				* primarily to provide a specific object to the return, update, or delete methods.
				* @returns JSON_KVP&
				*/
				JSON_KVP& dn(const std::string& t_key)
				{
					// get pointer to m_value
					JSON_Value* temp_value = std::get_if<JSON_Value>(&m_value);
					if (temp_value == nullptr)
					{
						JSON_KVP* error_kvp = heap_allocate_error_kvp();
						return *error_kvp;
					}
					// get pointer to m_value_individual - stored in m_value
					std::shared_ptr<Node>* temp_node = std::get_if<std::shared_ptr<Node>>(&temp_value->m_value_individual);
					if (temp_node == nullptr)
					{
						JSON_KVP* error_kvp = heap_allocate_error_kvp();
						return *error_kvp;
					}

					JSON_KVP& temp_kvp = (*temp_node)->find_by_key(t_key);
					return temp_kvp;
				}
			};

		private:
			std::string m_object_key;
			std::variant<std::monostate, JSON_KVP, std::vector<JSON_KVP>> m_kvp;

		private:
			JSON_KVP& find_by_key(const std::string& t_key)
			{
				std::vector<Node::JSON_KVP>* temp_kvp_array = std::get_if<std::vector<Node::JSON_KVP>>(&m_kvp);
				if (temp_kvp_array == nullptr)
				{
					JSON_KVP* error_kvp = heap_allocate_error_kvp();
					return *error_kvp;
				}
				else
				{
					for (int i = 0; i < temp_kvp_array->size(); i++)
					{
						Node::JSON_KVP& temp_kvp = (*temp_kvp_array)[i];
						if (format_value(temp_kvp.m_key) == t_key)
						{
							return temp_kvp;
						}
					}
					JSON_KVP* error_kvp = heap_allocate_error_kvp();
					return *error_kvp;
				}
			}

			std::pair<std::vector<Node::JSON_KVP>*, int> recursive_find_parent_vector_and_index(const std::string& t_key, const int t_function_level = 0)
			{
				std::vector<Node::JSON_KVP>* temp_kvp_array = std::get_if<std::vector<Node::JSON_KVP>>(&m_kvp);
				std::pair<std::vector<Node::JSON_KVP>*, int> return_value;
				if (temp_kvp_array == nullptr)
				{
					return std::make_pair(nullptr, -1);
				}
				else
				{
					for (int i = 0; i < temp_kvp_array->size(); i++)
					{
						Node::JSON_KVP& temp_kvp = (*temp_kvp_array)[i];
						Node::JSON_Value* temp_value = std::get_if<JSON_Value>(&temp_kvp.m_value);
						if (temp_value != nullptr && std::holds_alternative<std::shared_ptr<Node>>(temp_value->m_value_individual))
						{
							std::pair<std::vector<Node::JSON_KVP>*, int> temp_pair = std::make_pair(nullptr, 0);
							std::shared_ptr<Node>* temp_node = std::get_if<std::shared_ptr<Node>>(&temp_value->m_value_individual);
							if (std::holds_alternative<std::vector<Node::JSON_KVP>>((*temp_node)->m_kvp))
							{
								temp_pair = (*temp_node)->recursive_find_parent_vector_and_index(t_key, t_function_level + 1);
								if (t_function_level > 0)
								{
									return temp_pair;
								}
								else if (t_function_level == 0 && temp_pair.first != nullptr)
								{
									return temp_pair;
								}
							}
						}
						else if (format_value(temp_kvp.m_key) == t_key)
						{
							return std::make_pair(temp_kvp_array, i);
						}
						return std::make_pair(nullptr, -1); // Failed to find key
					}
				}
			}

		public:
			//*************************************** VALUE SETTERS ***************************************\\
		
			//Methods used to initialize the Node object with a given type.
			void init(const std::string &t_key, const JSON_Value &t_value) noexcept
			{
				JSON_KVP init_kvp;
				init_kvp.m_key = t_key;
				init_kvp.m_value = t_value;

				m_kvp = init_kvp;
			}
			void init_int (const std::string &t_key, const int t_value) noexcept
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_bool (const std::string &t_key, const bool t_value) noexcept
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_double (const std::string &t_key, const double t_value) noexcept
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_string (const std::string &t_key, const std::string &t_value) noexcept
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}
			void init_object (const std::string &t_key, const Node* &t_value) noexcept
			{
				JSON_Value init_value;
				init_value.m_value_individual = t_value;

				init(t_key, init_value);
			}

			//*************************************** STATE SETTERS ***************************************\\

			// declares that this node contains an array and initializes the value as empty array
			void init_array (const std::string& t_key)
			{
				JSON_KVP* temp_ptr = std::get_if<JSON_KVP>(&m_kvp);
				std::vector<JSON_Value> init_vector;
				temp_ptr->m_value = init_vector;
			}
			// declares that this node contains an object and initializes the node with an empty object
			void init_object (const std::string& t_key, const std::vector<JSON_KVP>& t_value_object) noexcept
			{
				m_object_key = t_key;
				m_kvp = t_value_object;
			}

		};

	private:
		Node main_list;

	private:

		//*************************************** STATIC HELPER FUNCTIONS ***************************************\\

		static std::string json_remove_space(std::string t_json_input)
		{
			std::string formatted_json;
			std::string::iterator it;
			it = t_json_input.begin();

			while (it != t_json_input.end())
			{
				if (*it == ' ')
				{
					it++;
				}
				else if (*it == '"')
				{
					formatted_json.push_back(*it); // Opening quote
					it++;

					while (*it != '"')
					{
						formatted_json.push_back(*it);
						it++;
					}

					formatted_json.push_back(*it); // Closing quote
					it++;
				}
				else if (*it == '\'')
				{
					formatted_json.push_back(*it); // Opening quote
					it++;

					while (*it != '\'')
					{
						formatted_json.push_back(*it);
						it++;
					}

					formatted_json.push_back(*it); // Closing quote
					it++;
				}
				else
				{
					formatted_json.push_back(*it);
					it++;
				}
			}

			return formatted_json;
		}
		
		/*
		* Heap allocates a JSON_Value object with the m_error_state flag set to true.
		* Object must be deleted by the return, update, or delete function when checking for error state.
		* @returns raw pointer
		*/
		static Node::JSON_Value* heap_allocate_error_value()
		{
			Node::JSON_Value* error_object(new Node::JSON_Value); 
			error_object->m_error_state = true;
			return error_object;
		}

		/*
		* Heap allocates a JSON_KVP object with the m_error_state flag set to true.
		* Object must be deleted by the return, update, or delete function when checking for error state.
		* @returns raw pointer
		*/
		static Node::JSON_KVP* heap_allocate_error_kvp()
		{
			Node::JSON_KVP* error_object(new Node::JSON_KVP);
			error_object->m_error_state = true;
			return error_object;
		}

		/* 
		* Checks if a string represents an integer, double, or neither.
		* @param t_string_input String to be evaluated
		* @returns Int indicating the type held in the string argument
		* 0 = NAN, 1 = INTEGER, 2 = DOUBLE
		*/
		const static int is_number(std::string t_string_input)
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
				else if (t_string_input[i] == '-')
				{
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
		* @returns Int indindicating the type held in the string argument:
		* 0 = NULL/SYNTAX ERROR, 1 = INTEGER, 2 = DOUBLE, 3 = BOOL, 4 = STRING, 5 = NODE, 6 = ARRAY
		*/
		const static int check_type(std::string t_string_input) noexcept
		{
			if (t_string_input[0] == '"') // check for string
			{
				return 4;
			}
			else if (t_string_input[0] == '[') // check for array
			{
				return 6;
			}
			else if (t_string_input[0] == '{') // check for object
			{
				return 5;
			}
			else if (t_string_input[0] == 'f' || t_string_input[0] == 't') // check for bool
			{
				return 3;
			}
			else
			{
				return is_number(t_string_input);
			}
		}

		/*
		* Formats and converts a string to a double
		* @param t_double_string String representing a double. May contain whitespace.
		* @returns A double
		*/
		const static double convert_to_double(std::string t_double_string)
		{
			// remove whitespace
			t_double_string.erase(std::remove_if(t_double_string.begin(), t_double_string.end(), ::isspace), t_double_string.end());

			return std::stod(t_double_string);
		}

		/*
		* Formats and converts a string to an int
		* @param t_integer_string String representing an int. May contain whitespace.
		* @returns An int
		*/
		const static int convert_to_int(std::string t_integer_string)
		{
			// remove whitespace
			t_integer_string.erase(std::remove_if(t_integer_string.begin(), t_integer_string.end(), ::isspace), t_integer_string.end());

			return std::stoi(t_integer_string);
		}

		/**
		* Removes leading and trailing white spaces from input.
		* @param t_string_input String to be formatted
		* @returns std::string
		*/
		const static std::string format_value(std::string t_string_input)
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
			for (int i = t_string_input.size() - 1; i > -1; i--)
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
				std::string::iterator it = t_string_input.end() - len_counter;
				t_string_input.erase(it, t_string_input.end());
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
		const static std::string remove_quotes(std::string t_string_input)
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

		/*
		* Overloaded method that converts an array represented in a JSON_KVP and returns a
		* "flat packed" JSON array.
		* @returns std::string
		*/
		const static std::string convert_to_text(const std::vector<Node::JSON_Value>& t_input_array)
		{
			std::stringstream output;
			output << "[";

			for (int i = 0; i < t_input_array.size(); i++)
			{
				Node::JSON_Value current_index = t_input_array[i];
				std::string converted_value = "";
				//determine type and convert to string
				if (std::holds_alternative<int>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<int>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<double>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<double>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<bool>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<bool>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<std::string>(current_index.m_value_individual))
				{
					converted_value = std::get<std::string>(current_index.m_value_individual);
				}
				// determine whether to end the array
				if (i < (t_input_array.size() - 1))
				{
					output << converted_value << ", ";
				}
				else
				{
					output << converted_value << ']';
				}
			}
			return output.str();
		}
		static std::string convert_to_text(const std::shared_ptr<std::vector<Node::JSON_Value>>& t_input_array)
		{
			std::stringstream output;
			output << "[ ";
			std::vector<Node::JSON_Value> temp_value_vector = *t_input_array;
			for (int i = 0; i < temp_value_vector.size(); i++)
			{
				Node::JSON_Value current_index = temp_value_vector[i];
				std::string converted_value = "";
				//determine type and convert to string
				if (std::holds_alternative<int>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<int>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<double>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<double>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<bool>(current_index.m_value_individual))
				{
					converted_value = std::to_string(std::get<bool>(current_index.m_value_individual));
				}
				else if (std::holds_alternative<std::string>(current_index.m_value_individual))
				{
					converted_value = std::get<std::string>(current_index.m_value_individual);
				}
				// determine whether to end the array
				if (i < (temp_value_vector.size() - 1))
				{
					output << converted_value << ", ";
				}
				else
				{
					output << converted_value << ']';
				}
			}
			return output.str();
		}

		//************************************************ CREATE ***********************************************\\

		/**
		* Parses a JSON object from a std::string.
		* This is the entry point to a recursive loop that will traverse a JSON object of unknown size and structure
		* while populating a vector of JSON_KVP objects that mirrors the original structure. Calls get_value()
		* which will then return a primitive, call read_array(), or make a recursive call to read_object().
		* Note that read_array also calls get_value() if the array contains an object.
		* @param t_object_input The object value to be parsed.
		* @returns A vector of JSON_KVP objects which is the basis for the JSON object.
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
			// if (*it != '{') { return temp_kvp_array; }
			// move iterator off of the opening brace
			//it++;
			// loop through key value pairs
			while (true)
			{
				// if object move iterator off opening brace (should only occur for the first opening brace in the object)
				if (*it == '{')
				{
					it++;
				}
				// skip white space in-between blocks
				while (*it == ' ' || *it == ',')
				{
					it++;
				}
				// check for end of object - this has to happen after the whitespace has been skipped
				// or the iterator goes off the end of the string.
				if (*it == '}') 
				{
					break;
				}
				// check for array and skip key parsing if found
				if (*it != '[')
				{
					// read key
					if (*it == '"')
					{
						it++;
						while (*it != '"')
						{
							key.push_back(*it);
							it++;
						}
						while (*it != ':') // skip to value
						{
							it++;
						}
						it++;
					}
					else
					{
						while (*it != ':')
						{
							key.push_back(*it);
							it++;
						}
						it++;
					}
					// format key
					key = format_value(key);
					// skip white space in-between blocks
					while (*it == ' ')
					{
						it++;
					}
				}
				// read value
				if (*it == '"') // string
				{
					do
					{
						value.push_back(*it);
						it++;
					} while (*it != '"');
					// grab closing quote
					value.push_back(*it);
					it++;
				}
				else if (*it == '\'') // also string
				{
					do
					{
						value.push_back(*it);
						it++;
					} while (*it != '\'');
					// grab closing quote
					value.push_back(*it);
					it++;
				}
				else if (*it == '[') // array
				{
					/*while (*it != ']')
					{
						value.push_back(*it);
						it++;
					}*/

					// Capture opening brace
					value.push_back(*it);
					it++;

					int array_counter = 1;
					while (array_counter != 0)
					{
						if (*it == '[') // must track how many nested arrays we are reading 
						{
							array_counter++;
						}
						else if (*it == ']')
						{
							array_counter--;
						}
						value.push_back(*it);
						it++;
					}
					//value.push_back(*it);
				}
				else if (*it == '{') // object
				{
					value.push_back(*it);
					it++; // move iterator off of opening brace so it is not double counted

					int object_counter = 1;
					while (object_counter != 0)
					{
						if (*it == '{') // must track how many nested objects we are reading 
						{
							object_counter++;
						}
						else if (*it == '}')
						{
							object_counter--;
						}
						value.push_back(*it);
						it++;
					}
					value.push_back(*it); // push closing brace - read object will overflow without it
				}
				else // primitive
				{
					while (*it != ',' && *it != '}')
					{
						value.push_back(*it);
						it++;
					}
				}
				
				std::vector<Node::JSON_Value> value_vector = get_value(value);
				if (value_vector.size() > 1) // is an array
				{
					temp_kvp_array.push_back(Node::JSON_KVP::make_kvp_array(format_value(key), value_vector));
				}
				else // not an array
				{
					temp_kvp_array.push_back(Node::JSON_KVP::make_kvp(key, get_value(value)[0]));
				}

				if (it == t_object_input.end() || *it == '}')
				{
					break;
				}
				else
				{
					key = "";
					value = "";
					it++;
				}
			}
			return temp_kvp_array;
		}

		/*
		* Parses an array represesnted in a string.
		* Like read_object() this is part of a recursive loop containing read_object(), read_array(), and get_value.
		* @param t_array_input String to be parsed.
		* @returns A vector of JSON_Value objects which is how this library represents an array.
		*/
		static std::vector<Node::JSON_Value> read_array(std::string t_array_input)
		{
			std::string::iterator it = t_array_input.begin();
			std::vector<Node::JSON_Value> temp_value_array;
			std::string value;

			// skip any leading white space
			while (*it == ' ') { it++; }
			// return an empty value array if not reading an array
			if (*it != '[') { return temp_value_array; }
			// move iterator off opening bracket
			it++;
			// loop through values
			while (*it != ']')
			{
				// read value
				if (*it == '"') // string
				{
					do
					{
						value.push_back(*it);
						it++;
					} while (*it != '"');
					// grab closing quote
					value.push_back(*it);
					it++;
				}
				else if (*it == '\'') // also string
				{
					do
					{
						value.push_back(*it);
						it++;
					} while (*it != '\'');
					// grab closing quote
					value.push_back(*it);
					it++;
				}
				else if (*it == '[') // array
				{
					// Capture opening brace
					value.push_back(*it);
					it++;

					int array_counter = 1;
					while (array_counter != 0)
					{
						if (*it == '[') // must track how many nested arrays we are reading 
						{
							array_counter++;
						}
						else if (*it == ']')
						{
							array_counter--;
						}
						value.push_back(*it);
						it++;
					}
				}
				// if opening brace is found read contents into buffer until closing brace
				else if (*it == '{') // object
				{
					value.push_back(*it);
					it++;

					int object_counter = 1;
					while (object_counter != 0)
					{
						if (*it == '{') // must track how many nested objects we are reading 
						{
							object_counter++;
						}
						else if (*it == '}')
						{
							object_counter--;
						}
						value.push_back(*it);
						it++;
					}

					// append closing brace and move iterator forward
					/*value.push_back('}');
					it++;*/
				}
				else
				{
					while (*it != ',' && *it != ']')
					{
						value.push_back(*it);
						it++;
					}
				}
			
		
				std::vector<Node::JSON_Value> value_vector = get_value(value);
				if (value_vector.size() > 1) // is an array
				{
					std::shared_ptr<std::vector<Node::JSON_Value>> nested_array(new std::vector<Node::JSON_Value>);
					*nested_array = value_vector;
					Node::JSON_Value temp_value;
					temp_value.m_value_individual = nested_array;
					temp_value_array.push_back(temp_value);
				}
				else // not an array
				{
					Node::JSON_Value temp_value = value_vector[0];
					temp_value_array.push_back(temp_value);
				}

				if (*it == ']')
				{
					break;
				}
				else
				{
					value = "";
					it++;
				}
			}
			return temp_value_array;
		}

		/**
		* Determines the type and extracts the value of string input.
		* This function also calls read_array() and read_object() which contain recursive calls to get_value().
		* This enables us to fully traverse the potential recursive tree structure contained in a Node.
		* @param t_string_input std::string input to extract the value from.
		* @returns A JSON_Value object.
		*/
		static std::vector<Node::JSON_Value> get_value(std::string t_value_input)
		{
			std::vector<Node::JSON_Value> temp_vector;
			// format input
			std::string value = format_value(t_value_input);
			// get type of input
			int type = check_type(value);

			switch (type)
			{
			case 0: // invalid input
			{
				return temp_vector;
				break;
			}
			case 1: // integer
			{
				Node::JSON_Value temp_value;
				temp_value.m_value_individual = convert_to_int(value);
				temp_vector.push_back(temp_value);
				break;
			}
			case 2: // double
			{
				Node::JSON_Value temp_value;
				temp_value.m_value_individual = convert_to_double(value);
				temp_vector.push_back(temp_value);
				break;
			}
			case 3: // bool
			{
				Node::JSON_Value temp_value;
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
				temp_vector.push_back(temp_value);
				break;
			}
			case 4: // string
			{
				Node::JSON_Value temp_value;
				remove_quotes(value);
				temp_value.m_value_individual = value;
				temp_vector.push_back(temp_value);
				break;
			}
			case 5: // node
			{
				Node::JSON_Value temp_value;
				std::shared_ptr<Node> temp_node_object(new Node);
				temp_node_object->init_object("", read_object(value));
				temp_value.m_value_individual = temp_node_object;
				temp_vector.push_back(temp_value);
				break;
			}
			case 6: // array
			{
				temp_vector = read_array(value);
				break;
			}
			}	
			return temp_vector;
		}

	public:
		/**
		* Parses string input using recursion to traverse a text JSON object and populate the JSON structure.
		* @param t_json_input JSON formatted text input.
		* @returns A JSON object
		* @see get_value(), read_array(), read_object()
		*/
		static JSON parse(std::string t_json_input)
		{
			JSON temp_list;
			temp_list.main_list.init_object("", read_object(json_remove_space(t_json_input)));
			return temp_list;			
		}

		/*
		* Reads a file into a string and removes all newlines and carriage returns.
		* @param t_file_path File path including file name and extension
		* @returns std::string
		*/
		static std::string read_file(std::string t_file_path)
		{
			std::ifstream json_file;
			std::string output_string;
			std::stringstream buf;

			json_file.open(t_file_path);
			if (!json_file.is_open())
			{
				return output_string;
			}

			buf << json_file.rdbuf();
			output_string = buf.str();
			output_string.erase(std::remove(output_string.begin(), output_string.end(), '\n'), output_string.end()); // remove newlines
			output_string.erase(std::remove(output_string.begin(), output_string.end(), '\r'), output_string.end());
			return output_string;
		}

		bool is_empty()
		{
			if (std::holds_alternative<std::monostate>(main_list.m_kvp))
			{
				return true;
			}
			else
			{
				std::vector<Node::JSON_KVP>* main_vector_ptr = std::get_if<std::vector<Node::JSON_KVP>>(&main_list.m_kvp);
				if (main_vector_ptr == nullptr)
				{
					return true;
				}
				else if (main_vector_ptr->size() > 0)
				{
					return false;
				}
			}
		}

		//************************************************ READ ************************************************\\

		/*
		* Returns an int contained in an array or -1 on error.
		* @param t_input Array to be evaluated (usually obtained with an())
		* @returns int
		*/
		static int r_int(const Node::JSON_Value& t_input)
		{			
			if (t_input.m_error_state == true) 
			{
				delete& t_input;
				return -1;
			}
			int output = -1;
			if (std::holds_alternative<int>(t_input.m_value_individual))
			{
				output = std::get<int>(t_input.m_value_individual);
			}
			return output;
		}
		/*
		* Returns an int contained in an object or -1 on error.
		* @param t_input Object to be evaluated (usually obtained with dn())
		* @returns int
		*/
		static int r_int(const Node::JSON_KVP& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return -1;
			}
			int output = -1;
			const Node::JSON_Value* temp_value = std::get_if<Node::JSON_Value>(&t_input.m_value);
			if (temp_value == nullptr)
			{
				return -1;
			}
			else
			{
				if (std::holds_alternative<int>(temp_value->m_value_individual))
				{
					output = std::get<int>(temp_value->m_value_individual);
				}
				else
				{
					return -1;
				}
			}
			return output;
		}
		/*
		* Returns a double contained in an array or -1 on error.
		* @param t_input Array to be evaluated (usually obtained with an())
		* @returns double
		*/
		static double r_double(const Node::JSON_Value& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return -1;
			}
			double output = -1;
			if (std::holds_alternative<double>(t_input.m_value_individual))
			{
				output = std::get<double>(t_input.m_value_individual);
			}
			return output;
		}
		/*
		* Returns a double contained in an object or -1 on error.
		* @param t_input Obejct to be evaluated (usually obtained with dn())
		* @returns double
		*/
		static double r_double(const Node::JSON_KVP& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return -1;
			}
			double output = -1;
			const Node::JSON_Value* temp_value = std::get_if<Node::JSON_Value>(&t_input.m_value);
			if (temp_value == nullptr)
			{
				return -1;
			}
			else
			{
				if (std::holds_alternative<double>(temp_value->m_value_individual))
				{
					output = std::get<double>(temp_value->m_value_individual);
				}
				else
				{
					return -1;
				}
			}
			return output;
		}
		/*
		* Returns a bool contained in an array or false on error.
		* @param t_input Array to be evaluated (usually obtained with an())
		* @returns bool
		*/
		static bool r_bool(const Node::JSON_Value& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return false;
			}
			bool output = false;
			if (std::holds_alternative<bool>(t_input.m_value_individual))
			{
				output = std::get<bool>(t_input.m_value_individual);
			}
			return output;
		}
		/*
		* Returns a bool contained in an object or false on error.
		* @param t_input Object to be evaluated (usually obtained with dn())
		* @returns bool
		*/
		static bool r_bool(const Node::JSON_KVP& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return false;
			}
			bool output = false;
			const Node::JSON_Value* temp_value = std::get_if<Node::JSON_Value>(&t_input.m_value);
			if (temp_value == nullptr)
			{
				return false;
			}
			else
			{
				if (std::holds_alternative<bool>(temp_value->m_value_individual))
				{
					output = std::get<bool>(temp_value->m_value_individual);
				}
				else
				{
					return false;
				}
			}
			return output;
		}
		/*
		* Returns a string contained in an array or an empty string on error.
		* @param t_input Array to be evaluated (usually obtained with an())
		* @returns std::string
		*/
		static std::string r_string(const Node::JSON_Value& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return "";
			}
			std::string output = "";
			if (std::holds_alternative<std::string>(t_input.m_value_individual))
			{
				output = std::get<std::string>(t_input.m_value_individual);
			}
			return output;
		}
		/*
		* Returns a string contained in an object or an empty string on error.
		* @param t_input Object to be evaluated (usually obtained with dn())
		* @returns std::string
		*/
		static std::string r_string(const Node::JSON_KVP& t_input)
		{
			if (t_input.m_error_state == true)
			{
				delete& t_input;
				return "";
			}
			std::string output = "";
			const Node::JSON_Value* temp_value = std::get_if<Node::JSON_Value>(&t_input.m_value);
			if (temp_value != nullptr)			
			{
				if (std::holds_alternative<std::string>(temp_value->m_value_individual))
				{
					output = std::get<std::string>(temp_value->m_value_individual);
				}
			}
			return output;
		}

		// ****NODE ACCESS****

		/*
		* Method that returns a reference to the JSON_Value object located at a specific index in an array.
		* A version of this exists in three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
		* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
		* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
		* primarily to provide a specific array index to the return, update, or delete methods.
		* @returns JSON_Value&
		*/
		Node::JSON_Value& an(int t_index)
		{
			std::vector<Node::JSON_KVP>* main_list_kvp = std::get_if<std::vector<Node::JSON_KVP>>(&main_list.m_kvp);
			if (main_list_kvp == nullptr)
			{
				Node::JSON_Value* error_value = heap_allocate_error_value();
				return *error_value;
			}
			
			std::vector<Node::JSON_Value> *temp_kvp_array = std::get_if<std::vector<Node::JSON_Value>>(&(*main_list_kvp)[0].m_value);


			if (t_index > temp_kvp_array->size())
			{
				Node::JSON_Value* error_value = heap_allocate_error_value();
				return *error_value;
			}
			else
			{
				Node::JSON_Value& temp_value = (*temp_kvp_array)[t_index];
				return temp_value;
			}		

		}

		/*
		* Method that returns a reference to a JSON_KVP object located inside of a JSON object. A version of this
		* method exists across three classes: JSON, JSON_KVP, and JSON_Value. All methods have the
		* same name to facilitate ease of use by the end user. Calls to an() and dn() can be chained in a similar
		* manner to using array notation and dot notation when accessing JSON objects in Javascript. This is used
		* primarily to provide a specific object to the return, update, or delete methods.
		* @returns JSON_KVP&
		*/
		Node::JSON_KVP& dn(std::string t_key)
		{
			return main_list.find_by_key(t_key);
		}

		//************************************************ UPDATE ***********************************************\\

		// Updates an objects key
		void static update_key(const std::string t_new_key, Node::JSON_KVP& t_object) noexcept
		{
			if (t_object.m_error_state == false)
			{
				t_object.m_key = t_new_key;
			}
			else
			{
				delete &t_object;
			}
		}

		// Overloaded method to update an objects value to a new value of any type
		void static update_value(const int t_new_value, Node::JSON_KVP& t_object) noexcept
		{
			if (t_object.m_error_state == false)
			{
				Node::JSON_Value* value_ptr = std::get_if<Node::JSON_Value>(&t_object.m_value);
				value_ptr->m_value_individual = t_new_value;
			}
			else
			{
				delete& t_object;
			}
		}
		void static update_value(const double t_new_value, Node::JSON_KVP& t_object) noexcept
		{
			if (t_object.m_error_state == false)
			{
				Node::JSON_Value* value_ptr = std::get_if<Node::JSON_Value>(&t_object.m_value);
				value_ptr->m_value_individual = t_new_value;
			}
			else
			{
				delete& t_object;
			}
		}
		void static update_value(const bool t_new_value, Node::JSON_KVP& t_object) noexcept
		{
			if (t_object.m_error_state == false)
			{
				Node::JSON_Value* value_ptr = std::get_if<Node::JSON_Value>(&t_object.m_value);
				value_ptr->m_value_individual = t_new_value;
			}
			else
			{
				delete& t_object;
			}
		}
		void static update_value(const std::string t_new_value, Node::JSON_KVP& t_object) noexcept
		{
			if (t_object.m_error_state == false)
			{
				Node::JSON_Value* value_ptr = std::get_if<Node::JSON_Value>(&t_object.m_value);
				value_ptr->m_value_individual = t_new_value;
			}
			else
			{
				delete& t_object;
			}
		}
		void static update_value(const int t_new_value, Node::JSON_Value& t_value) noexcept
		{
			if (t_value.m_error_state == false)
			{
				t_value.m_value_individual = t_new_value;
			}
			else
			{
				delete& t_value;
			}
		}
		void static update_value(const double t_new_value, Node::JSON_Value& t_value) noexcept
		{
			if (t_value.m_error_state == false)
			{
				t_value.m_value_individual = t_new_value;
			}
			else
			{
				delete& t_value;
			}
		}
		void static update_value(const bool t_new_value, Node::JSON_Value& t_value) noexcept
		{
			if (t_value.m_error_state == false)
			{
				t_value.m_value_individual = t_new_value;
			}
			else
			{
				delete& t_value;
			}
		}
		void static update_value(const std::string t_new_value, Node::JSON_Value& t_value) noexcept
		{
			if (t_value.m_error_state == false)
			{
				t_value.m_value_individual = t_new_value;
			}
			else
			{
				delete& t_value;
			}
		}

		//************************************************ DELETE ***********************************************\\

		// Traverses the entire JSON structure and deletes the first instance of the key that it finds.
		void remove_first_found(const std::string t_key)
		{
			std::pair<std::vector<Node::JSON_KVP>*, int> vector_reference_and_index = main_list.recursive_find_parent_vector_and_index(t_key);
			if (vector_reference_and_index.first != nullptr)
			{
				std::vector<Node::JSON_KVP>* temp_vector_ptr = vector_reference_and_index.first;
				int vector_index = vector_reference_and_index.second;
				temp_vector_ptr->erase(temp_vector_ptr->begin() + vector_index);
			}
		}

		// Overloaded method that deletes an index from an array or nested array
		void static remove_from_array(Node::JSON_Value& t_array, const int t_index)
		{
			if (t_array.m_error_state == false)
			{
				std::shared_ptr<std::vector<Node::JSON_Value>>* temp_array_ptr = std::get_if<std::shared_ptr<std::vector<Node::JSON_Value>>>(&t_array.m_value_individual);
				if (temp_array_ptr != nullptr)
				{
					(*temp_array_ptr)->erase((*temp_array_ptr)->begin() + t_index);
				}
			}
			else
			{
				delete& t_array;
			}
		}
		void static remove_from_array(Node::JSON_KVP& t_object, const int t_index)

		{
			if (t_object.m_error_state == false)
			{
				std::vector<Node::JSON_Value>* temp_value_ptr = std::get_if<std::vector<Node::JSON_Value>>(&t_object.m_value);
				if (temp_value_ptr != nullptr)
				{
					temp_value_ptr->erase(temp_value_ptr->begin() + t_index);
				}
			}
			else
			{
				delete& t_object;
			}
		}

		// Deletes a key value pair from an object
		void static remove_from_object(Node::JSON_KVP& t_object, const std::string t_key)
		{
			if (t_object.m_error_state == false)
			{
				Node::JSON_Value* temp_value_ptr = std::get_if<Node::JSON_Value>(&t_object.m_value);
				if (temp_value_ptr != nullptr)
				{
					std::shared_ptr<Node>* temp_node_ptr = std::get_if<std::shared_ptr<Node>>(&temp_value_ptr->m_value_individual);
					if (temp_node_ptr != nullptr)
					{
						std::vector<Node::JSON_KVP>* temp_object_vector_ptr = std::get_if<std::vector<Node::JSON_KVP>>(&(*temp_node_ptr)->m_kvp);
						if (temp_object_vector_ptr != nullptr)
						{
							for (int i = 0; i < temp_object_vector_ptr->size(); i++)
							{
								Node::JSON_KVP current_object = (*temp_object_vector_ptr)[i];
								if (format_value(current_object.m_key) == t_key)
								{
									temp_object_vector_ptr->erase(temp_object_vector_ptr->begin() + i);
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				delete& t_object;
			}
		}

		//*********************************************** SERIALIZE *********************************************\\

		/**
		* Serializes the contents of the curent JSON structure.
		* Will be a "flat packed" JSON object with no newline formatting.
		* @param t_node_object A nested JSON object within the JSON structure.
		* @returns std::string
		*/
		static std::string serialize(const Node& t_node_object)
		{
			std::stringstream output;
			output << "{";
			const std::vector<Node::JSON_KVP>* main_vector_ptr = std::get_if<std::vector<Node::JSON_KVP>>(&t_node_object.m_kvp);
			if (main_vector_ptr == nullptr)
			{
				return "NULL";
			}
			// read through KVP vector and insert keys and values into stream
			// makes recursive calls to serialize when encountering another object
			for (int i = 0; i < main_vector_ptr->size(); i++)
			{
				output << (*main_vector_ptr)[i].m_key;
				output << " : ";				

				if (std::holds_alternative<std::vector<Node::JSON_Value>>((*main_vector_ptr)[i].m_value))
				{
					const std::vector<Node::JSON_Value>* temp_value_ptr = std::get_if<std::vector<Node::JSON_Value>>(&(*main_vector_ptr)[i].m_value);
					output << convert_to_text(*temp_value_ptr);
				}
				else if (std::holds_alternative<Node::JSON_Value>((*main_vector_ptr)[i].m_value))
				{
					const Node::JSON_Value* temp_value_ptr = std::get_if<Node::JSON_Value>(&(*main_vector_ptr)[i].m_value);
					if (std::holds_alternative<std::shared_ptr<Node>>(temp_value_ptr->m_value_individual)) // is an object
					{
						const std::shared_ptr<Node>* temp_value_individual_ptr = std::get_if<std::shared_ptr<Node>>(&temp_value_ptr->m_value_individual);
						output << serialize(**temp_value_individual_ptr);
					}
					else // is a primitive value
					{
						std::string converted_value = "";
						// determine type and convert to string
						if (std::holds_alternative<int>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<int>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<double>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<double>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<bool>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<bool>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<std::string>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::get<std::string>(temp_value_ptr->m_value_individual);
						}
						output << converted_value;
					}
				}
				else // uninitialized variant
				{
					return "NULL";
				}

				if (i < (main_vector_ptr->size() - 1))
				{
					output << ", ";
				}
				else // end of object
				{
					output << '}';
				}
			}
			return output.str();
		}

		// there must be a cleaner way to do this - function needs to be able to handle a Node object as well as a JSON_list
		// object in order to make recursive calls
		static std::string serialize(const JSON& t_main_list)
		{
			std::stringstream output;
			output << "{";
			const std::vector<Node::JSON_KVP>* main_vector_ptr = std::get_if<std::vector<Node::JSON_KVP>>(&t_main_list.main_list.m_kvp);
			if (main_vector_ptr == nullptr)
			{
				return "NULL";
			}
			// read through KVP vector and insert keys and values into stream
			// makes recursive calls to serialize when encountering another object
			for (int i = 0; i < main_vector_ptr->size(); i++)
			{
				output << (*main_vector_ptr)[i].m_key;
				output << " : ";

				if (std::holds_alternative<std::vector<Node::JSON_Value>>((*main_vector_ptr)[i].m_value))
				{
					const std::vector<Node::JSON_Value>* temp_value_ptr = std::get_if<std::vector<Node::JSON_Value>>(&(*main_vector_ptr)[i].m_value);
					output << convert_to_text(*temp_value_ptr);
				}
				else if (std::holds_alternative<Node::JSON_Value>((*main_vector_ptr)[i].m_value))
				{
					const Node::JSON_Value* temp_value_ptr = std::get_if<Node::JSON_Value>(&(*main_vector_ptr)[i].m_value);
					if (std::holds_alternative<std::shared_ptr<Node>>(temp_value_ptr->m_value_individual)) // is an object
					{
						const std::shared_ptr<Node>* temp_value_individual_ptr = std::get_if<std::shared_ptr<Node>>(&temp_value_ptr->m_value_individual);
						output << serialize(**temp_value_individual_ptr);
					}
					else // is a primitive value
					{
						std::string converted_value = "";
						//determine type and convert to string
						if (std::holds_alternative<int>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<int>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<double>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<double>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<bool>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::to_string(std::get<bool>(temp_value_ptr->m_value_individual));
						}
						else if (std::holds_alternative<std::string>(temp_value_ptr->m_value_individual))
						{
							converted_value = std::get<std::string>(temp_value_ptr->m_value_individual);
						}
						output << converted_value;
					}
				}
				else // uninitialized variant
				{
					return "NULL";
				}
				if (i < (main_vector_ptr->size() - 1))
				{
					output << ", ";
				}
				else // end of object
				{
					output << '}';
				}
			}
			return output.str();
		}
	};
}

#endif // !jsonator.h

