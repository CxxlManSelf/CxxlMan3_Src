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
#include <vector>

#include <persist_storage.hpp>

namespace CXXL
{

// 包裹原始永緒資料型別的資料陣列
template <typename T>
struct PersistData_Src;

// 防問者模式中的防問者
// 提供對不同的 PersistData_Src<> 的 visit() 處理
// 不過由延伸類別才針對正確的 visit() 做處理
// 對不正確的 PersistData_Src<> 由本類別的 visit() 做處理
// 處理方式為結束掉程式
class ISerializeLoadVisitor
{
public:

    //virtual void cxxlFASTCALL visit(PersistData_Src<char8_t> *src) // C++ 標準程式庫不支援 char8_t
    virtual void cxxlFASTCALL visit(const PersistData_Src<char> *src)    
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
    virtual void cxxlFASTCALL accept(ISerializeLoadVisitor &visitor) const = 0;
};

template <typename T>
struct PersistData_Src : public PersistData_SrcBase
{
    // 存放原始永續資料
    std::vector<T> m_values;

    // move constructor
    PersistData_Src(std::vector<T> &&values) : m_values(std::move(values)) {}
    

    virtual void cxxlFASTCALL accept(ISerializeLoadVisitor &visitor) const override
    {
        visitor.visit(this);
    }

    void cxxlFASTCALL get(const T **v, std::size_t &count) const 
    { 
        *v = m_values.data(); 
        count = m_values.size(); 
    }
};

// 正確的實作 visitor
// 由 class SerializeLoad 選用正確的 visitor
template <typename T>
struct Visitor : public ISerializeLoadVisitor
{
    const T **m_p;
    size_t &m_count;

    // Constructor
    Visitor(const T **p, size_t &count) 
      : m_p(p), m_count(count) 
    {}    

    virtual void cxxlFASTCALL visit(const PersistData_Src<T> *src) override
    {
        src->get(m_p, m_count);
    }
};

// ISerializeLoad 的實作
// PD: 為永續資料儲存容器包裹的類別，比如 PersistData_String
template <typename PD>
class SerializeLoad : public ISerializeLoad
{
    std::shared_ptr<TreeNode<PD> > m_ATTRs_ptr; // 存放永續資料容器的 "_ATTRs" 子節點

    // 存放物件本身所有原始永續資料，在檢查階段從 m_ATTRs_ptr 取得
    std::vector<std::shared_ptr<PersistData_SrcBase> > m_PD_srcs;

    // m_PD_srcs 的 iterator
    std::vector<std::shared_ptr<PersistData_SrcBase> >::iterator m_PD_srcs_it;

    size_t m_index = 0; // 作為名稱的一部分

    // 處於哪種模式，檢查模式還是載入模式
    enum class Mode { Check, Load } m_mode = Mode::Check;

    // 檢查模式下的檢查工作
    template <typename T>
    SerializeLoadResult cxxlFASTCALL check(std::size_t count, const std::u8string &name)
    {
        // 將 m_index 轉成字串
        std::string temp_str = std::to_string(m_index++);
        std::u8string index_str(reinterpret_cast<const char8_t*>(temp_str.c_str()), 
                        temp_str.length());

        std::shared_ptr<TreeNode<PD> > PD_ptr = m_ATTRs_ptr->findChildByName(index_str + u8'.' + name);
        if(!PD_ptr) return SerializeLoadResult::CHK_FAILED; // 沒有指定名稱的子節點

        // 取得子節點的永續資料
        PD pd = PD_ptr->getData();
        std::vector<T> values = pd.template get<T>();

        if(count > 0 && values.size() != count) return SerializeLoadResult::CHK_FAILED; // 數量不一樣

        PersistData_SrcBase *pPDsrc = new PersistData_Src<T>(std::move(values));
        std::shared_ptr<PersistData_SrcBase > PDsrc_ptr( pPDsrc );

        m_PD_srcs.push_back( PDsrc_ptr );

        return SerializeLoadResult::CHK_SUCCESS;
    }


    virtual SerializeLoadResult cxxlFASTCALL operator()(const char8_t **p, size_t &count, const std::u8string &name) override
    {
        if(m_mode == Mode::Check) // 檢查階段
            //return check<char8_t>(count, name);  // C++ 標準程式庫不支援 char8_t
            return check<char>(count, name);
        else if(m_PD_srcs_it != m_PD_srcs.end()) // 讀取階段
        {
            // 選用正確的 visitor 來取得原始永續資料
            //Visitor<char8_t> visitor(p, count); // C++ 標準程式庫不支援 char8_t
            Visitor<char> visitor((const char**)p, count); 
            (*(m_PD_srcs_it++))->accept(visitor);
        }
        else // 已無資料可讀取
        {
            // 永續資料存取不對等
            std::cerr << "SerializeLoad: Persistent data access not match" << std::endl;
            exit(EXIT_FAILURE);
        }

        return SerializeLoadResult::LOAD_SUCCESS;
    }


public:
    SerializeLoad() 
    {}

    virtual ~SerializeLoad() 
    {}

    void cxxlFASTCALL setPD(std::shared_ptr<TreeNode<PD> > &ATTRs_ptr)
    {
        m_ATTRs_ptr = ATTRs_ptr;
    }

    void cxxlFASTCALL setLoadMode() 
    { 
        m_mode = Mode::Load;
        m_PD_srcs_it = m_PD_srcs.begin();
    }
    
};



}
#endif // __CXXLPERSIST_SERIALIZE_LOAD_HPP_CxxlMan3
