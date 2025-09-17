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
import { BaseRenderNode, TestCase } from './BaseRenderNode';

export class TextRenderNode extends BaseRenderNode {
  content: string | null = null
  paragraph: Paragraph | null = null

  updateContent(content: string) {
    this.content = content
  }

  layoutContent(content: TestCase, width: number): Paragraph {
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
    paragraphBuilder.addText(content.text);

    // 生成段落
    let paragraph = paragraphBuilder.build();
    // 布局
    let layout_start = systemDateTime.getTime(this.isNano)
    paragraph.layoutSync(width);
    let diff = systemDateTime.getTime(this.isNano) - layout_start
    this.layoutTime += diff

    this.paragraph = paragraph
    return paragraph;
  }

  // drawParagraph(canvas: drawing.Canvas, para: Paragraph) {
  //   // 绘制文本
  //   let draw_start = systemDateTime.getTime(this.isNano)
  //   para.paint(canvas, 0, 0);
  //   this.drawTime += systemDateTime.getTime(this.isNano) - draw_start
  // }

  draw(context: DrawContext) {
    // 获取canvas对象
    const canvas = context.canvas;
    if (this.paragraph == null) {
      return
    }

    canvas.drawRect(0, 0, 200, 200)
    this.paragraph.paint(canvas, 0, 0);
  }
}

export class TextNodeController extends NodeController {
  private nodeWidth: number = 1250;
  private nodeHeight: number = 1000;
  private rootNode: FrameNode | null = null;
  private pixelMap: image.PixelMap | null = null;
  private textRenderNode: TextRenderNode = new TextRenderNode();

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
}
