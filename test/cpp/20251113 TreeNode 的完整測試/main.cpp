#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <treenode.hpp>

using namespace CXXL;

// 測試計數器
int g_testCount = 0;
int g_passCount = 0;
int g_failCount = 0;

// 測試巨集
#define TEST_ASSERT(condition, message) \
    do { \
        g_testCount++; \
        if (!(condition)) { \
            std::cout << "  ❌ 測試 " << g_testCount << " 失敗: " << message << std::endl; \
            g_failCount++; \
        } else { \
            std::cout << "  ✅ 測試 " << g_testCount << " 通過: " << message << std::endl; \
            g_passCount++; \
        } \
    } while(0)

#define TEST_SECTION(name) \
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl; \
    std::cout << "📋 " << name << std::endl; \
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

// 自定義資料結構用於測試
struct TestData {
    int value;
    std::string description;

    TestData() : value(0), description("") {}
    TestData(int v, const std::string& desc)
        : value(v), description(desc) {}
};

// 測試函數宣告
void test_basic_creation();
void test_add_children();
void test_remove_children();
void test_find_children();
void test_child_navigation();
void test_move_children();
void test_data_operations();
void test_named_children();
void test_iteration();
void test_thread_safety();
void test_complex_scenarios();

int main(int, char**)
{
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   TreeNode 完整測試套件                         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;

    // 執行所有測試
    test_basic_creation();
    test_add_children();
    test_remove_children();
    test_find_children();
    test_child_navigation();
    test_move_children();
    test_data_operations();
    test_named_children();
    test_iteration();
    test_thread_safety();
    test_complex_scenarios();

    // 等待非同步刪除器完成
    AsyncNodeDeletor::wait();

    // 輸出測試結果
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   測試結果統計                                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
    std::cout << "  總測試數: " << g_testCount << std::endl;
    std::cout << "  通過: " << g_passCount << " ✅" << std::endl;
    std::cout << "  失敗: " << g_failCount << " ❌" << std::endl;

    if (g_failCount == 0) {
        std::cout << "\n  🎉 所有測試都通過了！" << std::endl;
    } else {
        std::cout << "\n  ⚠️  有 " << g_failCount << " 個測試失敗" << std::endl;
    }
    std::cout << "\n";

    return g_failCount > 0 ? 1 : 0;
}

// 測試基本建立功能
void test_basic_creation()
{
    TEST_SECTION("1. 基本建立功能測試");

    // 建立根節點
    auto root = TreeNode<int>::createRoot(u8"根節點");
    TEST_ASSERT(root != nullptr, "建立根節點");
    TEST_ASSERT(root->getName() == u8"根節點", "根節點名稱正確");
    TEST_ASSERT(root->getParent() == nullptr, "根節點沒有父節點");
    TEST_ASSERT(root->childCount() == 0, "根節點初始沒有子節點");

    // 測試 getData 和 setData
    root->setData(42);
    TEST_ASSERT(root->getData() == 42, "根節點資料設定正確");
}

// 測試新增子節點
void test_add_children()
{
    TEST_SECTION("2. 新增子節點測試");

    auto root = TreeNode<std::string>::createRoot(u8"根");

    // addChild (預設為 addBackChild)
    auto child1 = root->addChild(u8"子1");
    TEST_ASSERT(child1 != nullptr, "新增第一個子節點");
    TEST_ASSERT(root->childCount() == 1, "子節點數量為 1");
    TEST_ASSERT(child1->getParent() == root, "子節點的父節點正確");

    // addFrontChild
    auto child2 = root->addFrontChild(u8"子2");
    TEST_ASSERT(child2 != nullptr, "新增前端子節點");
    TEST_ASSERT(root->childCount() == 2, "子節點數量為 2");
    TEST_ASSERT(root->getFirstChild() == child2, "前端子節點位於第一個");

    // addBackChild
    auto child3 = root->addBackChild(u8"子3");
    TEST_ASSERT(child3 != nullptr, "新增後端子節點");
    TEST_ASSERT(root->childCount() == 3, "子節點數量為 3");
    TEST_ASSERT(root->getLastChild() == child3, "後端子節點位於最後一個");

    // insertBefore
    auto child4 = root->insertBefore(child3, u8"子4");
    TEST_ASSERT(child4 != nullptr, "在子3前插入子4");
    TEST_ASSERT(root->childCount() == 4, "子節點數量為 4");
    auto pos4 = root->getChildPosition(child4);
    TEST_ASSERT(pos4.has_value() && pos4.value() == 2, "子4位於索引2");

    // insertAfter
    auto child5 = root->insertAfter(child1, u8"子5");
    TEST_ASSERT(child5 != nullptr, "在子1後插入子5");
    TEST_ASSERT(root->childCount() == 5, "子節點數量為 5");
    auto pos5 = root->getChildPosition(child5);
    TEST_ASSERT(pos5.has_value() && pos5.value() == 2, "子5位於索引2");

    // 測試無名稱節點
    auto unnamed1 = root->addChild();
    auto unnamed2 = root->addChild();
    TEST_ASSERT(unnamed1 != nullptr && unnamed2 != nullptr, "可以建立多個無名稱節點");
    TEST_ASSERT(root->childCount() == 7, "包含無名稱節點的總數");
}

// 測試刪除子節點
void test_remove_children()
{
    TEST_SECTION("3. 刪除子節點測試");

    auto root = TreeNode<int>::createRoot(u8"根");
    auto child1 = root->addChild(u8"子1");
    auto child2 = root->addChild(u8"子2");
    auto child3 = root->addChild(u8"子3");

    // removeChild
    bool removed = root->removeChild(child2);
    TEST_ASSERT(removed, "成功刪除子2");
    TEST_ASSERT(root->childCount() == 2, "刪除後剩2個子節點");
    TEST_ASSERT(child2->getParent() == nullptr, "被刪除節點的父節點已清空");

    // removeChildByName
    removed = root->removeChildByName(u8"子1");
    TEST_ASSERT(removed, "依名稱刪除子1");
    TEST_ASSERT(root->childCount() == 1, "刪除後剩1個子節點");

    // 新增更多節點測試 removeFrontChild 和 removeBackChild
    root->addFrontChild(u8"前");
    root->addBackChild(u8"後");
    TEST_ASSERT(root->childCount() == 3, "新增後有3個子節點");

    removed = root->removeFrontChild();
    TEST_ASSERT(removed, "刪除前端子節點");
    TEST_ASSERT(root->childCount() == 2, "刪除前端後剩2個");

    removed = root->removeBackChild();
    TEST_ASSERT(removed, "刪除後端子節點");
    TEST_ASSERT(root->childCount() == 1, "刪除後端後剩1個");

    // clearChildren
    root->addChild(u8"A");
    root->addChild(u8"B");
    root->clearChildren();
    TEST_ASSERT(root->childCount() == 0, "清空所有子節點");
}

// 測試查找子節點
void test_find_children()
{
    TEST_SECTION("4. 查找子節點測試");

    auto root = TreeNode<double>::createRoot(u8"根");
    auto child1 = root->addChild(u8"Alpha");
    auto child2 = root->addChild(u8"Beta");
    auto child3 = root->addChild(u8"Gamma");

    // findChildByName
    auto found = root->findChildByName(u8"Beta");
    TEST_ASSERT(found == child2, "依名稱找到 Beta");

    found = root->findChildByName(u8"NotExist");
    TEST_ASSERT(found == nullptr, "找不到不存在的節點");

    // hasChild (by name)
    TEST_ASSERT(root->hasChild(u8"Alpha"), "hasChild 找到 Alpha");
    TEST_ASSERT(!root->hasChild(u8"Delta"), "hasChild 找不到 Delta");

    // hasChild (by pointer)
    TEST_ASSERT(root->hasChild(child1), "hasChild 用指標找到 child1");
    auto orphan = TreeNode<double>::createRoot(u8"孤兒");
    TEST_ASSERT(!root->hasChild(orphan), "hasChild 找不到孤兒節點");

    // getChildAt
    auto at1 = root->getChildAt(1);
    TEST_ASSERT(at1 == child2, "getChildAt(1) 是 child2");

    auto at_invalid = root->getChildAt(999);
    TEST_ASSERT(at_invalid == nullptr, "無效索引返回 nullptr");

    // getChildPosition
    auto pos = root->getChildPosition(child3);
    TEST_ASSERT(pos.has_value() && pos.value() == 2, "child3 在索引 2");
}

// 測試子節點導航
void test_child_navigation()
{
    TEST_SECTION("5. 子節點導航測試");

    auto root = TreeNode<int>::createRoot(u8"根");
    auto child1 = root->addChild(u8"1");
    auto child2 = root->addChild(u8"2");
    auto child3 = root->addChild(u8"3");

    // getFirstChild / getLastChild
    TEST_ASSERT(root->getFirstChild() == child1, "第一個子節點是 child1");
    TEST_ASSERT(root->getLastChild() == child3, "最後一個子節點是 child3");

    // getNextChild
    auto next = root->getNextChild(child1);
    TEST_ASSERT(next == child2, "child1 的下一個是 child2");

    next = root->getNextChild(child3);
    TEST_ASSERT(next == nullptr, "child3 沒有下一個");

    // getPreviousChild
    auto prev = root->getPreviousChild(child3);
    TEST_ASSERT(prev == child2, "child3 的上一個是 child2");

    prev = root->getPreviousChild(child1);
    TEST_ASSERT(prev == nullptr, "child1 沒有上一個");
}

// 測試移動子節點
void test_move_children()
{
    TEST_SECTION("6. 移動子節點測試");

    auto root = TreeNode<int>::createRoot(u8"根");
    auto child1 = root->addChild(u8"1");
    auto child2 = root->addChild(u8"2");
    auto child3 = root->addChild(u8"3");
    auto child4 = root->addChild(u8"4");

    // moveChildToFront
    bool moved = root->moveChildToFront(child3);
    TEST_ASSERT(moved, "移動 child3 到最前");
    TEST_ASSERT(root->getFirstChild() == child3, "child3 現在是第一個");

    // moveChildToBack
    moved = root->moveChildToBack(child1);
    TEST_ASSERT(moved, "移動 child1 到最後");
    TEST_ASSERT(root->getLastChild() == child1, "child1 現在是最後一個");

    // moveChildBefore
    moved = root->moveChildBefore(child4, child3);
    TEST_ASSERT(moved, "移動 child4 到 child3 之前");
    auto pos = root->getChildPosition(child4);
    TEST_ASSERT(pos.has_value() && pos.value() == 0, "child4 現在在索引 0");

    // moveChildAfter
    moved = root->moveChildAfter(child2, child1);
    TEST_ASSERT(moved, "移動 child2 到 child1 之後");
    TEST_ASSERT(root->getLastChild() == child2, "child2 現在是最後一個");
}

// 測試資料操作
void test_data_operations()
{
    TEST_SECTION("7. 資料操作測試");

    auto root = TreeNode<TestData>::createRoot(u8"根");

    // setData (copy)
    TestData data1(100, "測試資料1");
    root->setData(data1);
    TEST_ASSERT(root->getData().value == 100, "資料值正確 (copy)");
    TEST_ASSERT(root->getData().description == "測試資料1", "資料描述正確 (copy)");

    // setData (move)
    TestData data2(200, "測試資料2");
    root->setData(std::move(data2));
    TEST_ASSERT(root->getData().value == 200, "資料值正確 (move)");
    TEST_ASSERT(root->getData().description == "測試資料2", "資料描述正確 (move)");

    // 子節點資料
    auto child = root->addChild(u8"子");
    child->setData(TestData(300, "子節點資料"));
    TEST_ASSERT(child->getData().value == 300, "子節點資料值正確");
}

// 測試具名子節點的特殊規則
void test_named_children()
{
    TEST_SECTION("8. 具名子節點測試");

    auto root = TreeNode<int>::createRoot(u8"根");

    // 同名節點不可重複
    auto child1 = root->addChild(u8"重複名稱");
    auto child2 = root->addChild(u8"重複名稱");
    TEST_ASSERT(child1 != nullptr, "第一個同名節點建立成功");
    TEST_ASSERT(child2 == nullptr, "第二個同名節點建立失敗");
    TEST_ASSERT(root->childCount() == 1, "只有一個節點被建立");

    // 無名節點可以重複
    auto unnamed1 = root->addChild();
    auto unnamed2 = root->addChild();
    auto unnamed3 = root->addChild();
    TEST_ASSERT(unnamed1 != nullptr && unnamed2 != nullptr && unnamed3 != nullptr,
                "多個無名節點都建立成功");
    TEST_ASSERT(root->childCount() == 4, "總共4個節點（1個具名 + 3個無名）");

    // 刪除具名節點後可以再建立同名節點
    root->removeChildByName(u8"重複名稱");
    auto child3 = root->addChild(u8"重複名稱");
    TEST_ASSERT(child3 != nullptr, "刪除後可以再建立同名節點");
}

// 測試迭代功能
void test_iteration()
{
    TEST_SECTION("9. 迭代功能測試");

    auto root = TreeNode<int>::createRoot(u8"根");
    root->addChild(u8"1")->setData(1);
    root->addChild(u8"2")->setData(2);
    root->addChild(u8"3")->setData(3);

    // forEachChild
    int sum = 0;
    root->forEachChild([&sum](const auto& child) {
        sum += child->getData();
    });
    TEST_ASSERT(sum == 6, "forEachChild 正向遍歷（1+2+3=6）");

    // forEachChildReverse
    std::vector<int> values;
    root->forEachChildReverse([&values](const auto& child) {
        values.push_back(child->getData());
    });
    TEST_ASSERT(values.size() == 3, "反向遍歷取得3個值");
    TEST_ASSERT(values[0] == 3 && values[1] == 2 && values[2] == 1,
                "反向遍歷順序正確（3,2,1）");

    // 使用 iterator (begin/end)
    int count = 0;
    for (auto it = root->begin(); it != root->end(); ++it) {
        count++;
    }
    TEST_ASSERT(count == 3, "使用 iterator 遍歷3個節點");

    // 使用 range-based for
    count = 0;
    for (const auto& child : *root) {
        count++;
    }
    TEST_ASSERT(count == 3, "使用 range-based for 遍歷3個節點");
}

// 測試執行緒安全性（基本測試）
void test_thread_safety()
{
    TEST_SECTION("10. 執行緒安全性基本測試");

    auto root = TreeNode<int>::createRoot(u8"根");

    // 建立多個執行緒同時新增子節點
    std::vector<std::thread> threads;
    const int threadCount = 10;
    const int nodesPerThread = 10;

    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back([&root, i]() {
            for (int j = 0; j < nodesPerThread; j++) {
                // 使用無名節點避免名稱衝突
                root->addChild();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    TEST_ASSERT(root->childCount() == threadCount * nodesPerThread,
                "多執行緒新增節點數量正確");

    // 測試索引一致性
    TEST_ASSERT(root->validateIndexes(), "多執行緒操作後索引一致性正確");
}

// 測試複雜場景
void test_complex_scenarios()
{
    TEST_SECTION("11. 複雜場景測試");

    // 建立多層樹狀結構
    auto root = TreeNode<std::string>::createRoot(u8"公司");
    root->setData("總公司");

    auto rd = root->addChild(u8"研發部");
    rd->setData("Research & Development");
    auto rd_ai = rd->addChild(u8"AI組");
    auto rd_web = rd->addChild(u8"Web組");

    auto sales = root->addChild(u8"業務部");
    sales->setData("Sales");
    auto sales_domestic = sales->addChild(u8"國內");
    auto sales_foreign = sales->addChild(u8"國外");

    auto hr = root->addChild(u8"人資部");
    hr->setData("Human Resources");

    TEST_ASSERT(root->childCount() == 3, "根節點有3個部門");
    TEST_ASSERT(rd->childCount() == 2, "研發部有2個組");
    TEST_ASSERT(sales->childCount() == 2, "業務部有2個區域");
    TEST_ASSERT(hr->childCount() == 0, "人資部沒有子部門");

    // 測試跨層級查找
    TEST_ASSERT(rd_ai->getParent() == rd, "AI組的父節點是研發部");
    TEST_ASSERT(rd->getParent() == root, "研發部的父節點是公司");

    // 測試部門重組（移動節點）
    sales->moveChildToFront(sales_foreign);
    TEST_ASSERT(sales->getFirstChild() == sales_foreign, "國外業務移到最前");

    // 刪除整個部門
    root->removeChild(hr);
    TEST_ASSERT(root->childCount() == 2, "刪除人資部後剩2個部門");
    TEST_ASSERT(!root->hasChild(u8"人資部"), "人資部已不存在");

    // 統計所有員工（示範遞迴遍歷）
    std::function<int(const std::shared_ptr<const TreeNode<std::string>>&)> countNodes;
    countNodes = [&countNodes](const auto& node) -> int {
        int count = 1; // 自己
        node->forEachChild([&count, &countNodes](const auto& child) {
            count += countNodes(child);
        });
        return count;
    };

    int totalNodes = countNodes(root);
    TEST_ASSERT(totalNodes == 7, "整個組織共7個節點（含根）");

    // 測試清空後重建
    root->clearChildren();
    TEST_ASSERT(root->childCount() == 0, "清空所有部門");

    root->addChild(u8"新部門1");
    root->addChild(u8"新部門2");
    TEST_ASSERT(root->childCount() == 2, "重建2個新部門");

    // 測試索引完整性
    TEST_ASSERT(root->validateIndexes(), "複雜操作後索引一致性正確");
}
