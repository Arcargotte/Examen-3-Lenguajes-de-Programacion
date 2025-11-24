#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>

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

map<string, atomic> atomic_types = {
    {"char", {"char", 1, 1}},
    {"short", {"short", 2, 2}},
    {"int", {"int", 4, 4}},
    {"long", {"long", 8, 8}},
    {"float", {"float", 4, 4}},
    {"double", {"double", 8, 8}},
    {"bool", {"double", 1, 2}}
};

atomic_struct my_struct1 = {"MyStruct1", {"int", "char", "char", "int", "double", "bool"}};
atomic_struct my_struct2 = {"MyStruct2", {"short", "float", "char", "long"}};

map<string, atomic_struct> struct_types = {
    {"MyStruct1", my_struct1},
};

void print_struct_w_packing(const atomic_struct& at_struct, int word_size) {
    
}

void print_struct_wt_packing(const atomic_struct& at_struct, int word_size) {
    int total_bytes_lost = 0;
    int total_bytes_used = 0;
    int total_bytes_allocated = 0;
    int mem_index_ptr = 0;

    for (const auto& field_name : at_struct.fields) {
        cout << "Processing field: " << field_name << endl;
        atomic obj = atomic_types[field_name];
        cout << "Field size: " << obj.size << endl;
        cout << "Apuntando a: " << mem_index_ptr << endl;
        if (mem_index_ptr % obj.align == 0) {
            cout << "Eureka. Aqui es alineable" << endl;
            
            total_bytes_used += obj.size;

            mem_index_ptr += obj.size;
            cout << "BYTES USADOS HASTA AHORA: " << total_bytes_used << endl;
            
        } else {
            while (mem_index_ptr % obj.align != 0) {
                mem_index_ptr++;
                total_bytes_lost++;
            }
            cout << "Podemos alinear aqui: " << mem_index_ptr << endl;
            total_bytes_used += obj.size;

            mem_index_ptr += obj.size;
            cout << "Quedamos apuntando a: " << mem_index_ptr << endl;
            cout << "BYTES USADOS HASTA AHORA: " << total_bytes_used << endl;
            cout << "BYTES PERDIDOS HASTA AHORA: " << total_bytes_lost << endl;
        }
    }
    total_bytes_allocated = total_bytes_used + total_bytes_lost;
    cout << "Struct Type: " << at_struct.name << ", Bytes allocated: " << total_bytes_allocated << " bytes, Bytes lost: " << total_bytes_lost <<endl;
}

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

void push_atomic(vector<atomic>& atomic_types, const string& name, int size, int align) {
    atomic_types.push_back({name, size, align});
}

int main() {
    int word_size = 4; // Tamaño de palabra en bytes (32 bits)

    for (const auto& par : struct_types) {
        print_struct_wt_packing(par.second, word_size);
    }

    return 0;
}

