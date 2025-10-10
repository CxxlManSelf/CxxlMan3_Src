/***********************************************************
 * treenode.hpp 2.4.22
 *
 * 一個階層式的樹狀容器，每個節點可以包含
 * 一個可有可無物件，和它之下不限數量(也
 * 可以是 0)的子容器
 *
 * 採用 CRTP 架構，可由 TreeNodeBase 延伸
 * 出自定義類別。
 *
 * 每個節點可以 重複 無名子節點，但若有
 * 名稱則只能有一個，即名稱不可重複
 *
 * TreeNodeBase<D> 在建立新的子節點前，會先
 * 呼叫 D 的 canCreateChild 函式，延伸類
 * 別可視須要加入自己的檢查，以及呼叫上層類別
 *
 * 為了避免解構時同時解構子孫節點造成堆
 * 疊爆掉的問題，將解構的任務交由線程池
 * 來執行
 *
 * Thread-Safe Version - 使用 shared_mutex 提供讀寫鎖機制
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
#include <iostream>
#include <shared_mutex>
#include <condition_variable>
#include <queue>

#include "sysdef.hpp"
#include "commondef.hpp"

namespace CXXL
{

    // 為 TreeNodeBase 設計的線程池
    // 任務為 delete 節點
    // 線程只在有任務來時才會被
    // 建立，最多不會超過 CPU 提供的線程
    // 數量。任務滅少會逐一結束線程
    // 提供一個 wait() 用於等待所有線程結束
    class TreeNodeThreadPool
    {
        inline static size_t m_threadNum = 0;                    // 建立的線程數量
        inline static std::queue<std::function<void()>> m_tasks; // 線程任務列表
        inline static std::mutex m_mutex;                        // 線程鎖

        // 通知所有 wait() 的等待線程
        inline static std::condition_variable cv;
        inline static bool ready = true;

    protected:
        // 加入任務
        // 已建立的線程未達最大數量
        // 時會建立新的線程
        static void cxxlFASTCALL addTask(const std::function<void()> &task)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push(task);
            ready = false;

            if (m_threadNum < std::thread::hardware_concurrency())
            {
                m_threadNum++;
                std::thread thread([]()
                                   {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        if (m_tasks.size() == 0)
                        {
                            if(--m_threadNum == 0)
                            {
                                ready = true;
                                cv.notify_all();
                            }
                            return;
                        }
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    task();
                } });
                thread.detach();
            }
        }

    public:
        // 等待所有任務線程結束
        static void cxxlFASTCALL wait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            cv.wait(lock, []
                    { return ready; }); // 等待直到 ready 為 true
        }
    };

    // 基底樹狀容器
    // D: 為延伸類別
    template <typename D>
    class TreeNodeBase : private TreeNodeThreadPool
    {
        using NODE_PTR = std::shared_ptr<D>;
        using CNODE_PTR = std::shared_ptr<const D>;

        const std::u8string m_name; // 節點名稱

        // 主要的子節點列表
        std::list<NODE_PTR> m_children;

        // 名稱索引：名稱 -> shared_ptr
        std::unordered_map<std::u8string, NODE_PTR>
            m_nameIndex;

        // 子節點索引：D* -> list iterator
        std::unordered_map<D *,
                           typename std::list<NODE_PTR>::iterator>
            m_childIndex;

        // 父節點
        std::weak_ptr<D> m_parent;

        // 自己
        std::weak_ptr<D> m_self;

        // 讀寫鎖 - 使用 shared_mutex 允許多個讀取者同時存取
        mutable std::shared_mutex m_mutex;

        // 設定父節點和自己
        void cxxlFASTCALL setParentAndSelf(const NODE_PTR &parent,
                                           const NODE_PTR &self)
        {
            // 只在建構時設定，不用鎖
            // std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_parent = parent;
            m_self = self;
        }

        void cxxlFASTCALL traverse(const std::function<void(const CNODE_PTR &,
                                                            size_t)> &callback,
                                   size_t depth) const
        {
            callback(getSelf(), depth);

            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.size() != 0)
            {
                // 創建子節點的副本以避免在遍歷過程中鎖定衝突
                std::list<NODE_PTR> childrenCopy = m_children;
                lock.unlock(); // 釋放鎖避免死鎖

                for (auto &childNode : childrenCopy)
                    ((const D *)childNode.get())->traverse(callback, depth + 1);

                // lock.lock(); // 重新獲取鎖
                callback(CNODE_PTR(), 0);
            }
        }

        void cxxlFASTCALL traverse(const std::function<void(const NODE_PTR &, size_t)>
                                       &callback,
                                   size_t depth)
        {
            callback(getSelf(), depth);

            std::unique_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.size() != 0)
            {
                // 創建子節點的副本以避免在遍歷過程中鎖定衝突
                std::list<NODE_PTR> childrenCopy = m_children;
                lock.unlock(); // 釋放鎖避免死鎖

                for (auto &childNode : childrenCopy)
                    childNode->traverse(callback, depth + 1);

                // lock.lock(); // 重新獲取鎖
                callback(NODE_PTR(), 0);
            }
        }

        // 建構一個在解構時由 m_threadLimiter 進行刪除
        static NODE_PTR cxxlFASTCALL makeNode(const std::u8string &name)
        {
            // 向延伸類別詢問是否可以新增
            if (D::canCreateChild(name) == false)
                return nullptr;

            std::shared_ptr<D> node(new D(name),
                                    [](D *p)
                                    {
                                        TreeNodeThreadPool::addTask(
                                            [p]
                                            {
                                                ::delete p;
                                            });
                                    });

            return node;
        }

    protected:
        // 取得自己
        NODE_PTR cxxlFASTCALL getSelf()
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_self.lock();
        }

        // 取得自己
        CNODE_PTR cxxlFASTCALL getSelf() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_self.lock();
        }

        // Constructor
        explicit TreeNodeBase(const std::u8string &name)
            : m_name(name) {}

        // 創建根節點
        static NODE_PTR cxxlFASTCALL createRoot(const std::u8string &name)
        {

            NODE_PTR root(makeNode(name));

            // 不需要鎖定，因為此時還沒有其他線程能訪問這個新節點
            if(root)
                root->m_self = root;
            return root;
        }

        // TreeNodeBase 的要求
        // 可以用指定的名字建立子節點嗎?
        static bool cxxlFASTCALL canCreateChild(const std::u8string &name)
        {
            return true;
        }

    public:
        // Destructor
        virtual ~TreeNodeBase()
        {
            clearChildren();
        }

        // 取得節名稱
        const std::u8string &cxxlFASTCALL getName() const
        {
            // 名稱是 const 且在構造時設定，不需要鎖定
            return m_name;
        }

        // 取得父節點
        NODE_PTR cxxlFASTCALL getParent()
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_parent.lock();
        }

        // 取得父節點
        CNODE_PTR cxxlFASTCALL getParent() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_parent.lock();
        }

        // 新增子節點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL addChild(const std::u8string &name = u8"")
        {
            return addBackChild(name);
        }

        // 新增最前子節點 - O(1) 時間複雜度
        NODE_PTR cxxlFASTCALL addFrontChild(
            const std::u8string &name = u8"")
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            // 若有節點名稱則不可重複
            if (!name.empty() && m_nameIndex.find(name) != m_nameIndex.end())
                return nullptr;

            NODE_PTR newChild(makeNode(name));
            if(newChild == nullptr) return nullptr;

            auto it = m_children.insert(m_children.begin(),
                                        newChild);

            // 更新子節點索引
            m_childIndex[newChild.get()] = it;

            // 如果有名稱，更新名稱索引
            if (!name.empty())
                m_nameIndex[name] = newChild;

            NODE_PTR selfPtr = m_self.lock();
            lock.unlock(); // 釋放鎖避免死鎖

            newChild->setParentAndSelf(selfPtr, newChild);

            return newChild;
        }

        // 新增最後子節點 - O(1) 時間複雜度
        NODE_PTR cxxlFASTCALL addBackChild(
            const std::u8string &name = u8"")
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            // 若有節點名稱則不可重複
            if (!name.empty() && m_nameIndex.find(name) != m_nameIndex.end())
                return nullptr;

            NODE_PTR newChild(makeNode(name));
            if(newChild == nullptr) return nullptr;

            auto it = m_children.insert(m_children.end(),
                                        newChild);

            // 更新子節點索引
            m_childIndex[newChild.get()] = it;

            // 如果有名稱，更新名稱索引
            if (!name.empty())
                m_nameIndex[name] = newChild;

            NODE_PTR selfPtr = m_self.lock();
            lock.unlock(); // 釋放鎖避免死鎖

            newChild->setParentAndSelf(selfPtr, newChild);

            return newChild;
        }

        // 在特定子節點之前插入新節
        // 點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL insertBefore(
            const NODE_PTR &childNode,
            const std::u8string &name)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            // 透過子節點索引快速找到位置
            auto indexIt = m_childIndex.find(
                childNode.get());
            if (indexIt == m_childIndex.end())
                return nullptr;

            // 若有節點名稱則不可重複
            if (!name.empty() && m_nameIndex.find(name) != m_nameIndex.end())
                return nullptr;

            NODE_PTR newChild(makeNode(name));
            if(newChild == nullptr) return nullptr;


            auto newIt = m_children.insert(indexIt->second,
                                           newChild);

            // 更新索引
            m_childIndex[newChild.get()] = newIt;
            if (!name.empty())
                m_nameIndex[name] = newChild;

            NODE_PTR selfPtr = m_self.lock();
            lock.unlock(); // 釋放鎖避免死鎖

            newChild->setParentAndSelf(selfPtr, newChild);
            return newChild;
        }

        // 在特定子節點之後插入新節
        // 點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL insertAfter(
            const NODE_PTR &childNode,
            const std::u8string &name)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);


            // 透過子節點索引快速找到位置
            auto indexIt = m_childIndex.find(
                childNode.get());
            if (indexIt == m_childIndex.end())
                return nullptr;

            // 若有節點名稱則不可重複
            if (!name.empty() && m_nameIndex.find(name) != m_nameIndex.end())
                return nullptr;

            NODE_PTR newChild(makeNode(name));
            if(newChild == nullptr) return nullptr;

            auto newIt = m_children.insert(
                std::next(indexIt->second), newChild);

            // 更新索引
            m_childIndex[newChild.get()] = newIt;
            if (!name.empty())
                m_nameIndex[name] = newChild;

            NODE_PTR selfPtr = m_self.lock();
            lock.unlock(); // 釋放鎖避免死鎖

            newChild->setParentAndSelf(selfPtr, newChild);

            return newChild;
        }

        // 檢查指定名稱的子節點是否存
        // 在 - O(1)時間複雜度
        bool cxxlFASTCALL hasChild(const std::u8string &name) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_nameIndex.find(name) != m_nameIndex.end();
        }

        // 檢查指定子節點是否存
        // 在 - O(1)時間複雜度
        bool cxxlFASTCALL hasChild(const NODE_PTR &child)
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_childIndex.find(child.get()) != m_childIndex.end();
        }

        // 檢查指定子節點是否存
        // 在 - O(1)時間複雜度
        bool cxxlFASTCALL hasChild(
            const CNODE_PTR &child) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_childIndex.find((D *)child.get()) != m_childIndex.end();
        }

        // 取得子節點數量
        size_t cxxlFASTCALL childCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_children.size();
        }

        // 移除子節點 - O(1)時間複雜度
        bool cxxlFASTCALL removeChild(const NODE_PTR &child)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto indexIt = m_childIndex.find(child.get());
            if (indexIt != m_childIndex.end())
            {
                auto listIt = indexIt->second;

                // 如果有名稱，從名稱索引中移除
                lock.unlock();
                const auto &childName = child->getName();
                lock.lock();
                if (!childName.empty())
                    m_nameIndex.erase(childName);

                // 從列表和子節點索引中移除
                m_children.erase(listIt);
                m_childIndex.erase(indexIt);

                lock.unlock(); // 釋放鎖避免死鎖

                // 重置父節點關係
                std::unique_lock<std::shared_mutex> childLock(child->m_mutex);
                child->m_parent.reset();

                return true;
            }

            return false;
        }

        // 按名稱移除子節點 - O(1)時間複雜度
        bool cxxlFASTCALL removeChildByName(const std::u8string &name)
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            auto nameIt = m_nameIndex.find(name);
            if (nameIt != m_nameIndex.end())
            {
                NODE_PTR child_ptr = nameIt->second;
                lock.unlock(); // 升級為寫鎖
                return removeChild(child_ptr);
            }

            return false;
        }

        // 移除第一個子節點 - O(1) 時間複雜度
        bool cxxlFASTCALL removeFrontChild()
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.empty())
                return false;

            NODE_PTR child = m_children.front();
            lock.unlock(); // 升級為寫鎖
            return removeChild(child);
        }

        // 移除最後一個子節點 - O(1) 時間複雜度
        bool cxxlFASTCALL removeBackChild()
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.empty())
                return false;

            NODE_PTR child = m_children.back();
            lock.unlock(); // 升級為寫鎖
            return removeChild(child);
        }

        // 清空所有子節點 - O(n)時間複雜度
        void cxxlFASTCALL clearChildren()
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            // 先複製子節點列表
            std::list<NODE_PTR> childrenCopy = m_children;

            lock.unlock(); // 釋放鎖避免死鎖

            // 重置每個子節點的父節點關係
            for (const auto &child : childrenCopy)
            {
                std::unique_lock<std::shared_mutex> childLock(child->m_mutex);
                child->m_parent.reset();
            }

            lock.lock();

            m_children.clear();
            m_nameIndex.clear();
            m_childIndex.clear();
        }

        // 移動子節點到指定位置之
        // 前 - O(1) 時間複雜度
        bool cxxlFASTCALL moveChildBefore(
            const NODE_PTR &childToMove,
            const NODE_PTR &targetChild)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto moveIt = m_childIndex.find(
                childToMove.get());
            auto targetIt = m_childIndex.find(
                targetChild.get());

            if (moveIt == m_childIndex.end() ||
                targetIt == m_childIndex.end())
                return false;

            if (childToMove == targetChild)
                return true; // 相同節點，不需移動

            // 從原位置移除（但不刪除）
            auto listIt = moveIt->second;
            auto child = *listIt;
            m_children.erase(listIt);

            // 插入到新位置
            auto newIt = m_children.insert(
                targetIt->second, child);

            // 更新索引
            m_childIndex[childToMove.get()] = newIt;

            return true;
        }

        // 移動子節點到指定位置之
        // 後 - O(1) 時間複雜度
        bool cxxlFASTCALL moveChildAfter(
            const NODE_PTR &childToMove,
            const NODE_PTR &targetChild)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto moveIt =
                m_childIndex.find(childToMove.get());

            auto targetIt =
                m_childIndex.find(targetChild.get());

            if (moveIt == m_childIndex.end() ||
                targetIt == m_childIndex.end())
                return false;

            if (childToMove == targetChild)
                return true; // 相同節點，不需移動

            // 從原位置移除（但不刪除）
            auto listIt = moveIt->second;
            auto child = *listIt;
            m_children.erase(listIt);

            // 插入到新位置
            auto newIt = m_children.insert(
                std::next(targetIt->second), child);

            // 更新索引
            m_childIndex[childToMove.get()] = newIt;

            return true;
        }

        // 移動子節點到開
        // 頭 - O(1)時間複雜度
        bool cxxlFASTCALL moveChildToFront(const NODE_PTR &child)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto indexIt = m_childIndex.find(child.get());
            if (indexIt == m_childIndex.end())
                return false;

            auto listIt = indexIt->second;
            if (listIt == m_children.begin())
                return true; // 已經在開頭

            auto childPtr = *listIt;
            m_children.erase(listIt);
            auto newIt = m_children.insert(
                m_children.begin(), childPtr);

            // 更新索引
            m_childIndex[child.get()] = newIt;

            return true;
        }

        // 移動子節點到結
        // 尾 - O(1)時間複雜度
        bool cxxlFASTCALL moveChildToBack(const NODE_PTR &child)
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto indexIt = m_childIndex.find(child.get());
            if (indexIt == m_childIndex.end())
                return false;

            auto listIt = indexIt->second;
            if (std::next(listIt) == m_children.end())
                return true; // 已經在結尾

            auto childPtr = *listIt;
            m_children.erase(listIt);
            auto newIt = m_children.insert(
                m_children.end(), childPtr);

            // 更新索引
            m_childIndex[child.get()] = newIt;

            return true;
        }

        // 取得子節點在列表中的位
        // 置（0-based index）- O(n)時間複雜度
        std::optional<size_t> cxxlFASTCALL getChildPosition(
            const NODE_PTR &child)
        {
            return getChildPosition((CNODE_PTR)child);
        }

        // 取得子節點在列表中的位
        // 置（0-based index）- O(n)時間複雜度
        std::optional<size_t> cxxlFASTCALL getChildPosition(
            const CNODE_PTR &child) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto indexIt = m_childIndex.find((D *)child.get());
            if (indexIt == m_childIndex.end())
                return std::nullopt;

            size_t pos = 0;
            for (auto it = m_children.begin();
                 it != indexIt->second;
                 ++it, ++pos)
                ;
            return pos;
        }

        // 按位置取得子節點 - O(n)時間複雜度
        CNODE_PTR cxxlFASTCALL getChildAt(
            size_t position) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            if (position >= m_children.size())
                return nullptr;

            auto it = m_children.begin();
            std::advance(it, position);
            return std::const_pointer_cast<const D>(*it);
        }

        // 按位置取得子節點 - O(n)時間複雜度
        NODE_PTR cxxlFASTCALL getChildAt(size_t position)
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->getChildAt(position));
        }

        // 按名稱查找子節點 - O(1)時間複雜度
        CNODE_PTR cxxlFASTCALL findChildByName(
            const std::u8string &name) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_nameIndex.find(name);
            if (it != m_nameIndex.end())
                return (it->second);

            return nullptr;
        }

        // 按名稱查找子節點 - O(1) 時間複雜度
        NODE_PTR cxxlFASTCALL findChildByName(
            const std::u8string &name)
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->findChildByName(name));
        }

        // 取得第一個子節點 - O(1)時間複雜度
        CNODE_PTR cxxlFASTCALL getFirstChild() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_children.begin();
            if (it == m_children.end())
                return nullptr;

            return *it;
        }

        // 取得第一個子節點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL getFirstChild()
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->getFirstChild());
        }

        // 取得最後一個子節點 - O(1)時間複雜度
        CNODE_PTR cxxlFASTCALL getLastChild() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_children.rbegin();
            if (it == m_children.rend())
                return nullptr;

            return *it;
        }

        // 取得最後一個子節點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL getLastChild()
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->getLastChild());
        }

        // 取得指定子節點的下一個子節
        // 點 - O(1)時間複雜度
        CNODE_PTR cxxlFASTCALL getNextChild(
            const CNODE_PTR &child) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_childIndex.find((D *)child.get());
            if (it == m_childIndex.end())
                return nullptr;

            auto nextIt = std::next(it->second);
            if (nextIt == m_children.end())
                return nullptr;

            return *nextIt;
        }

        // 取得指定子節點的下一個子節
        // 點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL getNextChild(
            const NODE_PTR &child)
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->getNextChild(child));
        }

        // 取得指定子節點的上一個子節
        // 點 - O(1)時間複雜度
        CNODE_PTR cxxlFASTCALL getPreviousChild(
            const CNODE_PTR &child) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_childIndex.find((D *)child.get());
            if (it == m_childIndex.end())
                return nullptr;

            auto childIt = it->second;
            if (childIt == m_children.begin())
                return nullptr;

            auto prevIt = std::prev(childIt);

            return *prevIt;
        }

        // 取得指定子節點的上一個子節
        // 點 - O(1)時間複雜度
        NODE_PTR cxxlFASTCALL getPreviousChild(
            const NODE_PTR &child)
        {
            return std::const_pointer_cast<D>(
                ((const TreeNodeBase<D> *)this)->getPreviousChild(child));
        }

        // 遍歷整棵樹(深度優先)
        // callback 第一個參數表示得到的節點
        // 第二個參數 depth 表示節點深度
        // root depth 為 1，0 表示回上一個節點
        void cxxlFASTCALL traverse(const std::function<
                                   void(const CNODE_PTR &node, size_t depth)>
                                       &callback) const
        {
            callback(getSelf(), 1);

            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.size() != 0)
            {
                // 創建子節點的副本以避免在遍歷過程中鎖定衝突
                std::list<NODE_PTR> childrenCopy = m_children;
                lock.unlock(); // 釋放鎖避免死鎖

                for (auto &childNode : childrenCopy)
                    ((const D *)childNode.get())->traverse(callback, 2);

                // lock.lock(); // 重新獲取鎖
                callback(CNODE_PTR(), 0);
            }
        }

        // 遍歷整棵樹(深度優先)
        // callback 第一個參數表示得到的節點
        // 第二個參數 depth 表示節點深度
        // root depth 為 1，0 表示回上一個節點
        void cxxlFASTCALL traverse(const std::function<
                                   void(const NODE_PTR &node, size_t depth)>
                                       &callback)
        {
            callback(getSelf(), 1);

            std::unique_lock<std::shared_mutex> lock(m_mutex);
            if (m_children.size() != 0)
            {
                // 創建子節點的副本以避免在遍歷過程中鎖定衝突
                std::list<NODE_PTR> childrenCopy = m_children;
                lock.unlock(); // 釋放鎖避免死鎖

                for (auto &childNode : childrenCopy)
                    childNode->traverse(callback, 2);

                // lock.lock(); // 重新獲取鎖
                callback(NODE_PTR(), 0);
            }
        }

        // 遍歷子節點(不含孫節點)
        void cxxlFASTCALL forEachChild(const std::function<
                                       void(const CNODE_PTR &)>
                                           &callback) const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            std::list<NODE_PTR> childrenCopy = m_children;
            lock.unlock(); // 釋放鎖避免死鎖

            for (const auto &childNode : childrenCopy)
                callback(std::const_pointer_cast<const D>(childNode));
        }

        void cxxlFASTCALL forEachChild(const std::function<
                                       void(const NODE_PTR &)>
                                           &callback)
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            std::list<NODE_PTR> childrenCopy = m_children;
            lock.unlock(); // 釋放鎖避免死鎖

            for (auto &childNode : childrenCopy)
                callback(childNode);
        }

        // 反向遍歷子節點(不含孫節點)
        void cxxlFASTCALL forEachChildReverse(
            const std::function<void(const CNODE_PTR &)>
                &callback) const
        {
            std::shared_lock<std::shared_mutex>
                lock(m_mutex);
            std::list<CNODE_PTR> childrenCopy = m_children;
            lock.unlock(); // 釋放鎖避免死鎖

            for (auto it = childrenCopy.rbegin();
                 it != childrenCopy.rend(); ++it)
                callback(*it);
        }

        void cxxlFASTCALL forEachChildReverse(
            const std::function<void(const NODE_PTR &)>
                &callback)
        {
            std::shared_lock<std::shared_mutex>
                lock(m_mutex);
            std::list<NODE_PTR> childrenCopy = m_children;
            lock.unlock(); // 釋放鎖避免死鎖

            for (auto it = childrenCopy.rbegin();
                 it != childrenCopy.rend(); ++it)
                callback(*it);
        }

        
        // 偵錯用：檢查索引一致性
        bool cxxlFASTCALL validateIndexes() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            // 檢查子節點索引
            if (m_childIndex.size() != m_children.size())
                return false;

            for (const auto &child : m_children)
            {
                auto it = m_childIndex.find(child.get());
                if (it == m_childIndex.end() ||
                    *(it->second) != child)
                    return false;
            }

            // 檢查名稱索引
            for (const auto &pair : m_nameIndex)
            {
                const NODE_PTR &child = pair.second;
                if (m_childIndex.find(child.get()) ==
                    m_childIndex.end())
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
        // auto cbegin() const { return m_children.cbegin(); }
        // auto cend() const { return m_children.cend(); }
        // auto crbegin() const {
        //    return m_children.crbegin();}
        // auto crend() const { return m_children.crend(); }
    };

    // 取得 const iterator，用於高效遍歷
    // 不能取得 shared_ptr<const D>
    // auto cbegin() const { return m_children.cbegin(); }
    // auto cend() const { return m_children.cend(); }
    // auto crbegin() const {
    //    return m_children.crbegin();}
    // auto crend() const { return m_children.crend(); }

    /*
    // Iterator 類別，提供線程安全的遍歷
    class SafeIterator {
    private:
        std::list<NODE_PTR> m_childrenSnapshot;
        typename std::list<NODE_PTR>::iterator m_currentIt;

    public:
        SafeIterator(const std::list<NODE_PTR>& children, bool isBegin = true)
            : m_childrenSnapshot(children)
        {
            m_currentIt = isBegin ? m_childrenSnapshot.begin() : m_childrenSnapshot.end();
        }

        NODE_PTR operator*() { return *m_currentIt; }
        SafeIterator& operator++() { ++m_currentIt; return *this; }
        SafeIterator operator++(int) { SafeIterator tmp = *this; ++m_currentIt; return tmp; }
        bool operator==(const SafeIterator& other) const { return m_currentIt == other.m_currentIt; }
        bool operator!=(const SafeIterator& other) const { return !(*this == other); }
    };

    class SafeConstIterator {
    private:
        std::list<NODE_PTR> m_childrenSnapshot;
        typename std::list<NODE_PTR>::const_iterator m_currentIt;

    public:
        SafeConstIterator(const std::list<NODE_PTR>& children, bool isBegin = true)
            : m_childrenSnapshot(children)
        {
            m_currentIt = isBegin ? m_childrenSnapshot.cbegin() : m_childrenSnapshot.cend();
        }

        CNODE_PTR operator*() { return std::const_pointer_cast<const D>(*m_currentIt); }
        SafeConstIterator& operator++() { ++m_currentIt; return *this; }
        SafeConstIterator operator++(int) { SafeConstIterator tmp = *this; ++m_currentIt; return tmp; }
        bool operator==(const SafeConstIterator& other) const { return m_currentIt == other.m_currentIt; }
        bool operator!=(const SafeConstIterator& other) const { return !(*this == other); }
    };

    // 取得線程安全的 iterator，用於高效遍歷
    SafeIterator cxxlFASTCALL begin()
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return SafeIterator(m_children, true);
    }

    SafeIterator cxxlFASTCALL end()
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return SafeIterator(m_children, false);
    }

    // 取得線程安全的 const iterator
    SafeConstIterator cxxlFASTCALL cbegin() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return SafeConstIterator(m_children, true);
    }

    SafeConstIterator cxxlFASTCALL cend() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return SafeConstIterator(m_children, false);
    }
    };
    */

    // 自帶的實作
    template <typename T>
    class TreeNode : public TreeNodeBase<TreeNode<T>>
    {

        T m_data;                              // 節點資料
        mutable std::shared_mutex m_dataMutex; // 資料存取鎖

    protected:

        // TreeNodeBase 的要求
        // 可以用指定的名字建立子節點嗎?
        static bool cxxlFASTCALL canCreateChild(const std::u8string &name)
        {
            return TreeNodeBase<TreeNode<T>>::canCreateChild(name);
        }

        // 建構函式
        // TreeNodeBase 的要求
        explicit TreeNode(const std::u8string &name)
            : TreeNodeBase<TreeNode<T>>(name)
        {
        }

    public:
        // 取得此節點資料
        T &cxxlFASTCALL getData()
        {
            std::unique_lock<std::shared_mutex> lock(m_dataMutex);
            return m_data;
        }
        const T &getData() const
        {
            std::shared_lock<std::shared_mutex> lock(m_dataMutex);
            return m_data;
        }

        // 設定此節點資料
        // 用 std::copy
        void cxxlFASTCALL setData(const T &data)
        {
            std::unique_lock<std::shared_mutex> lock(m_dataMutex);
            m_data = data;
        }

        // 設定此節點資料
        // 用 std::move
        void cxxlFASTCALL setData(T &&data)
        {
            std::unique_lock<std::shared_mutex> lock(m_dataMutex);
            m_data = std::move(data);
        }

        // 創建根節點
        static std::shared_ptr<TreeNode<T>> cxxlFASTCALL createRoot(const std::u8string &name)
        {
            return TreeNodeBase<TreeNode<T>>::createRoot(name);
        }

        template <typename D>
        friend class TreeNodeBase;
    };

} // namespace CXXL
#endif // __CXXLCOMMON_TREENODE_HPP_CxxlMan3