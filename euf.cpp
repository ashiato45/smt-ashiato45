#include "euf.h"
#include <algorithm>

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

std::shared_ptr<EufPoolNode> EufPool::Add(const EufTerm& term){
    auto found = terms.find(term);
    if(found != terms.end()){
        return nodes[found->second];
    }

    auto pTerm = std::make_shared<EufTerm>(term);
    terms[term] = pTerm;

    auto node = std::make_shared<EufPoolNode>();
    node->term = pTerm;
    for(auto& i: term.args){
        auto childTermNode = Add(i);
        node->children.push_back(childTermNode);
        // childTermNode->children.push_back(childTermNode);
    }
    node->unionArrow = node;  // union-findのunionのarrowは、初期は自分をさす
    node->unionRank = 0;
    nodes[pTerm] = node;
    
    return node;
}

void EufPool::AddEquality(const EufTerm& left, const EufTerm& right){
    // union-findつかってleftからrightに等号はる
    

    // 直近先祖をたどり、あらたな等号をチェック(再帰的)

    assert(0);
}

bool EufPool::Equals(const EufTerm& left, const EufTerm& right){
    assert(0);
}

std::unordered_set<std::shared_ptr<EufPoolNode>> EufPool::CalcPredecessor(std::shared_ptr<EufPoolNode> node){
    std::unordered_set<std::shared_ptr<EufPoolNode>> res;
    // auto repr = FindRoot(node);
    // for(auto& i: nodes){
    //     // iの子のうち、nodeと同じグループに属しているものがあれば、iはnodeのpredecesssor。
    //     auto& children = i.second->children;
    //     if(std::any_of(children.begin(), children.end(), [&node](auto& child){return IsSame(child, node);})){
    //         res.emplace(i);
    //     }
    // }   

    return res;
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

std::shared_ptr<EufPoolNode> EufPool::FindRoot(std::shared_ptr<EufPoolNode> node){
    if(node->unionArrow != node){
        node->unionArrow = FindRoot(node->unionArrow);  // 経路圧縮
    }

    return node->unionArrow;
}

void EufPool::Union(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b){
    auto rootA = FindRoot(a);
    auto rootB = FindRoot(b);

    if(rootA != rootB){
        // BをAにつなぐとき、rank(A←B) = max(rank(A), rank(B) + 1)なので、
        // rank(A) > rank(B)のときには rank(A←B) = rank(A)
        // AをBにつなぐと rank(A→B) = rank(A) + 1なので、AがBよりでかいときにはBをAにつなぐ。
        if(rootA->unionRank > rootB->unionRank){
            // BをAにつなぐ
            rootB->unionArrow = rootA;
            if(rootA->unionRank == rootB->unionRank){
                rootA->unionRank++;
            }
        }else{
            rootA->unionArrow = rootB;
            if(rootA->unionRank == rootB->unionRank){
                rootB->unionRank++;
            }
        }

    }
}

bool EufPool::IsSame(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b){
    return FindRoot(a) == FindRoot(b);
}