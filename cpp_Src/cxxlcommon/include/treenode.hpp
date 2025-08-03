/***********************************************************
 * treenode.hpp 2.2.7
 *
 * 一個階層式的樹狀容器，每個節點可以包含一個可有可無物件，和它
 * 之下不限數量(也可以是 0)的子容器
 *
 * 採用 CRTP 架構，可由 TreeNodeBase 延伸出自定義類別
 * 每個節點可以 重複 無名子節點，但若有名稱則只能有一個，即名稱不可重複
 * 
 * Author: CxxlMan
 * Date: 2025-
************************************************************/
#ifndef __CXXLCOMMON_TREENODE_HPP_CxxlMan3
#define __CXXLCOMMON_TREENODE_HPP_CxxlMan3

#include <list>
#include <memory>
#include <string>
#include <functional>

#include <commondef.hpp>

namespace CXXL
{

// 基底樹狀容器
// D: 為延伸類別
template <typename D>    
class TreeNodeBase
{    
private:
    const std::u8string m_name;
    std::list<std::shared_ptr<D>> m_children;  // 主要的子節點列表
    
    // 名稱索引：名稱 -> list iterator
    std::unordered_map<std::u8string, typename std::list<std::shared_ptr<D>>::iterator> m_nameIndex;
    
    // 子節點索引：shared_ptr -> list iterator  
    std::unordered_map<std::shared_ptr<D>, typename std::list<std::shared_ptr<D>>::iterator> m_childIndex;
    
    void traverse(const std::function<void(const D&, size_t)> &callback, size_t depth) const
    {
        callback(static_cast<const D&>(*this), depth);

        for (const auto &childNode : m_children)
            childNode->traverse(callback, depth + 1);
        
        callback(static_cast<const D&>(*this), 0);
    }
    
    void traverse(const std::function<void(D&, size_t)> &callback, size_t depth) 
    {
        callback(static_cast<D&>(*this), depth);

        for (auto &childNode : m_children)
            childNode->traverse(callback, depth + 1);
        
        callback(static_cast<D&>(*this), 0);
    }
    
public:
    explicit TreeNodeBase(const std::u8string &name) : m_name(name) {}
    
    const std::u8string &getName() const { return m_name; }
    
    // 新增子節點 - O(1) 時間複雜度
    std::shared_ptr<D> addChild(const std::u8string &name)
    {
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        auto newChild = std::make_shared<D>(name);
        auto it = m_children.insert(m_children.end(), newChild);
        
        // 更新子節點索引
        m_childIndex[newChild] = it;
        
        // 如果有名稱，更新名稱索引
        if (!name.empty()) 
            m_nameIndex[name] = it;
        
        return newChild;
    }
    
    // 在特定子節點之前插入新節點 - O(1) 時間複雜度
    std::shared_ptr<D> insertBefore(const std::shared_ptr<D> &childNode, const std::u8string &name)
    {
        // 透過子節點索引快速找到位置
        auto indexIt = m_childIndex.find(childNode);
        if (indexIt == m_childIndex.end())
            return nullptr;
            
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        auto newChild = std::make_shared<D>(name);
        auto newIt = m_children.insert(indexIt->second, newChild);
        
        // 更新索引
        m_childIndex[newChild] = newIt;
        if (!name.empty())
            m_nameIndex[name] = newIt;
        
        return newChild;
    }
    
    // 在特定子節點之後插入新節點 - O(1) 時間複雜度
    std::shared_ptr<D> insertAfter(const std::shared_ptr<D> &childNode, const std::u8string &name)
    {
        // 透過子節點索引快速找到位置
        auto indexIt = m_childIndex.find(childNode);
        if (indexIt == m_childIndex.end())
            return nullptr;
            
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        auto newChild = std::make_shared<D>(name);
        auto newIt = m_children.insert(std::next(indexIt->second), newChild);
        
        // 更新索引
        m_childIndex[newChild] = newIt;
        if (!name.empty())
            m_nameIndex[name] = newIt;
        
        return newChild;
    }
    
    // 按名稱查找子節點 - O(1) 時間複雜度
    std::shared_ptr<const D> findChildByName(const std::u8string &name) const
    {
        if(name.empty())
            return nullptr;
            
        auto it = m_nameIndex.find(name);
        if (it != m_nameIndex.end())
            return std::const_pointer_cast<const D>(*(it->second));

        return nullptr;
    }

    // 按名稱查找子節點 - O(1) 時間複雜度
    std::shared_ptr<D> findChildByName(const std::u8string &name)
    {
        if(name.empty())
            return nullptr;
            
        auto it = m_nameIndex.find(name);
        if (it != m_nameIndex.end()) 
            return *(it->second);

        return nullptr;
    }
    
    // 檢查指定名稱的子節點是否存在 - O(1) 時間複雜度
    bool hasChild(const std::u8string &name) const
    {
        if(name.empty())
            return false;
        return m_nameIndex.find(name) != m_nameIndex.end();
    }
    
    // 檢查指定子節點是否存在 - O(1) 時間複雜度
    bool hasChild(const std::shared_ptr<D> &child) const
    {
        return m_childIndex.find(child) != m_childIndex.end();
    }

    // 檢查指定子節點是否存在 - O(1) 時間複雜度
    bool hasChild(const std::shared_ptr<const D> &child) const
    {
        std::shared_ptr<D> tmp_ptr = std::const_pointer_cast<D>(child);
        return m_childIndex.find(tmp_ptr) != m_childIndex.end();
    }
    
    size_t childCount() const
    {
        return m_children.size();
    }
    
    // 移除子節點 - O(1) 時間複雜度
    bool removeChild(const std::shared_ptr<D> &child)
    {
        auto indexIt = m_childIndex.find(child);
        if (indexIt != m_childIndex.end())
        {
            auto listIt = indexIt->second;
            
            // 如果有名稱，從名稱索引中移除
            const auto &childName = (*listIt)->getName();
            if (!childName.empty()) {
                m_nameIndex.erase(childName);
            }
            
            // 從列表和子節點索引中移除
            m_children.erase(listIt);
            m_childIndex.erase(indexIt);
            return true;
        }

        return false;
    }
    
    // 按名稱移除子節點 - O(1) 時間複雜度
    bool removeChildByName(const std::u8string &name)
    {
        if (name.empty())
            return false;
            
        auto nameIt = m_nameIndex.find(name);
        if (nameIt != m_nameIndex.end()) 
        {
            auto listIt = nameIt->second;
            auto child = *listIt;
            
            // 從所有索引和列表中移除
            m_children.erase(listIt);
            m_childIndex.erase(child);
            m_nameIndex.erase(nameIt);
            return true;
        }

        return false;
    }
    
    // 移動子節點到指定位置之前 - O(1) 時間複雜度
    bool moveChildBefore(const std::shared_ptr<D> &childToMove, const std::shared_ptr<D> &targetChild)
    {
        auto moveIt = m_childIndex.find(childToMove);
        auto targetIt = m_childIndex.find(targetChild);
        
        if (moveIt == m_childIndex.end() || targetIt == m_childIndex.end())
            return false;
            
        if (childToMove == targetChild)
            return true;  // 相同節點，不需移動
            
        // 從原位置移除（但不刪除）
        auto listIt = moveIt->second;
        auto child = *listIt;
        m_children.erase(listIt);
        
        // 插入到新位置
        auto newIt = m_children.insert(targetIt->second, child);
        
        // 更新索引
        m_childIndex[childToMove] = newIt;
        
        // 如果有名稱，更新名稱索引
        const auto &childName = child->getName();
        if (!childName.empty()) 
            m_nameIndex[childName] = newIt;        
        
        return true;
    }
    
    // 移動子節點到指定位置之後 - O(1) 時間複雜度
    bool moveChildAfter(const std::shared_ptr<D> &childToMove, const std::shared_ptr<D> &targetChild)
    {
        auto moveIt = m_childIndex.find(childToMove);
        auto targetIt = m_childIndex.find(targetChild);
        
        if (moveIt == m_childIndex.end() || targetIt == m_childIndex.end())
            return false;
            
        if (childToMove == targetChild)
            return true;  // 相同節點，不需移動
            
        // 從原位置移除（但不刪除）
        auto listIt = moveIt->second;
        auto child = *listIt;
        m_children.erase(listIt);
        
        // 插入到新位置
        auto newIt = m_children.insert(std::next(targetIt->second), child);
        
        // 更新索引
        m_childIndex[childToMove] = newIt;
        
        // 如果有名稱，更新名稱索引
        const auto &childName = child->getName();
        if (!childName.empty())
            m_nameIndex[childName] = newIt;
        
        return true;
    }
    
    // 移動子節點到開頭 - O(1) 時間複雜度
    bool moveChildToFront(const std::shared_ptr<D> &child)
    {
        auto indexIt = m_childIndex.find(child);
        if (indexIt == m_childIndex.end())
            return false;
            
        auto listIt = indexIt->second;
        if (listIt == m_children.begin())
            return true;  // 已經在開頭
            
        auto childPtr = *listIt;
        m_children.erase(listIt);
        auto newIt = m_children.insert(m_children.begin(), childPtr);
        
        // 更新索引
        m_childIndex[child] = newIt;
        const auto &childName = childPtr->getName();
        if (!childName.empty())
            m_nameIndex[childName] = newIt;
        
        return true;
    }
    
    // 移動子節點到結尾 - O(1) 時間複雜度
    bool moveChildToBack(const std::shared_ptr<D> &child)
    {
        auto indexIt = m_childIndex.find(child);
        if (indexIt == m_childIndex.end())
            return false;
            
        auto listIt = indexIt->second;
        if (std::next(listIt) == m_children.end())
            return true;  // 已經在結尾
            
        auto childPtr = *listIt;
        m_children.erase(listIt);
        auto newIt = m_children.insert(m_children.end(), childPtr);
        
        // 更新索引
        m_childIndex[child] = newIt;
        const auto &childName = childPtr->getName();
        if (!childName.empty())
            m_nameIndex[childName] = newIt;
        
        return true;
    }
    
    // 取得子節點在列表中的位置（0-based index）- O(n) 時間複雜度
    std::optional<size_t> getChildPosition(const std::shared_ptr<D> &child) const
    {
        auto indexIt = m_childIndex.find(child);
        if (indexIt == m_childIndex.end())
            return std::nullopt;
            
        size_t pos = 0;
        for (auto it = m_children.begin(); it != indexIt->second; ++it, ++pos);
        return pos;
    }

    // 取得子節點在列表中的位置（0-based index）- O(n) 時間複雜度
    std::optional<size_t> getChildPosition(const std::shared_ptr<const D> &child) const
    {
        std::shared_ptr<D> tmp_ptr = std::const_pointer_cast<D>(child);
        return getChildPosition(tmp_ptr);
    }

    
    // 按位置取得子節點 - O(n) 時間複雜度
    std::shared_ptr<const D> getChildAt(size_t position) const
    {
        if (position >= m_children.size())
            return nullptr;
            
        auto it = m_children.begin();
        std::advance(it, position);
        return std::const_pointer_cast<const D>(*it);
    }

    // 按位置取得子節點 - O(n) 時間複雜度
    std::shared_ptr<D> getChildAt(size_t position)
    {
        if (position >= m_children.size())
            return nullptr;
            
        auto it = m_children.begin();
        std::advance(it, position);
        return *it;
    }
    
    // 取得 iterator，用於高效遍歷
    typename std::list<std::shared_ptr<D>>::iterator begin() { return m_children.begin(); }
    typename std::list<std::shared_ptr<D>>::iterator end() { return m_children.end(); }
    typename std::list<std::shared_ptr<D>>::const_iterator begin() const { return m_children.begin(); }
    typename std::list<std::shared_ptr<D>>::const_iterator end() const { return m_children.end(); }
    typename std::list<std::shared_ptr<D>>::const_iterator cbegin() const { return m_children.cbegin(); }
    typename std::list<std::shared_ptr<D>>::const_iterator cend() const { return m_children.cend(); }
    
    // 遍歷整棵樹(深度優先)        
    void traverse(const std::function<void(const D&, size_t)> &callback) const
    {
        callback(static_cast<const D&>(*this), 1);

        for (const auto &childNode : m_children)
            childNode->traverse(callback, 2);
        
        callback(static_cast<const D&>(*this), 0);
    }
    
    void traverse(const std::function<void(D&, size_t)> &callback) 
    {
        callback(static_cast<D&>(*this), 1);

        for (auto &childNode : m_children)
            childNode->traverse(callback, 2);
        
        callback(static_cast<D&>(*this), 0);
    }
    
    // 遍歷子節點(不含孫節點)
    void forEachChild(const std::function<void(const D&)> &callback) const
    {
        for (const auto &childNode : m_children)
            callback(*childNode);
    }
    
    void forEachChild(const std::function<void(D&)> &callback)
    {
        for (auto &childNode : m_children)
            callback(*childNode);
    }
    
    // 清空所有子節點 - O(n) 時間複雜度
    void clearChildren()
    {
        m_children.clear();
        m_nameIndex.clear();
        m_childIndex.clear();
    }
    
    // 偵錯用：檢查索引一致性
    bool validateIndexes() const
    {
        // 檢查子節點索引
        if (m_childIndex.size() != m_children.size())
            return false;
            
        for (const auto &child : m_children)
        {
            auto it = m_childIndex.find(child);
            if (it == m_childIndex.end() || *(it->second) != child)
                return false;
        }
        
        // 檢查名稱索引
        for (const auto &pair : m_nameIndex)
        {
            if ((*(pair.second))->getName() != pair.first)
                return false;
        }
        
        return true;
    }
};

// 自帶的實作
template <typename T>
class TreeNode:public TreeNodeBase<TreeNode<T> >
{        
    T m_data; // 節點資料

public:
    // 建構函式
    // TreeNodeBase 的要求
    explicit TreeNode(const std::u8string &name)
        : TreeNodeBase<TreeNode<T> >(name)
    {
    }

    // 建構函式
    explicit TreeNode(T &&data, const std::u8string &name)
        : TreeNodeBase<TreeNode<T> >(name), m_data(std::move(data))
    {
    }

    // 取得此節點資料
    T &getData() { return m_data; }
    const T &getData() const 
    { 
        return m_data; 
    }

    // 設定此節點資料
    void setData(T &&data) 
    { 
        m_data = std::move(data); 
    }
};


} // namespace CXXL
#endif // __CXXLCOMMON_TREENODE_HPP_CxxlMan3
