/***********************************************************
 * database_types.hpp
 *
 * 定義階層式資料庫的資料類型介面和實作
 * 使用 IDataBase 作為基礎介面
 * 可以自由擴充各種資料類型而不需要修改源碼
 *
 * Author: CxxlMan
 * Date: 2025-
 ************************************************************/
#ifndef __DATABASE_TYPES_HPP_CxxlMan3
#define __DATABASE_TYPES_HPP_CxxlMan3

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cmath>

namespace CXXL
{

// 資料庫介面基類
class IDataBase
{
public:
    virtual ~IDataBase() = default;

    // 取得資料類型名稱
    virtual std::string getTypeName() const = 0;

    // 序列化為字串
    virtual std::string serialize() const = 0;

    // 從字串反序列化
    virtual bool deserialize(const std::string& str) = 0;

    // 複製
    virtual std::shared_ptr<IDataBase> clone() const = 0;
};

// ============ 工具函數 ============

// 解析 "key: 'value'" 格式的字串
// 注意：會跳過序列化字串開頭的 "TypeName|" 部分
inline std::map<std::string, std::string> parseFields(const std::string& str)
{
    std::map<std::string, std::string> fields;

    // 先找到第一個 '|' 之後的內容（跳過類型名稱）
    size_t start_pos = str.find('|');
    if (start_pos == std::string::npos)
        return fields;

    start_pos++; // 跳過 '|' 本身

    std::string current_key;
    std::string current_value;
    bool in_value = false;
    bool escaped = false;

    for (size_t i = start_pos; i < str.size(); ++i)
    {
        char c = str[i];

        if (escaped)
        {
            current_value += c;
            escaped = false;
            continue;
        }

        if (c == '\\' && in_value)
        {
            escaped = true;
            continue;
        }

        if (c == '\'' && !in_value)
        {
            in_value = true;
            continue;
        }

        if (c == '\'' && in_value)
        {
            in_value = false;
            if (!current_key.empty())
            {
                fields[current_key] = current_value;
                current_key.clear();
                current_value.clear();
            }
            continue;
        }

        if (in_value)
        {
            current_value += c;
        }
        else if (c == ':')
        {
            // 找到 key，去除前後空白
            current_key.erase(0, current_key.find_first_not_of(" \t"));
            current_key.erase(current_key.find_last_not_of(" \t") + 1);
            continue;
        }
        else if (!std::isspace(c) && c != ':' && c != '|')
        {
            current_key += c;
        }
    }

    return fields;
}

// 逸出字串中的單引號
inline std::string escapeValue(const std::string& str)
{
    std::string result;
    for (char c : str)
    {
        if (c == '\'')
            result += "\\'";
        else if (c == '\\')
            result += "\\\\";
        else
            result += c;
    }
    return result;
}

// ============ 具體資料類型 ============

// 字串資料
class StringData : public IDataBase
{
    std::string m_value;

public:
    StringData() = default;
    StringData(const std::string& value) : m_value(value) {}

    std::string getValue() const { return m_value; }
    void setValue(const std::string& value) { m_value = value; }

    std::string getTypeName() const override { return "StringData"; }

    std::string serialize() const override
    {
        return "StringData| value: '" + escapeValue(m_value) + "'";
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("value") != fields.end())
        {
            m_value = fields["value"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<StringData>(m_value);
    }
};

// 人員資料
class PersonData : public IDataBase
{
    std::string m_age;
    std::string m_email;

public:
    PersonData() = default;
    PersonData(const std::string& age, const std::string& email)
        : m_age(age), m_email(email) {}

    std::string getAge() const { return m_age; }
    std::string getEmail() const { return m_email; }

    void setAge(const std::string& age) { m_age = age; }
    void setEmail(const std::string& email) { m_email = email; }

    std::string getTypeName() const override { return "PersonData"; }

    std::string serialize() const override
    {
        std::ostringstream oss;
        oss << "PersonData| age: '" << escapeValue(m_age)
            << "' email: '" << escapeValue(m_email) << "'";
        return oss.str();
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("age") != fields.end() &&
            fields.find("email") != fields.end())
        {
            m_age = fields["age"];
            m_email = fields["email"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<PersonData>(m_age, m_email);
    }
};

// 產品資料
class ProductData : public IDataBase
{
    std::string m_name;
    std::string m_basePrice;
    std::string m_category;

public:
    ProductData() = default;
    ProductData(const std::string& name, const std::string& basePrice, const std::string& category)
        : m_name(name), m_basePrice(basePrice), m_category(category) {}

    std::string getName() const { return m_name; }
    std::string getBasePrice() const { return m_basePrice; }
    std::string getCategory() const { return m_category; }

    void setName(const std::string& name) { m_name = name; }
    void setBasePrice(const std::string& basePrice) { m_basePrice = basePrice; }
    void setCategory(const std::string& category) { m_category = category; }

    std::string getTypeName() const override { return "ProductData"; }

    std::string serialize() const override
    {
        std::ostringstream oss;
        oss << "ProductData| name: '" << escapeValue(m_name)
            << "' basePrice: '" << escapeValue(m_basePrice)
            << "' category: '" << escapeValue(m_category) << "'";
        return oss.str();
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("name") != fields.end() &&
            fields.find("basePrice") != fields.end() &&
            fields.find("category") != fields.end())
        {
            m_name = fields["name"];
            m_basePrice = fields["basePrice"];
            m_category = fields["category"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<ProductData>(m_name, m_basePrice, m_category);
    }
};

// 訂單資料
class OrderData : public IDataBase
{
    std::string m_orderDate;
    std::string m_customerPath;
    std::string m_totalAmount;

public:
    OrderData() = default;
    OrderData(const std::string& orderDate, const std::string& customerPath, const std::string& totalAmount)
        : m_orderDate(orderDate), m_customerPath(customerPath), m_totalAmount(totalAmount) {}

    std::string getOrderDate() const { return m_orderDate; }
    std::string getCustomerPath() const { return m_customerPath; }
    std::string getTotalAmount() const { return m_totalAmount; }

    void setOrderDate(const std::string& orderDate) { m_orderDate = orderDate; }
    void setCustomerPath(const std::string& customerPath) { m_customerPath = customerPath; }
    void setTotalAmount(const std::string& totalAmount) { m_totalAmount = totalAmount; }

    std::string getTypeName() const override { return "OrderData"; }

    std::string serialize() const override
    {
        std::ostringstream oss;
        oss << "OrderData| orderDate: '" << escapeValue(m_orderDate)
            << "' customerPath: '" << escapeValue(m_customerPath)
            << "' totalAmount: '" << escapeValue(m_totalAmount) << "'";
        return oss.str();
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("orderDate") != fields.end() &&
            fields.find("customerPath") != fields.end() &&
            fields.find("totalAmount") != fields.end())
        {
            m_orderDate = fields["orderDate"];
            m_customerPath = fields["customerPath"];
            m_totalAmount = fields["totalAmount"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<OrderData>(m_orderDate, m_customerPath, m_totalAmount);
    }
};

// 訂單項目資料
class OrderItemData : public IDataBase
{
    std::string m_productPath;
    std::string m_quantity;
    std::string m_unitPrice;
    std::string m_subtotal;

public:
    OrderItemData() = default;
    OrderItemData(const std::string& productPath, const std::string& quantity,
                  const std::string& unitPrice, const std::string& subtotal)
        : m_productPath(productPath), m_quantity(quantity),
          m_unitPrice(unitPrice), m_subtotal(subtotal) {}

    std::string getProductPath() const { return m_productPath; }
    std::string getQuantity() const { return m_quantity; }
    std::string getUnitPrice() const { return m_unitPrice; }
    std::string getSubtotal() const { return m_subtotal; }

    void setProductPath(const std::string& productPath) { m_productPath = productPath; }
    void setQuantity(const std::string& quantity) { m_quantity = quantity; }
    void setUnitPrice(const std::string& unitPrice) { m_unitPrice = unitPrice; }
    void setSubtotal(const std::string& subtotal) { m_subtotal = subtotal; }

    std::string getTypeName() const override { return "OrderItemData"; }

    std::string serialize() const override
    {
        std::ostringstream oss;
        oss << "OrderItemData| productPath: '" << escapeValue(m_productPath)
            << "' quantity: '" << escapeValue(m_quantity)
            << "' unitPrice: '" << escapeValue(m_unitPrice)
            << "' subtotal: '" << escapeValue(m_subtotal) << "'";
        return oss.str();
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("productPath") != fields.end() &&
            fields.find("quantity") != fields.end() &&
            fields.find("unitPrice") != fields.end() &&
            fields.find("subtotal") != fields.end())
        {
            m_productPath = fields["productPath"];
            m_quantity = fields["quantity"];
            m_unitPrice = fields["unitPrice"];
            m_subtotal = fields["subtotal"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<OrderItemData>(m_productPath, m_quantity, m_unitPrice, m_subtotal);
    }
};

// 日期資料
class DateData : public IDataBase
{
    std::string m_date; // 格式: YYYY-MM-DD

public:
    DateData() = default;
    DateData(const std::string& date) : m_date(date) {}

    std::string getDate() const { return m_date; }
    void setDate(const std::string& date) { m_date = date; }

    // 取得年份
    int getYear() const
    {
        if (m_date.size() >= 4)
            return std::stoi(m_date.substr(0, 4));
        return 0;
    }

    // 取得月份
    int getMonth() const
    {
        if (m_date.size() >= 7)
            return std::stoi(m_date.substr(5, 2));
        return 0;
    }

    // 取得日期
    int getDay() const
    {
        if (m_date.size() >= 10)
            return std::stoi(m_date.substr(8, 2));
        return 0;
    }

    // 設定日期（從年月日）
    void setDate(int year, int month, int day)
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << year << "-"
            << std::setw(2) << month << "-"
            << std::setw(2) << day;
        m_date = oss.str();
    }

    // 驗證日期格式
    bool isValid() const
    {
        if (m_date.size() != 10)
            return false;
        if (m_date[4] != '-' || m_date[7] != '-')
            return false;

        try
        {
            int year = getYear();
            int month = getMonth();
            int day = getDay();

            if (year < 1900 || year > 2100)
                return false;
            if (month < 1 || month > 12)
                return false;
            if (day < 1 || day > 31)
                return false;

            // 簡單的日期驗證
            if (month == 2 && day > 29)
                return false;
            if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
                return false;

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // 比較日期
    int compare(const DateData& other) const
    {
        return m_date.compare(other.m_date);
    }

    bool operator<(const DateData& other) const { return compare(other) < 0; }
    bool operator>(const DateData& other) const { return compare(other) > 0; }
    bool operator==(const DateData& other) const { return compare(other) == 0; }

    std::string getTypeName() const override { return "DateData"; }

    std::string serialize() const override
    {
        return "DateData| date: '" + escapeValue(m_date) + "'";
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("date") != fields.end())
        {
            m_date = fields["date"];
            return true;
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<DateData>(m_date);
    }
};

// 浮點數資料
class FloatData : public IDataBase
{
    double m_value;

public:
    FloatData() : m_value(0.0) {}
    FloatData(double value) : m_value(value) {}

    double getValue() const { return m_value; }
    void setValue(double value) { m_value = value; }

    // 數學運算
    FloatData operator+(const FloatData& other) const
    {
        return FloatData(m_value + other.m_value);
    }

    FloatData operator-(const FloatData& other) const
    {
        return FloatData(m_value - other.m_value);
    }

    FloatData operator*(const FloatData& other) const
    {
        return FloatData(m_value * other.m_value);
    }

    FloatData operator/(const FloatData& other) const
    {
        if (other.m_value != 0.0)
            return FloatData(m_value / other.m_value);
        return FloatData(0.0);
    }

    // 比較運算
    bool operator<(const FloatData& other) const { return m_value < other.m_value; }
    bool operator>(const FloatData& other) const { return m_value > other.m_value; }
    bool operator==(const FloatData& other) const
    {
        return std::abs(m_value - other.m_value) < 1e-9;
    }

    // 格式化輸出
    std::string toString(int precision = 2) const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << m_value;
        return oss.str();
    }

    // 從字串解析
    bool fromString(const std::string& str)
    {
        try
        {
            m_value = std::stod(str);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::string getTypeName() const override { return "FloatData"; }

    std::string serialize() const override
    {
        std::ostringstream oss;
        oss << "FloatData| value: '" << std::setprecision(15) << m_value << "'";
        return oss.str();
    }

    bool deserialize(const std::string& str) override
    {
        auto fields = parseFields(str);
        if (fields.find("value") != fields.end())
        {
            return fromString(fields["value"]);
        }
        return false;
    }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<FloatData>(m_value);
    }
};

// 空資料（用於容器節點）
class EmptyData : public IDataBase
{
public:
    std::string getTypeName() const override { return "EmptyData"; }

    std::string serialize() const override { return ""; }

    bool deserialize(const std::string& str) override { return true; }

    std::shared_ptr<IDataBase> clone() const override
    {
        return std::make_shared<EmptyData>();
    }
};

} // namespace CXXL

#endif // __DATABASE_TYPES_HPP_CxxlMan3
