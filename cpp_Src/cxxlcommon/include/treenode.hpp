/***********************************************************
 * treenode.hpp 2.0.2
 *
 * 一個階層式的樹狀容器，每個節點可以包含一個可有可無物件，和它
 * 之下不限數量(也可以是 0)的子容器
 *
 * 採用 CRTP 架構，可由 TreeNodeBase 延伸出自定義類別
 * 嚴構規定每個節點都須有名稱，同一層的節點名稱不可重複
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

namespace CXXL
{
    // D: 延伸類別
    template <typename D>    
    class TreeNodeBase
    {    
        const std::u8string m_name;                // 節點名稱
        std::list<std::shared_ptr<D> > m_children; // 子節點列表

    public:
        // 建構函式
        explicit TreeNodeBase(const std::u8string &name) : m_name(name) 
        {}

        // 取得此節點名稱
        const std::u8string &getName() const { return m_name; }

        // 新增子節點（在尾端加入）
        // 回傳新增的子節點
        // 若子節點名稱未指定或已存在回傳 nullptr
        std::shared_ptr<D> addChild(const std::u8string &name)
        {
            if(name.empty())
            {
                return nullptr;
            }

            if(findChildByName(name) != nullptr)
            {
                return nullptr;
            }

            auto newChild = std::make_shared<D>(name);
            m_children.push_back(newChild);
            return newChild;
        }

        // 在特定子節點之前插入新節點
        // 若 childNode 不存在回覆 nullptr
        // 若子節點名稱未指定或已存在回傳 nullptr
        std::shared_ptr<D> insertBefore(const std::shared_ptr<D> &childNode, const std::u8string &name)
        {
            auto it = std::find(m_children.begin(), m_children.end(), childNode);
            if (it == m_children.end())
            {
                return nullptr;
            }

            if(name.empty())
            {
                return nullptr;
            }

            if(findChildByName(name) != nullptr)
            {
                return nullptr;
            }

            auto newChild = std::make_shared<D>(name);
            m_children.insert(it, newChild);
            return newChild;
        }

        // 在特定子節點之後插入新節點
        // 若 childNode 不存在回覆 nullptr
        // 若子節點名稱未指定或已存在回傳 nullptr
        std::shared_ptr<D> insertAfter(const std::shared_ptr<D> &childNode, const std::u8string &name)
        {
            auto it = std::find(m_children.begin(), m_children.end(), childNode);
            if (it == m_children.end())
            {
                return nullptr;
            }

            if(name.empty())
            {
                return nullptr;
            }

            if(findChildByName(name) != nullptr)
            {
                return nullptr;
            }

            auto newChild = std::make_shared<D>(name);
            m_children.insert(std::next(it), newChild);
            return newChild;
        }

        // 尋找特定名稱的子節點
        std::shared_ptr<D> findChildByName(const std::u8string &name) const
        {
            for (const auto &child : m_children)
            {
                if (child->getName() == name)
                {
                    return child;
                }
            }
            return nullptr;
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

    };
} // namespace CXXL
#endif // __CXXLCOMMON_TREENODE_HPP_CxxlMan3
