#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <variant>
#include <sstream>

using namespace std;

struct atomic {
    string name;
    int size; // in bytes
    int align; // in bytes
};

struct atomic_struct{
    string name;
    vector<string> fields;
};

struct atomic_union{
    string name;
    vector<string> fields;
};

enum AtomicKind { ATOMIC, STRUCT, UNION };

struct atomic_type {
    AtomicKind kind;
    variant<atomic, atomic_struct, atomic_union> at;
};

map<string, atomic_type> types_arr = {
    { "char",   { ATOMIC, atomic{"char",   1, 1} }},
    { "short",  { ATOMIC, atomic{"short",  2, 2} }},
    { "int",    { ATOMIC, atomic{"int",    4, 4} }},
    { "long",   { ATOMIC, atomic{"long",   8, 8} }},
    { "float",  { ATOMIC, atomic{"float",  4, 4} }},
    { "double", { ATOMIC, atomic{"double", 8, 8} }},
    { "bool",   { ATOMIC, atomic{"bool",   1, 2} }},
    {"MyStruct1", {STRUCT, atomic_struct{"MyStruct1", {"int", "char", "char", "int", "double", "bool"}}}},
    {"MyStruct2", {STRUCT, atomic_struct{"MyStruct2", {"short", "float", "char", "long"}}}}

};

// vector<string> sort_struct_by_alignment(atomic_struct& at_struct) {
//     vector<string> sorted = at_struct.fields;  // copiamos los campos

//     // bubble sort minimalista y claro
//     for (int i = 0; i < sorted.size(); i++) {
//         for (int j = 0; j < sorted.size() - 1; j++) {

//             int alignA = atomic_types[sorted[j]].align;
//             int alignB = atomic_types[sorted[j + 1]].align;

//             // Queremos ordenar de forma DESCENDENTE (mayor alineación primero)
//             if (alignA < alignB) {
//                 string temp = sorted[j];
//                 sorted[j] = sorted[j + 1];
//                 sorted[j + 1] = temp;
//             }
//         }
//     }

//     return sorted;
// }


// void print_struct_optimal(const atomic_struct& at_struct, int word_size) {
//     int total_bytes_lost = 0;
//     int total_bytes_used = 0;
//     int total_bytes_allocated = 0;
//     int mem_index_ptr = 0;
//     int fits = 0;
//     vector<int> mem_arr = {};
//     vector<int> align_arr = sort_struct_by_alignment(*at_struct);

// }

// void print_struct_w_packing(const atomic_struct& at_struct, int word_size) {
// int total_bytes_lost = 0;
//     int total_bytes_used = 0;
//     int total_bytes_allocated = 0;
//     int mem_index_ptr = 0;

//     for (const auto& field_name : at_struct.fields) {
//         cout << "Processing field: " << field_name << endl;
//         atomic obj = atomic_types[field_name];
//         cout << "Field size: " << obj.size << endl;
//         cout << "Apuntando a: " << mem_index_ptr << endl;
//         cout << "Eureka. Aqui es alineable" << endl;
//         total_bytes_used += obj.size;

//         mem_index_ptr += obj.size;
//         cout << "BYTES USADOS HASTA AHORA: " << total_bytes_used << endl;
//     }
//     total_bytes_allocated = total_bytes_used + total_bytes_lost;
//     cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << total_bytes_allocated << " bytes, Bytes lost: " << total_bytes_lost <<endl;
// }

// void print_struct_wt_packing(const atomic_struct& at_struct, int mem_index_ptr = 0) {
//     int total_bytes_lost = 0;
//     int total_bytes_used = 0;
//     int total_bytes_allocated = 0;

//     for (const auto& field_name : at_struct.fields) {
//         cout << "Processing field: " << field_name << endl;
//         atomic obj = atomic_types[field_name];
//         cout << "Field size: " << obj.size << endl;
//         cout << "Apuntando a: " << mem_index_ptr << endl;
//         if (mem_index_ptr % obj.align == 0) {
//             cout << "Eureka. Aqui es alineable" << endl;
            
//             total_bytes_used += obj.size;

//             mem_index_ptr += obj.size;
//             cout << "BYTES USADOS HASTA AHORA: " << total_bytes_used << endl;
            
//         } else {
//             while (mem_index_ptr % obj.align != 0) {
//                 mem_index_ptr++;
//                 total_bytes_lost++;
//             }
//             cout << "Podemos alinear aqui: " << mem_index_ptr << endl;
//             total_bytes_used += obj.size;

//             mem_index_ptr += obj.size;
//             cout << "Quedamos apuntando a: " << mem_index_ptr << endl;
//             cout << "BYTES USADOS HASTA AHORA: " << total_bytes_used << endl;
//             cout << "BYTES PERDIDOS HASTA AHORA: " << total_bytes_lost << endl;
//         }
//     }
//     total_bytes_allocated = total_bytes_used + total_bytes_lost;
//     cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << total_bytes_allocated << " bytes, Bytes lost: " << total_bytes_lost <<endl;
// }

void print_atomic_wt_packing(const atomic& at, int word_size) {
    int lost_bytes;

    if (at.size < word_size && at.size > 0) {
        lost_bytes = word_size - at.size;
    } else if (at.size >= word_size && at.size > 0 && at.size % word_size != 0) {
        lost_bytes = word_size - (at.size % word_size);
    } else if (at.size >= word_size && at.size > 0 && at.size % word_size == 0) {
        lost_bytes = 0;
    }

    cout << "Atomic Type: " << at.name << ", Size: " << at.size << " bytes, Alignment: " << at.align << " bytes, Lost bytes: " << lost_bytes <<endl;
}

void push_atomic(map<string, atomic_type>& arr, const string& name, int size, int align){
    if (size <= 0 || align <= 0) {
        throw runtime_error("Error: Size and alignment must be positive integers.");
    }

    atomic_type at;
    at.kind = ATOMIC;
    at.at = atomic{name, size, align};
    arr[name] = at;
}

void push_struct(map<string, atomic_type>& arr, const string& name, vector<string> fields){
    atomic_type at;
    at.kind = STRUCT;
    at.at = atomic_struct{name, fields};
    arr[name] = at;
}

void push_union(map<string, atomic_type>& arr, const string& name, vector<string> fields){
    atomic_type at;
    at.kind = UNION;
    at.at = atomic_union{name, fields};
    arr[name] = at;
}

vector<string> split(const string& line) {
    vector<string> tokens;
    string token;
    istringstream iss(line);

    while (iss >> token)
        tokens.push_back(token);

    return tokens;
}

bool is_integer(const string& s) {
    size_t pos;
    try {
        stoi(s, &pos);
        return pos == s.size(); // si consumió todo el string, es válido
    } catch (...) {
        return false;
    }
}

// función para imprimir el mapa. BORRAR LUEGO
void print_types(const map<string, atomic_type>& types) {
for (const auto& [key, type] : types_arr) {
        cout << "Type name: " << key << endl;

        switch (type.kind) {
            case ATOMIC: {
                const atomic& a = get<atomic>(type.at);
                cout << "  Kind: ATOMIC\n";
                cout << "  Size: " << a.size << ", Align: " << a.align << "\n";
                break;
            }

            case STRUCT: {
                const atomic_struct& s = get<atomic_struct>(type.at);
                cout << "  Kind: STRUCT\n";
                cout << "  Fields: ";
                for (const auto& f : s.fields) {
                    cout << f << " ";
                }
                cout << "\n";
                break;
            }

            case UNION: {
                const atomic_union& u = get<atomic_union>(type.at);
                cout << "  Kind: UNION\n";
                cout << "  Fields: ";
                for (const auto& f : u.fields) {
                    cout << f << " ";
                }
                cout << "\n";
                break;
            }
        }

        cout << "-----------------------------\n";
    }
}

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
                    
                    continue;
                } catch (exception& e) {
                    cout << e.what() << endl;
                    continue;
                }
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

                    continue;
                } catch (exception& e) {
                    cout << e.what() << endl;
                    continue;
                }
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

                    continue;
                } catch (exception& e) {
                    cout << e.what() << endl;
                    continue;
                }
            }
            case 4:
            case 5:
                cout << "Saliendo del programa." << endl;
                return 0;
            case 6:
                print_types(types_arr);
                continue;
            default:
                cout << "Error: unknown command." << endl;
                cout << "Available commands: \nATOMICO, STRUCT, UNION, DESCRIBIR, SALIR." << endl;
                break;
        }
    }

    return 0;
}

