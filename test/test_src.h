// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <textra/layout_drawer.h>
#include <textra/layout_region.h>
#include <textra/run_delegate.h>
#include <textra/text_layout.h>

#include "src/textlayout/paragraph_impl.h"
namespace ttoffice {
namespace tttext {
class InlineBlockTestClass {
 private:
  InlineBlockTestClass() = default;
  ~InlineBlockTestClass() = default;
  InlineBlockTestClass(const InlineBlockTestClass&);
  InlineBlockTestClass& operator=(const InlineBlockTestClass&);

 public:
  //  static std::unique_ptr<LayoutRegion> BorderTest(float width, float
  //  height);
};

class ParagraphStyleTestClass {
 private:
  ParagraphStyleTestClass() = default;
  ~ParagraphStyleTestClass() = default;
  ParagraphStyleTestClass(const ParagraphStyleTestClass&);
  ParagraphStyleTestClass& operator=(const ParagraphStyleTestClass&);

 public:
  static void InitiateDefaultParagraph(TTParagraph& paragraph);
  static std::unique_ptr<LayoutRegion> LineGapTest(TTParagraph& paragraph,
                                                   float width, float height);
  static std::unique_ptr<LayoutRegion> LineHeightTest1(TTParagraph& paragraph,
                                                       float width,
                                                       float height);
  static std::unique_ptr<LayoutRegion> LineHeightTest2(TTParagraph& paragraph,
                                                       float width,
                                                       float height);
  static std::unique_ptr<LayoutRegion> VerticalAlignmentTest1(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> VerticalAlignmentTest2(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> VerticalAlignmentTest3(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> VerticalAlignmentTest4(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HorizontalAlignmentTest1(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HorizontalAlignmentTest2(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HorizontalAlignmentTest3(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HorizontalAlignmentTest4(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HorizontalAlignmentTest5(
      TTParagraph& paragraph, float width, float height);
};
class TextTestClass {
 private:
  TextTestClass() = default;
  ~TextTestClass() = default;
  TextTestClass(const TextTestClass&);
  TextTestClass& operator=(const TextTestClass&);

 public:
  static std::unique_ptr<LayoutRegion> FontStyleTest(TTParagraph& paragraph,
                                                     float width, float height);
  static std::unique_ptr<LayoutRegion> WordBreakTypeTest1(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> WordBreakTypeTest2(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HyphenatorEnableTest(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> HyphenatorDisableTest(
      TTParagraph& paragraph, float width, float height);
  static std::unique_ptr<LayoutRegion> SubSupTest(TTParagraph& paragraph,
                                                  float width, float height);
};

class ShapeTestClass {
 private:
  ShapeTestClass() = default;
  ~ShapeTestClass() = default;
  ShapeTestClass(const ShapeTestClass&);
  ShapeTestClass& operator=(const ShapeTestClass&);

 public:
  // todo
  //  static std::unique_ptr<LayoutRegion> SmallShapeTest(
  //      TTParagraph& paragraph, float width, float height,
  //      std::unique_ptr<RunDelegate> shape);
};

class BoundaryAnalystTestClass {
 public:
  BoundaryAnalystTestClass() = default;
  ~BoundaryAnalystTestClass() = default;
};

}  // namespace tttext
}  // namespace ttoffice
