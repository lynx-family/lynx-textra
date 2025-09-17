// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { DrawContext, FrameNode, NodeController, RenderNode } from '@kit.ArkUI';
import { drawing } from '@kit.ArkGraphics2D';
import { UIContext } from '@kit.ArkUI'
import { image } from '@kit.ImageKit'
import systemDateTime from '@ohos.systemDateTime';
// import { TTTextView, ParagraphStyle, RunStyle, Paragraph, RunDelegate } from '@lynx/lynxtextra'
import perftest from "libperftest.so";
import { BaseRenderNode } from './BaseRenderNode';

export class TextraRenderNode extends BaseRenderNode {
  initLayoutContext() {
    perftest.initLayoutContext(this, 0);
  }
}

export class TextraNodeController extends NodeController {
  private nodeWidth: number = 1250;
  private nodeHeight: number = 1000;
  private rootNode: FrameNode | null = null;
  public renderNode: TextraRenderNode = new TextraRenderNode();

  aboutToAppear(): void {
    this.renderNode.initLayoutContext();
    this.renderNode.buildNativeCanvas(this.nodeWidth, this.nodeHeight);
  }

  makeNode(uiContext: UIContext): FrameNode | null {
    this.rootNode = new FrameNode(uiContext);
    const rootRenderNode = this.rootNode.getRenderNode();
    if (rootRenderNode !== null) {
      this.renderNode.frame = {
        x: 0,
        y: 0,
        width: this.nodeWidth,
        height: this.nodeHeight
      }
      rootRenderNode.appendChild(this.renderNode);
    }
    return this.rootNode;
  }
}
