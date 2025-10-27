// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { TextraNodeController } from './TextraRenderNode';
import { TextRenderNode, TextNodeController } from './TextRenderNode'
import { resourceManager } from '@kit.LocalizationKit';
import util from '@ohos.util';
import Decimal from '@arkts.math.Decimal';
import { TextNativeNodeController } from './TextNativeRenderNode';
import { TextNativeNewNodeController } from './TextNativeNewRenderNode';
import { BaseRenderNode, RunStyle, RUN_LENGTH, TestCase } from './BaseRenderNode';

@Entry
@Component
struct Index {
  @State message: string = 'StartTest';
  fullContent: string = "";
  textNodeController = new TextNodeController();
  textNativeNodeController = new TextNativeNodeController();
  textraNodeController = new TextraNodeController();
  textNativeNewNodeController = new TextNativeNewNodeController();

  build() {
    Column() {
      NodeContainer(this.textNativeNewNodeController)
        .height('30%')
        .width('100%')
        .borderWidth(1).borderColor('#FF00FF00')
      NodeContainer(this.textraNodeController)
        .height('30%')
        .width('100%')
        .borderWidth(1).borderColor('#FF0000FF')
      NodeContainer(this.textNativeNodeController)
        .height('30%')
        .width('100%')
        .borderWidth(1).borderColor('#FFFF0000')
      Text(this.message)
        .id('Start')
        .fontSize($r('app.float.page_text_font_size'))
        .fontWeight(FontWeight.Bold)
        .alignRules({
          center: { anchor: '__container__', align: VerticalAlign.Center },
          middle: { anchor: '__container__', align: HorizontalAlign.Center }
        })
        .onClick(() => {
          let testCase = this.constructTestCases()
          this.startTest(this.textraNodeController.renderNode, testCase, "Textra")
          this.startTest(this.textNativeNewNodeController.renderNode, testCase, "NativeNewText")
          this.startTest(this.textNativeNodeController.renderNode, testCase, "NativeText")
        })
    }
    .height('100%')
    .width('100%')
  }

  aboutToAppear(): void {
    let textDecoder = new util.TextDecoder();
    let context = this.getUIContext().getHostContext();
    let rawBuffer = context?.resourceManager.getRawFileContentSync('hlm.txt')
    let content = textDecoder.decodeToString(rawBuffer)
    for (let k = 0; k < content.length; k++) {
      if (content[k] != '\n' && content[k] != '\r') {
        this.fullContent += content[k];
      }
    }
  }

  constructTestCases(): Array<TestCase> {
    let testCount = 1
    let testLength = 1
    let testCase: Array<TestCase> = new Array<TestCase>()
    let testContent = this.fullContent

    for (let k = 0; k < testCount; k++) {
      let one_case = new TestCase()
      let start = Decimal.random().mul(testContent.length - testLength).floor().toNumber()
      one_case.text = "  \t\tab"// testContent.substring(start, start + testLength);
      let style_start = 0
      while (style_start < one_case.text.length) {
        let run_style = new RunStyle()
        run_style.font_size = 24 + Decimal.random().mul(24).floor().toNumber();
        run_style.color = 0xFF000000 + Decimal.random().mul(0x00FFFFFF).floor().toNumber();
        one_case.styles.push(run_style)
        style_start += RUN_LENGTH;
      }
      testCase.push(one_case)
    }
    return testCase
  }

  startTest(node:BaseRenderNode, testCases: Array<TestCase>, tag:string) {
    let testCount = testCases.length;
    node.initPerf()
    for (let k = 0; k < testCount; k++) {
      let content = testCases[k]
      node.layoutContent(content, node.frame.width);
      node.drawParagraph()
    }
    node.printPerfResult(tag)
    node.invalidate();
  }
}


