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

export class TextraRenderNode extends RenderNode {
  content: string | null = null
  layoutTime: number = 0;
  drawTime: number = 0;
  isNano: boolean = true;
  private paragraph: Paragraph | null = null

  updateContent(content: string) {
    this.content = content
  }

  initPerf() {
    this.layoutTime = 0
    this.drawTime = 0
  }

  initLayoutContext() {
    perftest.initLayoutContext(this, 0);
  }

  buildNativeCanvas(width: number, height: number) {
    perftest.buildCanvas(this, width, height);
  }

  layoutContent(content: string, width: number) {
    perftest.buildParagraph(this, content);
    let layout_start = systemDateTime.getTime(this.isNano)
    perftest.layoutParagraph(this, width);
    let diff = systemDateTime.getTime(this.isNano) - layout_start
    this.layoutTime += diff;
  }

  drawParagraph() {
    let draw_start = systemDateTime.getTime(this.isNano)
    perftest.drawParagraph(null, this, 0, 0);
    this.drawTime += systemDateTime.getTime(this.isNano) - draw_start
  }

  // layoutContent(content: string, width: number): Paragraph {
  //   let para = new Paragraph;
  //   let runStyle = new RunStyle();
  //   runStyle.dpi = 1
  //   runStyle.text_size = 24
  //   runStyle.text_color = 0xffff0000
  //   para.addTextRun(content, runStyle)
  //   let layout_start = systemDateTime.getTime(this.isNano)
  //   this.tttextview.layoutParagraph(para, width)
  //   let diff = systemDateTime.getTime(this.isNano) - layout_start
  //   this.layoutTime += diff
  //   this.paragraph = para
  //   return para;
  // }
  //
  // drawParagraph(canvas: drawing.Canvas, para: Paragraph) {
  //   // 绘制文本
  //   let draw_start = systemDateTime.getTime(this.isNano)
  //   // this.tttextview.drawContent(canvas, 0, 0, para)
  //   this.drawTime += systemDateTime.getTime(this.isNano) - draw_start
  // }

  draw(context: DrawContext) {
    // 绘制文本
    let draw_start = systemDateTime.getTime(true)
    perftest.drawParagraph(context, this, 0, 0);
    this.drawTime += systemDateTime.getTime(true) - draw_start
  }
}

export class TextraNodeController extends NodeController {
  private nodeWidth: number = 1250;
  private nodeHeight: number = 1000;
  private rootNode: FrameNode | null = null;
  private pixelMap: image.PixelMap | null = null;
  private textRenderNode: TextraRenderNode = new TextraRenderNode();

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

  updateContent(text: string) {
    this.textRenderNode?.updateContent(text)
    this.textRenderNode?.invalidate()
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
    console.info(`Textra \t\t\tLayoutCost: ${avgLayoutCost}\t\tDrawCost:${avgDrawCost}`)
    this.textRenderNode.invalidate()
  }
}
