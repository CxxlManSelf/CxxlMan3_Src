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
        std::string m_values; // 永續資料陣列，以空格分隔


        // default constructor
        PersistData_String() = default;

        // copy constructor
        PersistData_String(const PersistData_String &other) = default;

        // move constructor
        PersistData_String(PersistData_String &&other) noexcept
            :m_values(std::move(other.m_values)) {}

        // Constructor
        // count: 永續資料陣列的元素數量
        // values: 永續資料陣列
        template <typename T>
        PersistData_String(const T *values, std::size_t count)
        {            
            // 將 values 陣列中的元素一個個轉為數值字串
            // 並以空格分隔
            for (std::size_t i = 0; i < count; ++i)
            {
                if(i != 0) this->m_values += " ";

                this->m_values += std::to_string(values[i]);
            }            
        }

        // Getter
        template <typename T> 
        std::vector<T> cxxlFASTCALL get() const
        {
            std::stringstream ss(this->m_values);

            std::vector<T> values;
            T value;
            while (ss >> value) values.push_back(value);

            return std::move(values);
        }        
    };


    // simpPersist_save() 的回傳值
    enum class PersistSaveResult
    {
        SUCCESS, // 成功保存        
        NAME_EMPTY_OR_EXIST, // 保存失敗，因為指定的 name 是空的或已經存在
        NOT_LOCKABLE // 保存失敗，因為 pPersistable 或其子孫物件中不能被鎖定
    };

    // simpPersist_load() 的回傳值
    enum class PersistLoadResult
    {        
        SUCCESS, // 成功讀取
        NAME_NOT_FOUND, // name 指定的名稱不存在
        NOT_LOCKABLE, // 讀取失敗，因為 pPersistable 或其子孫物件中不能被鎖定
        DATA_FORMAT_CORRUPT, // 讀取失敗，因為 PD_ptr 指定的資料格式錯誤或毀損
        DATA_NOT_MATCH,  // 讀取失敗，因為 PD_ptr 指定的資料不屬於 pPersistable
        LOAD_FAILED // 讀取失敗，因為永續儲存的 Load 檢查失敗

    };

    // 以文字方式保存永續資料
    // pPersistable 具有永續資料儲存功能的物件，不能為 nullptr
    // PD_ptr 用來保存永續資料的儲存體，不能為 nullptr
    // name 永續資料的名稱，PD_ptr 的根節點中不可以含有同名的
    //      子節點，永續資料將保存在這個子節點中
    PersistSaveResult CXXLPERSIST_DLLEXPORT
    cxxlFASTCALL simpPersist_save(IPersistChannel *pPersistable, 
       const std::shared_ptr<TreeNode<PersistData_String> > &PD_ptr, 
       const std::u8string &name);

    // 以文字方式讀取永續資料
    // pPersistable 具有永續資料儲存功能的物件，不能為 nullptr
    // PD_ptr 保存有永續資料的儲存體，不能為 nullptr
    // name 永續資料的名稱，PD_ptr 的根節點中必須含有這名稱的
    //      子節點，這個子節點須保存有 pPersistable 的永續資料
    PersistLoadResult CXXLPERSIST_DLLEXPORT
    cxxlFASTCALL simpPersist_load(IPersistChannel *pPersistable, 
        const std::shared_ptr<const TreeNode<PersistData_String> > &PD_ptr, 
        const std::u8string &name);



    




/*    
    class SimpleSerialize : public ISerialize
    {
    };
*/    

} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
