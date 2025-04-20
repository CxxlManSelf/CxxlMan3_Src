#include <iostream>

#include <uniptr.hpp>

using namespace CxxlMan3;

class MyClassB;

class MyClassA : public UniBase<UniBaseType::ALL> 
{

    std::mutex m_mutex;
    UniOwner<MyClassB> m_myB;

public:    
    // Constructor
    MyClassA() 
        : m_myB(this,
            [this](CxxlMan3::UniOwner<MyClassB> *myB, void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (myB->chkUniBase(pChk))
                    myB->destroy();
            })
    {}
  
    // Destructor
    ~MyClassA()
    {
        std::cout << "MyClassA destructor\n";
    }

    void cxxlFASTCALL addMyB(const UniPtr<MyClassB> &myB_ptr)
    {
        m_myB.setUniBase(myB_ptr);
    }
};

class MyClassB : public UniBase<UniBaseType::ALL> 
{
public:    
    std::mutex m_mutex;
    UniOwner<MyClassA> m_myA;

public:
    // Constructor
    MyClassB() 
        : m_myA(this,
            [this](CxxlMan3::UniOwner<MyClassA> *myA,void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (myA->chkUniBase(pChk))
                    myA->destroy();
            })
    {}

    // Destructor
    ~MyClassB()
    {
        std::cout << "MyClassB destructor\n";
    }
    
    void cxxlFASTCALL addMyA(const UniPtr<MyClassA> &myA_ptr)
    {
        m_myA.setUniBase(myA_ptr);        
    }
};

class MyRoot : public UniBase<UniBaseType::ONE>
{
    std::mutex m_mutex;
    UniOwner<MyClassA> m_myA;
public:
    // Constructor
    MyRoot() 
        : m_myA(this,
            [this](CxxlMan3::UniOwner<MyClassA> *myA,void *pChk)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (myA->chkUniBase(pChk))
                    myA->destroy();
            })            
    {}

    // Destructor
    ~MyRoot()
    {
        std::cout << "MyRoot destructor\n";
    }

    void cxxlFASTCALL addMyA(const UniPtr<MyClassA> &myA_ptr)
    {
        m_myA.setUniBase(myA_ptr);        
    }
};

int main(int, char**)
{
    {
        std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();
        std::cout << "已經取得核心銷毁控制器\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();
    
        UniPtr<MyClassA> myA_ptr(new MyClassA);
        UniPtr<MyClassB> myB_ptr = new MyClassB;
        std::cout << "已經建立了 myA_ptr 和 myB_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();
    
        myA_ptr->addMyB(myB_ptr);
        myB_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myA_ptr 和 myB_ptr 互相引用\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();
    
        UniPtr<MyRoot> myRoot_ptr = new MyRoot;
        std::cout << "已經建立了 myRoot_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();
    
        myRoot_ptr->addMyA(myA_ptr);
        std::cout << "已經讓 myRoot_ptr 持有 myA_ptr\n";
        std::cout << "按 <enter> 鍵繼續\n"; std::cin.get();
        
        myA_ptr.reset();
        myB_ptr.reset();
        std::cout << "已經釋放了 myA_ptr 和 myB_ptr\n";
        std::cout << "但彼此仍互相引用\n";
    
        std::cout << "按 <enter> 鍵後將會釋放 myRoot_ptr\n"; std::cin.get();    
        myRoot_ptr.reset();    
    }
    
    std::cout << "已經演釋完整個過程\n";
    std::cout << "按 <enter> 鍵結束程式\n"; std::cin.get();
}
