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

#include <iostream> // std::cerr
#include <cstdlib> // exit

#include <persist_storage.hpp>

namespace CXXL
{

template <typename T>
struct PersistData_Src;

class ISerializeLoadVisitor
{
public:

    virtual void cxxlFASTCALL visit(PersistData_Src<char8_t> *src)
    {
        // 不應該處理 PersistData_Src<char8_t>
        std::cerr << "PersistData_Src<char8_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::int8_t> *src)
    {
        // 不應該處理 PersistData_Src<std::int8_t>
        std::cerr << "PersistData_Src<std::int8_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::int16_t> *src)
    {
        // 不應該處理 PersistData_Src<std::int16_t>
        std::cerr << "PersistData_Src<std::int16_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::int32_t> *src)
    {
        // 不應該處理 PersistData_Src<std::int32_t>
        std::cerr << "PersistData_Src<std::int32_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::int64_t> *src)
    {
        // 不應該處理 PersistData_Src<std::int64_t>
        std::cerr << "PersistData_Src<std::int64_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::uint8_t> *src)
    {
        // 不應該處理 PersistData_Src<std::uint8_t>
        std::cerr << "PersistData_Src<std::uint8_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::uint16_t> *src)
    {
        // 不應該處理 PersistData_Src<std::uint16_t>
        std::cerr << "PersistData_Src<std::uint16_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::uint32_t> *src)
    {
        // 不應該處理 PersistData_Src<std::uint32_t>
        std::cerr << "PersistData_Src<std::uint32_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::uint64_t> *src)
    {
        // 不應該處理 PersistData_Src<std::uint64_t>
        std::cerr << "PersistData_Src<std::uint64_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::float32_t> *src)
    {
        // 不應該處理 PersistData_Src<std::float32_t>
        std::cerr << "PersistData_Src<std::float32_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::float64_t> *src)
    {
        // 不應該處理 PersistData_Src<std::float64_t>
        std::cerr << "PersistData_Src<std::float64_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    virtual void cxxlFASTCALL visit(PersistData_Src<std::float128_t> *src)
    {
        // 不應該處理 PersistData_Src<std::float128_t>
        std::cerr << "PersistData_Src<std::float128_t> is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }
};

struct PersistData_SrcBase
{
    virtual void cxxlFASTCALL accept(ISerializeLoadVisitor &visitor) = 0;
};

template <typename T>
struct PersistData_Src : public PersistData_SrcBase
{
    // 存放原始永續資料
    std::vector<T> m_values;

    virtual void cxxlFASTCALL accept(ISerializeLoadVisitor &visitor) override
    {
        visitor.visit(this);
    }

    void cxxlFASTCALL get(T **v, std::size_t &count) const 
    { 
        *v = m_values.data(); 
        count = m_values.size(); 
    }
};

// 實作 visitor
template <typename T>
struct Visitor : public ISerializeLoadVisitor
{
    T **m_p;
    size_t &m_count;

    // Constructor
    Visitor(T **p, size_t &count) 
      : m_p(p), m_count(count) 
    {}    

    virtual void cxxlFASTCALL visit(PersistData_Src<T> *src) override
    {
        src->get(m_p, m_count);
    }
};

template <typename PD>
class SerializeLoad : public ISerializeLoad
{
    std::shared<TreeNode<PD> > m_PD_ptr;

    // 存放物件本身所有原始永續資料，由 m_PD_ptr 取得
    std::vector<PersistData_SrcBase> m_PD_srcs;

    // m_PD_srcs 的 iterator
    std::vector<PersistData_SrcBase>::iterator m_PD_srcs_it;

    size_t m_index = 0; // 作為名稱的一部分

    // 處於哪種模式，檢查模式還是載入模式
    enum class Mode { Check, Load } m_mode = Mode::Check;

    // 檢查模式下的檢查工作
    template <typename T>
    SerializeLoadResult cxxlFASTCALL check(size_t count, const std::u8string &name)
    {
        // 將 m_index 轉成字串
        std::u8string index_str = std::to_string(m_index++);        

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_PD_ptr->findChildByName(index_str + u8'.' + name);
        if(!PD_ptr) return SerializeLoadResult::CHK_FAILED; // 沒有指定名稱的子節點

        // 取得子節點的永續資料
        PD pd = PD_ptr->getData();         
        std::vectot<T> values = pd.get();

        if(count > 0 && values.size() != count) return SerializeLoadResult::CHK_FAILED; // 數量不一樣

        m_PD_srcs.push_back(PersistData_Src<T>(std::move(values)));

        return SerializeLoadResult::CHK_SUCCESS;
    }


    virtual SerializeLoadResult cxxlFASTCALL operator()(char8_t **p, size_t &count, const std::u8string &name) override
    {
        if(m_mode == Mode::Check) 
            return check<char8_t>(count, name);
        else if(m_PD_srcs_it != m_PD_srcs.end())
        {
            Visitor<char8_t> visitor(p, count);
            (m_PD_srcs_it++)->accept(visitor);
            return SerializeLoadResult::LOAD_SUCCESS;
        }
        else // 已無資料可讀取
        {
            // 永續資料存取不對等
            std::cerr << "SerializeLoad: Persistent data access not match" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    virtual SerializeLoadResult cxxlFASTCALL operator()(std::int8_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::int16_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::int32_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::int64_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint8_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint16_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint32_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::uint64_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::float32_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::float64_t **p, size_t &count, const std::u8string &name) = 0;
    virtual SerializeLoadResult cxxlFASTCALL operator()(std::float128_t **p, size_t &count, const std::u8string &name) = 0;


public:
    SerializeLoad(std::shared_ptr<const TreeNode<PersistData_String> > PD_ptr) 
        : m_PD_ptr(PD_ptr)
    {}

    virtual ~SerializeLoad() {}

    virtual bool cxxlFASTCALL load(std::float64_t *p, size_t count, const std::u8string &name) override
    {
        std::shared_ptr<const TreeNode<PersistData_String> > PDroot_ptr = m_PD_ptr->findChildByName(name);
        if(!PDroot_ptr)
            return false;

        return PDroot_ptr->getData({count, p});
    }

    void cxxlFASTCALL setLoadMode() 
    { 
        m_mode = Mode::Load;
        m_PD_srcs_it = m_PD_srcs.begin();
    }
    
};



}
#endif // __CXXLPERSIST_SERIALIZE_LOAD_HPP_CxxlMan3
