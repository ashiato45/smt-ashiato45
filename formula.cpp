#include "formula.h"

#include <algorithm>
#include <cassert>
#include <numeric>

FormulaPtr Formula::MakeAtom(Minisat::Var atom) {
    return FormulaPtr{new Formula{Op::Op_Atom, atom, {}}};
}
FormulaPtr Formula::MakeAnd(FormulaPtr f1, FormulaPtr f2) {
    return FormulaPtr{new Formula{Op::Op_And, {}, {f1, f2}}};
}
FormulaPtr Formula::MakeOr(FormulaPtr f1, FormulaPtr f2) {
    return FormulaPtr{new Formula{Op::Op_Or, {}, {f1, f2}}};
}
FormulaPtr Formula::MakeNot(FormulaPtr f1) {
    return FormulaPtr{new Formula{Op::Op_Not, {}, {f1}}};
}

void Formula::AppendAsString(std::ostringstream& oss) {
    switch (op) {
        case Op::Op_And: {
            oss << "And[";
            for (int i = 0; i < terms.size(); i++) {
                terms[i]->AppendAsString(oss);
                if (i != terms.size() - 1) {
                    oss << ", ";
                }
            }
            oss << "]";
        } break;
        case Op::Op_Atom: {
            assert(terms.size() == 0);
            oss << static_cast<char>('a' + atom);
        } break;
        case Op::Op_Not: {
            assert(terms.size() == 1);
            oss << "Not[";
            terms[0]->AppendAsString(oss);
            oss << "]";
        } break;
        case Op::Op_Or: {
            oss << "Or[";
            for (int i = 0; i < terms.size(); i++) {
                terms[i]->AppendAsString(oss);
                if (i != terms.size() - 1) {
                    oss << ", ";
                }
            }
            oss << "]";
        } break;
        default:
            assert(0);
    }
}

bool Formula::Eval(std::map<Minisat::Var, bool>& assignment) {
    switch (op) {
        case Op::Op_And: {
            return std::accumulate(terms.begin(), terms.end(), true,
                                   [&assignment](bool l, FormulaPtr r) {
                                       return l && r->Eval(assignment);
                                   });
        } break;
        case Op::Op_Atom: {
            assert(terms.size() == 0);
            return assignment[atom];
        } break;
        case Op::Op_Not: {
            assert(terms.size() == 1);
            return !terms[0]->Eval(assignment);
        } break;
        case Op::Op_Or: {
            return std::accumulate(terms.begin(), terms.end(), false,
                                   [&assignment](bool l, FormulaPtr r) {
                                       return l || r->Eval(assignment);
                                   });
        } break;
        default:
            assert(0);
    }
}

// namespace {
// struct HelpTseitinData {
//     FormulaPtr freshPos;
//     FormulaPtr freshNeg;
//     std::vector<FormulaPtr> insidePos;
//     std::vector<FormulaPtr> insideNeg;
// };

// HelpTseitinData HelpTseitin(FormulaPtr f, Minisat::Solver& solver) {
//     assert(f->op == Op::Op_And || f->op == Op::Op_Or);
//     auto freshPos = Formula::MakeAtom(solver.newVar());
//     auto freshNeg = Formula::MakeNot(freshPos);
//     std::vector<FormulaPtr> insidePos(f->terms.size());
//     std::vector<FormulaPtr> insideNeg(f->terms.size());
//     std::transform(f->terms.begin(), f->terms.end(), insidePos.begin(),
//                    [&solver](FormulaPtr i) { return ApplyTseitin(i, solver); });
//     std::transform(f->terms.begin(), f->terms.end(), insideNeg.begin(),
//                    [&solver](FormulaPtr i) {
//                        return ApplyTseitin(Formula::MakeNot(i), solver);
//                    });

//     return HelpTseitinData{freshPos, freshNeg, insidePos, insideNeg};
// }
// }  // namespace

Minisat::Var ApplyTseitinHelp(FormulaPtr formula, Minisat::Solver& solver,
                          std::map<Minisat::Var, FormulaPtr>& subs) {
    switch (formula->op) {
        case Op::Op_Atom: {
            return -1;
        } break;
        case Op::Op_And:
        case Op::Op_Or:  {
            // まず子どもたちに適用して子どもたちをリテラルにする
            for (int i = 0; i < formula->terms.size(); i++) {
                auto replacedVar =
                    ApplyTseitinHelp(formula->terms[i], solver, subs);
                if (replacedVar >= 0) {
                    formula->terms[i] = Formula::MakeAtom(replacedVar);
                }
            }
            // このAnd節を置換して1文字にする
            auto newVar = solver.newVar();
            subs[newVar] = formula;
            return newVar;
        } break;
        case Op::Op_Not: {
            assert(formula->terms.size() == 1);
            auto term = formula->terms[0];
            switch (term->op) {
                case Op::Op_And: 
                case Op::Op_Or:
                {
                    auto var = ApplyTseitinHelp(term, solver, subs);
                    if(var >= 0){
                        formula->terms[0] = Formula::MakeAtom(var);
                    }
                    return -1;
                } break;
                case Op::Op_Atom: {
                    // Not Atomはプリミティブ
                    return -1;
                } break;
                case Op::Op_Not: {
                    assert(formula->terms.size() == 1);
                    assert(ApplyTseitinHelp(term, solver, subs) == -1);
                    return -1;
                } break;
                default: {
                    assert(0);
                }
            }
        } break;
        default:
            assert(0);
            break;
    }
}

FormulaPtr ApplyTseitin(FormulaPtr formula, Minisat::Solver& solver, std::map<Minisat::Var, FormulaPtr>& subs){
    auto lastVar = ApplyTseitinHelp(formula, solver, subs);
    if(lastVar >= 0){
        return Formula::MakeAtom(lastVar);
    }else{
        return formula;
    }
}

void PutIntoSolver(FormulaPtr formula, Minisat::Solver& solver){
    std::map<Minisat::Var, FormulaPtr> subs;
    auto lastVar = ApplyTseitin(formula, solver, subs);

}