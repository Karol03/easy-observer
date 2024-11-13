#include "catch2/catch_amalgamated.hpp"
#include "easy/doubleendedlinkedlist.hpp"


TEST_CASE("List is empty on initialization", "[list]")
{
    auto list = easy::DoubleEndedLinkedList<int>{};
    REQUIRE(list.empty());
    for (auto value : list)
    {
        REQUIRE_FALSE(&value);
    }
};

TEST_CASE("List appends elements in correct order", "[list]")
{
    struct Parameters
    {
        int append;
        std::vector<int> expected;
    };

    auto params = std::vector{
        Parameters{5, {5}},
        Parameters{11, {5, 11}},
        Parameters{14, {5, 11, 14}},
        Parameters{14, {5, 11, 14, 14}},
        Parameters{5, {5, 11, 14, 14, 5}},
        Parameters{1, {5, 11, 14, 14, 5, 1}},
        Parameters{1, {5, 11, 14, 14, 5, 1, 1}},
        Parameters{INT_MAX, {5, 11, 14, 14, 5, 1, 1, INT_MAX}},
        Parameters{INT_MIN, {5, 11, 14, 14, 5, 1, 1, INT_MAX, INT_MIN}}
    };

    auto list = easy::DoubleEndedLinkedList<int>{};

    for (const auto& param : params)
    {
        list.append(param.append);

        REQUIRE(param.expected.empty() == list.empty());
        auto i = 0u;
        for (const auto& element : list)
        {
            REQUIRE(element == param.expected[i++]);
        }
        REQUIRE(i == param.expected.size());
    }
};

TEST_CASE("List removes elements correctly", "[list]")
{
    struct Parameters
    {
        std::vector<int> append;
        std::vector<int> remove;
        std::vector<int> expected;
    };

    auto params = std::vector{
        Parameters{{5}, {}, {5}},
        Parameters{{2, 3}, {}, {5, 2, 3}},
        Parameters{{1, 7, 4}, {}, {5, 2, 3, 1, 7, 4}},
        Parameters{{}, {2}, {5, 3, 1, 7, 4}},
        Parameters{{}, {5, 4}, {3, 1, 7}},
        Parameters{{}, {3, 1}, {7}},
        Parameters{{}, {7}, {}},
        Parameters{{1, 2, 7}, {}, {1, 2, 7}},
        Parameters{{}, {7}, {1, 2}},
        Parameters{{}, {1, 2}, {}}
    };

    auto list = easy::DoubleEndedLinkedList<int>{};
    auto itemPtrs = std::unordered_map<int, easy::DoubleEndedLinkedList<int>::ItemPtr>{};

    auto paramNumber = 0;
    for (const auto& param : params)
    {
        for (const auto& app : param.append)
        {
            itemPtrs[app] = list.append(app);
        }

        for (const auto& rem : param.remove)
        {
            if (itemPtrs.contains(rem))
            {
                auto item = itemPtrs[rem];
                list.remove(item);
            }
        }

        REQUIRE(param.expected.empty() == list.empty());
        auto i = 0u;
        for (const auto& element : list)
        {
            INFO("for paramNumber[" << paramNumber << "], i == " << i << ", " << element << " == " << param.expected[i]);
            REQUIRE(element == param.expected[i++]);
        }
        REQUIRE(i == param.expected.size());
        ++paramNumber;
    }
};
