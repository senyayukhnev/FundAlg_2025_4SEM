//#include <gtest/gtest.h>
//#include "../include/deque.h"
//
//namespace my_container {
//
//    class DequeTest : public ::testing::Test {
//    protected:
//        Deque<int> empty_deque;
//        Deque<int> sample_deque;
//
//        void SetUp() override {
//            sample_deque.push_back(1);
//            sample_deque.push_back(2);
//            sample_deque.push_back(3);
//        }
//    };
//
//// Конструкторы
//    TEST_F(DequeTest, DefaultConstructor) {
//        EXPECT_TRUE(empty_deque.empty());
//        EXPECT_EQ(empty_deque.size(), 0);
//    }
//
//    TEST_F(DequeTest, InitializerListConstructor) {
//        Deque<int> dq{1, 2, 3};
//        EXPECT_EQ(dq.size(), 3);
//        EXPECT_EQ(dq[0], 1);
//        EXPECT_EQ(dq[1], 2);
//        EXPECT_EQ(dq[2], 3);
//    }
//
//    TEST_F(DequeTest, CopyConstructor) {
//        Deque<int> copy(sample_deque);
//        EXPECT_EQ(copy, sample_deque);
//    }
//
//    TEST_F(DequeTest, MoveConstructor) {
//        Deque<int> moved(std::move(sample_deque));
//        EXPECT_EQ(moved.size(), 3);
//        EXPECT_TRUE(sample_deque.empty());
//    }
//
//// Операторы
//    TEST_F(DequeTest, CopyAssignment) {
//        Deque<int> copy;
//        copy = sample_deque;
//        EXPECT_EQ(copy, sample_deque);
//    }
//
//    TEST_F(DequeTest, MoveAssignment) {
//        Deque<int> moved;
//        moved = std::move(sample_deque);
//        EXPECT_EQ(moved.size(), 3);
//        EXPECT_TRUE(sample_deque.empty());
//    }
//
//    TEST_F(DequeTest, SwapWorks) {
//        Deque<int> other{10, 20};
//        sample_deque.swap(other);
//        EXPECT_EQ(sample_deque[0], 10);
//        EXPECT_EQ(other[0], 1);
//    }
//
//// Методы доступа
//    TEST_F(DequeTest, AtThrowsOutOfRange) {
//        EXPECT_THROW(empty_deque.at(0), std::out_of_range);
//    }
//
//    TEST_F(DequeTest, AtReturnsCorrectElement) {
//        EXPECT_EQ(sample_deque.at(0), 1);
//        EXPECT_EQ(sample_deque.at(2), 3);
//    }
//
//    TEST_F(DequeTest, FrontBackAccess) {
//        EXPECT_EQ(sample_deque.front(), 1);
//        EXPECT_EQ(sample_deque.back(), 3);
//    }
//
//// Итераторы
//    TEST_F(DequeTest, ForwardIteration) {
//        std::vector<int> expected{1, 2, 3};
//        size_t i = 0;
//        for (const auto& val : sample_deque) {
//            EXPECT_EQ(val, expected[i++]);
//        }
//    }
//
//    TEST_F(DequeTest, ReverseIteration) {
//        std::vector<int> expected{3, 2, 1};
//        size_t i = 0;
//        for (auto it = sample_deque.rbegin(); it != sample_deque.rend(); ++it) {
//            EXPECT_EQ(*it, expected[i++]);
//        }
//    }
//
//// Размерность
//    TEST_F(DequeTest, SizeAndEmpty) {
//        EXPECT_FALSE(sample_deque.empty());
//        EXPECT_EQ(sample_deque.size(), 3);
//    }
//
//// Модификаторы
//    TEST_F(DequeTest, ClearDeque) {
//        sample_deque.clear();
//        EXPECT_TRUE(sample_deque.empty());
//    }
//
//    TEST_F(DequeTest, PushPopBackFront) {
//        empty_deque.push_back(5);
//        empty_deque.push_front(3);
//        EXPECT_EQ(empty_deque.front(), 3);
//        EXPECT_EQ(empty_deque.back(), 5);
//
//        empty_deque.pop_front();
//        EXPECT_EQ(empty_deque.front(), 5);
//        empty_deque.pop_back();
//        EXPECT_TRUE(empty_deque.empty());
//    }
//
//    TEST_F(DequeTest, ResizeIncreasesAndDecreases) {
//        sample_deque.resize(5, 42);
//        EXPECT_EQ(sample_deque.size(), 5);
//        EXPECT_EQ(sample_deque[4], 42);
//
//        sample_deque.resize(2);
//        EXPECT_EQ(sample_deque.size(), 2);
//    }
//
//// Сравнения
//    TEST_F(DequeTest, ComparisonOperators) {
//        Deque<int> a{1, 2, 3};
//        Deque<int> b{1, 2, 4};
//        EXPECT_EQ(sample_deque, a);
//        EXPECT_NE(sample_deque, b);
//        EXPECT_LT(sample_deque, b);
//        EXPECT_LE(sample_deque, b);
//        EXPECT_GT(b, sample_deque);
//        EXPECT_GE(b, sample_deque);
//    }
//
//}  // namespace my_container
#include <gtest/gtest.h>
#include "deque.h"

using namespace my_container;

class DequeTest : public ::testing::Test {
protected:
    Deque<int> deque;

    void SetUp() override {
        deque.push_back(1);
        deque.push_back(2);
        deque.push_back(3);
    }
};

TEST_F(DequeTest, PushBackPushFrontWorks) {
    deque.push_front(0);
    deque.push_back(4);
    EXPECT_EQ(deque.front(), 0);
    EXPECT_EQ(deque.back(), 4);
    EXPECT_EQ(deque.size(), 5);
}

TEST_F(DequeTest, PopBackPopFrontWorks) {
    deque.pop_back();   // removes 3
    deque.pop_front();  // removes 1
    EXPECT_EQ(deque.front(), 2);
    EXPECT_EQ(deque.back(), 2);
    EXPECT_EQ(deque.size(), 1);
}

TEST_F(DequeTest, ResizeIncreaseWithDefault) {
    deque.resize(5);
    EXPECT_EQ(deque.size(), 5);
}

TEST_F(DequeTest, ResizeIncreaseWithValue) {
    deque.resize(5, 99);  // 👈 добавь этот overload в Deque
    EXPECT_EQ(deque.size(), 5);
    EXPECT_EQ(deque.back(), 99);
}

TEST_F(DequeTest, ResizeDecrease) {
    deque.resize(1);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front(), 1);
}

TEST_F(DequeTest, ClearEmptiesDeque) {
    deque.clear();
    EXPECT_TRUE(deque.empty());
}

TEST_F(DequeTest, EqualityOperator) {
    Deque<int> other;
    other.push_back(1);
    other.push_back(2);
    other.push_back(3);
    EXPECT_TRUE(deque == other);
}

TEST_F(DequeTest, InequalityOperator) {
    Deque<int> other;
    other.push_back(1);
    other.push_back(2);
    EXPECT_TRUE(deque != other);
}

TEST_F(DequeTest, SwapWorks) {
    Deque<int> other;
    other.push_back(10);
    other.push_back(20);

    deque.swap(other);

    EXPECT_EQ(deque.front(), 10);
    EXPECT_EQ(other.front(), 1);
    EXPECT_EQ(other.size(), 3);
}

TEST_F(DequeTest, IteratorsAccessElements) {
    auto it = deque.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);

    auto cit = deque.cbegin();
    EXPECT_EQ(*cit, 1);
}

TEST_F(DequeTest, MaxSizeReturnsSomething) {
    EXPECT_GT(deque.max_size(), 1000);
}

