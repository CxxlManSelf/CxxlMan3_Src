/***********************************************************
 * treenode.hpp 2.1.5
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
    // D: 延伸類別
    template <typename D>    
    class TreeNodeBase
    {    
        // 節點名稱
        // 可以為空，可不只一個空名節點
        // 若不為空則不可重複
        const std::u8string m_name;                
        std::list<std::shared_ptr<D> > m_children; // 子節點列表

    public:
        // 建構函式
        explicit TreeNodeBase(const std::u8string &name) : m_name(name) 
        {}

        // 取得此節點名稱
        const std::u8string &getName() const { return m_name; }

        // 新增子節點（在尾端加入）
        // 回傳新增的子節點
        // 子節點名稱可為空，若子節點名稱有指定且已存在回傳 nullptr
        std::shared_ptr<D> addChild(const std::u8string &name)
        {
            // 若有節點名稱則不可重複
            if(!name.empty() && hasChild(name))
                return nullptr;

            auto newChild = std::make_shared<D>(name);
            m_children.push_back(newChild);
            return std::move(newChild);
        }

        // 在特定子節點之前插入新節點
        // 若 childNode 不存在回覆 nullptr
        // 子節點名稱可為空，若子節點名稱有指定且已存在回傳 nullptr
        std::shared_ptr<D> insertBefore(const std::shared_ptr<D> &childNode, const std::u8string &name)
        {
            // 檢查指定的子節點是否存在
            auto it = std::find(m_children.begin(), m_children.end(), childNode);
            if (it == m_children.end())
                return nullptr;

            // 若有節點名稱則不可重複
            if(!name.empty() && hasChild(name))
                return nullptr;

            std::shared_ptr<D> newChild = std::make_shared<D>(name);
            m_children.insert(it, newChild);
            return std::move(newChild);
        }

        // 在特定子節點之後插入新節點
        // 若 childNode 不存在回覆 nullptr
        // 子節點名稱可為空，若子節點名稱有指定且已存在回傳 nullptr
        std::shared_ptr<D> insertAfter(const std::shared_ptr<D> &childNode, const std::u8string &name)
        {
            // 檢查指定的子節點是否存在
            auto it = std::find(m_children.begin(), m_children.end(), childNode);
            if (it == m_children.end())
                return nullptr;

            // 若有節點名稱則不可重複
            if(!name.empty() && hasChild(name))
                return nullptr;

            std::shared_ptr<D> newChild = std::make_shared<D>(name);
            m_children.insert(std::next(it), newChild);
            return std::move(newChild);
        }

        // 尋找特定名稱的子節點
        std::shared_ptr<D> findChildByName(const std::u8string &name) const
        {
            if(name.empty())
                return nullptr;
                
            for (const auto &child : m_children)
            {
                if (child->getName() == name)
                    return child;
            }
            return nullptr;
        }

        // 檢查指定名稱的子節點是否存在
        bool hasChild(const std::u8string &name) const
        {
            if(name.empty())
                return false;

            for (const auto &child : m_children)
            {
                if (child->getName() == name)
                    return true;
            }
                
            return false;
        }

        // 取得子節點數量
        size_t childCount() const
        {
            return m_children.size();
        }

        // 移除子節點
        // 若 child 不存在回覆 false
        bool removeChild(const std::shared_ptr<D> &child)
        {
            auto it = std::find(m_children.begin(), m_children.end(), child);
            if (it != m_children.end())
            {
                m_children.erase(it);
                return true;
            }
            return false;
        }

        // 取得所有子節點
        const std::list<std::shared_ptr<D> > &getChildren() const
        {
            return m_children;
        }

        // 遍歷整棵樹(深度優先)        
        // depth: 節點深度，第一層為 1
        // callback: 回呼函式，depth 為 0 表示返回上一層
        void traverse(const std::function<void(const D&, size_t)> &callback, size_t depth = 1) const
        {
            callback(*this, depth);
            for (const auto &childNode : m_children)
            {
                childNode->traverse(callback, depth + 1);
            }
            callback(*this, 0);
        }
        
    };

    // 
    template <typename T>
    class TreeNode:public TreeNodeBase<TreeNode<T> >
    {        
        T m_data;                                // 節點資料

    public:
        // 建構函式
        // TreeNodeBase 的要求
        explicit TreeNode(const std::u8string &name)
            : TreeNodeBase<TreeNode<T> >(name)
        {
        }

        // 建構函式
        explicit TreeNode(const T &value, const std::u8string &name)
            : TreeNodeBase<TreeNode<T> >(name), m_data(value)
        {
        }

        // 取得此節點資料
        T &getData() { return m_data; }
        const T &getData() const { return m_data; }

        // 設定此節點資料
        void setData(const T &value) { m_data = value; }

        // 設定此節點資料
        void setData(T &&value) { m_data = std::move(value); }

    };
} // namespace CXXL
#endif // __CXXLCOMMON_TREENODE_HPP_CxxlMan3
