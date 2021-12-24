#include "verECS.hpp"
#include <span>
#include <vector>
#include <iostream>

struct Point{
    int x;
    int y;
};

struct String{
    char *str;
    int len;

    ~String(){
        delete[] str;
    }

    void newS(){
        str = new char[4];
        for(int i=0;i<3;++i){
            str[i] = 'a';
        }
        str[3] = '\0';
        len = 3;
    }
};

struct Tag{
    int tag;
};

void print_int_comp_arr(verECS::ECS ecs){

    std::span intSp = ecs.get_Comp_Span<int>();
    int size = intSp.size();
    for(int i=0;i<size;++i){
        std::cout << intSp[i] << '\n';
    }
    std::cout << '\n';
}

void inc_int_comp_arr(verECS::ECS ecs){
    std::span intSp = ecs.get_Comp_Span<int>();
    int size = intSp.size();

    for(int i=0;i<size;++i){
        ++intSp[i];
    }
}

void inc_tagged_int(verECS::ECS ecs){
    std::vector<int> idVec = ecs.get_EntId_Vec_With_Comp<Tag>();
}

void print_intVec_comp_arr(verECS::ECS ecs){

    std::span int_vec_span = ecs.get_Comp_Span<std::vector<int>>();
    int size = int_vec_span.size();
    for(int i=0;i<size;++i){
        int vSize = int_vec_span[i].size();
        for(int j=0;j<vSize;++j){
            std::cout << int_vec_span[i][j] << '\n'; 
        }
    }
    std::cout << '\n';

}

void print_string_comp_arr(verECS::ECS ecs){
    std::span s = ecs.get_Comp_Span<String*>();
    int size = s.size();

    for(int i=0;i<size;++i){
        std::cout << s[i]->str << '\n';
    }
    std::cout << '\n';


}

void add_20_to_tagged_int(verECS::ECS ecs){
    std::vector<int*> ints = ecs.get_Comp_Vec_Associated_To<int>({typeid(Tag)});

    for(auto i = ints.begin(); i != ints.end(); ++i){
        **i += 20;
    }

}

int main(){
    verECS::ECS ecs;
    int ent1 = ecs.new_Entity(), ent2 = ecs.new_Entity();

    ecs.push_Comp<int>(1, ent1);
    ecs.push_Comp<Tag>((Tag){.tag=1}, ent1);
    ecs.push_Comp<int>(2, ent2);

    add_20_to_tagged_int(ecs);
    print_int_comp_arr(ecs);

    ecs.delete_Comp<int>();
    ecs.delete_Comp<Tag>();

    return 0;
}