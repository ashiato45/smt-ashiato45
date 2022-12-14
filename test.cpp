#include <bitset>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

#include "formula.h"
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

// solverの側ではvarNum分の変数が作られていることを仮定する。
FormulaPtr MakeRandomFormula(std::mt19937& engine, int varNum, int maxDepth,
                             int depth = 0) {
    auto r = std::uniform_int_distribution<>(0, 3)(engine);
    if (depth >= maxDepth) {
        r = 10;
    }
    switch (r) {
        case 0: {
            return Formula::MakeNot(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 1: {
            return Formula::MakeAnd(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 2: {
            return Formula::MakeOr(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        default: {
            auto i = std::uniform_int_distribution<>(0, varNum - 1)(engine);
            return Formula::MakeAtom(i);
        } break;
    }
}

// TEST(FooTest, FormulaTseitinTest) {
//     std::mt19937 engine(42);
//     int varNum = 3;
//     for (int i = 0; i < 10; i++) {  // 10はテストケースの数
//         std::cout << "* Case " << i << std::endl;
//         auto f = MakeRandomFormula(engine, varNum, 5);  // 最大深さ5
//         std::ostringstream oss;
//         f->AppendAsString(oss);
//         oss << " === ";
//         // とりあえず総当たりでSATを判定する。
//         bool found = false;
//         for (int i = 0; i < 1 << varNum; i++) {
//             std::map<Minisat::Var, bool> assignment;
//             for (int j = 0; j < varNum; j++) {
//                 assignment[j] = (((i >> j) & 1) != 0);
//             }
//             if (f->Eval(assignment)) {
//                 found = true;
//                 oss << "SAT" << std::bitset<8>(i);
//                 break;
//             }
//         }
//         if (!found) {
//             oss << "UNSAT";
//         }
//         std::cout << oss.str() << std::endl;

//         // SATソルバで確認
//         Minisat::Solver solver;
//         std::map<Minisat::Var, FormulaPtr> subs;
//         for (int i = 0; i < varNum; i++) {
//             solver.newVar();
//         }
//         f = ApplyTseitin(f, solver, subs);
//         oss.str("");
//         oss.clear();
//         f->AppendAsString(oss);
//         std::cout << "Tseitin: " << oss.str() << std::endl;
//         for (auto i : subs) {
//             std::ostringstream temp;
//             i.second->AppendAsString(temp);
//             std::cout << static_cast<char>('a' + i.first) << ": " << temp.str()
//                       << std::endl;
//         }

//         // if(transformed->)
//     }
// }

TEST(FooTest, FormulaSatTest) {
    std::mt19937 engine(42);
    int varNum = 3;
    for (int i = 0; i < 100; i++) {  // 10はテストケースの数
        std::cout << "* Case " << i << std::endl;
        auto f = MakeRandomFormula(engine, varNum, 5);  // 最大深さ5
        std::ostringstream oss;
        f->AppendAsString(oss);
        oss << " === ";
        // とりあえず総当たりでSATを判定する。
        bool found = false;
        for (int i = 0; i < 1 << varNum; i++) {
            std::map<Minisat::Var, bool> assignment;
            for (int j = 0; j < varNum; j++) {
                assignment[j] = (((i >> j) & 1) != 0);
            }
            if (f->Eval(assignment)) {
                found = true;
                oss << "SAT" << std::bitset<8>(i);
                break;
            }
        }
        if (!found) {
            oss << "UNSAT";
        }
        std::cout << oss.str() << std::endl;

        // SATソルバで確認
        Minisat::Solver solver;
        for (int i = 0; i < varNum; i++) {
            solver.newVar();
        }

        PutIntoSolver(f, solver);
        for(auto i=solver.clausesBegin(); i != solver.clausesEnd(); ++i){
          
        } 
        auto ret = solver.solve();
        std::cout << "SATRES: " << ret << std::endl;
        // if(ret){
        //   for(int i=0; i < solver.nVars(); i++){
        //     std::cout << static_cast<char>('a' + i) << (solver.model[i] == Minisat::l_True) << std::endl;
        //   }
        // }

        ASSERT_EQ(ret, found);
        // ASSERT_TRUE(ret);
        // if(transformed->)
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}