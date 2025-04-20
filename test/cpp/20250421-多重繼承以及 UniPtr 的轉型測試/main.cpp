#include <iostream>
#include <uniptr.hpp>

using namespace CxxlMan3;

class MyBase : public UniBase<UniBaseType::ONE>
{
public:
    // Destructor
    virtual ~MyBase() 
    {
        std::cout << "MyBase destructor\n";
    }


    void doSomething()
    {
        std::cout << "Doing something in MyBase\n";
    }
};

class MyDerived1 : public MyBase
{
public:
    // Destructor
    virtual ~MyDerived1()
    {
        std::cout << "MyDerived1 destructor\n";
    }
    void doSomething()
    {
        std::cout << "Doing something in MyDerived1\n";
    }
};
class MyDerived2 : public MyBase
{
public:
    // Destructor
    virtual ~MyDerived2()
    {
        std::cout << "MyDerived2 destructor\n";
    }
    void doSomething()
    {
        std::cout << "Doing something in MyDerived2\n";
    }
};

class MultiDerived : public MyDerived1, public MyDerived2
{
public:
    // Destructor
    virtual ~MultiDerived()
    {
        std::cout << "MultiDerived destructor\n";
    }
    void doSomething()
    {
        std::cout << "Doing something in MultiDerived\n";
    }
};

int main(int, char **)
{
    std::shared_ptr<IDestrWaiter> destrWaiter_ptr = getDestructor();

    UniPtr<MultiDerived> multiDerived_ptr = new MultiDerived;
    multiDerived_ptr->doSomething(); std::cout << std::endl;

    UniPtr<MyDerived2> myDerived2_ptr =
        multiDerived_ptr.cast((MyDerived2 *)multiDerived_ptr.get());
    myDerived2_ptr->doSomething(); std::cout << std::endl;

    std::cout << "Hello, from mytest!\n";
}
