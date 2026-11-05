// include/util/min_heap.h
//
// YOU IMPLEMENT (this whole file; it is a template, so the code lives in the
// header). Spec §4.1.
//
// A binary MIN-heap stored in an array (a std::vector), plus top_k(), which
// uses a size-k min-heap to pick the k largest items in O(n log k).
//
// Rules for this file:
//   * Write sift-up and sift-down yourself.
//   * Do NOT use std::priority_queue, std::make_heap / push_heap / pop_heap /
//     sort_heap, std::sort / stable_sort / partial_sort / nth_element, or
//     std::set / multiset / map / multimap (full list: spec §7.3).
//   * std::vector is fine as the storage array; std::swap, std::reverse and
//     std::pair are fine too.
//
// `Less` is a "less-than" function object, exactly like the one std::sort
// takes: less(a, b) is true when a should come before b. The heap keeps the
// SMALLEST element (by less) at the top.

#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Less = std::less<T>>
class MinHeap
{
public:
    explicit MinHeap(Less less = Less()) : less_(std::move(less)) {}

    // Adds value. O(log n).
    void push(T value)
    {
        data_.push_back(std::move(value));
        sift_up(data_.size() - 1);
    }

    // The smallest element. Throws std::out_of_range if empty. O(1).
    const T &top() const
    {
        if (data_.empty())
        {
            throw std::out_of_range("MinHeap:: top call on empty heap");
        }
        return data_[0];
    }

    // Removes and returns the smallest element. Throws std::out_of_range if
    // empty. O(log n).
    T pop()
    {
        if (data_.empty())
        {
            throw std::out_of_range("MinHeap::pop on empty heap");
        }
        T rootVal = std::move(data_[0]);
        std::size_t n = data_.size() - 1;
        std::swap(data_[0], data_[n]);
        data_.pop_back();
        sift_down(0);
        return rootVal;
    }

    std::size_t size() const noexcept { return data_.size(); }
    bool empty() const noexcept { return data_.empty(); }
    void clear() noexcept { data_.clear(); }

private:
    // ===== YOUR HELPERS AND MEMBERS =====
    // Children of index i are 2i+1 and 2i+2; its parent is (i-1)/2.
    // Suggested: void sift_up(std::size_t i);  void sift_down(std::size_t i);
    // Clue: use std::size_t (not int) for indices, or -Wsign-compare warns
    // every time you compare with data_.size(). Careful: (i-1)/2 with i == 0
    // underflows, so sift_up must stop at i == 0.
    // Clue: in sift_down, the right child may not exist: check
    // 2*i+2 < data_.size() before reading it. Swap with the SMALLER child.

    std::vector<T> data_;
    Less less_;

    void sift_up(std::size_t i)
    {
        while (i > 0)
        {
            std::size_t p = (i - 1) / 2;
            if (less_(data_[i], data_[p]))
            {
                std::swap(data_[i], data_[p]);
                i = p;
            }
            else
                break;
        }
    }

    void sift_down(std::size_t i)
    {
        std::size_t n = data_.size();
        while (1)
        {
            std::size_t lc = 2 * i + 1;
            std::size_t rc = 2 * i + 2;
            if (lc >= n)
                break;
            std::size_t sc = lc;
            if (rc < n)
            {
                if (less_(data_[rc], data_[lc]))
                {
                    sc = rc;
                }
            }
            if (less_(data_[sc], data_[i]))
            {
                std::swap(data_[sc], data_[i]);
                i = sc;
            }
            else
                break;
        }
    }
};

// Returns the k LARGEST items of `items` (by `less`), largest first.
//   * k == 0 or items empty -> empty result
//   * k >= items.size()     -> every item, largest first
// Must keep a MinHeap of at most k items (the k largest seen so far, with
// the smallest of them on top), so it runs in O(n log k) time and O(k) extra
// space. Items that compare equal may come out in any order.
template <typename T, typename Less = std::less<T>>
std::vector<T> top_k(const std::vector<T> &items, std::size_t k, Less less = Less())
{

    if (k == 0)
    {
        return {};
    }
    MinHeap<T, Less> heap(less);
    for (std::size_t i = 0; i < items.size(); i++)
    {
        if (heap.size() < k)
        {
            heap.push(items[i]);
        }
        else if (less(heap.top(), items[i]))
        {
            heap.pop();
            heap.push(items[i]);
        }
    }
    std::vector<T> result;
    while (!heap.empty())
    {
        result.push_back(heap.pop());
    }
    std::reverse(result.begin(), result.end());
    return result;
}
