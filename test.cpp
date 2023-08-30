#include <bitset>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

#include "formula.h"
#include "gtest/gtest.h"
#include "minisat/simp/SimpSolver.h"

#include "euf.h"

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
std::shared_ptr<FormulaPred> MakeRandomFormula(std::mt19937& engine, int varNum, int maxDepth,
                             int depth = 0) {
    auto r = std::uniform_int_distribution<>(0, 3)(engine);
    if (depth >= maxDepth) {
        r = 10;
    }
    switch (r) {
        case 0: {
            return FormulaPred::MakeNot(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 1: {
            return FormulaPred::MakeAnd(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 2: {
            return FormulaPred::MakeOr(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        default: {
            auto i = std::uniform_int_distribution<>(0, varNum - 1)(engine);
            return FormulaPred::MakeAtom(i);
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

TEST(EufTest, TermConstTest){
    auto a = EufSymbol::MakeAtom("a");
    auto b = EufSymbol::MakeAtom("b");
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(EufTerm(a), EufTerm(b));
    // std::cout << fab.Print() << std::endl;
    ASSERT_EQ(fab.Print(), "f(a, b)");
    auto ffabb = f.Apply2(fab, EufTerm(b));
    ASSERT_EQ(ffabb.Print(), "f(f(a, b), b)");
    // std::cout << ffabb.Print() << std::endl;
}

TEST(EufTest, TermHashTest){
    auto a = EufSymbol::MakeAtom("a");
    auto b = EufSymbol::MakeAtom("b");
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(EufTerm(a), EufTerm(b));
    ASSERT_EQ(std::hash<EufTerm>{}(fab), 11670068950902842810ULL);
}

// TEST(FooTest, ClosureTest) {
//     // term集合をあらわすツリーをつくる
//     // formulaをつくる

//     EufTermTree pool;
//     auto f = pool.MakeEufFunc("f", 2);
//     auto a = pool.MakeEufAtom("a");
//     auto b = pool.MakeEufAtom("b");
//     auto term1 = MakeEufEq(f.apply2(a, b), a);
//     // auto term2 = MakeEufNeq(f.apply2(f.apply2(a, b)), b);
//     pool.ApplyClosure({term1});
// }

TEST(EufTest, PoolTest){
    auto a = EufSymbol::MakeAtom("a");
    auto b = EufSymbol::MakeAtom("b");
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(EufTerm(a), EufTerm(b));
    
    EufPool pool;
    pool.Add(fab);

    std::cout << pool;
}

TEST(EufTest, PoolTest2){
    EufPool pool;

    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto aNode = pool.Add(a);
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto bNode = pool.Add(b);
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto fabNode = pool.Add(fab);
    auto ffabb = f.Apply2(fab, b);
    auto ffabbNode = pool.Add(ffabb);

    std::cout << pool;
}

TEST(EufTest, UnionFind){
    EufPool pool;

    auto a = EufSymbol::MakeAtom("a");
    auto aNode = pool.Add(EufTerm(a));
    auto b = EufSymbol::MakeAtom("b");
    auto bNode = pool.Add(EufTerm(b));
    auto c = EufSymbol::MakeAtom("c");
    auto cNode = pool.Add(EufTerm(c));
    auto d = EufSymbol::MakeAtom("d");
    auto dNode = pool.Add(EufTerm(d));

    pool.Union(aNode, bNode);
    ASSERT_TRUE(pool.IsSame(aNode, bNode));
    ASSERT_FALSE(pool.IsSame(aNode, cNode));
    ASSERT_FALSE(pool.IsSame(aNode, dNode));
    pool.Union(aNode, cNode);
    ASSERT_TRUE(pool.IsSame(aNode, bNode));
    ASSERT_TRUE(pool.IsSame(aNode, cNode));
    ASSERT_TRUE(pool.IsSame(bNode, cNode));
    ASSERT_FALSE(pool.IsSame(aNode, dNode));
}

TEST(EufTest, CalcPredecessor){
    EufPool pool;

    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto aNode = pool.Add(a);
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto bNode = pool.Add(b);
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto fabNode = pool.Add(fab);
    auto ffabb = f.Apply2(fab, b);
    auto ffabbNode = pool.Add(ffabb);

    {
        auto pSet = pool.CalcPredecessor(aNode);
        decltype(pSet) pSetExpected;
        pSetExpected.insert(fabNode);
        ASSERT_EQ(pSet, pSetExpected);
    }

    {
        auto pSet = pool.CalcPredecessor(bNode);
        decltype(pSet) pSetExpected;
        pSetExpected.insert(fabNode);
        pSetExpected.insert(ffabbNode);
        ASSERT_EQ(pSet, pSetExpected);
    }

}

TEST(EufTest, Merge){
    EufPool pool;

    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto aNode = pool.Add(a);
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto bNode = pool.Add(b);
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto fabNode = pool.Add(fab);
    auto ffabb = f.Apply2(fab, b);
    auto ffabbNode = pool.Add(ffabb);

    pool.Merge(fabNode, aNode);


    std::cout << pool;
}

TEST(EufTest, IsSatisfiable){
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto ffabb = f.Apply2(fab, b);

    atoms.push_back({true, fab, a});
    atoms.push_back({false, ffabb, a});

    auto res = IsSatisfiable(atoms);
    std::cout << res.first << std::endl;
    std::cout << res.second << std::endl;

    ASSERT_FALSE(res.first);
}

TEST(EufTest, Print){
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto ffabb = f.Apply2(fab, b);

    auto form1 = EufFormula::MakeAtom({true, fab, a});
    auto form2 = EufFormula::MakeAtom({false, ffabb, a});
    
    std::cout << form1->ToString() << std::endl;
    std::cout << form2->ToString() << std::endl;

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}