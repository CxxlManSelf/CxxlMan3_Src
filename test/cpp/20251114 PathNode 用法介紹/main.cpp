#include <iostream>
#include <string>
#include <vector>
#include <pathnode.hpp>

static std::string toUTF8(const std::u8string &s)
{
	std::string out;
	out.reserve(s.size());
	for (char8_t ch : s) out.push_back(static_cast<char>(ch));
	return out;
}

// 輔助函數：印出分隔線
void printSection(const std::string &title)
{
	std::cout << "\n========== " << title << " ==========\n";
}

int main()
{
	using NODE = CXXL::PathNode<std::string>;

	{ // for CXXL::AsyncNodeDeletor::wait();
		auto root = NODE::createRoot(u8"root");
		if (!root) {
			std::cout << "createRoot failed\n";
			return 1;
		}

		// ========================================
		// 示範 1: // 創建單層無名節點
		// ========================================
		printSection("1. Single slash // creates one unnamed node");

		// /a//c 會創建: a -> 無名節點 -> c
		auto node1 = root->createNodeByPath(u8"/a//c");
		if (node1) {
			std::cout << "Created: /a//c\n";
			std::cout << "Result:  " << toUTF8(node1->getCurrentPath()) << "\n";
			std::cout << "說明：a 下有一個無名節點（位置 0），無名節點下有 c\n";
		}

		// ========================================
		// 示範 2: /// 創建兩層無名節點
		// ========================================
		printSection("2. Triple slash /// creates two unnamed nodes");

		// /b///d 會創建: b -> 無名節點 -> 無名節點 -> d
		auto node2 = root->createNodeByPath(u8"/b///d");
		if (node2) {
			std::cout << "Created: /b///d\n";
			std::cout << "Result:  " << toUTF8(node2->getCurrentPath()) << "\n";
			std::cout << "說明：兩個 {0} 是不同節點，都是各自父節點的第 0 個子節點\n";
		}

		// ========================================
		// 示範 3: {N} 索引的真實含義
		// ========================================
		printSection("3. Understanding {N} index (includes named and unnamed)");

		// 建立混合的有名和無名節點
		root->createNodeByPath(u8"/parent/named1");
		root->createNodeByPath(u8"/parent/");        // 無名節點
		root->createNodeByPath(u8"/parent/named2");
		root->createNodeByPath(u8"/parent/");        // 第二個無名節點

		std::cout << "Created nodes under /parent:\n";
		std::cout << "  - named1 (位置 0，有名節點，不能用 {0} 訪問)\n";
		std::cout << "  - unnamed (位置 1，可用 /parent/{1} 訪問)\n";
		std::cout << "  - named2 (位置 2，有名節點，不能用 {2} 訪問)\n";
		std::cout << "  - unnamed (位置 3，可用 /parent/{3} 訪問)\n\n";

		std::cout << "Children of /parent:\n";
		for (auto &n : root->listChildren(u8"/parent")) {
			std::cout << "  - " << toUTF8(n) << "\n";
		}

		// 驗證只能用 {1} 和 {3} 訪問無名節點
		auto testNode1 = root->findNodeByPath(u8"/parent/{0}");
		std::cout << "\n/parent/{0} -> " << (testNode1 ? "Found (named1 is named, should fail)" : "Not found (correct)") << "\n";

		auto testNode2 = root->findNodeByPath(u8"/parent/{1}");
		std::cout << "/parent/{1} -> " << (testNode2 ? toUTF8(testNode2->getCurrentPath()) + " (correct)" : "Not found") << "\n";

		// ========================================
		// 示範 4: // vs {N} 的重要差異
		// ========================================
		printSection("4. Critical difference: // (create) vs {N} (reference)");

		// 4a) // 可以創建不存在的父節點
		auto create1 = root->createNodeByPath(u8"/newpath//data");
		std::cout << "使用 // 創建: /newpath//data\n";
		std::cout << "結果: " << (create1 ? toUTF8(create1->getCurrentPath()) + " (成功)" : "失敗") << "\n";
		std::cout << "說明：即使 /newpath 不存在，// 也會自動創建\n\n";

		// 4b) {N} 不會創建節點，必須已存在
		auto create2 = root->createNodeByPath(u8"/another/{0}/data");
		std::cout << "使用 {N} 創建: /another/{0}/data\n";
		std::cout << "結果: " << (create2 ? toUTF8(create2->getCurrentPath()) : "失敗 (正確)") << "\n";
		std::cout << "說明：{0} 必須已經存在，否則失敗\n\n";

		// 4c) 先用 // 創建，再用 {N} 引用
		auto create3 = root->createNodeByPath(u8"/config//settings");
		std::cout << "先用 // 創建: /config//settings -> " << toUTF8(create3->getCurrentPath()) << "\n";

		auto create4 = root->createNodeByPath(u8"/config/{0}/username");
		std::cout << "再用 {0} 引用: /config/{0}/username -> "
		          << (create4 ? toUTF8(create4->getCurrentPath()) + " (成功)" : "失敗") << "\n";
		std::cout << "說明：{0} 在上一步已創建，所以可以引用\n";

		// ========================================
		// 示範 5: createIntermediates 參數
		// ========================================
		printSection("5. createIntermediates parameter control");

		// 5a) createIntermediates = true (預設)
		auto deep1 = root->createNodeByPath(u8"/level1/level2/level3", true);
		std::cout << "createIntermediates = true:\n";
		std::cout << "/level1/level2/level3 -> "
		          << (deep1 ? toUTF8(deep1->getCurrentPath()) + " (成功創建所有層級)" : "失敗") << "\n\n";

		// 5b) createIntermediates = false (只能創建最後一個)
		auto deep2 = root->createNodeByPath(u8"/new1/new2/new3", false);
		std::cout << "createIntermediates = false:\n";
		std::cout << "/new1/new2/new3 -> "
		          << (deep2 ? toUTF8(deep2->getCurrentPath()) : "失敗 (正確，因為中間節點不存在)") << "\n\n";

		// 5c) createIntermediates = false，但父節點存在
		root->createNodeByPath(u8"/existing/path", true);  // 先創建父節點
		auto deep3 = root->createNodeByPath(u8"/existing/path/leaf", false);
		std::cout << "createIntermediates = false (父節點已存在):\n";
		std::cout << "/existing/path/leaf -> "
		          << (deep3 ? toUTF8(deep3->getCurrentPath()) + " (成功)" : "失敗") << "\n";
		std::cout << "說明：只創建最後的 leaf，因為 /existing/path 已存在\n";

		// ========================================
		// 示範 6: // 和 createIntermediates 的交互
		// ========================================
		printSection("6. Interaction: // with createIntermediates");

		// 6a) // 在中間位置，createIntermediates = true
		auto inter1 = root->createNodeByPath(u8"/path1//data", true);
		std::cout << "createIntermediates=true, // 在中間:\n";
		std::cout << "/path1//data -> "
		          << (inter1 ? toUTF8(inter1->getCurrentPath()) + " (成功)" : "失敗") << "\n\n";

		// 6b) // 在中間位置，createIntermediates = false
		auto inter2 = root->createNodeByPath(u8"/path2//data", false);
		std::cout << "createIntermediates=false, // 在中間:\n";
		std::cout << "/path2//data -> "
		          << (inter2 ? toUTF8(inter2->getCurrentPath()) : "失敗 (正確)") << "\n";
		std::cout << "說明：// 不是最後節點，且 /path2 不存在，所以失敗\n\n";

		// 6c) 單個 / 在最後位置，createIntermediates = false
		root->createNodeByPath(u8"/path3", true);  // 先創建父節點
		auto inter3 = root->createNodeByPath(u8"/path3/", false);
		std::cout << "createIntermediates=false, 單個 / 在最後:\n";
		std::cout << "/path3/ -> "
		          << (inter3 ? toUTF8(inter3->getCurrentPath()) + " (成功)" : "失敗") << "\n";
		std::cout << "說明：單個 / 創建一個尾端無名節點，是路徑的最後節點，所以可以創建\n";

		// ========================================
		// 示範 7: 尾端無名節點
		// ========================================
		printSection("7. Trailing unnamed node (ending with /)");

		auto tail = root->createNodeByPath(u8"/endpoint/");
		if (tail) {
			std::cout << "Created: /endpoint/\n";
			std::cout << "Result:  " << toUTF8(tail->getCurrentPath()) << "\n";

			// 用 {0} 引用尾端無名節點
			auto foundTail = root->findNodeByPath(u8"/endpoint/{0}");
			std::cout << "Find by /endpoint/{0}: "
			          << (foundTail ? toUTF8(foundTail->getCurrentPath()) + " (正確)" : "Not found") << "\n";
		}

		// ========================================
		// 示範 8: 綜合範例 - 複雜路徑結構
		// ========================================
		printSection("8. Complex example");

		// 創建複雜的結構
		root->createNodeByPath(u8"/app/config/db/mysql");
		root->createNodeByPath(u8"/app/config/db/postgres");
		root->createNodeByPath(u8"/app//cache");  // app 下的無名節點包含 cache

		std::cout << "Created complex structure:\n";
		std::cout << "  /app/config/db/mysql\n";
		std::cout << "  /app/config/db/postgres\n";
		std::cout << "  /app//cache (app下的無名節點包含cache)\n\n";

		std::cout << "Children of /app:\n";
		for (auto &n : root->listChildren(u8"/app")) {
			std::cout << "  - " << toUTF8(n);
			if (n == u8"{0}") {
				std::cout << " (無名節點)";
			}
			std::cout << "\n";
		}

		std::cout << "\nChildren of /app/config:\n";
		for (auto &n : root->listChildren(u8"/app/config")) {
			std::cout << "  - " << toUTF8(n) << "\n";
		}

	} // 結束作用域，觸發非同步刪除

	// 等待所有非同步刪除完成
	CXXL::AsyncNodeDeletor::wait();

	std::cout << "\n========== All tests completed ==========\n";

	return 0;
}
