#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <variant>
#include <sstream>
#include <cmath>
#include <numeric>
#include "Functions.hpp"

// map<string, atomic_type> types_arr = {
//     { "char",   { ATOMIC, atomic{"char",   1, 1} }},
//     { "short",  { ATOMIC, atomic{"short",  2, 2} }},
//     { "int",    { ATOMIC, atomic{"int",    4, 4} }},
//     { "long",   { ATOMIC, atomic{"long",   8, 8} }},
//     { "float",  { ATOMIC, atomic{"float",  4, 4} }},
//     { "double", { ATOMIC, atomic{"double", 8, 8} }},
//     { "bool",   { ATOMIC, atomic{"bool",   1, 2} }},
//     {"MyStruct1", {STRUCT, atomic_struct{"MyStruct1", {"int", "char", "char", "int", "double", "bool"}, 19, 4}}},
//     {"MyStruct2", {STRUCT, atomic_struct{"MyStruct2", {"short", "float", "char", "long"}, 15, 2}}},
//     {"Union1", {UNION, atomic_union{"MyUnion1", {"double", "bool", "int"}, 8, 8}}},
//     {"Union2", {UNION, atomic_union{"MyUnion2", {"Union1", "MyStruct2"}, 15, 8}}}
// };


int main() {
    string line;
    int word_size = 4; // Tamaño de palabra en bytes (32 bits)
    map<string, int> cmd_available = {
        {"ATOMICO", 1},
        {"STRUCT", 2},
        {"UNION", 3}, 
        {"DESCRIBIR", 4}, 
        {"SALIR", 5},
        {"IMPRIMIR", 6}
    };
    vector<string> tokens;
    string cmd;
    cout << "Enter command:" << endl;
    while (true) {
        cout << "> ";
        getline(cin, line);
        tokens = split(line);

        if (tokens.size() == 0) {
            cout << "Error: Empty command. Try again." << endl;
            continue;
        }

        cmd = tokens[0];

        switch (cmd_available[cmd]){
            case 1:{
                try {
                    if (tokens.size() != 4) {
                        throw runtime_error("Error: Wrong number of arguments for ATOMIC type.\nUsage: ATOMIC <nombre> <representacion> <alineacion>.");
                    }

                    string field1 = tokens[1];

                    if (!is_integer(tokens[2]) || !is_integer(tokens[3])){
                        throw runtime_error("Error: Non-integer type for size or alignment. Try again.");
                    }
                    
                    int field2 = stoi(tokens[2]);

                    if (field2 <= 0) {
                        throw runtime_error("Error: Size must be a positive integer. Try again.");
                    }

                    int field3 = stoi(tokens[3]);

                    if (field3 <= 0) {
                        throw runtime_error("Error: Alignment must be a positive integer. Try again.");
                    }
                    
                    push_atomic(types_arr, field1, field2, field3);
                    
                    cout << "ATOMIC type " << field1 << " created successfully!"<< endl;
                    
                } catch (exception& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 2:{
                try {
                    if (tokens.size() < 3) {
                        throw runtime_error("Error: Wrong number of arguments for STRUCT type.\nUsage: STRUCT <nombre> [<tipo>].");
                    }
                    string struct_name = tokens[1];
                    vector<string> field_types;
                    

                    // Iteramos desde el tercer token
                    for (size_t i = 2; i < tokens.size(); i++) {

                        // Verificamos que exista en el mapa global types_arr
                        if (types_arr.find(tokens[i]) == types_arr.end()) {
                            throw runtime_error("Error: Type '" + tokens[i] + "' not found in type table.");
                        }

                        // Si existe, lo agregamos a la lista
                        field_types.push_back(tokens[i]);
                    }

                    // Si todos existen, creamos el struct
                    push_struct(types_arr, struct_name, field_types);

                    cout << "STRUCT type " << struct_name << " created successfully!"<< endl;

                } catch (exception& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 3: {
                try {
                    if (tokens.size() < 3) {
                        throw runtime_error("Error: Wrong number of arguments for UNION type.\nUsage: UNION <nombre> [<tipo>].");
                    }
                    string struct_name = tokens[1];
                    vector<string> field_types;

                    // Iteramos desde el tercer token
                    for (size_t i = 2; i < tokens.size(); i++) {

                        // Verificamos que exista en el mapa global types_arr
                        if (types_arr.find(tokens[i]) == types_arr.end()) {
                            throw runtime_error("Error: Type '" + tokens[i] + "' not found in type table.");
                        }

                        // Si existe, lo agregamos a la lista
                        field_types.push_back(tokens[i]);
                    }

                    // Si todos existen, creamos el struct
                    push_union(types_arr, struct_name, field_types);

                    cout << "UNION type " << struct_name << " created successfully!"<< endl;

                } catch (exception& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 4: {
                try
                {
                    if (tokens.size() != 2) {
                        throw runtime_error("Error: Wrong number of arguments for DESCRIBIR command.\nUsage: DESCRIBIR <nombre>.");
                    }

                    string type_name = tokens[1];

                    if (types_arr.find(type_name) == types_arr.end()) {
                        throw runtime_error("Error: Type '" + type_name + "' not found in type table.");
                    }

                    atomic_type type = types_arr[type_name];

                    switch (type.kind) {
                        case ATOMIC: {
                            const aatomic& a = get<aatomic>(type.at);
                            print_atomic(a, word_size);
                            break;
                        }
                        case STRUCT: {
                            const atomic_struct& s = get<atomic_struct>(type.at);
                            cout << "Strategy without packing: " << endl;
                            print_struct_wt_packing(s, word_size);
                            cout << "Strategy with packing: " << endl;
                            print_struct_w_packing(s, word_size);
                            cout << "Strategy with heuristics respecting alignment: " << endl;
                            print_struct_heuristics(s, word_size);
                            break;
                        }

                        case UNION: {
                            const atomic_union& u = get<atomic_union>(type.at);
                            print_union(u, word_size);
                            break;
                        }
                    }
                }
                catch(const exception& e){
                    cout << e.what() << '\n';
                }
                break;
            }
            case 5:
                cout << "Saliendo del programa." << endl;
                return 0;
            case 6:
                print_types(types_arr);
                break;
            default:
                cout << "Error: unknown command." << endl;
                cout << "Available commands: \nATOMICO, STRUCT, UNION, DESCRIBIR, SALIR." << endl;
                break;
        }
    }

    return 0;
}

