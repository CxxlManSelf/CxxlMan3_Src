/***********************************************************
 * treenode.hpp 2.3.14
 *
 * 一個階層式的樹狀容器，每個節點可以包含一個可有可無的物件，和它
 * 之下不限數量(也可以是 0)的子容器。
 *
 * 採用 CRTP 架構，可由 TreeNodeBase 延伸出自定義類別。
 * 每個節點可以 重複 無名子節點，但若有名稱則只能有一個，即名稱不可重複
 * 
 * Author: CxxlMan
 * Date: 2025-
************************************************************/
#ifndef __CXXLCOMMON_TREENODE_HPP_CxxlMan3
#define __CXXLCOMMON_TREENODE_HPP_CxxlMan3

#include <list>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <optional>

#include <sysdef.hpp>
#include <commondef.hpp>

namespace CXXL
{

// 基底樹狀容器
// D: 為延伸類別
template <typename D>    
class TreeNodeBase
{    
private:

    using NODE_PTR = std::shared_ptr<D>;
    using CNODE_PTR = std::shared_ptr<const D>;

    const std::u8string m_name;
    std::list<NODE_PTR> m_children;  // 主要的子節點列表
    
    // 名稱索引：名稱 -> shared_ptr
    std::unordered_map<std::u8string, NODE_PTR> m_nameIndex;    

    // 子節點索引：D* -> list iterator  
    std::unordered_map<D*, typename std::list<NODE_PTR>::iterator> m_childIndex;
    
    // 父節點
    std::weak_ptr<D> m_parent;

    // 自己
    std::weak_ptr<D> m_self;

    // 設定父節點和自己
    void setParentAndSelf(const NODE_PTR &parent, const NODE_PTR &self)
    {
        m_parent = parent;
        m_self = self;
    }

    void cxxlFASTCALL traverse(const std::function<void(const D&, size_t)> &callback, size_t depth) const
    {
        callback(static_cast<const D&>(*this), depth);

        for (const auto &childNode : m_children)
            childNode->traverse(callback, depth + 1);
        
        callback(static_cast<const D&>(*this), 0);
    }
    
    void cxxlFASTCALL traverse(const std::function<void(D&, size_t)> &callback, size_t depth) 
    {
        callback(static_cast<D&>(*this), depth);

        for (auto &childNode : m_children)
            childNode->traverse(callback, depth + 1);
        
        callback(static_cast<D&>(*this), 0);
    }
protected:    
    // Constructor
    explicit TreeNodeBase(const std::u8string &name) : m_name(name) {}
    
public:
    
    // 取得節點名稱
    const std::u8string& cxxlFASTCALL getName() const { return m_name; }

    // 取得父節點
    NODE_PTR cxxlFASTCALL getParent() const 
    { 
        return m_parent.lock(); 
    }
    


    // 新增子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL addChild(const std::u8string &name=u8"")
    {
        return addBackChild(name);
    }

    // 新增最前子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL addFrontChild(const std::u8string &name=u8"") 
    { 
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        NODE_PTR newChild(new D(name));
        auto it = m_children.insert(m_children.begin(), newChild);
        
        // 更新子節點索引
        m_childIndex[newChild.get()] = it;
        
        // 如果有名稱，更新名稱索引
        if (!name.empty()) 
            m_nameIndex[name] = newChild;

        newChild->setParentAndSelf(m_self.lock(), newChild);

        return newChild;
    }
    
    // 新增最後子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL addBackChild(const std::u8string &name=u8"") 
    { 
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        NODE_PTR newChild(new D(name));
        auto it = m_children.insert(m_children.end(), newChild);
        
        // 更新子節點索引
        m_childIndex[newChild.get()] = it;
        
        // 如果有名稱，更新名稱索引
        if (!name.empty()) 
            m_nameIndex[name] = newChild;

        newChild->setParentAndSelf(m_self.lock(), newChild);

        return newChild;
    }

    // 在特定子節點之前插入新節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL insertBefore(const NODE_PTR &childNode, const std::u8string &name)
    {
        // 透過子節點索引快速找到位置
        auto indexIt = m_childIndex.find(childNode.get());
        if (indexIt == m_childIndex.end())
            return nullptr;
            
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        NODE_PTR newChild(new D(name));
        auto newIt = m_children.insert(indexIt->second, newChild);
        
        // 更新索引
        m_childIndex[newChild.get()] = newIt;
        if (!name.empty())
            m_nameIndex[name] = newChild;
        
        newChild->setParentAndSelf(m_self.lock(), newChild);

        return newChild;
    }
    
    // 在特定子節點之後插入新節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL insertAfter(const NODE_PTR &childNode, const std::u8string &name)
    {
        // 透過子節點索引快速找到位置
        auto indexIt = m_childIndex.find(childNode.get());
        if (indexIt == m_childIndex.end())
            return nullptr;
            
        // 若有節點名稱則不可重複
        if(!name.empty() && hasChild(name))
            return nullptr;
            
        NODE_PTR newChild(new D(name));
        auto newIt = m_children.insert(std::next(indexIt->second), newChild);
        
        // 更新索引
        m_childIndex[newChild.get()] = newIt;
        if (!name.empty())
            m_nameIndex[name] = newChild;
        
        newChild->setParentAndSelf(m_self.lock(), newChild);
        
        return newChild;
    }
    
    

    // 檢查指定名稱的子節點是否存在 - O(1) 時間複雜度
    bool cxxlFASTCALL hasChild(const std::u8string &name) const
    {
        //if(name.empty())
        //    return false;
        return m_nameIndex.find(name) != m_nameIndex.end();
    }
    
    // 檢查指定子節點是否存在 - O(1) 時間複雜度
    bool cxxlFASTCALL hasChild(const NODE_PTR &child) const
    {
        return m_childIndex.find(child.get()) != m_childIndex.end();
    }

    // 檢查指定子節點是否存在 - O(1) 時間複雜度
    bool cxxlFASTCALL hasChild(const CNODE_PTR &child) const
    {        
        return m_childIndex.find((D*)child.get()) != m_childIndex.end();
    }
        
    // 取得子節點數量
    size_t cxxlFASTCALL childCount() const
    {
        return m_children.size();
    }
    

    // 移除子節點 - O(1) 時間複雜度
    bool cxxlFASTCALL removeChild(const NODE_PTR &child)
    {
        auto indexIt = m_childIndex.find(child.get());
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
    bool cxxlFASTCALL removeChildByName(const std::u8string &name)
    {
        //if (name.empty())
        //    return false;
            
        auto nameIt = m_nameIndex.find(name);
        if (nameIt != m_nameIndex.end()) 
        {
            NODE_PTR child_ptr = nameIt->second;            
            return removeChild(child_ptr);
        }

        return false;
    }

    // 移除第一個子節點 - O(1) 時間複雜度
    bool cxxlFASTCALL removeFrontChild()
    {
        if (m_children.empty())
            return false;
            
        NODE_PTR child = m_children.front();
        return removeChild(child);
    }

    // 移除最後一個子節點 - O(1) 時間複雜度
    bool cxxlFASTCALL removeBackChild()
    {
        if (m_children.empty())
            return false;
            
        NODE_PTR child = m_children.back();
        return removeChild(child);
    }
    
    // 清空所有子節點 - O(n) 時間複雜度
    void cxxlFASTCALL clearChildren()
    {
        m_children.clear();
        m_nameIndex.clear();
        m_childIndex.clear();
    }


    // 移動子節點到開頭 - O(1) 時間複雜度
    bool cxxlFASTCALL moveChildToFront(const NODE_PTR &child)
    {
        auto indexIt = m_childIndex.find(child.get());
        if (indexIt == m_childIndex.end())
            return false;
            
        auto listIt = indexIt->second;
        if (listIt == m_children.begin())
            return true;  // 已經在開頭
            
        auto childPtr = *listIt;
        m_children.erase(listIt);
        auto newIt = m_children.insert(m_children.begin(), childPtr);
        
        // 更新索引
        m_childIndex[child.get()] = newIt;
        
        return true;
    }
    
    // 移動子節點到結尾 - O(1) 時間複雜度
    bool cxxlFASTCALL moveChildToBack(const NODE_PTR &child)
    {
        auto indexIt = m_childIndex.find(child.get());
        if (indexIt == m_childIndex.end())
            return false;
            
        auto listIt = indexIt->second;
        if (std::next(listIt) == m_children.end())
            return true;  // 已經在結尾
            
        auto childPtr = *listIt;
        m_children.erase(listIt);
        auto newIt = m_children.insert(m_children.end(), childPtr);
        
        // 更新索引
        m_childIndex[child.get()] = newIt;
        
        return true;
    }
    
    // 移動子節點到指定位置之前 - O(1) 時間複雜度
    bool cxxlFASTCALL moveChildBefore(const NODE_PTR &childToMove, const NODE_PTR &targetChild)
    {
        auto moveIt = m_childIndex.find(childToMove.get());
        auto targetIt = m_childIndex.find(targetChild.get());
        
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
        m_childIndex[childToMove.get()] = newIt;
       
        return true;
    }
    
    // 移動子節點到指定位置之後 - O(1) 時間複雜度
    bool cxxlFASTCALL moveChildAfter(const NODE_PTR &childToMove, const NODE_PTR &targetChild)
    {
        auto moveIt = m_childIndex.find(childToMove.get());
        auto targetIt = m_childIndex.find(targetChild.get());
        
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
        m_childIndex[childToMove.get()] = newIt;
        
        return true;
    }
  

    // 取得子節點在列表中的位置（0-based index）- O(n) 時間複雜度
    std::optional<size_t> cxxlFASTCALL getChildPosition(const NODE_PTR &child) const
    {
        auto indexIt = m_childIndex.find(child.get());
        if (indexIt == m_childIndex.end())
            return std::nullopt;
            
        size_t pos = 0;
        for (auto it = m_children.begin(); it != indexIt->second; ++it, ++pos);
        return pos;
    }

    // 取得子節點在列表中的位置（0-based index）- O(n) 時間複雜度
    std::optional<size_t> cxxlFASTCALL getChildPosition(const CNODE_PTR &child) const
    {
        NODE_PTR tmp_ptr = std::const_pointer_cast<D>(child);
        return getChildPosition(tmp_ptr);
    }
   
    // 按位置取得子節點 - O(n) 時間複雜度
    CNODE_PTR cxxlFASTCALL getChildAt(size_t position) const
    {
        if (position >= m_children.size())
            return nullptr;
            
        auto it = m_children.begin();
        std::advance(it, position);
        return std::const_pointer_cast<const D>(*it);
    }

    // 按位置取得子節點 - O(n) 時間複雜度
    NODE_PTR cxxlFASTCALL getChildAt(size_t position)
    {
        if (position >= m_children.size())
            return nullptr;
            
        auto it = m_children.begin();
        std::advance(it, position);
        return *it;
    }


    // 按名稱查找子節點 - O(1) 時間複雜度
    CNODE_PTR cxxlFASTCALL findChildByName(const std::u8string &name) const
    {
        // if(name.empty())
        //    return nullptr;
            
        auto it = m_nameIndex.find(name);
        if (it != m_nameIndex.end())
            return std::const_pointer_cast<const D>(it->second);

        return nullptr;
    }

    // 按名稱查找子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL findChildByName(const std::u8string &name)
    {
      //  if(name.empty())
      //      return nullptr;
            
        auto it = m_nameIndex.find(name);
        if (it != m_nameIndex.end()) 
            return (it->second);

        return nullptr;
    }
    
    // 取得第一個子節點 - O(1) 時間複雜度
    CNODE_PTR cxxlFASTCALL getFirstChild() const
    {
        // if (m_children.empty())
        //    return nullptr;
            
        return std::const_pointer_cast<const D>(*m_children.begin());
    }
    
    // 取得第一個子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL getFirstChild()
    {
        //if (m_children.empty())
        //    return nullptr;
            
        return *m_children.begin();
    }

    // 取得最後一個子節點 - O(1) 時間複雜度
    CNODE_PTR cxxlFASTCALL getLastChild() const
    {
        //if (m_children.empty())
        //    return nullptr;
            
        return std::const_pointer_cast<const D>(*m_children.rbegin());
    }

    // 取得最後一個子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL getLastChild()
    {
        //if (m_children.empty())
        //    return nullptr;
            
        return *m_children.rbegin();
    }

    // 取得指定子節點的下一個子節點 - O(1) 時間複雜度
    CNODE_PTR cxxlFASTCALL getNextChild(const CNODE_PTR &child) const
    {
        auto it = m_childIndex.find((D*)child.get());
        if (it == m_childIndex.end())
            return nullptr;
            
        auto nextIt = std::next(it->second);
        if (nextIt == m_children.end())
            return nullptr;
            
        return std::const_pointer_cast<const D>(*nextIt);
    }

    // 取得指定子節點的下一個子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL getNextChild(const NODE_PTR &child)
    {
        auto it = m_childIndex.find(child.get());
        if (it == m_childIndex.end())
            return nullptr;
            
        auto nextIt = std::next(it->second);
        if (nextIt == m_children.end())
            return nullptr;
            
        return *nextIt;        
    }

    // 取得指定子節點的上一個子節點 - O(1) 時間複雜度
    CNODE_PTR cxxlFASTCALL getPreviousChild(const CNODE_PTR &child) const
    {
        auto it = m_childIndex.find((D*)child.get());
        if (it == m_childIndex.end())
            return nullptr;

        auto childIt = it->second;
        if(childIt == m_children.begin())
            return nullptr;

        auto prevIt = std::prev(childIt);
        return std::const_pointer_cast<const D>(*prevIt);
    }

    // 取得指定子節點的上一個子節點 - O(1) 時間複雜度
    NODE_PTR cxxlFASTCALL getPreviousChild(const NODE_PTR &child)
    {
        auto it = m_childIndex.find(child.get());
        if (it == m_childIndex.end())
            return nullptr;

        auto childIt = it->second;
        if(childIt == m_children.begin())
            return nullptr;

        auto prevIt = std::prev(childIt);
        return *prevIt;            
    }
    


    // 遍歷整棵樹(深度優先)
    void cxxlFASTCALL traverse(const std::function<void(const D&, size_t)> &callback) const
    {
        callback(static_cast<const D&>(*this), 1);

        for (const auto &childNode : m_children)
            childNode->traverse(callback, 2);
        
        callback(static_cast<const D&>(*this), 0);
    }
    
    // 遍歷整棵樹(深度優先)
    void cxxlFASTCALL traverse(const std::function<void(D&, size_t)> &callback) 
    {
        callback(static_cast<D&>(*this), 1);

        for (auto &childNode : m_children)
            childNode->traverse(callback, 2);
        
        callback(static_cast<D&>(*this), 0);
    }
    
    // 遍歷子節點(不含孫節點)
    void cxxlFASTCALL forEachChild(const std::function<void(const CNODE_PTR&)> &callback) const
    {
        for (const auto &childNode : m_children)
            callback(std::const_pointer_cast<const D>(childNode));
    }
    
    void cxxlFASTCALL forEachChild(const std::function<void(const NODE_PTR&)> &callback)
    {
        for (auto &childNode : m_children)
            callback(childNode);
    }
    
    
    // 偵錯用：檢查索引一致性
    bool cxxlFASTCALL validateIndexes() const
    {
        // 檢查子節點索引
        if (m_childIndex.size() != m_children.size())
            return false;
            
        for (const auto &child : m_children)
        {
            auto it = m_childIndex.find(child.get());
            if (it == m_childIndex.end() || *(it->second) != child)
                return false;
        }
        
        // 檢查名稱索引
        for (const auto &pair : m_nameIndex)
        {            
            const NODE_PTR &child = pair.second;
            if (m_childIndex.find(child.get()) == m_childIndex.end())
                return false;
            
            if (child->getName() != pair.first)
                return false;
        }
        
        return true;
    }


    // 取得 iterator，用於高效遍歷
    auto cxxlFASTCALL begin() { return m_children.begin(); }
    auto cxxlFASTCALL end() { return m_children.end(); }
    auto cxxlFASTCALL rbegin() { return m_children.rbegin(); }
    auto cxxlFASTCALL rend() { return m_children.rend(); }
    
    // 取得 const iterator，用於高效遍歷
    // 不能取得 shared_ptr<const D>
    //auto cxxlFASTCALL cbegin() const { return m_children.cbegin(); }
    //auto cxxlFASTCALL cend() const { return m_children.cend(); }
    //auto cxxlFASTCALL crbegin() const { return m_children.crbegin(); }
    //auto cxxlFASTCALL crend() const { return m_children.crend(); }

    // 創建根節點
    static NODE_PTR cxxlFASTCALL createRoot(const std::u8string &name)
    {
        NODE_PTR root(new D(name));
        root->m_self = root;
        return root;
    }
};

// 自帶的實作
template <typename T>
class TreeNode:public TreeNodeBase<TreeNode<T> >
{        
    T m_data; // 節點資料

protected:
    // 建構函式
    // TreeNodeBase 的要求
    explicit TreeNode(const std::u8string &name)
        : TreeNodeBase<TreeNode<T> >(name)
    {
    }

public:


    // 取得此節點資料
    T& cxxlFASTCALL getData()
    { 
        return m_data; 
    }
    const T &getData() const 
    { 
        return m_data; 
    }

    // 設定此節點資料
    // 用 std::copy
    void cxxlFASTCALL setData(T &data) 
    { 
        m_data = data; 
    }

    // 設定此節點資料
    // 用 std::move
    void cxxlFASTCALL setData(T &&data) 
    { 
        m_data = std::move(data); 
    }

    // 創建根節點
    static std::shared_ptr<TreeNode<T> > cxxlFASTCALL createRoot(const std::u8string &name)
    {
        return TreeNodeBase<TreeNode<T> >::createRoot(name);
    }      

    template <typename D>    
    friend class TreeNodeBase;
};


} // namespace CXXL
#endif // __CXXLCOMMON_TREENODE_HPP_CxxlMan3
