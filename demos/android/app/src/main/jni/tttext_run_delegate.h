// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_RUN_DELEGATE_H_
#define DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_RUN_DELEGATE_H_

#include <jni.h>
#include <textra/macro.h>
#include <textra/run_delegate.h>
class L_EXPORT TTTextRunDelegate : public tttext::RunDelegate {
 public:
  TTTextRunDelegate(int id, float ascent, float descent, float advance);

 public:
  float GetAdvance() const override { return advance_; }
  float GetAscent() const override { return ascent_; }
  float GetDescent() const override { return descent_; }

  int GetID() const { return id_; }
  void Draw(tttext::ICanvasHelper* canvas, float x, float y) override;

  bool Equals(RunDelegate* other) override {
    auto* java_other = (TTTextRunDelegate*)other;
    return id_ == java_other->id_;
  }

  void SetAscent(float ascent) { ascent_ = ascent; }
  void SetDescent(float descent) { descent_ = descent; }

 protected:
  int id_;
  float ascent_;
  float descent_;
  float advance_;
};

#endif  // DEMOS_ANDROID_APP_SRC_MAIN_JNI_TTTEXT_RUN_DELEGATE_H_
