#ifndef __verECS_H__
#define __verECS_H__

#include <span>
#include <vector>
#include <unordered_map>
#include <typeindex>


namespace verECS{

    struct CompAr{
        void *arr_;
        int cap_;
        int len_;
        std::vector<int> idVec_;

        CompAr(){
            arr_ = nullptr;
            cap_ = 0;
            len_ = 0;
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
            idVec_.push_back(entId);
            len_++;

            return len_-1;
        }

    };

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
        std::span<T> get_Comp_Span(){
            auto iter = map_.find(typeid(T));
            std::span<T> span;

            if(iter != map_.end()){
                CompAr &cAr = map_.at(typeid(T));
                span = std::span((T*)cAr.arr_,cAr.len_);
            }

            return span;
        }

        template<class T>
        CompAr helper_Get_Comp_Ar(){
            CompAr cAr;
            if(map_.find(typeid(T)) != map_.end())
                cAr = map_.at(typeid(T)); 
            return cAr;
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
        int get_Comp_Ind(){
            return compIdMap_.at(typeid(T));
        }

        template<class T>
        bool has_Comp(){
            return compIdMap_.find(typeid(T)) != compIdMap_.end();
        }

        bool has_Comp_Type_Ind(std::type_index tInd){
            return compIdMap_.find(tInd) != compIdMap_.end();
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
        bool entity_Has_Comp(int entId){
            return entList_[entId].has_Comp<T>();
        }

        bool entity_Has_Comp_Type_Ind(int entId, std::type_index tInd){
            return entList_[entId].has_Comp_Type_Ind(tInd);
        }

        template<class T>
        bool push_Comp(T val, int ent){
            bool bl = false;
            if((unsigned int)ent < entList_.size()){
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
                std::span<T> cSpan = compMap_.get_Comp_Span<T>();
                Entity ent = entList_[entId];
                int ind = ent.get_Comp_Ind<T>();
                r = &cSpan[ind];
            }

            return r;
        }

        // buscar array de componentes, filtrar se as entities não estão associadas a todos os Ts
        // devolver array
        // initializer list para nº variável de argumentos
        template<class T0>
        std::vector<T0*> get_Comp_Vec_Associated_To(std::initializer_list<std::type_index> tArgList){
            CompAr cAr = compMap_.helper_Get_Comp_Ar<T0>();
            std::vector<T0*> ret;
            std::vector<int> idVec;
            std::vector<std::type_index> tList = tArgList;
            unsigned int tInd=0;
            if(cAr.len_ > 0 && tList.size() != 0){
                for(int i=0;i<cAr.len_;++i){
                    if(entity_Has_Comp_Type_Ind(cAr.idVec_[i], tList[tInd])){
                        ret.push_back(&((T0*)cAr.arr_)[i]);
                        idVec.push_back(cAr.idVec_[i]);
                    }
                }
                for(tInd=1; tInd<tList.size();++tInd){
                    int size = ret.size();
                    std::vector<T0*> filtered;
                    std::vector<int> nIdVec;
                    for(int i=0;i<size;++i){
                        if(entity_Has_Comp_Type_Ind(idVec[i], tList[tInd])){
                            filtered.push_back(ret[i]);
                            nIdVec.push_back(idVec[i]);
                        }
                    }
                    ret = filtered;
                    idVec = nIdVec;
                }
            }

            return ret;
        }

        template<class T>
        std::span<T> get_Comp_Span(){
            return compMap_.get_Comp_Span<T>();
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


#endif