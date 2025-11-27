#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Functions.hpp"

using namespace std;

void setup_basic_atomics() {
    types_arr.clear();
    push_atomic(types_arr, "char", 1, 1);
    push_atomic(types_arr, "short", 2, 2);
    push_atomic(types_arr, "int", 4, 4);
    push_atomic(types_arr, "long", 8, 8);
    push_atomic(types_arr, "float", 4, 4);
    push_atomic(types_arr, "double", 8, 8);
    push_atomic(types_arr, "bool", 1, 2);
}

TEST_CASE("push_atomic crea tipos atomicos correctos") {
    types_arr.clear();
    push_atomic(types_arr, "mychar", 1, 1);
    REQUIRE(types_arr.count("mychar") == 1);
    REQUIRE(types_arr["mychar"].kind == ATOMIC);
    const aatomic &a = get<aatomic>(types_arr["mychar"].at);
    CHECK(a.name == "mychar");
    CHECK(a.size == 1);
    CHECK(a.align == 1);
}

TEST_CASE("push_struct crea tipos structs planos correctos") {
    types_arr.clear();
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    atomic_struct s = get<atomic_struct>(types_arr["MyStruct1"].at);
    CHECK(s.name == "MyStruct1");
    CHECK(s.size == 19);
    CHECK(s.align == 4);
}

TEST_CASE("push_struct crea tipos structs compuestos correctos") {
    types_arr.clear();
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_struct(types_arr, "MyStruct2", {"MyStruct1", "bool", "char"});
    atomic_struct s = get<atomic_struct>(types_arr["MyStruct2"].at);
    CHECK(s.name == "MyStruct2");
    CHECK(s.size == 21);
    CHECK(s.align == 4);
}

TEST_CASE("push_struct impide tipos recursivos") {
    types_arr.clear();
    setup_basic_atomics();
    CHECK_THROWS_AS(
        push_struct(types_arr, "MyStruct1",
            {"int", "char", "char", "int", "double", "bool", "MyStruct1"}
        ),
        std::runtime_error
    );
}

TEST_CASE("push_union crea tipos union planos correctos") {
    types_arr.clear();
    setup_basic_atomics();
    push_union(types_arr, "MyUnion1", {"int", "char", "char", "int", "double", "bool"});
    atomic_union s = get<atomic_union>(types_arr["MyUnion1"].at);
    CHECK(s.name == "MyUnion1");
    CHECK(s.size == 8);
    CHECK(s.align == 8);
}

TEST_CASE("push_union crea tipos union compuestos correctos") {
    types_arr.clear();
    setup_basic_atomics();
    push_union(types_arr, "MyUnion1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion2", {"MyUnion1", "bool", "char"});
    atomic_union s = get<atomic_union>(types_arr["MyUnion2"].at);
    CHECK(s.name == "MyUnion2");
    CHECK(s.size == 8);
    CHECK(s.align == 8);
}

TEST_CASE("push_union impide tipos recursivos") {
    types_arr.clear();
    setup_basic_atomics();
    CHECK_THROWS_AS(
        push_union(types_arr, "MyUnion1",
            {"int", "char", "char", "int", "double", "bool", "MyUnion1"}
        ),
        std::runtime_error
    );
}

TEST_CASE("push_struct puede crear strucs con unions anidados correctamente") {
    types_arr.clear();
    setup_basic_atomics();
    push_union(types_arr, "MyUnion1", {"int", "char", "char", "int", "double", "bool"});
    push_struct(types_arr, "MyStruct1", {"MyUnion1"});
    atomic_struct s = get<atomic_struct>(types_arr["MyStruct1"].at);
    CHECK(s.name == "MyStruct1");
    CHECK(s.size == 8);
    CHECK(s.align == 8);
}

TEST_CASE("push_union puede crear unions con structs anidados correctamente") {
    types_arr.clear();
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion1", {"MyStruct1"});
    atomic_union s = get<atomic_union>(types_arr["MyUnion1"].at);
    CHECK(s.name == "MyUnion1");
    CHECK(s.size == 19);
    CHECK(s.align == 4);
}

TEST_CASE("push_struct y calc_size/align_struct funcionan") {
    setup_basic_atomics();

    // crea estructura simple: S1 { int, char, short }
    vector<string> fields = {"int", "char", "short"};
    push_struct(types_arr, "S1", fields);

    REQUIRE(types_arr.count("S1") == 1);
    CHECK(types_arr["S1"].kind == STRUCT);

    const atomic_struct &s = get<atomic_struct>(types_arr["S1"].at);

    // calc_size_struct debe dar suma de tamaños
    int expected_size = get<aatomic>(types_arr["int"].at).size
                      + get<aatomic>(types_arr["char"].at).size
                      + get<aatomic>(types_arr["short"].at).size;
    CHECK(calc_size_struct(s) == expected_size);

    // calc_align_struct, según tu impl, toma la alineación del primer campo
    CHECK(calc_align_struct(s) == get<aatomic>(types_arr["int"].at).align);
}

TEST_CASE("push_union y calc_size/align_union funcionan") {
    setup_basic_atomics();

    // Union U1 { int, double, short } -> size = max(4,8,2)=8 ; align = lcm(4,8,2)=8
    vector<string> ufields = {"int", "double", "short"};
    push_union(types_arr, "U1", ufields);

    REQUIRE(types_arr.count("U1") == 1);
    CHECK(types_arr["U1"].kind == UNION);

    const atomic_union &u = get<atomic_union>(types_arr["U1"].at);

    CHECK(calc_size_union(u) == 8);
    CHECK(calc_align_union(u) == 8);
}

TEST_CASE("collect_struct_fields aplana structs anidados") {
    setup_basic_atomics();

    // Define inner struct I { char, short }
    push_struct(types_arr, "I", vector<string>{"char", "short"});
    // Define outer struct O { int, I, char }
    push_struct(types_arr, "O", vector<string>{"int", "I", "char"});

    const atomic_struct &outer = get<atomic_struct>(types_arr["O"].at);

    vector<string> acc;
    collect_struct_fields(outer, acc);

    // debe contener los atomic que aparecen al final: int, char, short, char (I's fields inserted in order)
    // collect_struct_fields recorre I y agrega su fields; orden esperado: int, char, short, char
    REQUIRE(!acc.empty());
    CHECK(acc.front() == "int");
    CHECK(std::find(acc.begin(), acc.end(), "short") != acc.end());
}

TEST_CASE("sort_struct_fields_by_alignment ordena tipos simples por align descendente") {
    setup_basic_atomics();
    // crea 3 atomics con diferentes align (ya definidos arriba)
    // struct test { char, int, short } -> alins: 1,4,2 => orden esperado: int, short, char
    push_struct(types_arr, "T", vector<string>{"char","int","short"});
    const atomic_struct &t = get<atomic_struct>(types_arr["T"].at);

    vector<string> out_init;
    vector<string> sorted = sort_struct_fields_by_alignment(t, out_init);

    REQUIRE(sorted.size() == 3);
    CHECK(sorted[0] == "int");
    CHECK(sorted[1] == "short");
    CHECK(sorted[2] == "char");
}

TEST_CASE("sort_struct_fields_by_alignment ordena tipos compuestos por align descendente") {
    setup_basic_atomics();
    // crea 3 atomics con diferentes align (ya definidos arriba)
    // struct test { char, int, short } -> alins: 1,4,2 => orden esperado: int, short, char
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion1", {"int", "double"});
    push_struct(types_arr, "T", vector<string>{"char","MyStruct1","MyUnion1"});
    const atomic_struct &t = get<atomic_struct>(types_arr["T"].at);

    vector<string> out_init;
    vector<string> sorted = sort_struct_fields_by_alignment(t, out_init);

    REQUIRE(sorted.size() == 8);
    CHECK(sorted[0] == "double");
    CHECK(sorted[1] == "MyUnion1");
    CHECK(sorted[2] == "int");
    CHECK(sorted[3] == "int");
    CHECK(sorted[4] == "bool");
    CHECK(sorted[5] == "char");
    CHECK(sorted[6] == "char");
    CHECK(sorted[7] == "char");
}

TEST_CASE("split tokeniza correctamente") {
    string s = "STRUCT MyStruct int char";
    vector<string> toks = split(s);
    REQUIRE(toks.size() == 4);
    CHECK(toks[0] == "STRUCT");
    CHECK(toks[1] == "MyStruct");
    CHECK(toks[2] == "int");
    CHECK(toks[3] == "char");
}

TEST_CASE("is_integer reconoce enteros válidos e inválidos") {
    CHECK(is_integer("123") == true);
    CHECK(is_integer("-42") == true);
    CHECK(is_integer("12abc") == false);
    CHECK(is_integer("") == false);
}

TEST_CASE("print_mem_layout_diagram no crashea y formatea índices") {
    vector<int> mem = {1,0,1,1, 0,0,1,1};
    // Solo llamamos para verificar que imprime sin fallas
    print_mem_layout_diagram(mem, 4);
    CHECK(true); // si llegamos acá está ok
}


TEST_CASE("print_atomic y print_union no crashean") {
    aatomic a{"mydouble", 8, 8};
    print_atomic(a, 4);

    atomic_union u{"Utest", {"int","char"}, 4, 4};
    print_union(u, 4);

    CHECK(true);
}

TEST_CASE("print_struct_w_packing funciona con tipos simples") {
    setup_basic_atomics();
    push_struct(types_arr, "S2", vector<string>{"char","int","short"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen
    print_struct_w_packing(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_wt_packing funciona con tipos simples") {
    setup_basic_atomics();
    push_struct(types_arr, "S2", vector<string>{"char","int","short"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen
    print_struct_wt_packing(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_heuristics funciona con tipos simples") {
    setup_basic_atomics();
    push_struct(types_arr, "S2", vector<string>{"char","int","short"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen
    print_struct_heuristics(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_w_packing funciona con tipos compuestos") {
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion1", {"int", "double"});
    push_struct(types_arr, "S2", vector<string>{"char","MyStruct1","MyUnion1"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen
    print_struct_w_packing(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_wt_packing funciona con tipos compuestos") {
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion1", {"int", "double"});
    push_struct(types_arr, "S2", vector<string>{"char","MyStruct1","MyUnion1"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen
    print_struct_wt_packing(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_heuristics funciona con tipos compuestos") {
    setup_basic_atomics();
    push_struct(types_arr, "MyStruct1", {"int", "char", "char", "int", "double", "bool"});
    push_union(types_arr, "MyUnion1", {"int", "double"});
    push_struct(types_arr, "S2", vector<string>{"char","MyStruct1","MyUnion1"});
    const atomic_struct &s2 = get<atomic_struct>(types_arr["S2"].at);

    // packing y no-packing: llamamos para que no crasheen

    print_struct_heuristics(s2, 4);

    CHECK(true);
}

TEST_CASE("print_struct_heuristics_aux retorna conteo consistente") {
    setup_basic_atomics();
    vector<string> fields = {"int","char"};
    vector<int> mem_arr;
    long unsigned int mem_ptr = 0;
    vector<int> bytes = {0,0,0};
    vector<int> res = print_struct_heuristics_aux(fields, mem_arr, mem_ptr, bytes);
    // bytes[2] debe ser suma de usados y lost (>= used)
    CHECK(res[2] >= res[0]);
}

TEST_CASE("calc_size/align para structs y unions compuestos") {
    setup_basic_atomics();
    // crea struct nested
    push_struct(types_arr, "Inner", {"short","char"});
    push_union(types_arr, "MyUnion", {"Inner","int"});
    const atomic_struct &inner = get<atomic_struct>(types_arr["Inner"].at);
    const atomic_union &u = get<atomic_union>(types_arr["MyUnion"].at);

    CHECK(calc_size_struct(inner) == 3); // short(2)+char(1)
    CHECK(calc_align_union(u) == lcm(get<atomic_struct>(types_arr["Inner"].at).align, get<aatomic>(types_arr["int"].at).align));
}

TEST_CASE("push_struct detecta recursividad simple") {
    setup_basic_atomics();
    // intento crear struct recursivo
    try {
        push_struct(types_arr, "R", {"R"});
        // si no lanza excepción --> falla
        FAIL("Expected runtime_error for recursive declaration");
    } catch (const runtime_error& e) {
        CHECK(string(e.what()).find("Recursive declaration") != string::npos);
    }
}

TEST_CASE("push_union detecta recursividad simple") {
    setup_basic_atomics();
    try {
        push_union(types_arr, "U", {"U"});
        FAIL("Expected runtime_error for recursive declaration");
    } catch (const runtime_error& e) {
        CHECK(string(e.what()).find("Recursive declaration") != string::npos);
    }
}

TEST_CASE("print_types itera e imprime sin fallar") {
    setup_basic_atomics();
    push_struct(types_arr, "StructForPrint", {"int","char"});
    push_union(types_arr, "UnionForPrint", {"int","short"});
    print_types();
    CHECK(true);
}