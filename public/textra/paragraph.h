// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_PARAGRAPH_H_
#define PUBLIC_TEXTRA_PARAGRAPH_H_

#include <textra/macro.h>
#include <textra/paragraph_style.h>
#include <textra/style.h>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace ttoffice {
namespace tttext {
/**
 * @brief Represents a paragraph of text with associated styles.
 *
 * Acts as the main input to the text layout system. A Paragraph consists of
 * styled text and optional non-text elements, and is responsible for providing
 * content to be laid out and rendered.
 *
 * Key functionalities:
 * - Text construction: Build the paragraph by appending runs. Runs can be
 * either normal text or non-text (via RunDelegate, which controls size and
 * drawing).
 * - Styling control: Apply styles during run addition or later using
 *   ApplyStyleInRange to modify styles over a specific text range.
 *
 * Paragraph works together with ParagraphStyle (for paragraph-level formatting)
 * and Style (for character-level formatting).
 */
class L_EXPORT Paragraph {
 protected:
  Paragraph() = default;

 public:
  virtual ~Paragraph() = default;
  void AddTextRun(const Style* style, const char* content) {
    AddTextRun(style, content, static_cast<uint32_t>(strlen(content)));
  }

 public:
  virtual ParagraphStyle& GetParagraphStyle() = 0;
  virtual uint32_t GetCharCount() const = 0;
  virtual std::string GetContentString(uint32_t start_char,
                                       uint32_t char_count) const = 0;
  virtual void SetParagraphStyle(const ParagraphStyle* style) = 0;
  virtual void AddTextRun(const Style* style, const char* content,
                          uint32_t length) = 0;
  virtual void AddShapeRun(const Style* style,
                           std::shared_ptr<RunDelegate> shape,
                           bool is_float) = 0;
  virtual void AddGhostShapeRun(const Style* style,
                                std::shared_ptr<RunDelegate> shape) = 0;
  virtual uint32_t GetRunCount() const = 0;
  virtual void ApplyStyleInRange(const Style& style, uint32_t start,
                                 uint32_t len) const = 0;
  virtual float GetMaxIntrinsicWidth() const = 0;
  virtual float GetMinIntrinsicWidth() const = 0;

  // This method is used after layout and can not recognise ideographic words.
  virtual std::pair<uint32_t, uint32_t> GetWordBoundary(
      uint32_t offset) const = 0;

 public:
  static std::unique_ptr<Paragraph> Create();
};
}  // namespace tttext
}  // namespace ttoffice
using TTParagraph = ::ttoffice::tttext::Paragraph;
#endif  // PUBLIC_TEXTRA_PARAGRAPH_H_
