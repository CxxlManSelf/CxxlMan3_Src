#include <iostream>
#include <fstream>

#include <pathnode.hpp>
#include <treenode_io.hpp>
#include "database_types.hpp"
#include "database_io.hpp"
#include "database_ui.hpp"

using namespace CXXL;

// 初始化範例資料庫
std::shared_ptr<DBNode> createSampleDatabase()
{
    // 建立根節點
    auto root = DBNode::createRoot(u8"購物系統");
    root->setData(std::make_shared<EmptyData>());

    // 建立 customers 節點
    auto customers = root->addChild(u8"customers");
    customers->setData(std::make_shared<EmptyData>());

    // 新增客戶：張三
    auto zhangsan = customers->addChild(u8"張三");
    auto zhangsanData = std::make_shared<PersonData>("30", "zhang.san@email.com");
    zhangsan->setData(zhangsanData);

    // 張三的個人屬性
    auto hobby = zhangsan->addChild(u8"hobby");
    hobby->setData(std::make_shared<StringData>("閱讀"));

    auto preference = zhangsan->addChild(u8"preference");
    preference->setData(std::make_shared<StringData>("喜歡科技產品"));

    // 建立 products 節點
    auto products = root->addChild(u8"products");
    products->setData(std::make_shared<EmptyData>());

    // 新增產品：筆記型電腦
    auto laptop = products->addChild(u8"LAPTOP001");
    auto laptopData = std::make_shared<ProductData>("筆記型電腦", "25000", "3C電子");
    laptop->setData(laptopData);

    // 筆電的產品屬性
    auto brand = laptop->addChild(u8"brand");
    brand->setData(std::make_shared<StringData>("ASUS"));

    auto model = laptop->addChild(u8"model");
    model->setData(std::make_shared<StringData>("ZenBook 14"));

    auto spec = laptop->addChild(u8"specification");
    spec->setData(std::make_shared<StringData>("Intel i7, 16GB RAM, 512GB SSD"));

    // 新增產品：無線滑鼠
    auto mouse = products->addChild(u8"MOUSE001");
    auto mouseData = std::make_shared<ProductData>("無線滑鼠", "1200", "3C電子");
    mouse->setData(mouseData);

    // 建立 orders 節點
    auto orders = root->addChild(u8"orders");
    orders->setData(std::make_shared<EmptyData>());

    // 新增訂單
    auto order001 = orders->addChild(u8"ORDER001");
    auto orderData = std::make_shared<OrderData>("2025-08-17", "/customers/張三", "26200");
    order001->setData(orderData);

    // 訂單項目 1: 筆記型電腦
    auto item1 = order001->addChild(u8"");  // 空名稱
    auto item1Data = std::make_shared<OrderItemData>(
        "/products/LAPTOP001", "1", "25000", "25000");
    item1->setData(item1Data);

    // 訂單項目 2: 無線滑鼠
    auto item2 = order001->addChild(u8"");  // 空名稱
    auto item2Data = std::make_shared<OrderItemData>(
        "/products/MOUSE001", "1", "1200", "1200");
    item2->setData(item2Data);

    return root;
}

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "  PathNode 階層式異質資料庫系統" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::string filename = "shopping_database.txt";
    std::shared_ptr<DBNode> root;

    // 嘗試載入現有資料庫
    std::cout << "正在檢查資料庫檔案: " << filename << std::endl;
    {
        root = DatabaseIO::load(filename);

        if (!root)
        {
            std::cout << "資料庫檔案不存在或載入失敗，創建新的範例資料庫..." << std::endl;
            root = createSampleDatabase();

            // 儲存初始資料庫
            if (DatabaseIO::save(filename, root))
            {
                std::cout << "範例資料庫已創建並儲存至: " << filename << std::endl;
            }
            else
            {
                std::cout << "警告: 無法儲存資料庫檔案" << std::endl;
            }
        }
        else
        {
            std::cout << "成功載入資料庫: " << filename << std::endl;
        }

        std::cout << "\n資料庫已就緒，啟動 UI 界面...\n" << std::endl;

        // 啟動 UI
        DatabaseUI ui(root, filename);
        ui.run();

    }

    std::cout << "\n感謝使用！再見！" << std::endl;

    // 等待所有背景執行緒結束（TreeNode 的刪除任務）
    AsyncNodeDeletor::wait();

    return 0;
}
