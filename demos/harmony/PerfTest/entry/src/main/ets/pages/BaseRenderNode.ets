// Copyright 2022 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
import { RenderNode } from "@kit.ArkUI";
import { hilog } from '@kit.PerformanceAnalysisKit';
import perftest from "libperftest.so";
import systemDateTime from "@ohos.systemDateTime";

export let RUN_LENGTH: number = 5000;

export class RunStyle {
  font_size: number = 0
  color: number = 0
}

export class TestCase {
  text: string = ""
  styles: Array<RunStyle> = []
}

export class BaseRenderNode extends RenderNode {
  layoutTime: number = 0;
  drawTime: number = 0;
  charCount: number = 0;
  isNano: boolean = false;

  initPerf() {
    this.layoutTime = 0
    this.drawTime = 0
    this.charCount = 0
  }

  buildNativeCanvas(width: number, height: number) {
    perftest.buildCanvas(this, width, height);
  }

  layoutContent(content: TestCase, width: number) {
    this.charCount += content.text.length;
    perftest.buildParagraph(this);
    let idx = 0
    while (idx * RUN_LENGTH < content.text.length) {
      let sub_str = content.text.substring(idx*RUN_LENGTH, Math.min((idx+1)*RUN_LENGTH, content.text.length))
      let style = content.styles[idx]
      perftest.appendContent(this, sub_str, style.font_size, style.color);
      idx++;
    }
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

  draw(context: DrawContext) {
    let draw_start = systemDateTime.getTime(true)
    perftest.drawParagraph(context, this, 0, 0);
    this.drawTime += systemDateTime.getTime(true) - draw_start
  }

  printPerfResult(test_tag: string) {
    let avgLayoutCost = this.layoutTime * 1000 / this.charCount
    let avgDrawCost = this.drawTime * 1000 / this.charCount
    hilog.info(0, "PerfResult", "%{public}s LayoutCost:%{public}s, DrawCost:%{public}s", test_tag.padEnd(15),
      avgLayoutCost.toPrecision(4), avgDrawCost.toPrecision(4))
  }
}