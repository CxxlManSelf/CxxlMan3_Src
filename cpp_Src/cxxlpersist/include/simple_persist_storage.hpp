/*****************************************************
 * simple_persist_storage.hpp v1.0.0
 *
 * 提供一個簡單的 IPersistStorage 實作
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
******************************************************/
#ifndef __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3

#include "persistence.hpp"

namespace CXXL
{

    // SimplePersistStorage 的容器操作介面
    class ISimplePersistContainer
    {
    public:
        virtual ~ISimplePersistContainer() {}

    };

    class SimpleSerializable: public ISerializable
    {

    };

    // 一個階層式的樹狀容器，每個節點可以包含一個物件，和他之下不限數量的子容器，子容器以加入的順序排列，並能 增、刪、改 任何位置的子容器
    class SimplePersistTree: public ISimplePersistContainer
    {
    public:
        class Node: public ISimplePersistContainer
        {
        public:
            Node() = default;
            Node(const std::shared_ptr<IPersistable> &object) : m_object(object) {}
            virtual ~Node() {}
            std::shared_ptr<IPersistable> m_object;
            std::vector<std::shared_ptr<Node>> m_children;
        };

        SimplePersistTree() = default;
        SimplePersistTree(const std::shared_ptr<Node> &root) : m_root(root) {}
        virtual ~SimplePersistTree() {}
        std::shared_ptr<Node> m_root;

        bool cxxlFASTCALL addChild(const std::shared_ptr<Node> &child, std::shared_ptr<Node> &parent = nullptr)
        {
            if (parent == nullptr)
            {
                m_root = child;
            }
            else
            {
                parent->m_children.push_back(child);
            }
            return true;
        }

        bool cxxlFASTCALL removeChild(std::shared_ptr<Node> &parent, size_t index)
        {
            if (parent == nullptr)
            {
                m_root = nullptr;
            }
            else
            {
                parent->m_children.erase(parent->m_children.begin() + index);
            }
            return true;
        }

        bool cxxlFASTCALL getChild(size_t index, std::shared_ptr<Node> &parent, std::shared_ptr<Node> &child)
        {
            if (parent == nullptr)
            {
                child = m_root;
            }
            else
            {
                child = parent->m_children[index];
            }
            return true;
        }

        bool cxxlFASTCALL setChild(std::shared_ptr<Node> &parent, size_t index, const std::shared_ptr<Node> &child)
        {
            if (parent == nullptr)
            {
                m_root = child;
            }
            else
            {
                parent->m_children[index] = child;
            }
            return true;
        }

    class SimplePersistStorage : public IPersistStorage
    {
    public:
        virtual ~SimplePersistStorage() {}
        virtual bool cxxlFASTCALL save(IPersistChannel *pPersistable) override;
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) override;
    };

} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
