#include "euf.h"

EufSymbol EufSymbol::MakeAtom(std::string name)
{
    EufSymbol res;
    res.name = name;
    return res;
}

EufSymbol EufSymbol::MakeFunction(std::string name, int operand)
{
    EufSymbol res;
    res.name = name;
    res.numOp = operand;
    return res;
}

EufTerm EufSymbol::Apply2(EufTerm x, EufTerm y){
    std::vector<EufTerm> args{x, y};
    assert(this->numOp == 2);
    return EufTerm(*this, args);
}

// explicit EufTerm(EufSymbol x){

// }

// EufTerm(EufSymbol f, std::vector<EufTerm> args);


std::string EufTerm::Print() const
{
    // 項を作る
    std::stringstream stream;
    stream << this->symbol.name;
    if(this->symbol.numOp == 0){
        return stream.str();
    }
    stream << "(";
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

std::pair<std::shared_ptr<EufTerm>, std::shared_ptr<EufPoolNode>> EufPool::Add(const EufTerm& term){
    auto found = terms.find(term);
    if(found != terms.end()){
        return {found->second, nodes[found->second]};
    }

    auto pTerm = std::make_shared<EufTerm>(term);
    terms[term] = pTerm;

    auto node = std::make_shared<EufPoolNode>();
    for(auto& i: term.args){
        auto childTermNode = Add(i);
        node->children.push_back(childTermNode.first);
        childTermNode.second->children.push_back(childTermNode.first);
    }
    node->unionArrow = node;
    nodes[pTerm] = node;
    
    return {pTerm, node};
}

void EufPool::AddEquality(const EufTerm& left, const EufTerm& right){
    // union-findつかってleftからrightに等号はる
    

    // 直近先祖をたどり、あらたな等号をチェック(再帰的)

    assert(0);
}

bool EufPool::Equals(const EufTerm& left, const EufTerm& right){
    assert(0);
}

// std::ostream& EufPool::operator<<(std::ostream& ostr, EufPool pool){
//     ostr << "digraph graphname{" << std::endl;
//     for(auto& i: pool.nodes){
//         for(auto& j: i.second->children){
//             ostr << "'" << i.first.Print() << "' -> '" << j.Print() << "';" << std::endl;
//         }
//     }
//     ostr << "}";

//     return ostr;
// }
