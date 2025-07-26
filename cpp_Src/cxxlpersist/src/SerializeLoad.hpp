/************************************************************************************************
 * SerializeLoad.hpp v0.1.0
 *
 *  ISerializeLoad 的實作
 *
 *
 * Author: CxxlMan
 * Date: 2025 -
 ************************************************************************************************/

#ifndef __CXXLPERSIST_SERIALIZE_LOAD_HPP_CxxlMan3
#define __CXXLPERSIST_SERIALIZE_LOAD_HPP_CxxlMan3

#include <persist_storage.hpp>

namespace CXXL
{

struct PersistData_SrcBase
{};

template <typename T>
struct PersistData_Src : public PersistData_SrcBase
{
    // 存放原始永續資料
    std::vector<T> m_values;
};


template <typename PD>
class SerializeLoad : public ISerializeLoad
{
    std::shared<TreeNode<PD> > m_PD_ptr;

    // 存放物件本身所有原始永續資料，由 m_PD_ptr 取得
    std::vector<PersistData_SrcBase> m_PD_srcs;

    size_t m_index = 0; // 作為名稱的一部分

    // 處於哪種模式，檢查模式還是載入模式
    enum class Mode { Check, Load } m_mode = Mode::Check;

    // 檢查模式下的檢查工作
    template <typename T>
    bool cxxlFASTCALL check(size_t count, const std::u8string &name)
    {
        // 將 m_index 轉成字串
        std::u8string index_str = std::to_string(m_index++);        

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_PD_ptr->findChildByName(index_str + u8'.' + name);
        if(!PD_ptr) return false; // 沒有指定名稱的子節點

        // 取得子節點的永續資料
        PD pd = PD_ptr->getData();         
        std::vectot<T> values = pd.get();

        if(values.size() != count) return false; // 數量不一樣

        m_PD_srcs.push_back(PersistData_Src<T>(std::move(values)));

        return true;
    }


    virtual bool cxxlFASTCALL operator()(char8_t *p, size_t count, const std::u8string &name) = 0;
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
    SerializeLoad(std::shared_ptr<const TreeNode<PersistData_String> > PD_ptr) 
        : m_PD_ptr(PD_ptr)
    {}

    virtual ~SerializeLoad() {}

    virtual bool cxxlFASTCALL load(std::float64_t *p, size_t count, const std::u8string &name) override
    {
        std::shared_ptr<const TreeNode<PersistData_String> > PDroot_ptr = m_PD_ptr->addChild(name);
        if(!PDroot_ptr)
            return false;

        return PDroot_ptr->getData({count, p});
    }

private:
    std::shared_ptr<const TreeNode<PersistData_String> > m_PD_ptr;
};



}
#endif // __CXXLPERSIST_SERIALIZE_LOAD_HPP_CxxlMan3
