#include <treenode.hpp>
#include <iostream>

using namespace CXXL;

int main(int, char**)
{
    // 此程式所有 TreeNodeBase 物件在此區段誕生和死亡
    // 確保 AsyncNodeDeletor::wait(); 執行前
    // 所有 TreeNodeBase 物件的解構都交給 AsyncNodeDeletor
    {
        // 指定 TreeNode 的型別
        using NODE = CXXL::TreeNode<std::string>;

        // 建立根節點
        std::shared_ptr<NODE> root = NODE::createRoot(u8"root");

        // 建立子節點並設定資料
        root->addChild(u8"child1")->setData("hello world child1");

        // 建立子節點
        std::shared_ptr<NODE> child2 = root->addChild(u8"child2");
        child2->setData("hello world child2"); // 設定這個節點的資料

        // 建立子節點但沒有資料
        std::shared_ptr<NODE> child3 = root->addChild(u8"child3"); 

        // 建立孫節點並設定資料
        child2->addChild(u8"child2-1")->setData("hello world child2-1");
        child2->addChild(u8"child2-2")->setData("hello world child2-2");
        child3->addChild(u8"child3-1")->setData("hello world child3-1");
        child3->addChild(u8"child3-2")->setData("hello world child3-2");

        // 無名子節點
        child2->addChild(u8"")->setData("hello world child2-3");
        child2->addChild(u8"");
    }

    // 等待所有線程結束
    // 所有 TreeNodeBase 物件的解構都交給 AsyncNodeDeletor
    AsyncNodeDeletor::wait();

    std::cout << "Hello, from MyTest!\n";
}
