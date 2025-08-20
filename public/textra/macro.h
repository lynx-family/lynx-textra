// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_MACRO_H_
#define PUBLIC_TEXTRA_MACRO_H_

#define L_EXPORT __attribute__((visibility("default")))
#define L_HIDDEN __attribute__((visibility("hidden")))

namespace ttoffice {
namespace tttext {}
}  // namespace ttoffice
namespace tttext = ttoffice::tttext;

#define TTDYNAMIC_CAST static_cast
#define TTCONST_CAST const_cast

#ifdef TTTEXT_DEBUG
#include <cassert>
#define TTASSERT(x) assert(x)
#else
#define TTASSERT(x)
#endif  // TTTEXT_DEBUG

#ifdef ENABLE_GTEST
#include <gtest/gtest_prod.h>
#define TT_FRIEND_TEST(test_case_name, test_name) \
  FRIEND_TEST(test_case_name, test_name)
#else
#define TT_FRIEND_TEST(test_case_name, test_name)
#endif  // ENABLE_GTEST

#endif  // PUBLIC_TEXTRA_MACRO_H_
