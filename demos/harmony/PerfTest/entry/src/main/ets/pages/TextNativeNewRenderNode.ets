// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { FrameNode, NodeController, RenderNode } from '@kit.ArkUI';
import { drawing } from '@kit.ArkGraphics2D';
import { UIContext } from '@kit.ArkUI'
import { text } from '@kit.ArkGraphics2D'
import { image } from '@kit.ImageKit'
import { common2D } from '@kit.ArkGraphics2D'
import systemDateTime from '@ohos.systemDateTime';
import perftest from "libperftest.so";


export class TextNativeNewRenderNode extends RenderNode {
  content: string | null = null
  layoutTime: number = 0;
  drawTime: number = 0;
  isNano: boolean = true;
  paragraph: Paragraph | null = null

  initPerf() {
    this.layoutTime = 0
    this.drawTime = 0
  }

  initLayoutContext() {
    perftest.initLayoutContext(this, 2);
  }

  buildNativeCanvas(width: number, height: number) {
    perftest.buildCanvas(this, width, height);
  }

  layoutContent(content: string, width: number) {
    let layout_start = systemDateTime.getTime(this.isNano)
    perftest.buildParagraph(this, content);
    perftest.layoutParagraph(this, width);
    let diff = systemDateTime.getTime(this.isNano) - layout_start
    this.layoutTime += diff;
  }

  drawParagraph() {
    let draw_start = systemDateTime.getTime(this.isNano)
    perftest.drawParagraph(null, this, 0, 0);
    this.drawTime += systemDateTime.getTime(this.isNano) - draw_start
  }

  draw(context: DrawContext) {
    perftest.drawParagraph(context, this, 0, 0);
  }
}

export class TextNativeNewNodeController extends NodeController {
  private nodeWidth: number = 1250;
  private nodeHeight: number = 1000;
  private rootNode: FrameNode | null = null;
  private textRenderNode: TextNativeNewRenderNode = new TextNativeNewRenderNode();

  aboutToAppear(): void {
    this.textRenderNode.initLayoutContext();
    this.textRenderNode.buildNativeCanvas(this.nodeWidth, this.nodeHeight);
  }

  makeNode(uiContext: UIContext): FrameNode | null {
    this.rootNode = new FrameNode(uiContext);
    const rootRenderNode = this.rootNode.getRenderNode();
    if (rootRenderNode !== null) {
      this.textRenderNode.frame = {
        x: 0,
        y: 0,
        width: this.nodeWidth,
        height: this.nodeHeight
      }
      rootRenderNode.appendChild(this.textRenderNode);
    }
    return this.rootNode;
  }

  startTest(testCases: Array<string>) {
    if (this.textRenderNode == undefined) {
      return
    }

    let testCount = testCases.length;
    this.textRenderNode.initPerf()
    let charCount = 0;
    for (let k = 0; k < testCount; k++) {
      let content = testCases[k]
      charCount += content.length
      this.textRenderNode.layoutContent(content, this.nodeWidth)
      this.textRenderNode.drawParagraph()
    }
    let avgLayoutCost = this.textRenderNode.layoutTime * 1000 / testCount / charCount
    let avgDrawCost = this.textRenderNode.drawTime * 1000 / testCount / charCount
    console.info(`NativeTextNew \tLayoutCost: ${avgLayoutCost}\t\tDrawCost:${avgDrawCost}`)
    this.textRenderNode.invalidate();
  }
}
