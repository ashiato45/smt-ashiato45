#include "minisat/simp/SimpSolver.h"
#include <memory>
#include <vector>

enum Op{
    Op_Atom,
    Op_And,
    Op_Or,
    Op_Not,
    Op_Count
};

struct Formula;
using FormulaPtr = std::shared_ptr<Formula>;

struct Formula
{
    Op op;
    Minisat::Var atom;
    std::vector<std::shared_ptr<Formula>> terms;

    static FormulaPtr MakeAtom(Minisat::Var atom);
    static FormulaPtr MakeAnd(FormulaPtr f1, FormulaPtr f2);
    static FormulaPtr MakeOr(FormulaPtr f1, FormulaPtr f2);
    static FormulaPtr MakeNot(FormulaPtr f1);

};


FormulaPtr ApplyTseitin(FormulaPtr form, std::shared_ptr<Minisat::Solver> solver);