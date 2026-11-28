// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
#define PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_

#include <stddef.h>
#include <textra/layout_definition.h>
#include <textra/macro.h>

#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#define TEXTRA_HAS_HYPHENATION 1

namespace ttoffice {
namespace tttext {
class LayoutRegion;
class LayoutPosition;
class TextLayoutImpl;
class TTTextContextImpl;

/// Controls whether layout may insert a hyphen when breaking a line.
enum class HyphenationMode {
  kNone,         ///< Disallow discretionary breaks at soft hyphens.
  kManual,       ///< Allow discretionary breaks at soft hyphens.
  kAuto,         ///< Also allow dictionary-provided breaks.
  kUnspecified,  ///< Keep the default line-breaking behavior.
};

// Return a UTF-32 break offset strictly before the given offset, or zero.
using HyphenationLookup =
    std::function<uint32_t(std::u32string_view, uint32_t)>;

/**
 * @brief A class manages the text layout configurations and intermediate layout
 * states.
 *
 * The configurations should be specified by clients before the layout
 * process. The intermediate layout states are set by TextLayout during the
 * layout process.
 */
class L_EXPORT TTTextContext {
 public:
  TTTextContext();
  ~TTTextContext();

  /// Copies a process-wide key/value option, replacing any existing value.
  /// Thread-safe. Null inputs are ignored. Options are shared by all contexts
  /// and are not cleared by Reset() or context destruction.
  static void SetGlobalOption(const char* key, const char* value);

  /// Copies count options from parallel arrays under one lock. Each array must
  /// contain at least count elements. Null arrays or pairs are ignored.
  static void SetGlobalOption(const char* const* keys,
                              const char* const* values, size_t count);

 public:
  // Layout Configurations getters/setters

  /**
   * @brief Controls whether the last line of text can overflow page height
   * boundaries.
   *
   * Value: Boolean. When enabled, the last line may overflow beyond the page
   * height if the line's top position is within the page bounds; when disabled,
   * the last line is always constrained to fit within the page height.
   * Default is true.
   */
  bool IsLastLineCanOverflow() const;
  void SetLastLineCanOverflow(bool lastLineCanOverflow);
  /**
   * @brief Controls whether paragraph spacing is applied before the first line.
   *
   * Value: Boolean. When enabled, the first line starts at the top of the
   * layout area without leading space. Default is false.
   */
  bool IsSkipSpacingBeforeFirstLine() const;
  void SetSkipSpacingBeforeFirstLine(bool skipSpacingBeforeFirstLine);

  /**
   * @brief Force harmony system shaper only use low Level API from Harmony OS
   */
  bool IsHarmonyShaperForceLowAPI() const;
  void SetHarmonyShaperForceLowAPI(bool harmonyShaperForceLowAPI);

  /**
   *@brief Enable font style adjustments based on different systems.
   */
  bool IsEnableSystemFontAdjust() const;
  void SetEnableSystemFontAdjust(bool enable_system_font_adjust);

  /**
   * @brief Aligns Java shaper font metrics with Android StaticLayout.
   *
   * Rounds ascent/descent with floor(value + 0.5) and truncates the font size
   * used for shaping to an integer. Fractional advances are preserved.
   * On API 29+, the system default font family uses two shared Chinese fallback
   * metrics fonts selected by Unicode range, for layouts without includePad.
   * Default is false. Configure before layout and keep unchanged while caches
   * are in use. Only affects the Java shaper.
   */
  bool IsAndroidMetricsRound() const;
  void SetAndroidMetricsRound(bool enabled);

  void EnableFeature(FeatureOption feature_option, bool value);

  /// Selects discretionary break behavior for subsequent layout.
  /// Automatic mode uses the supplied lookup or the system dictionary on macOS.
  void SetHyphenation(HyphenationMode mode, HyphenationLookup lookup = {});

  // Layout state getters/setters
 public:
  void Reset();
  void SetLayoutPosition(uint32_t run_idx, uint32_t char_idx_in_run) const;
  std::pair<uint32_t, uint32_t> GetLayoutPosition() const;
  float GetLayoutBottom() const;
  void SetLayoutBottom(float layout_bottom);
  const TTTextContextImpl& GetImpl();

 private:
  friend class TextLayoutImpl;
  friend class TextLayoutTest;
  friend class TTTextContextTest;
  L_HIDDEN LayoutPosition& GetPositionRef();

 private:
  std::unique_ptr<TTTextContextImpl> impl_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TTTEXT_CONTEXT_H_
