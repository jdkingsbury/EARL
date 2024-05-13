// MIT License

// Copyright (c) 2023 malloc-nbytes

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

// File: utils.h
// Description:
//   A bunch of helpful utility functions.

#ifdef DEBUG
#define DEBUG_ASSERT(expr) assert(expr)
#else
#define DEBUG_ASSERT(expr) ((void)0)
#endif

#define NOOP(x) ((void)(x))
#define UNIMPLEMENTED(msg, rettype)            \
  fprintf(stderr, "UNIMPLEMENTED: " msg "\n"); \
  return rettype

void *utils_safe_malloc(size_t bytes);
int utils_streq(const char *s1, const char *s2);

#endif // UTILS_H