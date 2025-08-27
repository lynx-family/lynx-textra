// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { TTTextView, ParagraphStyle, RunStyle, Paragraph, RunDelegate } from '@lynx/lynxtextra'
import { createNode } from './Button'

@Entry
@Component
struct Index {
  private text_view = new TTTextView();

  build() {
    Column() {
      Row() {
        ContentSlot(this.text_view)
      }.height('90%')

      Button('Ok', { type: ButtonType.Normal, stateEffect: true })
        .borderRadius(8)
        .backgroundColor(0x317aff)
        .width(90)
        .height(30)
        .onClick(() => {
          console.info('Button onClick')
          let btn_node = createNode(this.getUIContext());
          let paragraph = new Paragraph().setParagraphStyle(
            new ParagraphStyle().setAlignment('left')
          )
            .addTextRun('😄Кириллица👿hello😄هذا نص عربي لا معنى له ، فقط👿', new RunStyle().appendFontFamily('sans-serif').setTextSize(24))
            .addTextRun('😄👿', new RunStyle().setTextSize(24).setTextColor(0xff00ff00))
            .addRunDelegate(new RunDelegate(btn_node.getFrameNode()))

          this.text_view.appendParagraph(paragraph)
        })
    }
  }
}