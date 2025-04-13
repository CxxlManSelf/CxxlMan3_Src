#include <iostream>

#include <liferes.hpp>

using namespace CxxlMan3;

class MyClassB;

class MyClassA : public LifeRes<LifeResType::ALL> 
{

    std::mutex m_mutex;
    LifeOwner<MyClassB> m_myB;

public:    
    // Constructor
    MyClassA() 
        : m_myB( std::shared_ptr<MyClassB>(),
            this,
            [this](void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_myB.chkLifeRes(pChk))
                    m_myB.destroy();
            })
    {}
  
    // Destructor
    ~MyClassA()
    {
        std::cout << "MyClassA destructor" << std::endl;
    }

    void cxxlFASTCALL addMyB(const std::shared_ptr<MyClassB> &myB_ptr)
    {
        m_myB.setLifeRes(myB_ptr);
    }
};

class MyClassB : public LifeRes<LifeResType::ALL> 
{
public:    
    std::mutex m_mutex;
    LifeOwner<MyClassA> m_myA;

public:
    // Constructor
    MyClassB() 
        : m_myA( std::shared_ptr<MyClassA>(),
            this,
            [this](void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_myA.chkLifeRes(pChk))
                    m_myA.destroy();
            })
    {}

    // Destructor
    ~MyClassB()
    {
        std::cout << "MyClassB destructor" << std::endl;
    }
    
    void cxxlFASTCALL addMyA(const std::shared_ptr<MyClassA> &myA_ptr)
    {
        m_myA.setLifeRes(myA_ptr);        
    }
};

class MyRoot : public LifeRes<LifeResType::ONE>
{
    std::mutex m_mutex;
    LifeOwner<MyClassA> m_myA;
public:
    // Constructor
    MyRoot() 
        : m_myA( std::shared_ptr<MyClassA>(),
            this,
            [this](void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_myA.chkLifeRes(pChk))
                    m_myA.destroy();
            })            
    {}

    // Destructor
    ~MyRoot()
    {
        std::cout << "MyRoot destructor" << std::endl;
    }

    void cxxlFASTCALL addMyA(const std::shared_ptr<MyClassA> &myA_ptr)
    {
        m_myA.setLifeRes(myA_ptr);        
    }
};

int main(int, char**)
{
    {
        std::shared_ptr<ICore> core_ptr = getDestructor();
        std::cout << "已經取得核心銷毁控制器" << std::endl;
        std::cout << "按 <enter> 鍵繼續" << std::endl; std::cin.get();
    
        std::shared_ptr<MyClassA> myA_ptr = std::make_shared<MyClassA>();
        std::shared_ptr<MyClassB> myB_ptr = std::make_shared<MyClassB>();
        std::cout << "已經建立了 myA_ptr 和 myB_ptr" << std::endl;
        std::cout << "按 <enter> 鍵繼續" << std::endl; std::cin.get();
    
        myA_ptr->addMyB(myB_ptr);
        myB_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myA_ptr 和 myB_ptr 互相引用" << std::endl;
        std::cout << "按 <enter> 鍵繼續" << std::endl; std::cin.get();
    
        std::shared_ptr<MyRoot> myRoot_ptr = std::make_shared<MyRoot>();
        std::cout << "已經建立了 myRoot_ptr" << std::endl;
        std::cout << "按 <enter> 鍵繼續" << std::endl; std::cin.get();
    
        myRoot_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myRoot_ptr 持有 myA_ptr" << std::endl;
        std::cout << "按 <enter> 鍵繼續" << std::endl; std::cin.get();
        
        myA_ptr.reset();
        myB_ptr.reset();
        std::cout << "已經釋放了 myA_ptr 和 myB_ptr" << std::endl;
        std::cout << "但彼此仍互相引用" << std::endl;
    
        std::cout << "按 <enter> 鍵後將會釋放 myRoot_ptr" << std::endl; std::cin.get();    
        myRoot_ptr.reset();    
    }
    
    std::cout << "已經演釋完整個過程" << std::endl;
    std::cout << "按 <enter> 鍵結束程式" << std::endl; std::cin.get();
}
