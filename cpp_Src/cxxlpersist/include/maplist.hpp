/*************************************************************
 * maplist.hpp v0.1.0
 * 
 * 結合了順序和索引的功能。這個容器使用std::list來維護加入的順序，同時
 * 使用std::unordered_map來存儲物件的位址作為索引。
 * 專為 class ChildLinkSet 設計
 * 
 * Author: CxxlMan
 * Date: 2025-
**************************************************************/
#ifndef __CXXLPERSIST_MAPLIST_HPP_CxxlMan3
#define __CXXLPERSIST_MAPLIST_HPP_CxxlMan3

#include <list>
#include <unordered_map>

#include "commondef.hpp"


namespace CXXL
{
    class ChildLinkSet;

    template <typename T, typename HOLDER> // HOLDER 為 UniOwner<T>
    class MapList
    {
        // 使用std::list來維護加入的順序
        std::list<HOLDER> m_orderedList;

        // 使用std::unordered_map來存儲物件的位址作為索引
        std::unordered_map<T *, typename std::list<T>::iterator> m_map;

        using iterator = typename std::list<HOLDER>::iterator;

        // 加入一個新物件
        // 同一個物件只能加入一次
        bool cxxlFASTCALL add(const HOLDER &&holder)
        {
            auto uniBase_ptr = holder.getUniBase();
            T *pUniBase = (T *)uniBase_ptr.get();
            auto it = m_map.find(pUniBase);
            if (it == m_map.end())
            {
                m_orderedList.push_back(value);
                m_map[&m_orderedList.back()] = std::prev(m_orderedList.end());
                return true;
            }
            else
                return false;
        }

        // 刪除一個物件
        void cxxlFASTCALL remove(const T *pUniBase)
        {
            auto it = m_map.find(pUniBase);
            if (it != m_map.end())
            {
                m_orderedList.erase(it->second);
                m_map.erase(it);
            }
        }

        // 取得容器中的物件數量
        size_t size() const
        {
            return m_orderedList.size();
        }

        // 清除所有物件
        void cxxlFASTCALL clear()
        {
            m_orderedList.clear();
            m_map.clear();
        }

        // 迭代器支援       
        iterator cxxlFASTCALL begin() { return m_orderedList.begin(); }
        iterator cxxlFASTCALL end() { return m_orderedList.end(); }

    public:
        MapList() = default;
        ~MapList() = default;

        friend class ChildLinkSet;
    };


} // namespace CXXL

#endif // __CXXLPERSIST_MAPLIST_HPP_CxxlMan3