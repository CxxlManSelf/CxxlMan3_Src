#include <iostream>
#include <threadmgr.hpp>
#include <semaphore.hpp>

using namespace CxxlMan3;

class MyTest
{
    cxxlSemaphore m_semaphore{1,0};
public:
    
    void signal()
    {
        m_semaphore.release();
    }

    void test()
    {
        cxxlSemaphoreHelper semaphore(m_semaphore);
        std::cout << "Hello, from class MyTest!\n";

    }
};

int main(int, char **)
{
    MyTest myTest;    

    // ThreadPool thread;
    ThreadLimiter thread; // 解構函數會等待所有子執行緒結束
    thread(&MyTest::test, &myTest);
    myTest.signal();

    std::cout << "Hello, from mytest!\n";
}
