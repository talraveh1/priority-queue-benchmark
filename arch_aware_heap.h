#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

// software prefetch
#include <immintrin.h>

template <std::uint32_t Arity>
class MinHeapT
{
    using uint32_t = std::uint32_t;

public:
    // heap shape (compile-time)
    static constexpr uint32_t arity = Arity;

private:

    static constexpr std::size_t heap_alignment = 64;

    struct HeapDeleter
    {
        void operator()(int* p) const noexcept
        {
            ::operator delete[](p, std::align_val_t(heap_alignment));
        }
    };

    const int maxDepth;
    const uint32_t capacity;
    std::unique_ptr<int[], HeapDeleter> heap;

    int root{};

    uint32_t size{0u};

    static uint64_t pow_u64(uint64_t base, int exp)
    {
        uint64_t r = 1;
        for (int i = 0; i < exp; ++i) {
            if (base != 0 && r > (std::numeric_limits<uint64_t>::max() / base))
                throw std::overflow_error("arity^depth overflow");
            r *= base;
        }
        return r;
    }

    static constexpr uint64_t pow_u64_constexpr(uint64_t base, int exp) noexcept
    {
        // used only for compile-time constants (e.g., prefetch thresholds).
        // experiment parameter ranges are small enough to avoid overflow here.
        uint64_t r = 1;
        for (int i = 0; i < exp; ++i)
            r *= base;
        return r;
    }

    static uint32_t calcCapacity(const int depth)
    {
        if (depth < 0)
            throw std::invalid_argument("depth must be >= 0");
        if (arity < 2)
            throw std::invalid_argument("arity must be >= 2");

        const uint64_t numer = pow_u64(arity, depth + 1) - 1ull;
        const uint64_t denom = arity - 1ull;
        const uint64_t cap = numer / denom;
        if (cap > std::numeric_limits<uint32_t>::max())
            throw std::overflow_error("heap capacity exceeds uint32_t");
        return static_cast<uint32_t>(cap);
    }

    static inline __attribute__((always_inline)) uint32_t parent(uint32_t idx)
    {
        // nodes stored in `heap[]` are indexed in level order
        // excluding the root. The root's children are heap[0..arity-1].
        // for other nodes: parent(child) = floor(child/arity) - 1.
        return (idx / arity) - 1u;
    }

    static inline __attribute__((always_inline)) uint32_t child(uint32_t idx)
    {
        // return the first child of node at index idx in heap[]
        return arity * (idx + 1u);
    }

    static constexpr int pf_hint =
        (arity == 2u) ? _MM_HINT_T0 :
        (arity == 4u) ? _MM_HINT_T1 :
                        _MM_HINT_T2;

public:
    explicit MinHeapT(int maxDepth) :
        maxDepth(maxDepth), capacity(calcCapacity(maxDepth)),
        heap(static_cast<int*>(::operator new[](sizeof(int) * capacity, std::align_val_t(heap_alignment))))
    {
        assert(maxDepth >= 0);
    }

    std::string name() const { return "MinHeap"; }

    static bool isPowerOfTwo(unsigned n) { return n > 0 && (n & n - 1) == 0; }

    static int getPowerOfTwoExp(unsigned n)
    {
        assert(isPowerOfTwo(n));
        return std::numeric_limits<unsigned>::digits - 1 - std::countl_zero(n);
    }

    static int minDepthForSize(uint32_t required)
    {
        if (required == 0u)
            return 0;
        if (arity < 2)
            throw std::invalid_argument("arity must be >= 2");

        uint64_t cap = 0;
        uint64_t level_count = 1;
        int depth = 0;
        while (cap < required) {
            cap += level_count;
            if (cap >= required)
                return depth;
            if (level_count > (std::numeric_limits<uint64_t>::max() / arity))
                throw std::overflow_error("heap capacity exceeds uint64_t");
            level_count *= arity;
            ++depth;
        }
        return depth;
    }

    inline __attribute__((always_inline)) bool isEmpty() const noexcept { return size == 0u; }
    inline __attribute__((always_inline)) bool isFull() const noexcept { return size == capacity; }
    inline bool empty() const noexcept { return isEmpty(); }

    // number of elements currently in the heap
    int getSize() const noexcept { return static_cast<int>(size); }
    std::size_t size() const noexcept { return size; }

    // maximum number of elements the heap can hold
    int getCapacity() const noexcept { return static_cast<int>(capacity); }

    const int* data() const noexcept { return heap.get(); }

    bool validateHeapProperty() const
    {
        if (size == 0u || size == 1u)
            return true;

        const uint32_t nonRootCount = size - 1u;
        for (uint32_t c = 0u; c < nonRootCount; ++c) {
            const int cv = heap[c];
            if (c < arity) {
                if (root > cv)
                    return false;
            } else {
                const uint32_t p = parent(c);
                if (heap[p] > cv)
                    return false;
            }
        }
        return true;
    }

    bool push(int v)
    {
        if (isFull())
            return false;

        if (size == 0u) {
            root = v;
            size = 1u;
            return true;
        }

        // insert at the end of the rootless array.
        uint32_t idx = size - 1u, p;
        int pv;
        heap[idx] = v;
        ++size;

        if (idx < 1'048'575u) { // heaps of up to 20 levels
            // branchless lifting
            while (idx >= arity) {
                p = parent(idx);
                pv = heap[p];
                heap[idx] = std::max(v, pv);
                v = std::min(v, pv);
                idx = p;
            }
        } else { // heaps deeper than 20 levels
            // perform branchless lifting (avoids long paths)
            p = parent(idx);
            pv = heap[p];
            heap[idx] = std::max(v, pv);
            v = std::min(v, pv);
            idx = p;
            p = parent(idx);
            pv = heap[p];
            heap[idx] = std::max(v, pv);
            v = std::min(v, pv);
            idx = p;
            p = parent(idx);
            pv = heap[p];
            heap[idx] = std::max(v, pv);
            v = std::min(v, pv);
            idx = p;
                        
            // switch to a branchy approach thereafter
            while (idx >= arity) {
                p = parent(idx);
                pv = heap[p];
                if (v >= pv)
                    break;
                heap[idx] = pv;
                idx = p;
            }
        }

        // finalize against the root or the current position
        heap[idx] = std::max(v, root);
        root = std::min(v, root);

        return true;
    }

    int top() const
    {
        if (isEmpty())
            throw std::runtime_error("empty heap");

        return root;
    }

    template <bool Condition, class V, class I>
    inline __attribute__((always_inline)) void update_best_if(const V* __restrict heapPtr, I i, I& b, V& bv) noexcept
    {
        if constexpr (Condition) {
            const V v = heapPtr[i];
            if (v < bv) {
                b = i;
                bv = v;
            }
        }
    }

    bool pop()
    {
        if (isEmpty())
            return false;

        if (--size == 0u)
            return true;

        // if we popped the root from a 2-element heap, promote the only child.
        if (size == 1u) {
            root = heap[0u];
            return true;
        }

        // move the last element into the root and sift down
        const uint32_t nonRootCount = size - 1u;
        int v = heap[nonRootCount];

        // hoist pointer once (avoid repeated heap.get())
        int* __restrict h = heap.get();

        auto prefetch_children_block = [&](uint32_t node) noexcept {
            // Prefetch the children block of `node` (one cache line in your layout).
            _mm_prefetch(reinterpret_cast<const char*>(h + child(node)), pf_hint);
        };

        // root step: choose the min among root's children at heap[0..arity-1].
        uint32_t best = 0u;
        int bestVal = heap[0u];

        if (nonRootCount < arity) {
            for (uint32_t i = 1u; i < nonRootCount; ++i)
                if (heap[i] < bestVal)
                    bestVal = heap[i], best = i;
        } else {
            // unroll the first arity comparisons
            update_best_if<true>(h, 1u, best, bestVal);
            update_best_if<arity >= 4u>(h, 2u, best, bestVal);
            update_best_if<arity >= 4u>(h, 3u, best, bestVal);
            update_best_if<arity >= 8u>(h, 4u, best, bestVal);
            update_best_if<arity >= 8u>(h, 5u, best, bestVal);
            update_best_if<arity >= 8u>(h, 6u, best, bestVal);
            update_best_if<arity >= 8u>(h, 7u, best, bestVal);
        }

        // if the last element is smaller than the min child, we're done
        if (v <= bestVal) {
            root = v;
            return true;
        }

        // promote the min child to the root
        root = bestVal;
        uint32_t hole = best;

        // if there are no grandchildren, the hole cannot have children.
        if (nonRootCount <= arity) {
            heap[hole] = v;
            return true;
        }

        const uint32_t lastParent = parent(nonRootCount - 1u);

        // root-step path-only prefetch: we are likely to continue with `hole`.
        // prefetch `hole`'s children block for the first loop iteration.
        if (hole <= lastParent) {
            prefetch_children_block(hole);
        }

        // sift the displaced value down through the rootless array.
        while (hole <= lastParent) {
            const uint32_t first = child(hole);

            // find the best child among heap[first .. first+arity-1]
            best = first, bestVal = heap[first];

            const uint32_t remaining = nonRootCount - first;
            if (remaining < arity) {
                for (uint32_t i = first + 1u; i < nonRootCount; ++i)
                    if (heap[i] < bestVal)
                        bestVal = heap[i], best = i;
            } else {
                // unroll the comparisons
                update_best_if<true>(h, first + 1u, best, bestVal);
                update_best_if<arity >= 4u>(h, first + 2u, best, bestVal);
                update_best_if<arity >= 4u>(h, first + 3u, best, bestVal);
                update_best_if<arity >= 8u>(h, first + 4u, best, bestVal);
                update_best_if<arity >= 8u>(h, first + 5u, best, bestVal);
                update_best_if<arity >= 8u>(h, first + 6u, best, bestVal);
                update_best_if<arity >= 8u>(h, first + 7u, best, bestVal);
            }

            // path-only prefetch: after selecting `best`, prefetch `best`'s children block
            // (i.e., the next iteration's `heap[child(best) .. child(best)+arity-1]`).
            if (best <= lastParent) {
                prefetch_children_block(best);
            }

            if (v <= bestVal)
                break;

            h[hole] = bestVal;
            hole = best;
        }

        h[hole] = v;

        return true;
    }
};

#ifndef NHPQ_MINHEAP_ARITY
#define NHPQ_MINHEAP_ARITY 2u
#endif

using MinHeap = MinHeapT<NHPQ_MINHEAP_ARITY>;
