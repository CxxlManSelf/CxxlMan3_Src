/***********************************************************
 * database_ui.hpp
 *
 * 簡單的命令列 UI 界面，用於處理客戶、產品和訂單
 *
 * Author: CxxlMan
 * Date: 2025-
 ************************************************************/
#ifndef __DATABASE_UI_HPP_CxxlMan3
#define __DATABASE_UI_HPP_CxxlMan3

#include "database_io.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

namespace CXXL
{

class DatabaseUI
{
    std::shared_ptr<DBNode> m_root;
    std::string m_filename;

    // 字串轉換輔助函數
    std::u8string toU8(const std::string& str)
    {
        std::u8string result;
        for (char c : str)
            result.push_back(static_cast<char8_t>(c));
        return result;
    }

    std::string fromU8(const std::u8string& str)
    {
        std::string result;
        for (char8_t c : str)
            result.push_back(static_cast<char>(c));
        return result;
    }

    // 取得輸入
    std::string getInput(const std::string& prompt)
    {
        std::cout << prompt;
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    // 顯示節點內容
    void displayNode(const std::shared_ptr<DBNode>& node, int indent = 0)
    {
        if (!node)
            return;

        std::string indentStr(indent * 2, ' ');
        std::string name = fromU8(node->getName());
        auto data = node->getData();

        std::cout << indentStr << "[" << name << "]";
        if (data && !data->serialize().empty())
        {
            std::cout << " = \"" << data->serialize() << "\"";
        }
        std::cout << std::endl;
    }

    // 列出路徑下的所有子節點
    void listNodes(const std::string& path)
    {
        auto node = m_root->findNodeByPath(toU8(path));
        if (!node)
        {
            std::cout << "路徑不存在: " << path << std::endl;
            return;
        }

        std::cout << "\n=== " << path << " 的內容 ===" << std::endl;
        displayNode(node);

        node->forEachChild([this](const std::shared_ptr<const DBNode>& child)
        {
            auto mutableChild = std::const_pointer_cast<DBNode>(child);
            displayNode(mutableChild, 1);
        });
        std::cout << std::endl;
    }

public:
    DatabaseUI(const std::shared_ptr<DBNode>& root, const std::string& filename)
        : m_root(root), m_filename(filename)
    {
    }

    // 主選單
    void run()
    {
        while (true)
        {
            std::cout << "\n========== 購物系統資料庫 ==========" << std::endl;
            std::cout << "1. 客戶管理" << std::endl;
            std::cout << "2. 產品管理" << std::endl;
            std::cout << "3. 訂單管理" << std::endl;
            std::cout << "4. 瀏覽資料庫結構" << std::endl;
            std::cout << "5. 資料工具 (日期/數值)" << std::endl;
            std::cout << "6. 儲存並離開" << std::endl;
            std::cout << "0. 離開 (不儲存)" << std::endl;
            std::cout << "====================================" << std::endl;

            std::string choice = getInput("請選擇: ");

            if (choice == "1")
                customerMenu();
            else if (choice == "2")
                productMenu();
            else if (choice == "3")
                orderMenu();
            else if (choice == "4")
                browseMenu();
            else if (choice == "5")
                dataToolsMenu();
            else if (choice == "6")
            {
                if (DatabaseIO::save(m_filename, m_root))
                    std::cout << "資料已儲存至 " << m_filename << std::endl;
                else
                    std::cout << "儲存失敗!" << std::endl;
                break;
            }
            else if (choice == "0")
            {
                break;
            }
        }
    }

    // 客戶管理選單
    void customerMenu()
    {
        while (true)
        {
            std::cout << "\n===== 客戶管理 =====" << std::endl;
            std::cout << "1. 列出所有客戶" << std::endl;
            std::cout << "2. 新增客戶" << std::endl;
            std::cout << "3. 查看客戶詳細資料" << std::endl;
            std::cout << "4. 為客戶新增屬性" << std::endl;
            std::cout << "0. 返回主選單" << std::endl;

            std::string choice = getInput("請選擇: ");

            if (choice == "1")
            {
                listNodes("/customers");
            }
            else if (choice == "2")
            {
                std::string name = getInput("客戶姓名: ");
                std::string age = getInput("年齡: ");
                std::string email = getInput("Email: ");

                auto customersNode = m_root->findNodeByPath(toU8("/customers"));
                if (customersNode)
                {
                    auto customer = customersNode->addChild(toU8(name));
                    if (customer)
                    {
                        auto personData = std::make_shared<PersonData>(age, email);
                        customer->setData(personData);
                        std::cout << "客戶 " << name << " 已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗! 客戶名稱可能重複" << std::endl;
                    }
                }
            }
            else if (choice == "3")
            {
                std::string name = getInput("客戶姓名: ");
                std::string path = "/customers/" + name;
                listNodes(path);
            }
            else if (choice == "4")
            {
                std::string name = getInput("客戶姓名: ");
                std::string attrName = getInput("屬性名稱: ");
                std::string attrValue = getInput("屬性值: ");

                std::string path = "/customers/" + name;
                auto customerNode = m_root->findNodeByPath(toU8(path));
                if (customerNode)
                {
                    auto attrNode = customerNode->addChild(toU8(attrName));
                    if (attrNode)
                    {
                        auto stringData = std::make_shared<StringData>(attrValue);
                        attrNode->setData(stringData);
                        std::cout << "屬性已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗! 屬性名稱可能重複" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到客戶: " << name << std::endl;
                }
            }
            else if (choice == "0")
            {
                break;
            }
        }
    }

    // 產品管理選單
    void productMenu()
    {
        while (true)
        {
            std::cout << "\n===== 產品管理 =====" << std::endl;
            std::cout << "1. 列出所有產品" << std::endl;
            std::cout << "2. 新增產品" << std::endl;
            std::cout << "3. 查看產品詳細資料" << std::endl;
            std::cout << "4. 為產品新增屬性" << std::endl;
            std::cout << "0. 返回主選單" << std::endl;

            std::string choice = getInput("請選擇: ");

            if (choice == "1")
            {
                listNodes("/products");
            }
            else if (choice == "2")
            {
                std::string code = getInput("產品代碼: ");
                std::string name = getInput("產品名稱: ");
                std::string price = getInput("基本價格: ");
                std::string category = getInput("類別: ");

                auto productsNode = m_root->findNodeByPath(toU8("/products"));
                if (productsNode)
                {
                    auto product = productsNode->addChild(toU8(code));
                    if (product)
                    {
                        auto productData = std::make_shared<ProductData>(name, price, category);
                        product->setData(productData);
                        std::cout << "產品 " << code << " 已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗! 產品代碼可能重複" << std::endl;
                    }
                }
            }
            else if (choice == "3")
            {
                std::string code = getInput("產品代碼: ");
                std::string path = "/products/" + code;
                listNodes(path);
            }
            else if (choice == "4")
            {
                std::string code = getInput("產品代碼: ");
                std::string attrName = getInput("屬性名稱: ");
                std::string attrValue = getInput("屬性值: ");

                std::string path = "/products/" + code;
                auto productNode = m_root->findNodeByPath(toU8(path));
                if (productNode)
                {
                    auto attrNode = productNode->addChild(toU8(attrName));
                    if (attrNode)
                    {
                        auto stringData = std::make_shared<StringData>(attrValue);
                        attrNode->setData(stringData);
                        std::cout << "屬性已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗! 屬性名稱可能重複" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到產品: " << code << std::endl;
                }
            }
            else if (choice == "0")
            {
                break;
            }
        }
    }

    // 訂單管理選單
    void orderMenu()
    {
        while (true)
        {
            std::cout << "\n===== 訂單管理 =====" << std::endl;
            std::cout << "1. 列出所有訂單" << std::endl;
            std::cout << "2. 新增訂單" << std::endl;
            std::cout << "3. 查看訂單詳細資料" << std::endl;
            std::cout << "4. 為訂單新增商品" << std::endl;
            std::cout << "0. 返回主選單" << std::endl;

            std::string choice = getInput("請選擇: ");

            if (choice == "1")
            {
                listNodes("/orders");
            }
            else if (choice == "2")
            {
                std::string orderCode = getInput("訂單編號: ");
                std::string date = getInput("訂單日期: ");
                std::string customerName = getInput("客戶姓名: ");
                std::string totalAmount = getInput("總金額: ");

                std::string customerPath = "/customers/" + customerName;

                auto ordersNode = m_root->findNodeByPath(toU8("/orders"));
                if (ordersNode)
                {
                    auto order = ordersNode->addChild(toU8(orderCode));
                    if (order)
                    {
                        auto orderData = std::make_shared<OrderData>(date, customerPath, totalAmount);
                        order->setData(orderData);
                        std::cout << "訂單 " << orderCode << " 已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗! 訂單編號可能重複" << std::endl;
                    }
                }
            }
            else if (choice == "3")
            {
                std::string orderCode = getInput("訂單編號: ");
                std::string path = "/orders/" + orderCode;
                listNodes(path);
            }
            else if (choice == "4")
            {
                std::string orderCode = getInput("訂單編號: ");
                std::string productCode = getInput("產品代碼: ");
                std::string quantity = getInput("數量: ");
                std::string unitPrice = getInput("單價: ");
                std::string subtotal = getInput("小計: ");

                std::string productPath = "/products/" + productCode;
                std::string orderPath = "/orders/" + orderCode;

                auto orderNode = m_root->findNodeByPath(toU8(orderPath));
                if (orderNode)
                {
                    auto itemNode = orderNode->addChild(toU8(""));  // 空名稱
                    if (itemNode)
                    {
                        auto itemData = std::make_shared<OrderItemData>(
                            productPath, quantity, unitPrice, subtotal);
                        itemNode->setData(itemData);
                        std::cout << "訂單項目已新增" << std::endl;
                    }
                    else
                    {
                        std::cout << "新增失敗!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到訂單: " << orderCode << std::endl;
                }
            }
            else if (choice == "0")
            {
                break;
            }
        }
    }

    // 瀏覽選單
    void browseMenu()
    {
        std::cout << "\n===== 資料庫結構 =====" << std::endl;
        std::cout << "根節點路徑: /" << std::endl;
        std::cout << "常用路徑:" << std::endl;
        std::cout << "  /customers - 客戶資料" << std::endl;
        std::cout << "  /products - 產品資料" << std::endl;
        std::cout << "  /orders - 訂單資料" << std::endl;

        std::string path = getInput("\n輸入要瀏覽的路徑 (空白返回): ");
        if (!path.empty())
        {
            listNodes(path);
        }
    }

    // 資料工具選單
    void dataToolsMenu()
    {
        while (true)
        {
            std::cout << "\n===== 資料工具 (日期/數值) =====" << std::endl;
            std::cout << "1. 新增日期資料" << std::endl;
            std::cout << "2. 新增浮點數資料" << std::endl;
            std::cout << "3. 查看並比較日期" << std::endl;
            std::cout << "4. 計算浮點數" << std::endl;
            std::cout << "0. 返回主選單" << std::endl;

            std::string choice = getInput("請選擇: ");

            if (choice == "1")
            {
                std::string path = getInput("節點路徑 (例: /customers/張三): ");
                std::string name = getInput("屬性名稱: ");
                std::string date = getInput("日期 (YYYY-MM-DD): ");

                auto node = m_root->findNodeByPath(toU8(path));
                if (node)
                {
                    auto dateNode = node->addChild(toU8(name));
                    if (dateNode)
                    {
                        auto dateData = std::make_shared<DateData>(date);
                        if (dateData->isValid())
                        {
                            dateNode->setData(dateData);
                            std::cout << "日期資料已新增" << std::endl;
                        }
                        else
                        {
                            node->removeChild(dateNode);
                            std::cout << "日期格式錯誤!" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "新增失敗! 名稱可能重複" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到節點: " << path << std::endl;
                }
            }
            else if (choice == "2")
            {
                std::string path = getInput("節點路徑 (例: /products/LAPTOP001): ");
                std::string name = getInput("屬性名稱: ");
                std::string value = getInput("數值: ");

                auto node = m_root->findNodeByPath(toU8(path));
                if (node)
                {
                    auto floatNode = node->addChild(toU8(name));
                    if (floatNode)
                    {
                        auto floatData = std::make_shared<FloatData>();
                        if (floatData->fromString(value))
                        {
                            floatNode->setData(floatData);
                            std::cout << "浮點數資料已新增: " << floatData->toString(4) << std::endl;
                        }
                        else
                        {
                            node->removeChild(floatNode);
                            std::cout << "數值格式錯誤!" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "新增失敗! 名稱可能重複" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到節點: " << path << std::endl;
                }
            }
            else if (choice == "3")
            {
                std::string path1 = getInput("日期1路徑: ");
                std::string path2 = getInput("日期2路徑: ");

                auto node1 = m_root->findNodeByPath(toU8(path1));
                auto node2 = m_root->findNodeByPath(toU8(path2));

                if (node1 && node2)
                {
                    auto date1 = std::dynamic_pointer_cast<DateData>(node1->getData());
                    auto date2 = std::dynamic_pointer_cast<DateData>(node2->getData());

                    if (date1 && date2)
                    {
                        std::cout << "\n日期1: " << date1->getDate();
                        std::cout << " (" << date1->getYear() << "年"
                                  << date1->getMonth() << "月"
                                  << date1->getDay() << "日)" << std::endl;

                        std::cout << "日期2: " << date2->getDate();
                        std::cout << " (" << date2->getYear() << "年"
                                  << date2->getMonth() << "月"
                                  << date2->getDay() << "日)" << std::endl;

                        std::cout << "\n比較結果:" << std::endl;
                        if (*date1 < *date2)
                            std::cout << "  日期1 早於 日期2" << std::endl;
                        else if (*date1 > *date2)
                            std::cout << "  日期1 晚於 日期2" << std::endl;
                        else
                            std::cout << "  日期1 等於 日期2" << std::endl;
                    }
                    else
                    {
                        std::cout << "節點不是日期資料!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到指定的節點!" << std::endl;
                }
            }
            else if (choice == "4")
            {
                std::string path1 = getInput("數值1路徑: ");
                std::string path2 = getInput("數值2路徑: ");

                auto node1 = m_root->findNodeByPath(toU8(path1));
                auto node2 = m_root->findNodeByPath(toU8(path2));

                if (node1 && node2)
                {
                    auto float1 = std::dynamic_pointer_cast<FloatData>(node1->getData());
                    auto float2 = std::dynamic_pointer_cast<FloatData>(node2->getData());

                    if (float1 && float2)
                    {
                        std::cout << "\n數值1: " << float1->toString(4) << std::endl;
                        std::cout << "數值2: " << float2->toString(4) << std::endl;

                        std::cout << "\n計算結果:" << std::endl;
                        auto sum = (*float1) + (*float2);
                        auto diff = (*float1) - (*float2);
                        auto prod = (*float1) * (*float2);
                        auto quot = (*float1) / (*float2);

                        std::cout << "  加法: " << sum.toString(4) << std::endl;
                        std::cout << "  減法: " << diff.toString(4) << std::endl;
                        std::cout << "  乘法: " << prod.toString(4) << std::endl;
                        std::cout << "  除法: " << quot.toString(4) << std::endl;
                    }
                    else
                    {
                        std::cout << "節點不是浮點數資料!" << std::endl;
                    }
                }
                else
                {
                    std::cout << "找不到指定的節點!" << std::endl;
                }
            }
            else if (choice == "0")
            {
                break;
            }
        }
    }
};

} // namespace CXXL

#endif // __DATABASE_UI_HPP_CxxlMan3
