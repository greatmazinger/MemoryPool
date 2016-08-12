/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 * Copyright (c) 2016 Bill Quith
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

#include <iostream>
#include <cassert>
#include <vector>

#include "MemoryPool.hpp"
#include "StackAlloc.hpp"

#define MSG(M) fprintf(stdout, M "\n")
#define TEST(X) if (!(X)) { fprintf(stderr, #X " failed."); std::abort(); }

struct Obj
{
  static size_t count;

  char bulk[80];
  
  Obj() { ++count; }
  ~Obj() { --count; }
};

size_t Obj::count = 0;


template <typename O>
class AllocTest
{
public:
  void alloc(int n) {
    count_ += n;
    for (int i=0; i < n; ++i) {
      Obj *o = pool_.newElement();
      ptrs_.push_back(o);
    }
  }
  
  void dealloc(int n) {
    count_ -= n;
    for (int i=0; i < n; ++i) {
      auto p = ptrs_.back();
      pool_.deleteElement(p);
      ptrs_.pop_back();
    }
  }
  
  // shuffle pointers so dealloc random
  void shuffle(int n = -1) {
    const auto nb = ptrs_.size();
    if (n < 0)
      n = count_ / 4;
    for (int i=0; i < n; ++i) {
      const int s = rand() % nb, d = rand() % nb;
      std::swap(ptrs_[s], ptrs_[d]);
    }
  }
  
  int count() const { return count_; }
  
private:
  MemoryPool<O> pool_;
  std::vector<Obj*> ptrs_;
  int count_ = 0;
};

static void test()
{
  MSG("Simple allocation test");
 
  AllocTest<Obj> tpool;
  Obj::count = 0;
  TEST(Obj::count == 0);
  const int sc = 1000;
  
  for (int m=0; m < 10; ++m)
  {
    
    tpool.alloc((rand() & 15)*sc);
    TEST(Obj::count == tpool.count());
    tpool.shuffle();
    TEST(Obj::count == tpool.count());
    tpool.dealloc(tpool.count()/2);
    TEST(Obj::count == tpool.count());
  }

  tpool.dealloc(tpool.count());
  TEST(Obj::count == 0);

  MSG("passed");
}

int main()
{
  std::srand(std::time(0));
  test();
  return 0;
}
