#pragma once
#include <iostream>
#include "flat_hash_map.h"

#ifdef _DEBUG
#define TIM_ASSERT(cond) \
    do { if(!(cond)) { tim::handleAssert(__LINE__, __FILE__, ""); } }while(0) 

#else
#define TIM_ASSERT(cond)
#endif

#define TIM_FRAME_LATENCY 2

namespace tim
{
    void handleAssert(int _line, const char* _file, const char* _msg);

    template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>, typename A = std::allocator<std::pair<K, V> > >
    using flat_hash_map = ska::flat_hash_map<K,V,H,E,A>;

    template<typename T, typename H = std::hash<T>, typename E = std::equal_to<T>, typename A = std::allocator<T> >
    using flat_hash_set = ska::flat_hash_set<T,H,E,A>;
}
