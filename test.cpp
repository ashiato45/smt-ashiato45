#include <iostream>
#include <random>

#include "gtest/gtest.h"
#include "minisat/simp/SimpSolver.h"

namespace {

// テスト対象となるクラス Foo のためのフィクスチャ
class FooTest : public ::testing::Test {
   protected:
    // 以降の関数で中身のないものは自由に削除できます．
    //

    FooTest() {
        // テスト毎に実行される set-up をここに書きます．
    }

    virtual ~FooTest() {
        // テスト毎に実行される，例外を投げない clean-up をここに書きます．
    }

    // コンストラクタとデストラクタでは不十分な場合．
    // 以下のメソッドを定義することができます：

    virtual void SetUp() {
        // このコードは，コンストラクタの直後（各テストの直前）
        // に呼び出されます．
    }

    virtual void TearDown() {
        // このコードは，各テストの直後（デストラクタの直前）
        // に呼び出されます．
    }

    // ここで宣言されるオブジェクトは，テストケース内の全てのテストで利用できます．
};

}  // namespace

TEST(FooTest, MinisatTest) {
    Minisat::SimpSolver solver;
    solver.newVar();
    solver.newVar();
    solver.addClause(Minisat::mkLit(0), Minisat::mkLit(1));
    solver.addClause(~Minisat::mkLit(0), ~Minisat::mkLit(1));
    solver.addClause(~Minisat::mkLit(0));

    auto ret = solver.solve();
    ASSERT_TRUE(ret);
    ASSERT_EQ(solver.model[0], Minisat::l_False);
    ASSERT_EQ(solver.model[1], Minisat::l_True);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}