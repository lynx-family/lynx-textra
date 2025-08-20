// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// common.ets
import { BuilderNode, UIContext } from '@kit.ArkUI'

export class Params {
  text: string = "this is a text"

  constructor(text: string) {
    this.text = text;
  }
}

let buttonNode: BuilderNode<[Params]> | null = null;

@Builder
export function buttonBuilder(params: Params) {
  Column() {
    Button(params.text)
      .fontSize(12)
      .borderRadius(8)
      .borderWidth(2)
      .backgroundColor(Color.Orange)
  }
}

export function createNode(uiContext: UIContext) {
  buttonNode = new BuilderNode(uiContext);
  buttonNode.build(wrapBuilder<[Params]>(buttonBuilder), new Params("This is a Button"));
  return buttonNode;
}