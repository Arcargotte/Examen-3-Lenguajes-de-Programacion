#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

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

/**
 * Structs for modelling atomic types, structs and unions.
 * 
 * The special type "atomic_type" is a variant type for creating a dict that can contains all three types in a single structures
 * such that declarations using the same identifier overlap one another and to ease accesability for other functions and procedures
 * that consults the dictionary as a map. It contains a discriminant field "kind" that identifies if it's an atomic type, a struct
 * or a union. This kind is of type enum AtomicKind that lists the types ATOMIC, STRUCT, UNION and maps each type with integers 0, 1
 * and 2. 
 */

struct aatomic {
    string name;
    int size;
    int align;
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
    variant<aatomic, atomic_struct, atomic_union> at;
};

// map<string, atomic_type> types_arr = {};

map<string, atomic_type> types_arr = {
    { "char",   { ATOMIC, aatomic{"char",   1, 1} }},
    { "short",  { ATOMIC, aatomic{"short",  2, 2} }},
    { "int",    { ATOMIC, aatomic{"int",    4, 4} }},
    { "long",   { ATOMIC, aatomic{"long",   8, 8} }},
    { "float",  { ATOMIC, aatomic{"float",  4, 4} }},
    { "double", { ATOMIC, aatomic{"double", 8, 8} }},
    { "bool",   { ATOMIC, aatomic{"bool",   1, 2} }},
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

/**
 * Prints type layout in memory.
 * 
 * @param mem_arr Contains either 1s or 0s. 1 stands for an active byte in memory occupied by the type.
 * @param word_size Defines the word size in the memory layout to visually check for type alignment.
 */
void print_mem_layout_diagram(const vector<int>& mem_arr, int word_size = 4) {
    
    cout << " Memory Layout Diagram (each '1' represents a byte):";

    for (long unsigned int i = 0; i < mem_arr.size(); i++) {

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

/**
 * Collects recursively the fields in a struct.
 * 
 * @param at_struct Struct type.
 * @param accumulator Contains the atomic fields collected so far during execution.
 */
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

/**
 * Sorts a list of types in function of their alignment.
 * 
 * This is an auxiliary function for the heuristics to optimize the memory layout of a type
 * 
 * @param at_struct Struct type.
 * @param fields Contains the atomic fields of the struct.
 * @return vector of atomic fields sorted by alignment.
 */
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
                align_a = get<aatomic>(ta.at).align;
            else if (ta.kind == STRUCT)
                align_a = get<atomic_struct>(ta.at).align;
            else if (ta.kind == UNION)
                align_a = get<atomic_union>(ta.at).align;

            if (tb.kind == ATOMIC)
                align_b = get<aatomic>(tb.at).align;
            else if (tb.kind == STRUCT)
                align_b = get<atomic_struct>(tb.at).align;
            else if (tb.kind == UNION)
                align_b = get<atomic_union>(tb.at).align;

            return align_a > align_b;
        });

    fields = all_fields;
    return fields;
}

/**
 * Auxiliary function to print_struct_heuristics.
 * 
 * Determines whether n bytes can be allocated from mem_index_ptr in array of memory.
 * 
 * @param n num of bytes to allocate.
 * @param mem_index_ptr pointer of memory position.
 * @param mem_arr array of memory.
 * @return true if n bytes can be allocated from mem_index_ptr. Return false otherwise
 */
bool can_allocate_n_bytes (int n, long unsigned int mem_index_ptr, vector<int>& mem_arr) {

    if (mem_arr.size() == 0){
        return true;
    }

    int i = n;

    while (mem_arr[mem_index_ptr] != 1 && i > 0 && mem_index_ptr < mem_arr.size()){
        mem_index_ptr++;
        i--;
    }

    if (mem_arr[mem_index_ptr] == 1 && i > 0){
        return false;
    } else if (mem_index_ptr >= mem_arr.size()){

        return true;
    }

    return true;
}

/**
 * Auxiliary function to print_struct_heuristics.
 * 
 * Builds the mem_arr for memory layout using the heuristic and keeps the number of bytes allocated
 * 
 * @param at_struct Struct type.
 * @param fields Contains the atomic fields of the struct.
 * @return vector of integers. bytes[0] contains the num of active bytes, bytes[1] contains the num of wasted bytes to alignment, bytes[2] contains the total number of bytes occupied
 */
vector<int> print_struct_heuristics_aux(vector<string> fields, vector<int>& mem_arr, long unsigned int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : fields) {
        bool is_allocated = false;
        if (types_arr[field_name].kind == ATOMIC){
            aatomic type = get<aatomic>(types_arr[field_name].at);

            while (!is_allocated){
                if (mem_index_ptr % type.align == 0 && can_allocate_n_bytes(type.size, mem_index_ptr, mem_arr)) {
                    int i = 0;
                    while (i < type.size){
                        if (mem_index_ptr < mem_arr.size()){
                            mem_arr[mem_index_ptr] = 1;
                        } else {
                            mem_arr.push_back(1);
                        }
                        bytes[0]++;
                        mem_index_ptr++;
                        i++;
                    }
                    is_allocated = true;
                    mem_index_ptr = 0;
                }

                if (mem_index_ptr < mem_arr.size()){
                    mem_index_ptr++;
                } else {
                    mem_arr.push_back(0);
                }
            }
        } else if (types_arr[field_name].kind == UNION) {
            atomic_union type = get<atomic_union>(types_arr[field_name].at);

            while (!is_allocated){
                if (mem_index_ptr % type.align == 0 && can_allocate_n_bytes(type.size, mem_index_ptr, mem_arr)) {
                    int i = 0;
                    while (i < type.size){
                        if (mem_index_ptr < mem_arr.size()){
                            mem_arr[mem_index_ptr] = 1;
                        } else {
                            mem_arr.push_back(1);
                        }
                        bytes[0]++;
                        mem_index_ptr++;
                        i++;
                    }
                    is_allocated = true;
                    mem_index_ptr = 0;
                }

                if (mem_index_ptr < mem_arr.size()){
                    mem_index_ptr++;
                } else {
                    mem_arr.push_back(0);
                }
            }
        }
    }

    for (const auto& cell : mem_arr) {
        if (cell == 0){
            bytes[1]++;
        }
    }

    bytes[2] = bytes[0] + bytes[1];

    return bytes;
}
/**
 * Applies heuristic to set up the memory layout for a type maximizing space and time.
 * 
 * The heuristic goes as follows: for each type, lay in memory those with greater alignment first. This heuristic assume that the greater the alignment of the type, the most likely it is
 * to waste bytes. So it lays them in memory first taking advantage of mem address 0 and the fact that greater alignments are less likely to have valid mem addresses to align with within the size
 * of the structure. For example, consider types A and B with alignment 4 and 8 in a struct of size 16. There are more multiples of 4 in 16 than there are multiples of 8, so we lay first
 * B in memory and move on to A. 
 * 
 * @param at_struct Struct type.
 * @param word_size Sets the size of a word for printing memory layout.
 */
void print_struct_heuristics(const atomic_struct& at_struct, int word_size = 4) {
    vector<string> init = {};
    vector<string> fields = sort_struct_fields_by_alignment(at_struct, init);

    vector<int> mem_arr = {};
    long unsigned int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_heuristics_aux(fields, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

/**
 * Calculates the alignment for an union type
 * 
 * @param at_union Union type.
 * @return the lcm of the alignments of the fields defined in the union tyoe.
 */
int calc_align_union (const atomic_union& at_union){
    int align_accumulated = 1;
    for (const auto& field_name : at_union.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            align_accumulated = lcm(align_accumulated, get<aatomic>(types_arr[field_name].at).align);
        } else if (types_arr[field_name].kind == STRUCT) {
            align_accumulated = lcm(align_accumulated, get<atomic_struct>(types_arr[field_name].at).align);
        } else if (types_arr[field_name].kind == UNION) {
            align_accumulated = lcm(align_accumulated, get<atomic_union>(types_arr[field_name].at).align);
        }
    }
    return align_accumulated;
}

/**
 * Calculates the alignment for a struct type.
 * 
 * It takes the alignment of the first atomic field to set the alignment for the rest of the structure. If the firs field is an struct, calculated the alignment recursively
 * 
 * @param at_struct Struct type.
 * @return the alignment of the first atomic field of the struct.
 */
int calc_align_struct (const atomic_struct& at_struct){
    int align_accumulated = 0;
    auto first_field_type = types_arr[at_struct.fields[0]];
    if (first_field_type.kind == ATOMIC){
        align_accumulated = get<aatomic>(first_field_type.at).align;
    } else if (first_field_type.kind == STRUCT) {
        align_accumulated = get<atomic_struct>(first_field_type.at).align;
    } else if (first_field_type.kind == UNION) {
        align_accumulated = get<atomic_union>(first_field_type.at).align;
    }
    return align_accumulated;
}

/**
 * Calculates the size of an union type.
 * 
 * It takes the field with greater size to define the size of the union type.
 * 
 * @param at_union Union type.
 * @return the size of the greatest field of the union.
 */
int calc_size_union (const atomic_union& at_union) {
    int size_accumulated = 0;
    for (const auto& field_name : at_union.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            size_accumulated = max(size_accumulated, get<aatomic>(types_arr[field_name].at).size);
        } else if (types_arr[field_name].kind == STRUCT) {
            size_accumulated = max(size_accumulated, get<atomic_struct>(types_arr[field_name].at).size);
        } else if (types_arr[field_name].kind == UNION) {
            size_accumulated = max(size_accumulated, get<atomic_union>(types_arr[field_name].at).size);
        }
    }
    return size_accumulated;
}
/**
 * Calculates the size of a struct type.
 * 
 * It takes the sum of all the fields of the struct.
 * 
 * @param at_union Union type.
 * @return the sum of all the fields of the struct.
 */
int calc_size_struct (const atomic_struct& at_struct) {
    int size_accumulated = 0;
    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            size_accumulated += get<aatomic>(types_arr[field_name].at).size;
        } else if (types_arr[field_name].kind == STRUCT) {
            size_accumulated += get<atomic_struct>(types_arr[field_name].at).size;
        } else if (types_arr[field_name].kind == UNION) {
            size_accumulated += get<atomic_union>(types_arr[field_name].at).size;
        }
    }
    return size_accumulated;
}

/**
 * Auxiliary function to printing the memory layout of a struct using a non-packing strategy.
 * 
 * Builds the mem_arr for memory layout using the strategy and keeps the number of bytes allocated
 * 
 * @param at_struct Struct type.
 * @param mem_arr Memory layout of the type.
 * @param mem_index_ptr Mem index to the last byte placed in memory.
 * @param bytes Union type.
 * @return vector of integers. bytes[0] contains the num of active bytes, bytes[1] contains the num of wasted bytes to alignment, bytes[2] contains the total number of bytes occupied
 */
vector<int> print_struct_wt_packing_aux(const atomic_struct& at_struct, vector<int>& mem_arr, int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            aatomic type = get<aatomic>(types_arr[field_name].at);

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

/**
 * Auxiliary function to printing the memory layout of a struct using a packing strategy.
 * 
 * Builds the mem_arr for memory layout using the strategy and keeps the number of bytes allocated
 * 
 * @param at_struct Struct type.
 * @param mem_arr Memory layout of the type.
 * @param mem_index_ptr Mem index to the last byte placed in memory.
 * @param bytes Union type.
 * @return vector of integers. bytes[0] contains the num of active bytes, bytes[1] contains the num of wasted bytes to alignment, bytes[2] contains the total number of bytes occupied
 */
vector<int> print_struct_w_packing_aux(const atomic_struct& at_struct, vector<int>& mem_arr, int& mem_index_ptr, vector<int>& bytes) {

    for (const auto& field_name : at_struct.fields) {
        if (types_arr[field_name].kind == ATOMIC){
            aatomic type = get<aatomic>(types_arr[field_name].at);
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

/**
 * Prints the memory layout of a struct using a packing strategy.
 * 
 * @param at_struct Struct type.
 * @param word_size Defines the word size to check for type alignment in memory layout
 */
void print_struct_w_packing(const atomic_struct& at_struct, int word_size = 4){
    vector<int> mem_arr = {};
    int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_w_packing_aux(at_struct, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

/**
 * Prints the memory layout of a struct using a non-packing strategy.
 * 
 * @param at_struct Struct type.
 * @param word_size Defines the word size to check for type alignment in memory layout
 */
void print_struct_wt_packing(const atomic_struct& at_struct, int word_size = 4){
    vector<int> mem_arr = {};
    int mem_index_ptr = 0;
    vector<int> bytes_init = {0, 0, 0};
    vector<int> bytes = print_struct_wt_packing_aux(at_struct, mem_arr, mem_index_ptr, bytes_init);

    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << bytes[2] << " bytes, Bytes lost: " << bytes[1] <<endl;

    print_mem_layout_diagram(mem_arr, word_size);
}

/**
 * Prints the memory layout of a union type.
 * 
 * @param at_struct Struct type.
 * @param word_size Defines the word size to check for type alignment in memory layout
 */
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

/**
 * Prints the memory layout of an atomic type.
 * 
 * @param at Atomic type.
 * @param word_size Defines the word size to check for type alignment in memory layout
 */
void print_atomic(const aatomic& at, int word_size = 4) {
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

/**
 * Stores pair (key, value) in global map to keep list of atomic types during execution.
 * 
 * Creates an atomic type.
 * 
 * @param arr Map of types.
 * @param name Sets the name of the atomic type
 * @param size Set the size of atomic type.
 * @param align Sets the alignment of atomic type.
 */
void push_atomic(map<string, atomic_type>& arr, const string& name, int size, int align){
    if (size <= 0 || align <= 0) {
        throw runtime_error("Error: Size and alignment must be positive integers.");
    }

    atomic_type at;
    at.kind = ATOMIC;
    at.at = aatomic{name, size, align};
    arr[name] = at;
}

/**
 * Stores pair (key, value) in global map to keep list of atomic types during execution.
 * 
 * Creates a struct type.
 * 
 * @param arr Map of types.
 * @param name Sets the name of the struct type
 * @param fields Arr of strings containing the identifiers of types existing in the type map. They are the fields of the struct type.
 */
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

/**
 * Stores pair (key, value) in global map to keep list of atomic types during execution.
 * 
 * Creates a union type.
 * 
 * @param arr Map of types.
 * @param name Sets the name of the union type
 * @param fields Arr of strings containing the identifiers of types existing in the type map. They are the fields of the union type.
 */

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

/**
 * Parses the input of the user in tokens.
 * 
 * @param line Input string given by user.
 * @return Arr of tokens collected in a line separed by whitespaces.
 */
vector<string> split(const string& line) {
    vector<string> tokens;
    string token;
    istringstream iss(line);

    while (iss >> token)
        tokens.push_back(token);

    return tokens;
}

/**
 * Checks if a token is an integer.
 * 
 * @param s Token.
 * @return True if token is integer. False otherwise.
 */
bool is_integer(const string& s) {
    size_t pos;
    try {
        stoi(s, &pos);
        return pos == s.size(); // si consumió todo el string, es válido
    } catch (...) {
        return false;
    }
}

/**
 * Auxiliary function that lists the types defined so far during execution of the program.
 * 
 * @param types Map of types.
 */
void print_types() {
    for (const auto& [key, type] : types_arr) {
        cout << "Type name: " << key << endl;

        switch (type.kind) {
            case ATOMIC: {
                const aatomic& a = get<aatomic>(type.at);
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
#endif