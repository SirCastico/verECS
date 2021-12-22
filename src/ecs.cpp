#include <vector>
#include <unordered_map>
#include <typeindex>
#include <iostream>
#include <span>

struct CompAr{
    void *arr_;
    int cap_;
    int len_;
    int entId_;

    CompAr(){
        arr_ = nullptr;
        cap_ = 0;
        len_ = 0;
        entId_ = -1;
    }

    template<class T>
    void delete_Arr(){            
        delete[] (T*) arr_;
    }

    template<class T>
    int push_Comp(T val, int entId){
        T *arr = (T*) arr_; 
        if(!arr || len_==cap_){
            cap_ = 2 * cap_ + 1;
            T* nArr = new T[cap_];

            for(int i=0;i<len_;++i){
                nArr[i] = arr[i];
            }

            delete[] arr;
            arr = nArr;
            arr_ = nArr;
            
        }
        arr[len_] = val;
        len_++;
        entId_ = entId;

        return len_-1;
    }

};

namespace verECS{

    class CompMap{
        std::unordered_map<std::type_index, CompAr> map_;

        public:
        template<class T>
        void new_Comp(){
            CompAr ar;

            map_.try_emplace(typeid(T), ar);
        }

        template<class T>
        int push_Comp(T val, int entId){
            int ind = -1;
            auto iter = map_.find(typeid(T));

            if(iter == map_.end()){
                this->new_Comp<T>();
            }
            CompAr *cAr = &(map_.at(typeid(T)));
            ind = cAr->push_Comp<T>(val, entId);

            return ind;
        }

        template<class T>
        void delete_Comp(){
            auto iter = map_.find(typeid(T));

            if(iter != map_.end()){
                CompAr cAr = map_.at(typeid(T));
                cAr.delete_Arr<T>();
            }
        }

        template<class T>
        std::span<T> get_Comp_Arr(){
            auto iter = map_.find(typeid(T));
            std::span<T> span;

            if(iter != map_.end()){
                CompAr cAr = map_.at(typeid(T));
                span = std::span((T*)cAr.arr_,cAr.len_);
            }

            return span;
        }
    };

    class Entity{
        public:
        int entId_; // talvez não necessário
        std::unordered_map<std::type_index, int> compIdMap_;

        Entity(){
            entId_ = -1;
        }

        template<class T>
        int get_Comp_Id(){
            return compIdMap_.at(typeid(T));
        }
    };

    class ECS{
        CompMap compMap_;
        std::vector<Entity> entList_;

        public:

        int new_Entity(){
            Entity ent;
            ent.entId_ = entList_.size();
            entList_.push_back(ent);

            return ent.entId_;
        }

        template<class T>
        bool push_Comp(T val, int ent){
            bool bl = false;
            if(ent < entList_.size()){
                Entity *entity = &entList_[ent];
                auto iter = entity->compIdMap_.find(typeid(T));

                if(iter == entity->compIdMap_.end()){
                    entity->compIdMap_.emplace(typeid(T),compMap_.push_Comp<T>(val, ent));
                    bl = true;
                }
            }

            return bl;
        }

        template<class T>
        T *get_Ent_Comp_P(int entId){
            T *r=nullptr;
            if(entId < entList_.size()){
                std::span<T> cSpan = compMap_.get_Comp_Arr<T>();
                Entity ent = entList_[entId];
                int ind = ent.get_Comp_Id<T>();
                r = &cSpan[ind];
            }

            return r;
        }

        // buscar array de componentes, filtrar se as entities não estão associadas a todos os Ts
        // devolver array
        template<class T0, class... Ts>
        std::span<T0> get_Comp_Associated_To(){

        }

        template<class T>
        std::span<T> get_Comp_Arr(){
            return compMap_.get_Comp_Arr<T>();
        }

        template<class T>
        std::vector<int> get_EntId_Vec_With_Comp(){
            std::vector<int> idVec;
            int i=0;
            for(Entity ent : entList_){
                if(ent.compIdMap_.find(typeid(T)) != ent.compIdMap_.end()){
                    idVec.push_back(i);
                }
                i++;
            }
            return idVec;
        }

        template<class T>
        void delete_Comp(){
            for(Entity ent : entList_){
                if(ent.compIdMap_.find(typeid(T)) != ent.compIdMap_.end()){
                    ent.compIdMap_.erase(typeid(T));
                }
            }
            compMap_.delete_Comp<T>();
        }
    };
}

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

    std::span intSp = ecs.get_Comp_Arr<int>();
    int size = intSp.size();
    for(int i=0;i<size;++i){
        std::cout << intSp[i] << '\n';
    }
    std::cout << '\n';
}

void inc_int_comp_arr(verECS::ECS ecs){
    std::span intSp = ecs.get_Comp_Arr<int>();
    int size = intSp.size();

    for(int i=0;i<size;++i){
        ++intSp[i];
    }
}

void inc_tagged_int(verECS::ECS ecs){
    std::vector<int> idVec = ecs.get_EntId_Vec_With_Comp<Tag>();
}

void print_intVec_comp_arr(verECS::CompMap map){

    std::span int_vec_span = map.get_Comp_Arr<std::vector<int>>();
    int size = int_vec_span.size();
    for(int i=0;i<size;++i){
        int vSize = int_vec_span[i].size();
        for(int j=0;j<vSize;++j){
            std::cout << int_vec_span[i][j] << '\n'; 
        }
    }
    std::cout << '\n';

}

void print_string_comp_arr(verECS::CompMap map){
    std::span s = map.get_Comp_Arr<String*>();
    int size = s.size();

    for(int i=0;i<size;++i){
        std::cout << s[i]->str << '\n';
    }
    std::cout << '\n';


}

int main(){
    verECS::ECS ecs;
    int ent1 = ecs.new_Entity(), ent2 = ecs.new_Entity();

    ecs.push_Comp<int>(1, ent1);
    ecs.push_Comp<Tag>((Tag){.tag=1}, ent1);
    ecs.push_Comp<int>(2, ent2);

    inc_int_comp_arr(ecs);
    print_int_comp_arr(ecs);

    ecs.delete_Comp<int>();
    ecs.delete_Comp<Tag>();

    return 0;
}