## Introduction

LynxTextra is a high-performance, cross-platform text layout engine. Its core is
implemented in C++ and wrapped with platform-specific bindings, making it easy
to integrate on iOS, Android, OpenHarmony, macOS, etc.

## Installation

```bash
ohpm install @lynx/lynxtextra
```

## How to use

You can use LynxTextra in your HarmonyOS project. add dependency in oh-package.json5 like this:

```json5
{
  "dependencies": {
    "@lynx/lynxtextra": "0.1.1",
  }
}
```

Then attach a `TTTextView` to a `ContentSlot`, and append a `Paragraph` to the `TTTextView`. You can add text with style to paragraph before it was append to `TTTextView`.

```ts
import { TTTextView, ParagraphStyle, RunStyle, Paragraph, RunDelegate } from '@lynx/lynxtextra'

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
        .onClick(() => {
          console.info('Button onClick')
          let paragraph = new Paragraph().setParagraphStyle(
            new ParagraphStyle().setAlignment('left')
          )
            .addTextRun('😄👿hello😄هذا نص عربي لا معنى له ، فقط👿', new RunStyle().setTextSize(24).setTextColor(0xffffff00))
            .addTextRun('😄中国Кириллица👿', new RunStyle().setTextSize(24).setTextColor(0xff00ff00))
          this.text_view.appendParagraph(paragraph)
        })
    }
  }
}
```
