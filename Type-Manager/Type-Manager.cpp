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

using namespace std;

struct atomic {
    string name;
    int size; // in bytes
    int align; // in bytes
};

struct atomic_struct{
    string name;
    vector<string> fields;
    int size = 0;
    int align = 0;
};

struct atomic_union{
    string name;
    vector<string> fields;
    int size = 0;
    int align = 0;
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
    {"MyStruct1", {STRUCT, atomic_struct{"MyStruct1", {"int", "char", "char", "int", "double", "bool"}, 19, 4}}},
    {"MyStruct2", {STRUCT, atomic_struct{"MyStruct2", {"short", "float", "char", "long"}, 15, 2}}},
    {"Union1", {UNION, atomic_union{"MyUnion1", {"double", "bool", "int"}, 8, 8}}},
    {"Union2", {UNION, atomic_union{"MyUnion2", {"Union1", "MyStruct2"}, 15, 8}}}
};


//DECLARACIONES
int calc_size_union (const atomic_union& at_union);
int calc_size_struct (const atomic_struct& at_struct);
int calc_align_union (const atomic_union& at_union);
int calc_align_struct (const atomic_struct& at_struct);
//DECLARACIONES

void print_mem_layout_diagram(const vector<int>& mem_arr, int word_size = 4) {
    
    cout << " Memory Layout Diagram (each '1' represents a byte):";

    for (int i = 0; i < mem_arr.size(); i++) {

        string idx = to_string(i);
        while (idx.size() < 3) idx = " " + idx;

        if (i % word_size == 0) {
            cout << "\n" << " " << idx << " | " << mem_arr[i] << " ";
        }
        else if ((i + 1) % word_size == 0) {
            cout << mem_arr[i] << " |";
        }
        else {
            cout << mem_arr[i] << " ";
        }
    }

    cout << endl;
    cout << " - - -" << endl;
}


void collect_struct_fields(const atomic_struct& at_struct, vector<string>& accumulator){
    for (const auto& field_name : at_struct.fields) {

        auto& t = types_arr[field_name];

        if (t.kind == STRUCT) {
            const auto& inner_struct = get<atomic_struct>(t.at);
            collect_struct_fields(inner_struct, accumulator);
        } else if (t.kind == ATOMIC || t.kind == UNION) {
            accumulator.push_back(field_name);
        }
    }
}

vector<string> sort_struct_fields_by_alignment(const atomic_struct& at_struct, vector<string>& fields) {
    vector<string> all_fields;

    collect_struct_fields(at_struct, all_fields);

    sort(all_fields.begin(), all_fields.end(),
        [](const string& a, const string& b) {
            int align_a = 0;
            int align_b = 0;

            auto& ta = types_arr[a];
            auto& tb = types_arr[b];

            if (ta.kind == ATOMIC)
                align_a = get<atomic>(ta.at).align;
            else if (ta.kind == STRUCT)
                align_a = get<atomic_struct>(ta.at).align;
            else if (ta.kind == UNION)
                align_a = get<atomic_union>(ta.at).align;

            if (tb.kind == ATOMIC)
                align_b = get<atomic>(tb.at).align;
            else if (tb.kind == STRUCT)
                align_b = get<atomic_struct>(tb.at).align;
            else if (tb.kind == UNION)
                align_b = get<atomic_union>(tb.at).align;

            return align_a > align_b;
        });

    fields = all_fields;
    return fields;
}

vector<int> print_struct_heuristics_aux(vector<string> fields, vector<int>& mem_arr, int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : fields) {
        if (types_arr[field_name].kind == ATOMIC){
            atomic type = get<atomic>(types_arr[field_name].at);

            if (mem_index_ptr % type.align == 0) {
                int i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            } else {
                int i = 0;
                while (mem_index_ptr % type.align != 0){
                    mem_arr.push_back(0);
                    bytes[1]++;
                    mem_index_ptr++;
                }
                i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            }
        } else if (types_arr[field_name].kind == UNION) {
            atomic_union type = get<atomic_union>(types_arr[field_name].at);
            if (mem_index_ptr % type.align == 0) {
                int i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            } else {
                int i = 0;
                while (mem_index_ptr % type.align != 0){
                    mem_arr.push_back(0);
                    bytes[1]++;
                    mem_index_ptr++;
                }
                i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            }
        }
    }
    bytes[2] = bytes[0] + bytes[1];

    return bytes;
}

void print_struct_heuristics(const atomic_struct& at_struct, int word_size = 4) {
    vector<string> init = {};
    vector<string> fields = sort_struct_fields_by_alignment(at_struct, init);

    vector<int> mem_arr = {};
    int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_heuristics_aux(fields, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

int calc_align_union (const atomic_union& at_union){
    int align_accumulated = 1;
    for (const auto& field_name : at_union.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            align_accumulated = lcm(align_accumulated, get<atomic>(types_arr[field_name].at).align);
        } else if (types_arr[field_name].kind == STRUCT) {
            align_accumulated = lcm(align_accumulated, get<atomic_struct>(types_arr[field_name].at).align);
        } else if (types_arr[field_name].kind == UNION) {
            align_accumulated = lcm(align_accumulated, get<atomic_union>(types_arr[field_name].at).align);
        }
    }
    return align_accumulated;
}

int calc_align_struct (const atomic_struct& at_struct){
    int align_accumulated = 0;
    auto first_field_type = types_arr[at_struct.fields[0]];
    if (first_field_type.kind == ATOMIC){
        align_accumulated = get<atomic>(first_field_type.at).align;
    } else if (first_field_type.kind == STRUCT) {
        align_accumulated = get<atomic_struct>(first_field_type.at).align;
    } else if (first_field_type.kind == UNION) {
        align_accumulated = get<atomic_union>(first_field_type.at).align;
    }
    return align_accumulated;
}

int calc_size_union (const atomic_union& at_union) {
    int size_accumulated = 0;
    for (const auto& field_name : at_union.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            size_accumulated = max(size_accumulated, get<atomic>(types_arr[field_name].at).size);
        } else if (types_arr[field_name].kind == STRUCT) {
            size_accumulated = max(size_accumulated, get<atomic_struct>(types_arr[field_name].at).size);
        } else if (types_arr[field_name].kind == UNION) {
            size_accumulated = max(size_accumulated, get<atomic_union>(types_arr[field_name].at).size);
        }
    }
    return size_accumulated;
}

int calc_size_struct (const atomic_struct& at_struct) {
    int size_accumulated = 0;
    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            size_accumulated += get<atomic>(types_arr[field_name].at).size;
        } else if (types_arr[field_name].kind == STRUCT) {
            size_accumulated += get<atomic_struct>(types_arr[field_name].at).size;
        } else if (types_arr[field_name].kind == UNION) {
            size_accumulated += get<atomic_union>(types_arr[field_name].at).size;
        }
    }
    return size_accumulated;
}

vector<int> print_struct_wt_packing_aux(const atomic_struct& at_struct, vector<int>& mem_arr, int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            atomic type = get<atomic>(types_arr[field_name].at);

            if (mem_index_ptr % type.align == 0) {
                int i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            } else {
                int i = 0;
                while (mem_index_ptr % type.align != 0){
                    mem_arr.push_back(0);
                    bytes[1]++;
                    mem_index_ptr++;
                }
                i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            }
        } 
        else if (types_arr[field_name].kind == STRUCT) {
            atomic_struct type = get<atomic_struct>(types_arr[field_name].at);
            bytes = print_struct_wt_packing_aux(type, mem_arr, mem_index_ptr, bytes);
        } else if (types_arr[field_name].kind == UNION) {
            atomic_union type = get<atomic_union>(types_arr[field_name].at);
            if (mem_index_ptr % type.align == 0) {
                int i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            } else {
                int i = 0;
                while (mem_index_ptr % type.align != 0){
                    mem_arr.push_back(0);
                    bytes[1]++;
                    mem_index_ptr++;
                }
                i = 0;
                while (i < type.size){
                    mem_arr.push_back(1);
                    bytes[0]++;
                    mem_index_ptr++;
                    i++;
                }
            }
        }
    }
    bytes[2] = bytes[0] + bytes[1];

    return bytes;
}

vector<int> print_struct_w_packing_aux(const atomic_struct& at_struct, vector<int>& mem_arr, int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            atomic type = get<atomic>(types_arr[field_name].at);
            int i = 0;
            while (i < type.size){
                mem_arr.push_back(1);
                bytes[0]++;
                mem_index_ptr++;
                i++;
            }
        } 
        else if (types_arr[field_name].kind == STRUCT) {
            atomic_struct type = get<atomic_struct>(types_arr[field_name].at);
            bytes = print_struct_w_packing_aux(type, mem_arr, mem_index_ptr, bytes);
        } else if (types_arr[field_name].kind == UNION) {
            atomic_union type = get<atomic_union>(types_arr[field_name].at);
            int i = 0;
            while (i < type.size){
                mem_arr.push_back(1);
                bytes[0]++;
                mem_index_ptr++;
                i++;
            }
        }
    }
    bytes[2] = bytes[0] + bytes[1];

    return bytes;
}

void print_struct_w_packing(const atomic_struct& at_struct, int word_size = 4){
    vector<int> mem_arr = {};
    int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_w_packing_aux(at_struct, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

void print_struct_wt_packing(const atomic_struct& at_struct, int word_size = 4){
    vector<int> mem_arr = {};
    int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_wt_packing_aux(at_struct, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

void print_union (const atomic_union& at, int word_size = 4){
    int num_of_cells = ceil((double)at.size / word_size) * word_size;
    if (num_of_cells == 0){
        num_of_cells = word_size;
    }
    vector<int> mem_arr(num_of_cells, 0);

    for (int i = 0; i < at.size; i++) {
        mem_arr[i] = 1;
    }

    print_mem_layout_diagram(mem_arr, word_size);

    cout << "Union Type: " << at.name << "\nSize: " << at.size << " bytes\nAlignment: " << at.align << " bytes"<<endl;

}

void print_atomic(const atomic& at, int word_size = 4) {
    int num_of_cells = ceil((double) at.size / word_size) * word_size;

    if (num_of_cells == 0){
        num_of_cells = word_size;
    }

    vector<int> mem_arr(num_of_cells, 0);

    for (int i = 0; i < at.size; i++) {
        mem_arr[i] = 1;
    }

    print_mem_layout_diagram(mem_arr, word_size);

    cout << "Atomic Type: " << at.name << "\nSize: " << at.size << " bytes\nAlignment: " << at.align << " bytes"<<endl;

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

    for (const auto& f : fields) {
        if (f == name) {
            throw runtime_error("Error: Recursive declaration of type '" + name + "'");
        }
    }

    atomic_type at;
    at.kind = STRUCT;
    at.at = atomic_struct{name, fields};
    atomic_struct* at_struct = &(get<atomic_struct>(at.at));
    int size = calc_size_struct(get<atomic_struct>(at.at));
    int align = calc_align_struct(get<atomic_struct>(at.at));
    at_struct->size = size;
    at_struct->align = align;
    arr[name] = at;
}

void push_union(map<string, atomic_type>& arr, const string& name, vector<string> fields){
    for (const auto& f : fields) {
        if (f == name) {
            throw runtime_error("Error: Recursive declaration of type '" + name + "'");
        }
    }
    atomic_type at;
    at.kind = UNION;
    at.at = atomic_union{name, fields};
    atomic_union * at_union = &(get<atomic_union>(at.at));
    int size = calc_size_union(get<atomic_union>(at.at));
    int align = calc_align_union(get<atomic_union>(at.at));
    at_union->size = size;
    at_union->align = align;
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

                vector<string> fields_init = {};
                vector<string> fields = sort_struct_fields_by_alignment(s, fields_init);
                cout << "\n  Size: " << s.size << " bytes" << endl;
                cout << "  Align: " << s.align << " bytes" << endl;
                cout << "  Fields: " << endl;
                for (const auto& f : fields) {
                    cout << "  " << f << " ";
                }
                cout << endl;
                break;
            }

            case UNION: {
                const atomic_union& u = get<atomic_union>(type.at);
                cout << "  Kind: UNION\n";
                cout << "  Fields: ";
                for (const auto& f : u.fields) {
                    cout << f << " ";
                }
                cout << "\n  Size: " << u.size << " bytes" << endl;
                cout << "  Align: " << u.align << " bytes" << endl;
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
                            const atomic& a = get<atomic>(type.at);
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

