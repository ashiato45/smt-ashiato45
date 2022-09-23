#include "formula.h"

#include <algorithm>
#include <cassert>

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

namespace {
struct HelpTseitinData {
    FormulaPtr freshPos;
    FormulaPtr freshNeg;
    std::vector<FormulaPtr> insidePos;
    std::vector<FormulaPtr> insideNeg;
};

HelpTseitinData HelpTseitin(FormulaPtr f,
                            std::shared_ptr<Minisat::Solver> solver) {
    assert(f->op == Op::Op_And || f->op == Op::Op_Or);
    auto freshPos = Formula::MakeAtom(solver->newVar());
    auto freshNeg = Formula::MakeNot(freshPos);
    std::vector<FormulaPtr> insidePos(f->terms.size());
    std::vector<FormulaPtr> insideNeg(f->terms.size());
    std::transform(f->terms.begin(), f->terms.end(), insidePos.begin(),
                   [solver](FormulaPtr i) { return ApplyTseitin(i, solver); });
    std::transform(f->terms.begin(), f->terms.end(), insidePos.begin(),
                   [solver](FormulaPtr i) {
                       return ApplyTseitin(Formula::MakeNot(i), solver);
                   });

    return HelpTseitinData{freshPos, freshNeg, insidePos, insideNeg};
}
}  // namespace

FormulaPtr ApplyTseitin(FormulaPtr f, std::shared_ptr<Minisat::Solver> solver) {
    switch (f->op) {
        case Op::Op_Atom: {
            return f;
        } break;
        case Op::Op_And: {
            // (a∧b)を、fresh variable cをつかい(c ∧
            // (c<->a∧b))にする。後半は(c->a∧b ∧ a∧b->c)で、さらに(¬c∨(a∧b) ∧
            // (¬(a∧b)∨c))で、 分配すると(¬c∨a ∧ ¬c∨b ∧
            // ¬a∨¬b∨c)になる。まとめると、(c ∧ ¬c∨a ∧ ¬c∨b ∧
            // ¬a∨¬b∨c)になる。これを複数個に一般化する。

            // 中身のabたちと、¬aや¬bを先に処理する。
            auto [freshPos, freshNeg, insidePos, insideNeg] =
                HelpTseitin(f, solver);

            std::vector<FormulaPtr> newTerms;
            newTerms.push_back(freshPos);  // cの部分
            // ¬c∨a ∧ ¬c∨bたちの部分
            for (auto i : insidePos) {
                newTerms.push_back(Formula::MakeOr(freshNeg, i));
            }
            // ¬a∨¬b∨c の部分
            std::vector<FormulaPtr> lastPart;
            lastPart.push_back(freshPos);
            for (auto i : insideNeg) {
                lastPart.push_back(i);
            }
            newTerms.push_back(
                FormulaPtr{new Formula{Op::Op_Or, {}, lastPart}});

            return FormulaPtr{new Formula{Op::Op_And, {}, newTerms}};
        } break;
        case Op::Op_Or: {
            // (a∨b)を、fresh variable cをつかい(c ∧
            // (c<->a∨b))にする。後半は(c->a∨b ∧ a∨b->c)で、さらに(¬c∨a∨b ∧
            // (¬a∧¬b)∨c))で、 分配すると(¬c∨a∨b ∧ ¬a∨c ∧
            // ¬b∨c)になる。これを複数個に一般化する。

            // 中身のabたちと、¬aや¬bを先に処理する。
            auto [freshPos, freshNeg, insidePos, insideNeg] =
                HelpTseitin(f, solver);

            std::vector<FormulaPtr> newTerms;
            newTerms.push_back(freshPos);  // cの部分
            // ¬a∨c ∧ ¬b∨cたちの部分
            for (auto i : insideNeg) {
                newTerms.push_back(Formula::MakeOr(freshPos, i));
            }
            // ¬c∨a∨b の部分
            std::vector<FormulaPtr> lastPart;
            lastPart.push_back(freshNeg);
            for (auto i : insidePos) {
                lastPart.push_back(i);
            }
            newTerms.push_back(
                FormulaPtr{new Formula{Op::Op_Or, {}, lastPart}});

            return FormulaPtr{new Formula{Op::Op_And, {}, newTerms}};
        } break;
        case Op::Op_Not: {
            assert(f->terms.size() == 1);
            auto term = f->terms[0];
            // Notを下に叩きこんでApply
            switch (term->op) {
                case Op::Op_And: {
                    std::vector<FormulaPtr> newTerms(term->terms);
                    std::transform(term->terms.begin(), term->terms.end(),
                                   newTerms.begin(), [](FormulaPtr i) {
                                       return Formula::MakeNot(i);
                                   });
                    return ApplyTseitin(
                        FormulaPtr{new Formula{Op::Op_Or, {}, newTerms}},
                        solver);
                } break;
                case Op::Op_Atom: {
                    // Not Atomはプリミティブ
                    return f;
                } break;
                case Op::Op_Not: {
                    assert(f->terms.size() == 1);
                    // Not Notなのでうらがえす
                    return ApplyTseitin(term->terms[0], solver);
                } break;
                case Op::Op_Or: {
                    // And notにする
                    std::vector<FormulaPtr> newTerms(term->terms);
                    std::transform(term->terms.begin(), term->terms.end(),
                                   newTerms.begin(), [](FormulaPtr i) {
                                       return Formula::MakeNot(i);
                                   });
                    return ApplyTseitin(
                        FormulaPtr{new Formula{Op::Op_And, {}, newTerms}},
                        solver);
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