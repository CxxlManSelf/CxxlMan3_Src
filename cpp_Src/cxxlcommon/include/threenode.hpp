/***********************************************************
 * threenode.hpp 0.1.0
 *
 * 一個階層式的樹狀容器，每個節點可以包含一個物件，和它
 * 之下不限數量的子容器
 *
 * Author: CxxlMan
 * Date: 2025-
 ************************************************************/
#ifndef __CXXLCOMMON_THREENODE_HPP_CxxlMan3
#define __CXXLCOMMON_THREENODE_HPP_CxxlMan3

#include <list>
#include <memory>

namespace CXXL
{
    template <typename T>
    class ThreeNode
    {
        T m_data;                                           // 節點資料
        std::list<std::shared_ptr<TreeNode<T>>> m_children; // 子節點列表
    public:
        // 建構函式
        explicit TreeNode(const T &value) : m_data(value) {}

        // 取得節點資料
        T &getData() { return m_data; }
        const T &getData() const { return m_data; }

        // 設定節點資料
        void setData(const T &value) { m_data = value; }

        // 尋找特定值的子節點
        std::shared_ptr<TreeNode<T>> findChild(const T &value) const
        {
            for (const auto &child : m_children)
            {
                if (child->getData() == value)
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

        // 新增子節點（在尾端加入）
        std::shared_ptr<TreeNode<T>> addChild(const T &value)
        {
            auto newChild = std::make_shared<TreeNode<T>>(value);
            m_children.push_back(newChild);
            return newChild;
        }

        // 在特定子節點之前插入新節點
        // 若 child 不存在回覆 nullptr
        std::shared_ptr<TreeNode<T>> insertBefore(const std::shared_ptr<TreeNode<T>> &child, const T &value)
        {
            auto it = std::find(m_children.begin(), m_children.end(), child);
            if (it == m_children.end())
            {
                return nullptr;
            }

            auto newChild = std::make_shared<TreeNode<T>>(value);
            m_children.insert(it, newChild);
            return newChild;
        }

        // 在特定子節點之後插入新節點
        // 若 child 不存在回覆 nullptr
        std::shared_ptr<TreeNode<T>> insertBefore(const std::shared_ptr<TreeNode<T>> &child, const T &value)
        {
            auto it = std::find(m_children.begin(), m_children.end(), child);
            if (it == m_children.end())
            {
                return nullptr;
            }

            auto newChild = std::make_shared<TreeNode<T>>(value);
            m_children.insert(std::next(it), newChild);
            return newChild;
        }

        // 移除子節點
        // 若 child 不存在回覆 false
        bool removeChild(const std::shared_ptr<TreeNode<T>> &child)
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
        const std::list<std::shared_ptr<TreeNode<T>>> &getChildren() const
        {
            return m_children;
        }

        // 遍歷整棵樹
        void traverse(const std::function<void(const TreeNode<T> &, int)> &callback, int depth = 0) const
        {
            callback(*this, depth);
            for (const auto &child : m_children)
            {
                child->traverse(callback, depth + 1);
            }
        }
    };
} // namespace CXXL
#endif // __CXXLCOMMON_THREENODE_HPP_CxxlMan3
