/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#ifdef _MSC_VER
#if !defined(_ITERATOR_DEBUG_LEVEL) || _ITERATOR_DEBUG_LEVEL != 0
#error "_ITERATOR_DEBUG_LEVEL should be defined as 0 to use this allocator \
for MSVC's STL containers"
#endif
#endif

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

template <typename T, size_t BlockSize = 4096, bool LeaveSingleFreeBlock = false>
class MemoryPool
{
  public:
    /* Member types */
    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::true_type  propagate_on_container_move_assignment;
    typedef std::true_type  propagate_on_container_swap;

    template <typename U> struct rebind {
      typedef MemoryPool<U, BlockSize, LeaveSingleFreeBlock> other;
    };

    /* Member functions */
    MemoryPool() noexcept;
    MemoryPool(const MemoryPool& memoryPool) noexcept;
    MemoryPool(MemoryPool&& memoryPool) noexcept;
    template <class U> MemoryPool(const MemoryPool<U, BlockSize, LeaveSingleFreeBlock>& memoryPool) noexcept;

    ~MemoryPool() noexcept;

    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept;

    pointer address(reference x) const noexcept;
    const_pointer address(const_reference x) const noexcept;

    // Can only allocate one object at a time. n and hint are ignored
    pointer allocate(size_type n = 1, const_pointer hint = 0);
    void deallocate(pointer p, size_type n = 1);

    size_type max_size() const noexcept;

    template <class U, class... Args> void construct(U* p, Args&&... args);
    template <class U> void destroy(U* p);

    template <class... Args> pointer newElement(Args&&... args);
    void deleteElement(pointer p);

  private:
    union Slot_ {
      value_type element;
      Slot_* next;
    };

    typedef char* data_pointer_;

	struct Block_ {
	  Block_* nextBlock;
	  Slot_* freeSlotsListHead;
	  size_type freeSlotsCount;

	  Slot_ slots[0];
	};

	Block_* firstBlock_;
	Block_* lastBlock_;
	Block_* firstFreeBlock_;

	static constexpr size_type NumberOfSlotsPerBlock = (BlockSize - offsetof(Block_,slots)) / sizeof(Slot_);
	static constexpr size_type LastSlotIndex = NumberOfSlotsPerBlock - 1;
	
    Block_* allocateBlock();

    static_assert(BlockSize >= sizeof(Block_) + sizeof(Slot_), "BlockSize too small.");
};

#include "MemoryPool.tcc"

#endif // MEMORY_POOL_H
