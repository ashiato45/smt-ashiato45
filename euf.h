#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <cassert>
#include <functional>
#include <memory>

class EufTerm;

class EufSymbol{
    public:
    std::string name;
    int numOp = 0;  // 0は定数

    static EufSymbol MakeAtom(std::string name);
    static EufSymbol MakeFunction(std::string name, int operand);

    EufTerm Apply2(EufTerm x, EufTerm y);
};

class EufTerm{
    public:
    EufSymbol symbol;
    std::vector<EufTerm> args;

    explicit EufTerm(EufSymbol x): 
    symbol(x)
    {
        assert(x.numOp == 0);
    }

    EufTerm(EufSymbol f, std::vector<EufTerm> args):
    symbol(f),
    args(args)
    {
    }

    std::string Print() const;
};

template<> struct std::hash<EufTerm>{
    std::size_t operator()(EufTerm const& s) const noexcept
    {
        return std::hash<std::string>{}(s.Print());
    }
};

struct EufPoolNode{
    std::vector<std::shared_ptr<EufPoolNode>> children;
    EufTerm term;
    std::shared_ptr<EufPoolNode> unionArrow;
};

class EufPool{};

// class EufTermTree{
//     public:

// };

// termからそれに対応するノードを引くにひは？
// 文字列のhashをつかおう。違う引数の同じシンボルが2度はでないようにする。

    // EufTermTree pool;
    // auto f = pool.MakeEufFunc("f", 2);
    // auto a = pool.MakeEufAtom("a");
    // auto b = pool.MakeEufAtom("b");
    // auto term1 = MakeEufEq(f.apply2(a, b), a);
    // // auto term2 = MakeEufNeq(f.apply2(f.apply2(a, b)), b);
    // pool.ApplyClosure({term1});