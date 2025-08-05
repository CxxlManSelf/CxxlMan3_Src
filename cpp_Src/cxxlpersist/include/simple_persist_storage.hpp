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
#include <sstream>

#include "cxxlpersist.hpp"
#include "persist_storage.hpp"
#include "treenode.hpp"

namespace CXXL
{

// 以文字方式保存永續資料
struct PersistData_String
{
    std::string m_values; // 永續資料陣列，以空格分隔

    PersistData_String() = default;

    // Constructor
    // 將各種型態的永續資料陣列轉為字串
    // count: 永續資料陣列的元素數量
    // values: 永續資料陣列
    template <typename T>
    PersistData_String(const T *values, std::size_t count)
    {            
        std::stringstream ss;
        // 將 values 陣列中的元素一個個轉為數值字串
        // 並以空格分隔
        for (std::size_t i = 0; i < count; ++i)
        {
            if(i != 0) ss << ' ';
            
            ss << values[i];                
        }

        this->m_values = ss.str();
    }

    // Getter
    // 將字串轉為各種型態的永續資料陣列
    template <typename T> 
    std::vector<T> cxxlFASTCALL get(void) const
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


} // namespace CXXL
#endif // __CXXLPERSIST_SIMPLE_PERSIST_STORAGE_HPP_CxxlMan3
