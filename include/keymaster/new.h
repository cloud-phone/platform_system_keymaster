/*
**
** Copyright 2017, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef INCLUDE_KEYMASTER_NEW_
#define INCLUDE_KEYMASTER_NEW_

#include <stddef.h>

namespace std {
struct nothrow_t;
extern const nothrow_t nothrow;
}  // namespace std

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif

void* operator new(size_t __sz, const std::nothrow_t&) _NOEXCEPT;
void* operator new[](size_t __sz, const std::nothrow_t&) _NOEXCEPT;
void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif  // INCLUDE_KEYMASTER_NEW_
