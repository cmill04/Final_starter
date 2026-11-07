// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"

MemoryIndex::~MemoryIndex()
{
    clear();
}

bool MemoryIndex::insert(MemoryRecord rec)
{

    if (!root_)
    {
        root_ = new Node{std::move(rec)};
        ++count_;
        return true;
    }
    Node *cur = root_;
    while (1)
    {
        if (rec.id == cur->rec.id)
            return false;
        if (rec.id < cur->rec.id)
        {
            if (!cur->left)
            {
                cur->left = new Node{std::move(rec)};
                break;
            }
            else
            {
                cur = cur->left;
            }
        }
        else
        {
            if (!cur->right)
            {
                cur->right = new Node{std::move(rec)};
                break;
            }
            else
            {
                cur = cur->right;
            }
        }
    }
    count_++;
    return true;
}

const MemoryRecord *MemoryIndex::find(std::uint64_t id) const
{
     Node *cur = root_;
     while(cur){
        if (id == cur->rec.id){
            return &cur->rec;
        }
        if(id < cur->rec.id){
            cur = cur->left;
        }
        else cur = cur->right; 
     }
    return nullptr;
}

bool MemoryIndex::remove(std::uint64_t id)
{

    bool removed = false; 
    root_ = remove_node(root_, id, &removed);
    if (removed) count_--;
    return removed; 
}

MemoryIndex::Node* MemoryIndex::remove_node(Node* n, std::uint64_t id, bool* removed) {
   if (!n){
    return nullptr;
   }
   if (id < n->rec.id){
     n->left = remove_node(n->left, id, removed);
     return n;
   }
   if(id > n->rec.id){
    n->right = remove_node(n->right, id, removed);
    return n;
   }
   else *removed = true; 
   if (!n->left){
    Node* r = n->right;
   delete n;
   return r; 
   }
   if (!n->right){
    Node* l = n->left;
    delete n;
    return l; 
   }

   Node* s = n->right;
   while (s->left){
    s= s->left;
   }
    n->rec = s->rec;
    n->right = remove_node(n->right, s->rec.id, removed);
    return n;
}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const
{
    return height_of(root_);
}

std::size_t MemoryIndex::height_of(const Node* n) {
    if (!n){
        return 0;
    }
   std::size_t Lheight = height_of(n->left);
      std::size_t Rheight = height_of(n->right);
      if (Lheight >= Rheight){
        return 1 + Lheight;
      }
      else return 1 + Rheight; 

}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord &)> &fn) const
{
    in_order(root_, fn);
}

void MemoryIndex::in_order(const Node* n, const std::function<void(const MemoryRecord&)>& fn) {
    if (!n){
        return;
    }
    in_order(n->left, fn);
    fn(n->rec);
    in_order(n->right, fn);
}

void MemoryIndex::clear()
{
    destroy(root_);
    root_ = nullptr; 
    count_ = 0;
}

void MemoryIndex::destroy(Node* n){
    if (!n){
        return;
    }
    destroy(n->left);
    destroy(n->right);
    delete n;
}
