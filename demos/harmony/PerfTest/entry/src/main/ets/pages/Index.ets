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
      NodeContainer(this.textNativeNodeController)
        .height('30%')
        .width('100%')
        .borderWidth(1).borderColor('#FF0000FF')
      NodeContainer(this.textraNodeController)
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
          // this.textNodeController.startTest(testCase)
          this.textraNodeController.startTest(testCase)
          this.textNativeNodeController.startTest(testCase)
          this.textNativeNewNodeController.startTest(testCase)
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

  constructTestCases(): Array<string> {
    let testCount = 500
    let testLength = 500
    let testCase: Array<string> = new Array<string>()
    let testContent = this.fullContent

    for (let k = 0; k < testCount; k++) {
      let start = Decimal.random().mul(testContent.length - testLength).floor().toNumber()
      let substr = testContent.substring(start, start + testLength);
      testCase.push(substr)
    }
    return testCase
  }
}


