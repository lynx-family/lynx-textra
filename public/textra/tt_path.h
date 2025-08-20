// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef PUBLIC_TEXTRA_TT_PATH_H_
#define PUBLIC_TEXTRA_TT_PATH_H_
#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace ttoffice {
namespace tttext {
using PathPointF = std::array<float, 2>;
enum class PathType { kLines, kArc, kBezier, kMoveTo, kMultiPath };
class Path {
 public:
  explicit Path(PathType type) : type_(type) {}
  virtual ~Path() = default;
  PathType type_;
};
class LinesPath final : public Path {
 public:
  LinesPath() : Path(PathType::kLines) {}
  explicit LinesPath(std::vector<PathPointF> points)
      : Path(PathType::kLines), points_(std::move(points)) {}
  ~LinesPath() override = default;
  std::vector<PathPointF> points_;
};
class ArcPath final : public Path {
 public:
  ArcPath() : Path(PathType::kArc) {}
  PathPointF start_{};
  PathPointF mid_{};
  PathPointF end_{};
  float radius_{};
};
class BezierPath final : public Path {
 public:
  BezierPath() : Path(PathType::kBezier) {}
  ~BezierPath() override = default;
  std::vector<PathPointF> points_;
};
class MoveToPath final : public Path {
 public:
  MoveToPath() : Path(PathType::kMoveTo) {}
  explicit MoveToPath(PathPointF point)
      : Path(PathType::kMoveTo), point_(point) {}
  ~MoveToPath() override = default;
  PathPointF point_{};
};
class MultiPath final : public Path {
 public:
  MultiPath() : Path(PathType::kMultiPath) {}
  explicit MultiPath(std::vector<std::unique_ptr<Path>> path_list)
      : Path(PathType::kMultiPath), path_list_(std::move(path_list)) {}
  ~MultiPath() override = default;
  std::vector<std::unique_ptr<Path>> path_list_;
};
}  // namespace tttext
}  // namespace ttoffice
#endif  // PUBLIC_TEXTRA_TT_PATH_H_
