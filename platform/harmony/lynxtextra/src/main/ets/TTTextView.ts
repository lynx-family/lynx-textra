// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { FrameNode, NodeContent } from "@kit.ArkUI";
import tttext from "liblynxtextra.so";

import { Paragraph, RunStyle } from "./Paragraph";

export class TTTextView extends NodeContent {
  constructor() {
    super();
    tttext.createNativeTextNode(this);
  }

  appendParagraph(paragraph: Paragraph) {
    tttext.appendParagraphToTextNode(this, paragraph);
  }
}
