#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <cassert>

enum EufKind{
    None,
    Atom,
    Function
};

class EufTerm;

class EufSymbol{
    public:
    std::string name;
    int numOp = 0;
    EufKind kind = None;

    static EufSymbol MakeAtom(std::string name);
    static EufSymbol MakeFunction(std::string name, int operand);

    EufTerm Apply2(EufTerm x, EufTerm y);
};

class EufTerm{
    public:
    EufKind kind = None;
    EufSymbol symbol;
    std::vector<EufTerm> args;

    explicit EufTerm(EufSymbol x): 
    kind(EufKind::Atom),
    symbol(x)
    {
        assert(x.kind == EufKind::Atom);
    }

    EufTerm(EufSymbol f, std::vector<EufTerm> args):
    kind(EufKind::Function),
    symbol(f),
    args(args)
    {
        assert(f.kind == EufKind::Function);
    }

    std::string Print();
};

// class EufTermTree{
//     public:

// };

// termからそれに対応するノードを引くにひは？

    // EufTermTree pool;
    // auto f = pool.MakeEufFunc("f", 2);
    // auto a = pool.MakeEufAtom("a");
    // auto b = pool.MakeEufAtom("b");
    // auto term1 = MakeEufEq(f.apply2(a, b), a);
    // // auto term2 = MakeEufNeq(f.apply2(f.apply2(a, b)), b);
    // pool.ApplyClosure({term1});