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

#ifndef MEMORY_BLOCK_TCC
#define MEMORY_BLOCK_TCC



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
noexcept
{
  firstBlock_ = nullptr;
  lastBlock_ = nullptr;
  firstFreeBlock_ = nullptr;
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool)
noexcept
{
  firstBlock_ = memoryPool.firstBlock_;
  memoryPool.firstBlock_ = nullptr;
  lastBlock_ = memoryPool.lastBlock_;
  memoryPool.lastBlock_ = nullptr;
  firstFreeBlock_ = memoryPool.firstFreeBlock_;
  memoryPool.firstFreeBlock_ = nullptr;
}


template <typename T, size_t BlockSize>
template<class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U, BlockSize>& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>&
MemoryPool<T, BlockSize>::operator=(MemoryPool&& memoryPool)
noexcept
{
  if (this != &memoryPool)
  {
	std::swap(firstBlock_, memoryPool.firstBlock_);
    std::swap(lastBlock_, memoryPool.lastBlock_);
	std::swap(firstFreeBlock_, memoryPool.firstFreeBlock_);
  }
  return *this;
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool()
noexcept
{
  Block_* currBlock = firstBlock_;
  while (currBlock != nullptr) {
    Block_* nextBlock = currBlock->nextBlock;
    operator delete(reinterpret_cast<void*>(currBlock));
    currBlock = nextBlock;
  }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::address(reference x)
const noexcept
{
  return &x;
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer
MemoryPool<T, BlockSize>::address(const_reference x)
const noexcept
{
  return &x;
}



template <typename T, size_t BlockSize>
typename MemoryPool<T, BlockSize>::Block_*
MemoryPool<T, BlockSize>::allocateBlock()
{
  // Allocate space for the new block
  data_pointer_ newBlockData = reinterpret_cast<data_pointer_>
                           (operator new(BlockSize));

  Block_* newBlock = reinterpret_cast<Block_*>(newBlockData);
  newBlock->nextBlock = nullptr;
  newBlock->freeSlotsListHead = &(newBlock->slots[0]);
  newBlock->freeSlotsListHead->next = nullptr;
  newBlock->freeSlotsCount = NumberOfSlotsPerBlock;
  newBlock->slots[0].next = nullptr;

  if (lastBlock_ == nullptr) {
    firstBlock_ = newBlock;
  }
  else {
    lastBlock_->nextBlock = newBlock;
  }
  lastBlock_ = newBlock;

  return newBlock;
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
  if (firstFreeBlock_ == nullptr) {
    Block_* newBlock = allocateBlock();
	firstFreeBlock_ = newBlock;
  }

  Slot_* resSlot = firstFreeBlock_->freeSlotsListHead;

  if (--firstFreeBlock_->freeSlotsCount == 0) {
	firstFreeBlock_->freeSlotsListHead = nullptr;
    do {
	  firstFreeBlock_ = firstFreeBlock_->nextBlock;
	} while (firstFreeBlock_ != nullptr && firstFreeBlock_->freeSlotsCount == 0);
  }
  else {
	  if (resSlot->next == nullptr) {
		  resSlot->next = resSlot + 1;
		  resSlot->next->next = nullptr;
	  }
	  firstFreeBlock_->freeSlotsListHead = resSlot->next;
  }

  return reinterpret_cast<pointer>(resSlot);
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n)
{
  if (p != nullptr) {
	bool firstFreeBlockFollowsAfterCurrBlock = true;
	const uintptr_t searchAddress = reinterpret_cast<uintptr_t>(p);
	for (Block_ *currBlock = firstBlock_, *prevBlock = nullptr;
	  currBlock != nullptr;
	  prevBlock = currBlock, currBlock = currBlock->nextBlock)
	{
	  if (firstFreeBlock_ == currBlock) {
		firstFreeBlockFollowsAfterCurrBlock = false;
	  }

      const uintptr_t firstSlotAddress = reinterpret_cast<uintptr_t>(&currBlock->slots[0]);
      const uintptr_t lastSlotAddress = reinterpret_cast<uintptr_t>(&currBlock->slots[LastSlotIndex]);
	  if (searchAddress >= firstSlotAddress
	    && searchAddress <= lastSlotAddress) {
		Slot_* currSlot = reinterpret_cast<Slot_*>(p);

		// insert deallocated memory to free slots list
		currSlot->next = currBlock->freeSlotsListHead;
		currBlock->freeSlotsListHead = currSlot;
		
		// the block should be deleted?
		if (++currBlock->freeSlotsCount == NumberOfSlotsPerBlock) {
		  if (firstFreeBlock_ == currBlock) {
			// if firstFreeBlock_ points to currBlock, which should
			// be deallocated, search for the next free block
			do {
			  firstFreeBlock_ = firstFreeBlock_->nextBlock;
			} while (firstFreeBlock_ != nullptr &&
				firstFreeBlock_->freeSlotsCount == 0);
		  }
		  
		  if (currBlock == firstBlock_) {
			// if firstBlock_ points to currBlock,
			// link the firstBlock_ to next block
		    firstBlock_ = currBlock->nextBlock;
		  }
		  else {
			// link the prev block to next after curr block
			prevBlock->nextBlock = currBlock->nextBlock;
		  }

		  if (currBlock == lastBlock_) {
			// if lastBlock_ points to currBlock,
			// link the lastBlock_ to prev block
		    lastBlock_ = prevBlock;
		  }
		  operator delete(reinterpret_cast<void*>(currBlock));
		}
		else if (firstFreeBlockFollowsAfterCurrBlock) {
		  firstFreeBlock_ = currBlock;
		}
        break;
      }
    }
  }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::max_size()
const noexcept
{
  size_type maxBlocks = (size_type)-1 / BlockSize;
  return (BlockSize - sizeof(Block_)) / sizeof(Slot_) * maxBlocks;
}



template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void
MemoryPool<T, BlockSize>::construct(U* p, Args&&... args)
{
  new (p) U (std::forward<Args>(args)...);
}



template <typename T, size_t BlockSize>
template <class U>
inline void
MemoryPool<T, BlockSize>::destroy(U* p)
{
  p->~U();
}



template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(Args&&... args)
{
  pointer result = allocate();
  construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
  if (p != nullptr) {
    p->~value_type();
    deallocate(p);
  }
}



#endif // MEMORY_BLOCK_TCC
