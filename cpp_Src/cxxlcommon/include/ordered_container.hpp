/*****************************************************************
 * ordered_container.hpp v1.0.0
 *
 * 結合了順序和索引的功能。這個容器使用std::list來維護加入的順序，同時
 * 使用std::unordered_map來存儲物件的位址作為索引。
 *
 * Author: CxxlMan
 * Date: 2025-
 ******************************************************************/
#ifndef __CXXLCOMMON_ORDERED_CONTAINER_HPP_CxxlMan3
#define __CXXLCOMMON_ORDERED_CONTAINER_HPP_CxxlMan3

#include <list>
#include <unordered_map>

#include "commondef.hpp"

namespace CXXL
{
    template <typename T>
    class OrderedContainer
    {
        // 使用std::list來維護加入的順序
        std::list<T> m_orderedList;

        // 使用std::unordered_map來存儲物件的位址作為索引
        std::unordered_map<T *, typename std::list<T>::iterator> m_map;

    public:
        OrderedContainer() {}
        ~OrderedContainer() {}

        using iterator = typename std::list<T>::iterator;

        // 加入一個新物件
        void cxxlFASTCALL add(const T &value)
        {
            auto it = m_map.find(&value);
            if (it == m_map.end())
            {
                m_orderedList.push_back(value);
                m_map[&m_orderedList.back()] = std::prev(m_orderedList.end());
            }
        }

        // 刪除一個物件
        void cxxlFASTCALL remove(const T &value)
        {
            auto it = m_map.find(&value);
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

        // 取得指定索引的物件
        T& cxxlFASTCALL at(size_t index)
        {
            return m_orderedList.at(index);
        }

        // 迭代器支援       
        iterator cxxlFASTCALL begin() { return m_orderedList.begin(); }
        iterator cxxlFASTCALL end() { return m_orderedList.end(); }
    };

} // namespace CXXL
#endif // __CXXLCOMMON_ORDERED_CONTAINER_HPP_CxxlMan3
