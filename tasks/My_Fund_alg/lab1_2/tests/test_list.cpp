//
// Created by senya on 08.05.2025.
//
#include <gtest/gtest.h>
#include "../include/list.h"

namespace my_container {
    namespace {

        class ListTest : public ::testing::Test {
        protected:
            List<int> empty_list;
            List<int> sample_list;

            ListTest() : empty_list(), sample_list({1, 2, 3}) {}

            void SetUp() override {}
        };

        TEST_F(ListTest, DefaultConstructor) {
            EXPECT_TRUE(empty_list.empty());
            EXPECT_EQ(empty_list.size(), 0);
        }

        TEST_F(ListTest, InitializerListConstructor) {
            EXPECT_EQ(sample_list.size(), 3);
            EXPECT_EQ(sample_list.front(), 1);
            EXPECT_EQ(sample_list.back(), 3);
        }

        TEST_F(ListTest, CopyConstructor) {
            List<int> copy(sample_list);
            EXPECT_EQ(copy.size(), 3);
            EXPECT_EQ(copy, sample_list);
        }

        TEST_F(ListTest, MoveConstructor) {
            List<int> moved(std::move(sample_list));
            EXPECT_EQ(moved.size(), 3);
            EXPECT_TRUE(sample_list.empty());
        }

        TEST_F(ListTest, CopyAssignment) {
            List<int> copy;
            copy = sample_list;
            EXPECT_EQ(copy.size(), 3);
            EXPECT_EQ(copy, sample_list);
        }

        TEST_F(ListTest, MoveAssignment) {
            List<int> moved;
            moved = std::move(sample_list);
            EXPECT_EQ(moved.size(), 3);
            EXPECT_TRUE(sample_list.empty());
        }

        TEST_F(ListTest, FrontBack) {
            EXPECT_EQ(sample_list.front(), 1);
            EXPECT_EQ(sample_list.back(), 3);
        }

        TEST_F(ListTest, FrontBackThrowWhenEmpty) {
            EXPECT_THROW(empty_list.front(), std::out_of_range);
            EXPECT_THROW(empty_list.back(), std::out_of_range);
        }

        TEST_F(ListTest, Iterators) {
            auto it = sample_list.begin();
            EXPECT_EQ(*it, 1);
            ++it;
            EXPECT_EQ(*it, 2);
            ++it;
            EXPECT_EQ(*it, 3);
            ++it;
            EXPECT_EQ(it, sample_list.end());
        }

        TEST_F(ListTest, ConstIterators) {
            const List<int> constList = sample_list;
            int sum = 0;
            for (auto it = constList.begin(); it != constList.end(); ++it) {
                sum += *it;
            }
            EXPECT_EQ(sum, 6);
        }

        TEST(ListFrontBackTest, SingleElement) {
            my_container::List<std::string> list;
            list.push_back("hello");
            const auto& const_list = list;

            EXPECT_EQ(const_list.front(), "hello");
            EXPECT_EQ(const_list.back(), "hello");
            EXPECT_EQ(&const_list.front(), &const_list.back());
        }

        TEST(ListFrontBackTest, MultipleElements) {
            const my_container::List<int> list = {10, 20, 30, 40};

            EXPECT_EQ(list.front(), 10);
            EXPECT_EQ(list.back(), 40);
            EXPECT_NE(&list.front(), &list.back());
        }

        TEST(ListFrontBackTest, AfterModifications) {
            my_container::List<double> list;
            list.push_back(3.14);

            EXPECT_EQ(list.front(), 3.14);
            EXPECT_EQ(list.back(), 3.14);

            list.push_front(2.71);
            const auto& const_list = list;
            EXPECT_EQ(const_list.front(), 2.71);
            EXPECT_EQ(const_list.back(), 3.14);

            list.pop_back();
            EXPECT_EQ(const_list.front(), 2.71);
            EXPECT_EQ(const_list.back(), 2.71);
        }

        TEST(ListFrontBackTest, StringType) {
            my_container::List<std::string> list;
            list.push_front("world");
            list.push_front("hello");

            const auto& const_list = list;
            EXPECT_EQ(const_list.front(), "hello");
            EXPECT_EQ(const_list.back(), "world");

            list.front() = "test";
            EXPECT_EQ(const_list.front(), "test");
        }

        TEST_F(ListTest, InsertErase) {
            auto it = sample_list.begin();
            ++it;
            sample_list.insert(it, 99);
            List<int> expected{1, 99, 2, 3};
            EXPECT_EQ(sample_list, expected);

            it = sample_list.begin();
            ++it;
            sample_list.erase(it);
            expected = {1, 2, 3};
            EXPECT_EQ(sample_list, expected);
        }

        TEST_F(ListTest, Insert) {
            auto it = sample_list.begin();
            ++it;
            sample_list.insert(it, 99);

            my_container::List<int> expected{1, 99, 2, 3};
            EXPECT_EQ(sample_list, expected);
        }

        TEST_F(ListTest, Resize) {
            sample_list.resize(5);
            EXPECT_EQ(sample_list.size(), 5);
            EXPECT_EQ(sample_list.back(), 0);

            sample_list.resize(2);
            EXPECT_EQ(sample_list.size(), 2);
            EXPECT_EQ(sample_list.back(), 2);
        }

        TEST_F(ListTest, Swap) {
            List<int> other{4, 5, 6};
            sample_list.swap(other);
            EXPECT_EQ(sample_list.size(), 3);
            EXPECT_EQ(sample_list.back(), 6);
            EXPECT_EQ(other.size(), 3);
            EXPECT_EQ(other.back(), 3);
        }

        TEST_F(ListTest, Comparisons) {
            List<int> list_eq{1, 2, 3};
            List<int> list_less{1, 2};
            List<int> list_greater{1, 2, 4};

            EXPECT_EQ(sample_list, list_eq);
            EXPECT_NE(sample_list, list_less);
            EXPECT_LT(list_less, sample_list);
        }

        TEST_F(ListTest, MaxSize) {
            EXPECT_EQ(empty_list.max_size(), std::numeric_limits<size_t>::max());
            EXPECT_GT(empty_list.max_size(), 0);

            EXPECT_EQ(sample_list.max_size(), std::numeric_limits<size_t>::max());

            my_container::List<int> another_list{4, 5, 6};
            EXPECT_EQ(empty_list.max_size(), another_list.max_size());


        }

    }
}


template<typename T>
class DummyContainer : public Container<T> {
public:
    DummyContainer() = default;

    Container<T> &operator=(const Container<T> &) override { return *this; }

    bool operator==(const Container<T> &) const override { return false; }

    bool operator!=(const Container<T> &) const override { return true; }

    size_t size() const override { return 0; }

    size_t max_size() const override { return 0; }

    bool empty() const override { return true; }
};
TEST(ListAssignmentTest, AssignBetweenLists) {
    my_container::List<int> list1 = {1, 2, 3};
    my_container::List<int> list2;

    list2 = static_cast<const Container<int>&>(list1);

    EXPECT_EQ(list1.size(), list2.size());
    EXPECT_TRUE(std::equal(list1.begin(), list1.end(), list2.begin()));
    EXPECT_NE(&list1, &list2);
}

TEST(ListAssignmentTest, SelfAssignment) {
    my_container::List<int> list = {5, 10, 15};
    const auto* original_data_ptr = &list.front();

    list = static_cast<const Container<int>&>(list);

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front(), 5);
    EXPECT_EQ(&list.front(), original_data_ptr);

}

TEST(ListAssignmentTest, AssignFromIncompatibleContainer) {
    my_container::List<int> list;
    DummyContainer<int> dummy;

    EXPECT_THROW(
            list = static_cast<const Container<int>&>(dummy),
            std::invalid_argument
    );
}

TEST(ListAssignmentTest, DeepCopyVerification) {
    my_container::List<int> list1 = {100, 200};
    my_container::List<int> list2;

    list2 = list1;
    list1.push_back(300);

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list2.back(), 200);
}

TEST(ListAssignmentTest, EmptyListAssignment) {
    my_container::List<std::string> list1;
    my_container::List<std::string> list2 = {"test"};

    list2 = list1;

    EXPECT_TRUE(list2.empty());
    EXPECT_EQ(list2.size(), 0);
}

TEST(ListAssignmentTest, AssignViaContainerPointer) {
    my_container::List<int> list1 = {10, 20, 30};
    my_container::List<int> list2;
    const Container<int>* containerPtr = &list1;

    list2 = *containerPtr;

    EXPECT_EQ(list1.size(), list2.size());
    EXPECT_TRUE(std::equal(list1.begin(), list1.end(), list2.begin()));
}


TEST(ListReverseIteratorsTest, EmptyList) {
    const my_container::List<int> list;
    EXPECT_EQ(list.rbegin(), list.rend());
    EXPECT_EQ(list.crbegin(), list.crend());
}

TEST(ListReverseIteratorsTest, SingleElement) {
    my_container::List<std::string> list;
    list.push_back("hello");

    auto rit = list.rbegin();
    ASSERT_NE(rit, list.rend());
    EXPECT_EQ(*rit, "hello");
    EXPECT_EQ(++rit, list.rend());
}

TEST(ListReverseIteratorsTest, ReverseTraversal) {
    my_container::List<int> list = {1, 2, 3, 4, 5};
    const std::vector<int> expected = {5, 4, 3, 2, 1};

    std::vector<int> result;
    for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
        result.push_back(*rit);
    }
    EXPECT_EQ(result, expected);

    const auto& clist = list;
    std::vector<int> cresult;
    for (auto rit = clist.crbegin(); rit != clist.crend(); ++rit) {
        cresult.push_back(*rit);
    }
    EXPECT_EQ(cresult, expected);
}

TEST(ListReverseIteratorsTest, MixedOperations) {
    my_container::List<int> list = {100, 200, 300};

    auto rit1 = list.rbegin();
    auto rit2 = rit1++;

    EXPECT_EQ(*rit1, 200);
    EXPECT_EQ(*rit2, 300);
    EXPECT_EQ(rit2.base(), list.end());
}