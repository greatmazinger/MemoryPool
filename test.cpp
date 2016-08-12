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

static void test()
{
  MSG("Simple allocation test");
  
  MemoryPool<Obj> pool;
  Obj::count = 0;
  
  const int nb = 100000;
  std::vector<Obj*> ptrs;

  TEST(Obj::count == 0);
  for (int i=0; i < nb; ++i)
  {
    Obj *o = pool.newElement();
    ptrs.push_back(o);
  }
  TEST(Obj::count == nb);
  
  // shuffle ptrs
  for (int i=0; i < nb/4; ++i)
  {
    const int s = rand() % nb, d = rand() % nb;
    std::swap(ptrs[s], ptrs[d]);
  }

  for (int i=0; i < nb; ++i)
  {
    pool.deleteElement(ptrs[i]);
  }
  ptrs.clear();
  TEST(Obj::count == 0);

  MSG("passed");
}

int main()
{
  std::srand(std::time(0));
  test();
  return 0;
}
