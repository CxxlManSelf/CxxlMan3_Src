/************************************************************************************************
 * SerializeSave.hpp v0.1.0
 *
 *  ISerializeSave 的實作
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLPERSIST_SERIALIZE_SAVE_HPP_CxxlMan3
#define __CXXLPERSIST_SERIALIZE_SAVE_HPP_CxxlMan3

#include <persist_storage.hpp>

namespace CXXL
{

// ISerializeSave 的實作
// PD: 為永續資料儲存容器包裹的類別，比如 PersistData_String
template <typename PD>    
class SerializeSave:public ISerializeSave
{
    std::shared_ptr<TreeNode<PD> > m_ATTRs_ptr; // 存放永續資料容器的 "_ATTRs" 子節點

    size_t m_index = 0; // 作為名稱的一部分，以免出現重複的名稱


    template <typename T>
    void cxxlFASTCALL _save(const T *p, size_t count, const std::u8string &name)
    {
        // 將 m_index 轉成字串
        std::string temp_str = std::to_string(m_index++);
        std::u8string index_str(reinterpret_cast<const char8_t*>(temp_str.c_str()), 
                        temp_str.length());

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_ATTRs_ptr->addChild(index_str + u8'.' + name);

        if(p == nullptr) return;
        PD_ptr->setData({p, count});
    }

    virtual void cxxlFASTCALL operator()(const char8_t *p, size_t count, const std::u8string &name) override
    {
        // _save(p, count, name); // C++ 標準程式庫不支援 char8_t
        _save((const char*)p, count, name);
    }
/*
    virtual void cxxlFASTCALL operator()(const std::int8_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::int16_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::int32_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::int64_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::uint8_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::uint16_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(const std::uint32_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }
    virtual void cxxlFASTCALL operator()(const std::uint64_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }
    virtual void cxxlFASTCALL operator()(const std::float32_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }
    virtual void cxxlFASTCALL operator()(const std::float64_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }
    virtual void cxxlFASTCALL operator()(const std::float128_t *p, size_t count, const std::u8string &name) override
    {
        _save(p, count, name);
    }
*/

public:
    // Constructor
    SerializeSave(std::shared_ptr<TreeNode<PD> > ATTRs_ptr) 
        : m_ATTRs_ptr(ATTRs_ptr)
    {}

    // Destructor
    virtual ~SerializeSave() {}
};

} // namespace CXXL

#endif // __CXXLPERSIST_SERIALIZE_SAVE_HPP_CxxlMan3
