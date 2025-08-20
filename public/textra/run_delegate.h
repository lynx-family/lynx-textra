// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_RUN_DELEGATE_H_
#define PUBLIC_TEXTRA_RUN_DELEGATE_H_

#include <limits>

namespace ttoffice {
namespace tttext {
class TextLineImpl;
class ICanvasHelper;
/**
 * @brief Defines the size and drawing behavior of a non-text run, such as an
 * inline image.
 *
 * When the text layout system encounters a run backed by a RunDelegate, it
 * queries the delegate for dimensions and drawing logic. This enables seamless
 * layout and rendering of both text and non-text content within the same
 * system.
 */
class RunDelegate {
 public:
  RunDelegate() = default;
  virtual ~RunDelegate() = default;

 public:
  /**
   * Returns the ascent value, the distance from the baseline to the highest
   * point of the run (typically negative).
   */
  virtual float GetAscent() const = 0;

  /**
   * Returns the descent value, the distance from the baseline to the lowest
   * point of the run (typically positive).
   */
  virtual float GetDescent() const = 0;

  /**
   * Returns the advance width of the run, the horizontal distance of the run
   * (typically positive).
   */
  virtual float GetAdvance() const = 0;

  virtual bool Equals(RunDelegate* other) { return this == other; }

  /**
   * Performs layout calculations for this run. Override this method to
   * implement custom layout behavior.
   */
  virtual void Layout() {}

  /**
   * Draws the run content onto the canvas.
   */
  virtual void Draw(ICanvasHelper* canvas, float x, float y) {}

  float GetXOffset() { return x_offset_; }

  float GetYOffset() { return y_offset_; }

 private:
  void SetOffset(float x, float y) {
    x_offset_ = x;
    y_offset_ = y;
  }

 private:
  float x_offset_{std::numeric_limits<float>::lowest()};
  float y_offset_{std::numeric_limits<float>::lowest()};
  friend class TextLineImpl;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_RUN_DELEGATE_H_
