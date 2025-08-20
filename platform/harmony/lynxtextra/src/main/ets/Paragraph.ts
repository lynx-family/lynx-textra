// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { ComponentContent, display, FrameNode } from "@kit.ArkUI";
import tttext from "liblynxtextra.so";

enum ParagraphStyleAlign {
  left = 0,
  center = 1,
  right = 2,
}

export class ParagraphStyle {
  alignment?: ParagraphStyleAlign = null;

  setAlignment(alignment: string): ParagraphStyle {
    let align = alignment.toLowerCase();
    if (align === "left") {
      this.alignment = ParagraphStyleAlign.left;
    } else if (align === "center") {
      this.alignment = ParagraphStyleAlign.center;
    } else if (align === "right") {
      this.alignment = ParagraphStyleAlign.right;
    }
    return this;
  }
}

class Color {
  private readonly color: number;

  constructor(color: number) {
    this.color = color;
  }

  static makeWithARGB(
    alpha: number,
    red: number,
    green: number,
    blue: number
  ): Color {
    alpha = Math.max(0, Math.min(255, alpha));
    red = Math.max(0, Math.min(255, red));
    green = Math.max(0, Math.min(255, green));
    blue = Math.max(0, Math.min(255, blue));

    let color = (alpha << 24) | (red << 16) | (green << 8) | blue;
    return new Color(color);
  }

  value(): number {
    return this.color;
  }
}

export class RunStyle {
  dpi: number = 1;
  font_families?: string[] = null;
  text_size?: number = null;
  text_color?: number = 0xff000000;

  constructor() {
    let displayClass = display.getDefaultDisplaySync();
    this.dpi = displayClass.scaledDensity;
  }

  appendFontFamily(font_family: string): RunStyle {
    if (this.font_families == null) {
      this.font_families = [];
    }
    this.font_families.push(font_family);
    return this;
  }

  setTextSize(size: number): RunStyle {
    this.text_size = size * this.dpi;
    return this;
  }

  setTextColor(color: number): RunStyle {
    this.text_color = color;
    return this;
  }
}

export class RunDelegate {
  node: FrameNode;
  width: number = 20;
  height: number = 20;

  constructor(node: FrameNode) {
    this.node = node;
  }
}

export class Paragraph {
  private run_delegate_list: FrameNode[] = [];

  constructor() {
    tttext.bindNativeParagraph(this);
  }

  setParagraphStyle(para_style: ParagraphStyle): Paragraph {
    tttext.setParagraphStyle(this, para_style);
    return this;
  }

  addTextRun(content: string, style: RunStyle): Paragraph {
    tttext.addTextRun(this, content, style);
    return this;
  }

  addRunDelegate(delegate: RunDelegate): Paragraph {
    this.run_delegate_list.push(delegate.node);
    tttext.addFrameNode(this, delegate.node, delegate.width, delegate.height);
    return this;
  }
}
