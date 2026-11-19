// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0.
// Standalone host test; no graphics SDK or engine library is needed.
#include <cstdio>
#include <cstdlib>
#include <examples/capi/platform/textra_native_support.hpp>
#include <type_traits>

#define CHECK(x)            \
  do {                      \
    if (!(x)) std::abort(); \
  } while (0)
namespace {
struct State {
  const void* native;
  int calls = 0;
  int destroyed = 0;
  int mode = 0;
};
void TEXTRA_CALL Destroy(void* userdata, void* object) {
  ++static_cast<State*>(userdata)->destroyed;
  delete static_cast<int*>(object);
}
textra_status_t TEXTRA_CALL Import(void* userdata, const void* native,
                                   void** out) {
  auto& state = *static_cast<State*>(userdata);
  CHECK(native == state.native);
  ++state.calls;
  *out = (state.mode == 0 || state.mode == 2) ? new int(1) : nullptr;
  return state.mode == 1 || state.mode == 2 ? TEXTRA_STATUS_UNSUPPORTED
                                            : TEXTRA_STATUS_OK;
}
textra_status_t TEXTRA_CALL Create(void* userdata, void* native, void** out) {
  return Import(userdata, native, out);
}

template <typename Options, typename Member, typename Initializer>
void CheckOwnership(Member member, Initializer initialize) {
  int native = 0;
  State state{&native};
  struct Extended {
    Options options{};
    uint64_t extension = 0x12345678;
  } record;
  auto& options = record.options;
  options.struct_size = sizeof(record);
  options.*member = &native;
  options.userdata = &state;
  auto api = [&]() {
    if constexpr (std::is_same_v<Options, textra_font_manager_options_t>)
      return options.manager_callbacks;
    else
      return options.callbacks;
  };
  // Errors, including a partially created object, leave all input fields
  // intact.
  for (int mode : {1, 2, 3}) {
    state.mode = mode;
    CHECK(initialize(&options) ==
          (mode == 3 ? TEXTRA_STATUS_NOT_FOUND : TEXTRA_STATUS_UNSUPPORTED));
    CHECK(options.*member == &native && !api());
    CHECK(options.userdata == &state);
    CHECK(options.struct_size == sizeof(record));
    if constexpr (!std::is_same_v<Options, textra_font_manager_options_t>)
      CHECK(options.backend == 0 && options.flags == 0);
    CHECK(record.extension == 0x12345678);
  }
  CHECK(state.calls == 3 && state.destroyed == 1);
  state.mode = 0;
  CHECK(initialize(&options) == TEXTRA_STATUS_OK);
  auto object = options.*member;
  CHECK(object && object != &native && api());
  if constexpr (!std::is_same_v<Options, textra_font_manager_options_t>)
    CHECK(options.backend == TEXTRA_RENDERING_BACKEND_SKITY);
  CHECK(options.userdata == &state && options.struct_size == sizeof(record));
  CHECK(record.extension == 0x12345678);
  // Reinitialization must neither wrap the adapter nor acquire another
  // reference.
  CHECK(initialize(&options) == TEXTRA_STATUS_INVALID_ARGUMENT);
  CHECK(options.*member == object && state.calls == 4 && state.destroyed == 1);
  api()->destroy(options.userdata, const_cast<void*>(object));
  CHECK(state.destroyed == 2);
  CHECK(initialize(nullptr) == TEXTRA_STATUS_INVALID_ARGUMENT);
}
}  // namespace

int main() {
  textra_backend_font_manager_api_t manager{};
  manager.destroy = Destroy;
  textra_backend_typeface_api_t face{};
  face.destroy = Destroy;
  CheckOwnership<textra_font_manager_options_t>(
      &textra_font_manager_options_t::font_manager,
      [&](textra_font_manager_options_t* options) {
        auto status =
            textra_native::InitializeManager(options, manager, face, Create);
        if (!status) CHECK(options->typeface_callbacks == &face);
        return status;
      });
  CheckOwnership<textra_typeface_options_t>(
      &textra_typeface_options_t::native_font,
      [&](textra_typeface_options_t* options) {
        return textra_native::InitializeTypeface(
            options, TEXTRA_RENDERING_BACKEND_SKITY, face, Import);
      });
  std::puts(
      "In-place initialization rollback, reinitialization and ownership tests "
      "passed");
}
