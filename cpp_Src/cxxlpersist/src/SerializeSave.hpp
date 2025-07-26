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

template <typename PD>    
class SerializeSave:public ISerializeSave
{
    std::shared<TreeNode<PD> > m_PD_ptr;

    size_t m_index = 0; // 作為名稱的一部分


    template <typename T>
    void cxxlFASTCALL Save(T *p, size_t count, const std::u8string &name)
    {
        if(p == nullptr) return false;

        // 將 m_index 轉成字串
        std::u8string index_str = std::to_string(m_index++);        

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_PD_ptr->addChild(index_str + u8'.' + name);
        // if(!PD_ptr) return false; // 加了編號不可能發生

        PD_ptr->setData({count, p});
    }

    virtual void cxxlFASTCALL operator()(char8_t *p, size_t count, const std::u8string &name) override
    {
        Save(p, count, name);
    }

    virtual void cxxlFASTCALL operator()(std::int8_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::int16_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::int32_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::int64_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::uint8_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::uint16_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::uint32_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::uint64_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::float32_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::float64_t *p, size_t count, const std::u8string &name) = 0;
    virtual void cxxlFASTCALL operator()(std::float128_t *p, size_t count, const std::u8string &name) = 0;

public:
    // Constructor
    SerializeSave(std::shared<TreeNode<PD> > PD_ptr) 
        : m_PD_ptr(PD_ptr)
    {}

    // Destructor
    virtual ~SerializeSave() {}

    //  ISerializeSave  (p, count, name)   name  ,  p  count   
    virtual bool operator()(std::int8_t *p, size_t count, const std::u8string &name) = 0;
};

} // namespace CXXL

#endif // __CXXLPERSIST_SERIALIZE_SAVE_HPP_CxxlMan3
