#include "euf.h"
#include <algorithm>

#include "boost/boost/container_hash/hash.hpp"
namespace boost
{
    std::size_t hash_value(const EufTerm& et){
        // TODO: あとでboost流にかきなおしたほうがいいかも
        return std::hash<EufTerm>{}(et);
    }
}


    std::size_t std::hash<EufAtom>::operator()(EufAtom const& a) const noexcept
    {
        size_t hash_value = 0;
        boost::hash_combine(hash_value, a.equality);
        boost::hash_combine(hash_value, a.left);
        boost::hash_combine(hash_value, a.right);
        return hash_value;
    }

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
    auto nLeft = nodes[terms[left]];
    auto nRight = nodes[terms[right]];
    return IsSame(nLeft, nRight);
}

std::unordered_set<std::shared_ptr<EufPoolNode>> EufPool::CalcPredecessor(std::shared_ptr<EufPoolNode> node){
    std::unordered_set<std::shared_ptr<EufPoolNode>> res;
    auto repr = FindRoot(node);
    for(auto i: nodes){
        // iの子のうち、nodeと同じグループに属しているものがあれば、iはnodeのpredecesssor。
        auto& children = i.second->children;
        if(std::any_of(children.begin(), children.end(), [this, &node](std::shared_ptr<EufPoolNode> child){return IsSame(child, node);})){
            res.insert(i.second);
        }
    }   
    return res;
}


void EufPool::Merge(const EufTerm& left, const EufTerm& right){
    auto leftNode = nodes[terms[left]];
    auto rightNode = nodes[terms[right]];

    Merge(leftNode, rightNode);
}

void EufPool::Merge(std::shared_ptr<EufPoolNode> left, std::shared_ptr<EufPoolNode> right){
    if(FindRoot(left) == FindRoot(right)){
        return;
    }

    auto preLeft = CalcPredecessor(left);
    auto preRight = CalcPredecessor(right);

    Union(left, right);

    for(auto& x: preLeft){
        for(auto& y: preRight){
            if(!IsSame(x, y) && Congruent(x, y)){
                Merge(x, y);
            }
        }
    }
}

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

bool EufPool::Congruent(std::shared_ptr<EufPoolNode> a, std::shared_ptr<EufPoolNode> b){
    if(a->term->symbol != b->term->symbol){
        return false;  // 2つがそもそも違うシンボルならcongruentではない
    }

    if(a->term->args.size() != b->term->args.size()){
        return false;  // シンボルがおなじでも、オペランドの数が違うならcongruentでない
    }

    // 第i引数同士に等号があるか？1個でもちがうならだめ。
    for(int i=0; i < a->term->args.size(); i++){
        auto& ai = a->children[i];
        auto& bi = b->children[i];
        if(!IsSame(ai, bi)){
            return false;
        }
    }

    return true;
}

template<>
void EufFormula::AppendAsString(std::ostringstream& oss) const{
    oss << atom.left.Print();
    if(atom.equality){
        oss << " == ";
    }else{
        oss << " != ";
    }
    oss << atom.right.Print() << std::endl;
}

EufModel EufSolve(EufFormula formula){
    assert(0);
}