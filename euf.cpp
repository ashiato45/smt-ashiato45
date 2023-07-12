#include "euf.h"

EufSymbol EufSymbol::MakeAtom(std::string name)
{
    EufSymbol res;
    res.kind = EufKind::Atom;
    res.name = name;
    return res;
}

EufSymbol EufSymbol::MakeFunction(std::string name, int operand)
{
    EufSymbol res;
    res.kind = EufKind::Function;
    res.name = name;
    res.numOp = operand;
    return res;
}

EufTerm EufSymbol::Apply2(EufTerm x, EufTerm y){
    std::vector<EufTerm> args{x, y};
    assert(this->kind == EufKind::Function);
    assert(this->numOp == 2);
    return EufTerm(*this, args);
}

// explicit EufTerm(EufSymbol x){

// }

// EufTerm(EufSymbol f, std::vector<EufTerm> args);


std::string EufTerm::Print() const
{
    switch (this->kind)
    {
    case Atom:
    {
        return this->symbol.name;
    }
    break;
    case Function:
    {
        // 項を作る
        std::stringstream stream;
        stream << this->symbol.name << "(";
        for(int i=0; i < this->args.size(); i++){
            stream << this->args[i].Print();
            if(i != this->args.size() - 1){
                stream << ", ";
            }
        }
        stream << ")";
        // push
        return stream.str();
    }
    break;
    default:
        assert(0);
    }
    assert(0);
    return "";
}
