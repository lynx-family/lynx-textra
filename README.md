## Introduction

LynxTextra is a high-performance, cross-platform text layout engine. Its core is
implemented in C++ and wrapped with platform-specific bindings, making it easy
to integrate on iOS, Android, OpenHarmony, macOS, etc.

It supports a wide range of text layout features, including:

| Category | Attribute | Example |
|----------|-----------|---------|
| **Paragraph Style** | horizontal_alignment | ![Horizontal Alignment](examples/demo_images/paragraphstyle_horizontal_alignment.png) |
| | vertical_alignment | ![Vertical Alignment](examples/demo_images/paragraphstyle_vertical_alignment.png) |
| | indent | ![Indent](examples/demo_images/paragraphstyle_indent.png) |
| | paragraph spacing | *Note: space_before and space_after currently not used by the layout engine.* |
| | line spacing | ![Line Spacing](examples/demo_images/paragraphstyle_spacing.png) |
| | write_direction | ![Write Direction](examples/demo_images/paragraphstyle_write_direction.png) |
| | ellipsis | ![Ellipsis](examples/demo_images/paragraphstyle_ellipsis.png) |
| | max_lines | ![Max Lines](examples/demo_images/paragraphstyle_max_lines.png) |
| | line_height_override | ![Line Height Override](examples/demo_images/paragraphstyle_line_height_override.png) |
| | half_leading | ![Half Leading](examples/demo_images/paragraphstyle_half_leading.png) |
| | enable_text_bounds | ![Enable Text Bounds](examples/demo_images/paragraphstyle_enable_text_bounds.png) |
| | overflow_wrap | ![Overflow Wrap](examples/demo_images/paragraphstyle_overflow_wrap.png) |
| | line_break_strategy | ![Line Break Strategy](examples/demo_images/paragraphstyle_allow_break_around_punctuation.png) |
| **Text Style** | font_descriptor | ![Font Descriptor](examples/demo_images/style_font_descriptor.png) |
| | text_size | ![Text Size](examples/demo_images/style_text_size.png) |
| | foreground_color | ![Foreground Color](examples/demo_images/style_foreground_color.png) |
| | background_color | ![Background Color](examples/demo_images/style_background_color.png) |
| | decoration_color<br>decoration_type<br>decoration_style<br>decoration_thickness_multiplier | ![Decoration](examples/demo_images/style_decoration.png) |
| | vertical_alignment | ![Vertical Alignment](examples/demo_images/style_vertical_alignment.png) |
| | letter_spacing | ![Word & Letter Spacing](examples/demo_images/style_word_letter_spacing.png) |
| | word_break | ![Word Break](examples/demo_images/style_word_break.png) |
| | baseline_offset | ![Baseline Offset](examples/demo_images/style_baseline_offset.png) |
| | text_shadow_list | ![Text Shadow](examples/demo_images/style_text_shadow.png) |
| **Other Configuration** | last_line_can_overflow | ![Last Line Overflow](examples/demo_images/configuration_last_line_can_overflow.png) |
| | skip_spacing_before_first_line | ![Skip Spacing](examples/demo_images/configuration_skip_spacing_before_first_line.png) |

## Getting Started

See [demos/README.md](demos/README.md) for details.

## Building

See [platform/README.md](platform/README.md) for details.

## Testing

See [test/README.md](test/README.md) for details.

## How to Contribute

### [Code of Conduct][coc]

We are devoted to ensuring a positive, inclusive, and safe environment for all contributors. Please find our [Code of Conduct][coc] for detailed information.

[coc]: CODE_OF_CONDUCT.md

### [Contributing Guide][contributing]

We welcome you to join and become a member of Lynx Authors. It's people like you that make this project great.

Please refer to our [contributing guide][contributing] for details.

[contributing]: CONTRIBUTING.md

## Discussions

Bugs and feature requests are filed in [Github Issues](https://github.com/lynx-family/lynx-textra/issues)

Large discussions and proposals are discussed in [Github Discussions](https://github.com/lynx-family/lynx-textra/discussions)

You are always very welcome to join the [Discord Channel](https://discord.gg/mXk7jqdDXk) and meet others who are enthusiastic about making LynxTextra great.

## Credits

LynxTextra incorporates or draws inspiration from the design and implementation of the following open-source libraries. We gratefully acknowledge the work of the developers and the open-source communities behind these projects:
- [apache/openoffice](https://github.com/apache/openoffice)
- [freetype2](https://github.com/freetype/freetype)
- [glfw](https://github.com/glfw/glfw)
- [google/skia](https://github.com/google/skia)
- [googletest](https://github.com/google/googletest)
- [harfbuzz](https://github.com/harfbuzz/harfbuzz)
- [icu](https://github.com/unicode-org/icu)
- [libcxx](https://github.com/llvm/llvm-project)
- [libpng](https://www.libpng.org/pub/png/libpng.html)
- [libunibreak](https://github.com/adah1972/libunibreak)
- [rapidjson](https://github.com/Tencent/rapidjson)
- [zlib](https://github.com/madler/zlib)

## [License][license]

LynxTextra is licensed under the Apache License 2.0. See [LICENSE][license] for details.

[license]: LICENSE