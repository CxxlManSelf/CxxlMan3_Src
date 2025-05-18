/*****************************************************
 * simple_persist_storage.hpp v1.0.0
 *
 * CxxlMan3 提供的一個簡單 IPersistStorage 實作
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
 ******************************************************/
#ifndef __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
#define __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3

#include <cstdint>
#include <stdfloat>
#include <memory>

#include "cxxlpersist.hpp"
#include "persist_storage.hpp"
#include "treenode.hpp"

namespace CXXL
{

    // ISimplePersistStorage 使用的容器介面
    class ISimplePersistContainer
    {

    public:
        virtual ~ISimplePersistContainer() {}

        // 儲存
        virtual void cxxlFASTCALL
        save(std::int8_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::int16_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::int32_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::int64_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::uint8_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::uint16_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::uint32_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::uint64_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::float32_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::float64_t *p, size_t count, const std::u8string &name) = 0;

        virtual void cxxlFASTCALL
        save(std::float128_t *p, size_t count, const std::u8string &name) = 0;

        // 讀取
        virtual bool cxxlFASTCALL
        load(std::int8_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::int16_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::int32_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::int64_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint8_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint16_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint32_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::uint64_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::float32_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::float64_t *p, size_t count, const std::u8string &name) = 0;

        virtual bool cxxlFASTCALL
        load(std::float128_t *p, size_t count, const std::u8string &name) = 0;
    };

    // 提供一個包裹 TreeNode<std::string> 的 ISimplePersistContainer 容器實作
    class ISimplePersistContainer_StringTreeNode : public ISimplePersistContainer
    {
    protected:
        ISimplePersistContainer_StringTreeNode() = default;

    public:
        virtual ~ISimplePersistContainer_StringTreeNode() {}

        // 取得 TreeNode<std::string>
        virtual std::shared_ptr<TreeNode<std::string>> cxxlFASTCALL getTreeNode() const = 0;

        static std::shared_ptr<ISimplePersistContainer> CXXLPERSIST_DLLEXPORT 
        cxxlFASTCALL create(const std::shared_ptr<TreeNode<std::u8string>> &treeNode_ptr =
            std::make_shared<TreeNode<std::u8string> >(std::u8string()));
    };


    // 提供一個簡易版的 IPersistStorage 實作
    class ISimplePersistStorage : public IPersistStorage
    {
    protected:
        ISimplePersistStorage() = default;

    public:
        virtual ~ISimplePersistStorage() {}

        virtual bool cxxlFASTCALL save(IPersistChannel *pPersistable) override;
        virtual bool cxxlFASTCALL load(IPersistChannel *pPersistable) override;

        virtual std::shared_ptr<ISimplePersistContainer>
            cxxlFASTCALL getSimplePersistContainer() const = 0;

        static std::shared_ptr<IPersistStorage>
            CXXLPERSIST_DLLEXPORT create(const std::shared_ptr<ISimplePersistContainer> &container_ptr = 
                ISimplePersistContainer_StringTreeNode::create());
    };

    class SimpleSerialize : public ISerialize
    {
    };

} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
