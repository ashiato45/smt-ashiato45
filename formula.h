#include "minisat/simp/SimpSolver.h"
#include <memory>
#include <vector>
#include <sstream>
#include <map>

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

    void AppendAsString(std::ostringstream& oss);
    bool Eval(std::map<Minisat::Var, bool>& assignment);
};


FormulaPtr ApplyTseitin(FormulaPtr form, Minisat::Solver& solver);