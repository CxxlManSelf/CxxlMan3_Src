/************************************************
 * pathnode.hpp 1.1.2
 *
 * 基於 TreeNodeBase 的路徑操作節點類別
 * 提供類似檔案系統的路徑操作功能
 *
 * 根節點的名稱不會出現在完整路徑中，而
 * 是 "/" 代表根節點
 * 空名節點的名稱為 {N}
 * 
 * 建立節點(createNodeByPath())時可以指定從根節點(以 "/" 開頭)開
 * 始搜尋，還是從目前節點開始搜尋
 * 
 * 建立節點最後的參數 createIntermediates 可以指定是否自動創建中間
 * 不存在的節點(還是內定)，還是只能創建最後的節點(但中間節點不存在不會建立) * 
 * 
 * 另外建立空名節點可以這樣做
 * 保留尾端的空段 ("/a/b/" -> ["/","a","b",""]) (不能這樣用 "/a/b/{N}")
 * 保留中間的連續空段 ("/a//c" -> ["/","a","","c"]) (createIntermediates == true 時)
 *
 * 建立節點可以用 {N} 特殊標記指定中間節點，但 {N} 若不存在，不會建立
 * 
 * 不能用空名取得子節點，須用 {N} 特殊標記
 * 
 * 特殊標記說明：
 * - {0}, {1}, {2}... 用於標記無名子節點的位置
 * - 這些標記不能用作一般節點名稱
 * - 位置計算包含有名和無名子節點，即陣列的 index 值
 *
 * 保留字說明：
 * - {0}, {1}, {2}... 用於標記無名子節點的位
 *   置，不能用作一般節點名稱
 * - "." 代表當前節點，不能用作一般節點名稱
 * - ".." 代表父節點，不能用作一般節點名稱
 * - "/" 代表節點的分隔符，名稱中不可包含此字元
 *
 * Author: CxxlMan
 * Date: 2025-
 ************************************************/
#ifndef __CXXLCOMMON_PATHNODE_HPP_CxxlMan3
#define __CXXLCOMMON_PATHNODE_HPP_CxxlMan3


#include "treenode.hpp"

namespace CXXL
{

// 路徑操作節點類別
// D: 延伸類別
template <typename D>
class PathNodeBase : public TreeNodeBase<D>
{

    using NODE_PTR = std::shared_ptr<D>;
    using CNODE_PTR = std::shared_ptr<const D>;

    // 路徑分隔符
    static constexpr char8_t PATH_SEPARATOR = u8'/';

    // 判斷是否為位置標記，例如 "{0}", "{1}"...
    // 位置標記格式：'{' 數字 '}' 且不含其他字元
    static bool isPositionMarker(const std::u8string &name)
    {
        if (name.empty())
            return false;

        if (name.size() < 3)
            return false; // 最短為 "{0}"
        if (name.front() != u8'{' || name.back() != u8'}')
            return false;
        for (size_t i = 1; i + 1 < name.size(); ++i)
        {
            char8_t c = name[i];
            if (c < u8'0' || c > u8'9')
                return false;
        }
        return true;
    }

    // 從位置標記中提取位置編號 (如 "{3}" -> 3)
    static std::optional<size_t> extractPosition(const std::u8string &marker)
    {
        if (!isPositionMarker(marker))
            return std::nullopt;

        std::string markerStr(marker.begin(), marker.end());
        size_t start = markerStr.find('{') + 1;
        size_t end = markerStr.find('}');
        std::string numStr = markerStr.substr(start, end - start);

        try
        {
            unsigned long long value = std::stoull(numStr);
            if (value > std::numeric_limits<size_t>::max())
                return std::nullopt;
            return static_cast<size_t>(value);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    // 生成位置標記字串 (如 3 -> "{3}")
    static std::u8string generatePositionMarker(
        size_t position)
    {
        std::string marker = "{" +
                                std::to_string(position) + "}";
        return std::u8string(marker.begin(), marker.end());
    }

    // 分割路徑字串
    // path 若是絕對路徑則在開頭加上分隔符 '/'
    // 僅保留尾端的空段 ("/a/b/" -> ["/","a","b",""])
    // 保留中間的連續空段 ("/a//c" -> ["/","a","","c"])
    static std::vector<std::u8string> splitPath(
        const std::u8string &path)
    {
        std::vector<std::u8string> segments;

        if (path.empty())
            return segments;

        // 單獨的根路徑 "/"
        if (path.size() == 1 && path[0] == PATH_SEPARATOR)
        {
            segments.emplace_back(u8"/");
            return segments;
        }

        // 是絕對路徑（保留一個 "/" 標記在最前面）
        size_t len = path.size();
        size_t i = 0;
        if (path[0] == PATH_SEPARATOR)
        {
            segments.emplace_back(u8"/");
            ++i;
        }

        std::u8string segment;
        for (; i < len; ++i)
        {
            char8_t ch = path[i];
            if (ch == PATH_SEPARATOR)
            {
                // 每遇到 '/' 就 push 當前 segment（允許為空，代表無名節點），並清空 segment
                segments.push_back(segment);
                segment.clear();
            }
            else
            {
                segment.push_back(ch);
            }
        }

        // 推入最後一個 segment（可能為空，如果路徑以 '/' 結尾則最後一次 separator 已 push 出空段）
        segments.push_back(segment);

        return segments;
    }

    // 根據子節點在父節點中的實際位置標記
    std::optional<std::u8string> getChildPositionMarker(
        const CNODE_PTR &child) const
    {
        auto position = this->getChildPosition(child);
        if (!position.has_value())
            return std::nullopt;
        return generatePositionMarker(position.value());
    }

protected:

    // TreeNodeBase 的要求
    // 可以用指定的名字建立子節點嗎?
    // - 合法：空字串（允許無名節點）、或 非 "{N}" 保留樣式
    //         不是 "." ".."
    //         不包含 "/"
    static bool canCreateChild(const std::u8string &name)
    {
        if (name == u8"." || name == u8".." ||
            name.find(u8'/') != std::u8string::npos)
            return false;

        if (isPositionMarker(name))
            return false;

        return TreeNodeBase<D>::canCreateChild(name);
    }


    // 建構函式 - TreeNodeBase 的要求
    explicit PathNodeBase(const std::u8string &name) : TreeNodeBase<D>(name)
    {
    }

public:
    // 獲取節點的完整路徑
    // 回傳值：從根節點到當前節點的完整路徑
    // 若有錯回傳空字串
    std::u8string getCurrentPath() const
    {
        // 傳回 "/aa/bb/cc"
        std::list<std::u8string> pathComponents;

        // 從當前節點向上遍歷到根節點
        auto current = this->getSelf();
        auto parent = current->getParent();

        // 無父節點的節點是根節點
        while (parent)
        {
            // 是無名節點
            if (current->getName().empty())
            {
                // 無名節點：使用位置標記
                auto marker =
                    parent->getChildPositionMarker(current);
                if (marker.has_value())
                {
                    // 放在pathComponents最前面
                    pathComponents.push_front(
                        marker.value());
                }
                else
                    return u8"";
            }
            else
            {
                // 有名節點：使用節點名稱
                pathComponents.push_front(
                    current->getName());
            }

            current = parent;
            parent = current->getParent();
        }

        // 本身是根節點
        if (pathComponents.empty())
            return u8"/";

        std::u8string result = u8"/";

        for (const std::u8string &pathComponent :
                pathComponents)
        {
            if (result != u8"/")
                result += u8"/";
            result += pathComponent;
        }

        return result;
    }

    // 根據路徑查找節點
    // path: 要查找的路徑 (絕對路徑以 "/" 開頭，
    //       相對路徑則從當前節點開始)
    // 回傳值：找到的節點，找不到則回傳 nullptr
    [[nodiscard]] CNODE_PTR findNodeByPath(
        const std::u8string &path) const
    {
        if (path.empty())
            return nullptr;

        auto components = splitPath(path);
        if (components.empty())
            return nullptr;

        CNODE_PTR current;

        // 決定搜尋的起始點，是以絕對路徑開頭還是相對路徑
        if (components[0] == u8"/")
        {
            current = getRootNode();
            components.erase(components.begin());
        }
        else
            current = this->getSelf();

        // 逐步按路徑巡航到最後一個節點即為要找的節點
        for (const auto &component : components)
        {
            if (component == u8"..")
                current = current->getParent();
            else if (component == u8".")
                continue;
            else
            {
                // 檢查是否為空名子節點的位置標記
                if (isPositionMarker(component))
                {
                    // 位置標記：按位置查找子節點
                    auto position =
                        extractPosition(component);
                    if (position.has_value())
                    {
                        current = current->getChildAt(
                            position.value());
                        // 須是無名子節點
                        if (current && !current->getName().empty())
                            return nullptr;
                    }
                    else
                        return nullptr;
                }
                else
                    // 一般名稱：按名稱查找子節點
                    current =
                        current->findChildByName(component);
            }

            if (!current)
                return nullptr;
        }

        return current;
    }

    // 根據路徑查找節點
    // path: 要查找的路徑 (絕對路徑以 "/" 開頭，
    //       相對路徑則從當前節點開始)
    // 回傳值：找到的節點，找不到則回傳 nullptr
    [[nodiscard]] NODE_PTR findNodeByPath(const std::u8string &path)
    {
        return std::const_pointer_cast<D>(
            (static_cast<const PathNodeBase<D>*>(this)->findNodeByPath(path))
        );
    }

    // 根據路徑創建節點
    // path: 要創建的節點路徑
    // createIntermediates: 是否自動創建中間路徑的節點
    // 回傳值：創建的節點，失敗則回傳 nullptr
    // ❌ [[nodiscard]] NODE_PTR createNodeByPath( 
    // 可以純建立子節點而不使用
    NODE_PTR createNodeByPath(
        const std::u8string &path,
        bool createIntermediates = true)
    {
        if (path.empty())
            return nullptr;

        auto components = splitPath(path);
        if (components.empty())
            return nullptr;

        NODE_PTR current;

        // 決定搜尋的起始點，是以絕對路徑開頭還是相對路徑
        if (components.front() == u8"/")
        {
            // 絕對路徑：從根節點開始
            current = getRootNode();
            components.erase(components.begin());
        }
        else
        {
            // 相對路徑：從當前節點開始
            // 使用 TreeNodeBase 的 m_self 成員來獲取
            // 當前節點的 shared_ptr
            current = this->getSelf();
        }

        // 可以巡行到的最後節點
        NODE_PTR last = current;

        // 逐步按路徑巡航至不存在為止
        auto it = components.begin();
        while (it != components.end())
        {
            const std::u8string &component = *it;
            if (component == u8"..")
            {
                current = current->getParent();
                if (!current) // 路徑錯誤
                    return nullptr;
            }
            else if (component == u8".")
            {
                ++it;
                continue;
            }
            else
            {
                // 檢查是否為空名子節點的位置標記
                if (isPositionMarker(component))
                {
                    // 位置標記：按位置查找子節點
                    auto position =
                        extractPosition(component);
                    if (position.has_value())
                    {
                        current = current->getChildAt(
                            position.value());
                        // 須是無名子節點
                        if (current && !current->getName().empty())
                            return nullptr;
                    }
                    else
                        return nullptr;
                }
                else
                {
                    // 一般名稱：按名稱查找子節點
                    current =
                        current->findChildByName(component);
                }
            }

            if (!current)
                break;

            last = current;
            ++it;
        }

        // 全部存在不用創建
        if (it == components.end())
            return last;

        // 如果不允許創建中間路徑的節點
        if (!createIntermediates && it + 1 != components.end())
            return nullptr;

        current = last;
        NODE_PTR killNode = nullptr; // 若建立失敗將由這節點刪除
        const std::u8string *pComponent;

        // 開始創建不存在的節點
        while (it != components.end())
        {
            pComponent = &(*it);
            if (*pComponent == u8"..")
            {
                break;
            }
            else if (*pComponent == u8".")
            {
                break;
            }
            else
            {
                current = current->addChild(*pComponent);
                if (current && killNode == nullptr)
                    killNode = current;
            }

            if (!current)
                break;

            ++it;
        }

        if (it != components.end())
        {
            // 刪除所有創建的子節點
            if (killNode)
            {
                auto parent = killNode->getParent();
                if (parent)
                    parent->removeChild(killNode);
            }
            return nullptr;
        }

        return current;
    }

    // 根據路徑刪除節點
    // path: 要刪除的節點路徑
    // 回傳值：是否成功刪除
    bool removeNodeByPath(const std::u8string &path)
    {
        auto nodeToRemove = findNodeByPath(path);
        if (!nodeToRemove)
            return false;

        auto parent = nodeToRemove->getParent();
        if (!parent)
        {
            // 不能刪除根節點
            return false;
        }

        return parent->removeChild(nodeToRemove);
    }

    // 列出指定路徑下的所有子節點名稱和位置標記
    // path: 要列出的路徑 (空字串表示當前節點)
    // 回傳值：子節點的名稱列表，無名節點以位置標記表示
    std::vector<std::u8string> listChildren(
        const std::u8string &path = u8"") const
    {
        std::vector<std::u8string> result;

        CNODE_PTR targetNode;
        if (path.empty())
        {
            // 使用 TreeNodeBase 的 m_self 成員來獲取
            // 當前節點的 shared_ptr
            targetNode = this->getSelf();
            if (!targetNode)
                return result; // 如果 m_self 失效則回傳空結果
        }
        else
        {
            targetNode = findNodeByPath(path);
        }

        if (!targetNode)
            return result;

        // 遍歷所有子節點
        size_t position = 0;
        CNODE_PTR child = targetNode->getFirstChild();
        while (child)
        {
            if (child->getName().empty())
            {
                // 無名子節點：使用位置標記
                result.push_back(
                    generatePositionMarker(position));
            }
            else
            {
                // 有名子節點：使用節點名稱
                result.push_back(child->getName());
            }
            ++position;
            child = targetNode->getNextChild(child);
        }

        return result;
    }

    // 獲取根節點
    CNODE_PTR getRootNode() const
    {
        // 使用 TreeNodeBase 的 m_self 成員來獲取
        // 當前節點的 shared_ptr
        CNODE_PTR current = this->getSelf();

        // 向上追溯直到找到根節點（沒有父節點的節點）
        while (auto parent = current->getParent())
        {
            current = parent;
        }

        return current;
    }

    // 獲取根節點
    NODE_PTR getRootNode()
    {
        return std::const_pointer_cast<D>(
            ((const PathNodeBase<D> *)this)->getRootNode());
    }

    // 創建根節點的靜態方法
    static NODE_PTR createRoot(const std::u8string &name)
    {
        return TreeNodeBase<D>::createRoot(name);
    }

    // 使 TreeNodeBase 能夠存取 protected 建構函式
    template <typename T>
    friend class TreeNodeBase;
};

// 路徑節點的簡單實作範例
template <typename T>
class PathNode : public PathNodeBase<PathNode<T>>
{

    T m_data; // 節點資料

protected:

    // TreeNodeBase 的要求
    // 可以用指定的名字建立子節點嗎?
    static bool canCreateChild(const std::u8string &name)
    {
        return PathNodeBase<PathNode<T>>::canCreateChild(name);
    }

    // 建構函式
    explicit PathNode(const std::u8string &name)
        : PathNodeBase<PathNode<T>>(name) {}

public:
    // 資料存取方法
    T &getData() { return m_data; }
    const T &getData() const { return m_data; }
    void setData(const T &data) { m_data = data; }
    void setData(T &&data) { m_data = std::move(data); }

    // 創建根節點
    static std::shared_ptr<PathNode<T>> createRoot(const std::u8string &name)
    {
        return std::static_pointer_cast<PathNode<T>>(PathNodeBase<PathNode<T>>::createRoot(name));
    }

    // 使基底類別能夠存取 protected 建構函式
    template <typename D>
    friend class PathNodeBase;
    template <typename D>
    friend class TreeNodeBase;
};

}
#endif // #define __CXXLCOMMON_PATHNODE_HPP_CxxlMan3