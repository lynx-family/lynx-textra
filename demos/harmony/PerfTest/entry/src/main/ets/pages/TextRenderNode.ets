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

export class TextRenderNode extends RenderNode {
  width: number = 200
  content: string | null = null
  layoutTime: number = 0;
  drawTime: number = 0;
  isNano: boolean = true;
  paragraph: Paragraph | null = null

  constructor(w: number) {
    super();
    this.width = w
  }

  updateContent(content: string) {
    this.content = content
  }

  initPerf() {
    this.layoutTime = 0
    this.drawTime = 0
  }

  layoutContent(content: string): Paragraph {
    let myTextStyle: text.TextStyle = {
      // 文本颜色
      color: {
        alpha: 255,
        red: 255,
        green: 0,
        blue: 0
      },
      // 文本大小
      fontSize: 24
    };

    let myParagraphStyle: text.ParagraphStyle = {
      textStyle: myTextStyle,
    };
    let fontCollection = text.FontCollection.getGlobalInstance();
    let paragraphBuilder = new text.ParagraphBuilder(myParagraphStyle, fontCollection);
    // 更新文本样式
    paragraphBuilder.pushStyle(myTextStyle);
    // 添加文本
    paragraphBuilder.addText(content);

    // 生成段落
    let paragraph = paragraphBuilder.build();
    // 布局
    let layout_start = systemDateTime.getTime(this.isNano)
    paragraph.layoutSync(this.width);
    let diff = systemDateTime.getTime(this.isNano) - layout_start
    this.layoutTime += diff

    this.paragraph = paragraph
    return paragraph;
  }

  drawParagraph(canvas: drawing.Canvas, para: Paragraph) {
    // 绘制文本
    let draw_start = systemDateTime.getTime(this.isNano)
    para.paint(canvas, 0, 0);
    this.drawTime += systemDateTime.getTime(this.isNano) - draw_start
    console.info(`RenderNode draw width = ${this.width}`);
  }

  draw(context: DrawContext) {
    // 获取canvas对象
    const canvas = context.canvas;
    if (this.paragraph == null) return

    canvas.drawRect(0, 0, 200, 200)
    this.paragraph.paint(canvas, 0, 0);
    console.info(`RenderNode draw width = ${this.width}`);
  }
}

export class TextNodeController extends NodeController {
  private nodeWidth: number = 1250;
  private nodeHeight: number = 1000;
  private rootNode: FrameNode | null = null;
  private pixelMap: image.PixelMap | null = null;
  private textRenderNode: TextRenderNode = new TextRenderNode(this.nodeWidth);

  aboutToAppear(): void {
    const color: ArrayBuffer =
      new ArrayBuffer(this.nodeWidth * this.nodeHeight * 4);
    let opts: image.InitializationOptions =
      { editable: true, pixelFormat: 3, size: { height: this.nodeHeight, width: this.nodeWidth } };
    this.pixelMap = image.createPixelMapSync(color, opts);

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
    let canvas: drawing.Canvas | null = null;
    if (this.pixelMap) {
      canvas = new drawing.Canvas(this.pixelMap)
    }
    for (let k = 0; k < testCount; k++) {
      let content = testCases[k]
      charCount += content.length
      let para = this.textRenderNode.layoutContent(content)
      if (canvas) {
        this.textRenderNode.drawParagraph(canvas, para)
      }
    }
    let avgLayoutCost = this.textRenderNode.layoutTime * 1000 / testCount / charCount
    let avgDrawCost = this.textRenderNode.drawTime * 1000 / testCount / charCount
    console.info(`Text LayoutCost: ${avgLayoutCost}, DrawCost:${avgDrawCost}`)
    this.textRenderNode.invalidate();
  }
}
