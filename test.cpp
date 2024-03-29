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
std::shared_ptr<PropFormula> MakeRandomFormula(std::mt19937& engine, int varNum, int maxDepth,
                             int depth = 0) {
    auto r = std::uniform_int_distribution<>(0, 3)(engine);
    if (depth >= maxDepth) {
        r = 10;
    }
    switch (r) {
        case 0: {
            return PropFormula::MakeNot(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 1: {
            return PropFormula::MakeAnd(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        case 2: {
            return PropFormula::MakeOr(
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1),
                MakeRandomFormula(engine, varNum, maxDepth, depth + 1));
        } break;
        default: {
            auto i = std::uniform_int_distribution<>(0, varNum - 1)(engine);
            return PropFormula::MakeAtom(i);
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

TEST(FooTest, Walk) {
    auto a = PropFormula::MakeAtom(1);
    auto b = PropFormula::MakeAtom(2);
    auto ab = PropFormula::MakeAnd(a, b);
    auto c = PropFormula::MakeAtom(3);
    auto d = PropFormula::MakeAtom(4);
    auto d1 = PropFormula::MakeNot(d);
    auto cd1 = PropFormula::MakeAnd(c, d1);
    auto abcd1 = PropFormula::MakeOr(ab, cd1);

    abcd1->Walk([](PropFormula& form){
        std::cout << "hoge: " << form.ToString() << std::endl;
    });
}

TEST(EufTest, MakeDict){
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto ffabb = f.Apply2(fab, b);

    auto form1 = EufFormula::MakeAtom({true, fab, a});
    auto form2 = EufFormula::MakeAtom({false, ffabb, a});

    std::vector<EufAtom> formulae;
    formulae.push_back(form1->atom);
    formulae.push_back(form2->atom);

    Minisat::SimpSolver solver;
    auto [euf2prop, prop2euf] = MakeEufAtomDictionary<decltype(formulae)>(formulae, solver);

    ASSERT_EQ(euf2prop.size(), 2);
    ASSERT_EQ(prop2euf.size(), 2);

    ASSERT_EQ(prop2euf[euf2prop[form1->atom]], form1->atom);
    ASSERT_EQ(prop2euf[euf2prop[form2->atom]], form2->atom);
    ASSERT_EQ(euf2prop[prop2euf[0]], 0);
    ASSERT_EQ(euf2prop[prop2euf[1]], 1);


}

TEST(EufTest, EufSolveNaiveHelp){
    Minisat::Solver solver;
    // auto a = PropFormula::MakeAtom(1);
    // auto b = PropFormula::MakeAtom(2);
    // auto ab = PropFormula::MakeAnd(a, b);
    solver.newVar();
    solver.newVar();

    auto a = PropFormula::MakeAtom(0);
    auto b = PropFormula::MakeAtom(1);
    auto ab = PropFormula::MakeAnd(a, b);
            for(auto i=solver.clausesBegin(); i != solver.clausesEnd(); ++i){
          
        } 
    PutIntoSolver(ab, solver);
    auto ret = solver.solve();

}

TEST(EufTest, EufSolveNaive){
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto ffabb = f.Apply2(fab, b);

    auto form1 = EufFormula::MakeAtom({true, fab, a});
    auto form2 = EufFormula::MakeAtom({false, ffabb, a});

    auto form = EufFormula::MakeAnd(form1, form2);
    
    auto model = EufSolveNaive(*form.get());

    for(auto [k, v]: model.assignment){
        std::cout << k.left.Print() << (k.equality ? "==" : "!=") << k.right.Print() << "!!" << v << std::endl;
    }
    // EufTermとしての中身は無視しているので、両方trueにしておけば充足できる。
    ASSERT_TRUE(model.satisfiable);
    

}


TEST(EufTest, EufSolve){
    // https://www21.in.tum.de/teaching/sar/SS20/6.pdf
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto f = EufSymbol::MakeFunction("f", 2);
    auto fab = f.Apply2(a, b);
    auto ffabb = f.Apply2(fab, b);

    auto form1 = EufFormula::MakeAtom({true, fab, a});
    auto form2 = EufFormula::MakeAtom({false, ffabb, a});

    auto form = EufFormula::MakeAnd(form1, form2);
    
    auto model = EufSolve(*form.get());

    for(auto [k, v]: model.assignment){
        std::cout << k.left.Print() << (k.equality ? "==" : "!=") << k.right.Print() << "!!" << v << std::endl;
    }
    ASSERT_FALSE(model.satisfiable);
    

}

TEST(EufTest, EufSolve2){
    // https://www21.in.tum.de/teaching/sar/SS20/6.pdf
    std::vector<EufAtom> atoms;
    auto a = EufTerm(EufSymbol::MakeAtom("a"));
    auto b = EufTerm(EufSymbol::MakeAtom("b"));
    auto c = EufTerm(EufSymbol::MakeAtom("c"));
    auto f = EufSymbol::MakeFunction("f", 1);
    auto g = EufSymbol::MakeFunction("g", 2);
    auto fa = f.Apply1(a);
    auto gfab = g.Apply2(fa, b);
    auto fc = f.Apply1(c);
    auto gfca = g.Apply2(fc, a);

    auto form1 = EufFormula::MakeAtom({true, a, b});
    auto form2 = EufFormula::MakeAtom({true, b, c});
    auto form3 = EufFormula::MakeAtom({true, gfab, gfca});
    auto form4 = EufFormula::MakeAtom({false, fa, b});

    auto form = EufFormula::MakeAnd(form1, form2);
    form = EufFormula::MakeAnd(form, form3);
    form = EufFormula::MakeAnd(form, form4);
    
    auto model = EufSolve(*form.get());

    for(auto [k, v]: model.assignment){
        std::cout << k.left.Print() << (k.equality ? "==" : "!=") << k.right.Print() << "!!" << v << std::endl;
    }
    ASSERT_TRUE(model.satisfiable);
    

}

TEST(EufTest, EufSolve3){
    // https://www.csa.iisc.ac.in/~deepakd/logic-2021/EUF.pdf
    std::vector<EufAtom> atoms;
    auto z = EufTerm(EufSymbol::MakeAtom("z"));
    auto x1 = EufTerm(EufSymbol::MakeAtom("x1"));
    auto y1 = EufTerm(EufSymbol::MakeAtom("y1"));
    auto x2 = EufTerm(EufSymbol::MakeAtom("x2"));
    auto y2 = EufTerm(EufSymbol::MakeAtom("y2"));
    auto u1 = EufTerm(EufSymbol::MakeAtom("u1"));
    auto u2 = EufTerm(EufSymbol::MakeAtom("u2"));

    auto add = EufSymbol::MakeFunction("add", 2);
    auto mul = EufSymbol::MakeFunction("mul", 2);

    auto x1y1 = add.Apply2(x1, y1);
    auto x2y2 = add.Apply2(x2, y2);
    auto u1u2 = mul.Apply2(u1, u2);
    auto x1y1x2y2 = mul.Apply2(x1y1, x2y2);

    auto s1 = EufFormula::MakeAtom({true, z, x1y1x2y2});
    auto t1 = EufFormula::MakeAtom({true, u1, x1y1});
    auto t2 = EufFormula::MakeAtom({true, u2, x2y2});
    auto t3 = EufFormula::MakeAtom({true, z, u1u2});

    auto left = EufFormula::MakeAnd(t1, EufFormula::MakeAnd(t2, t3));
    auto right = s1;
    auto checking = EufFormula::MakeOr(EufFormula::MakeNot(left), right);

    auto negChecking = EufFormula::MakeNot(checking);
    
    auto model = EufSolve(*negChecking.get());

    for(auto [k, v]: model.assignment){
        std::cout << k.left.Print() << (k.equality ? "==" : "!=") << k.right.Print() << "!!" << v << std::endl;
    }
    ASSERT_FALSE(model.satisfiable);
    

}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}