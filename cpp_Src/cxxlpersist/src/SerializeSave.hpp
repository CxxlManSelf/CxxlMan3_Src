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


    template <typename T>
    bool cxxlFASTCALL Save(T *p, size_t count, const std::u8string &name)
    {
        if(p == nullptr) return false;

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_PD_ptr->addChild(name);
        if(!PD_ptr) return false; // 名稱未指定或已存在

        return PD_ptr->setData({count, p});
    }

    virtual bool cxxlFASTCALL operator()(char8_t *p, size_t count, const std::u8string &name) override
    {
        return Save(p, count, name);
    }

    virtual bool cxxlFASTCALL operator()(std::int8_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::int16_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::int32_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::int64_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::uint8_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::uint16_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::uint32_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::uint64_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::float32_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::float64_t *p, size_t count, const std::u8string &name) = 0;
    virtual bool cxxlFASTCALL operator()(std::float128_t *p, size_t count, const std::u8string &name) = 0;

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
