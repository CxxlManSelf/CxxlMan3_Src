/*****************************************************
 * simple_persist_storage.hpp v1.0.0
 *
 * CxxlMan3 提供的一個簡易版的 IPersistStorage 實作
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
        save(char8_t *p, size_t count, const std::u8string &name) = 0;

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
        load(char8_t *p, size_t count, const std::u8string &name) = 0;

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

    // 提供一個包裹 TreeNode<std::u8string> 的 ISimplePersistContainer 容器實作
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

    
    // 這個類別在存放 IPersistChannel 和 ISimplePersistContainer 兩者的實作物件指標
    // 並提供兩者之間的操作
    class PCnC
    {
    public:
        PCnC() = default;
        ~PCnC() = default;

        std::shared_ptr<IPersistChannel> pPersistChannel_ptr;
        std::shared_ptr<ISimplePersistContainer> pSimplePersistContainer_ptr;
    };

    // 以文字方式保存永續資料
    struct PersistData_String
    {
        std::string count; // 永續資料陣列的元素數量
        std::string values; // 永續資料陣列，以空格分隔
    };


    // simpPersist_save() 的回傳值
    enum class PersistSaveResult
    {
        SUCCESS, // 成功保存        
        NAME_CONFLICT, // 保存失敗，因為指定的 name 已經存在        
        NOT_LOCKABLE // 保存失敗，因為 pPersistable 或其子孫物件中不能被鎖定
    };

    // simpPersist_load() 的回傳值
    enum class PersistLoadResult
    {        
        SUCCESS, // 成功讀取
        

    };

    // 以文字方式保存永續資料
    // pPersistable 具有永續資料儲存功能的物件，不能為 nullptr
    // pTreeNode 用來保存永續資料，不能為 nullptr
    // name 永續資料的名稱，pTreeNode 的根節點中不可以含有同名的子節點
    PersistSaveResult cxxlFASTCALL simpPersist_save(IPersistChannel *pPersistable, 
       const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr, 
       const std::u8string &name);

    // 以文字方式讀取永續資料
    PersistLoadResult cxxlFASTCALL simpPersist_load(IPersistChannel *pPersistable, 
        const std::shared_ptr<const TreeNode<PersistData_String> > &PD_ptr, 
        const std::u8string &name);



    




/*    
    class SimpleSerialize : public ISerialize
    {
    };
*/    

} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
