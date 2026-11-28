# C API demo 和渲染对照

Android、iOS、Skity demo 增加了 C API 模式。公共示例 `textra_demo.h`
只依赖纯 C 头文件，演示字体集合、Style、Paragraph、布局和绘制的完整调用。
宿主通过各平台的独立原生回调创建 Canvas、FontManager 和 Typeface。

对照使用同一平台上的相同字体、24px 字号、颜色、320×200 画布及布局参数，
分别运行原 C++ 适配和 C API。8 组输入覆盖英文连字、中英文混排、换行、RTL、
粗体、斜体、多段文本及字体回退。检查原始像素、布局宽高、行数、非空输出，
并再次运行 C API 比较重复绘制结果。每组保存 C++、C API、重复 C API 三张图片
和 `results.json`。`verify_results.py` 会独立解码图片再次逐字节验证，生成 HTML 图库。

## Android

现有 `demos/android/app` 的菜单增加 **C API rendering comparison**，进入
`CapiDemoActivity`。也可以直接启动：

```sh
adb shell am start -n com.lynx.textra.demo/com.lynx.textra.demo.CapiDemoActivity
```

JNI 示例位于 `app/src/main/jni/capi_demo.cc`，编入原 `android_test` 库。
报告和 PNG 保存至 app 私有目录 `files/capi-comparison`。首次 C API 绘制先于
旧 `TTText.Initial`，验证独立 JNI 初始化；C++ 对照随后使用旧字体管理器及
JavaCanvasHelper 命令缓冲区。

可用 SDK 工具单独构建相同 Activity/JNI/Java SDK，无需 Gradle 或 AndroidX。
先构建启用 JavaShaper 的 Android arm64 核心库，再运行：

```sh
python3 test/capi/build_android_demo.py \
  --sdk "$ANDROID_HOME" --ndk "$ANDROID_NDK" \
  --core-library out/capi-demo-android/liblynxtextra.so \
  --output /tmp/textra-capi-android-demo
adb install -r /tmp/textra-capi-android-demo/textra-capi-demo.apk
adb shell am start -S -n com.lynx.textra.capidemo/com.lynx.textra.demo.CapiDemoActivity
adb exec-out run-as com.lynx.textra.capidemo tar -cf - -C files/capi-comparison . > /tmp/android-results.tar
mkdir -p /tmp/textra-demo-parity/android
tar -xf /tmp/android-results.tar -C /tmp/textra-demo-parity/android
```

该独立打包脚本面向 macOS + Android arm64，默认 SDK platform 35/build-tools
35.0.0，需 Java、NDK、Android SDK。可通过参数指定版本。应用包名为
`com.lynx.textra.capidemo`；原 demo 包名为 `com.lynx.textra.demo`。

## iOS

打开 `demos/darwin/ios/TextlayoutDemo/TextlayoutDemo.xcworkspace`，选择
`TextlayoutDemo` scheme，在 Run Arguments 添加 `--textra-capi`。
启动时运行比较，并在页面显示 C API 结果；不带参数时使用原 demo 页面。
报告保存至 `Documents/capi-comparison`。

```sh
xcrun simctl launch booted com.lynx.textra.TextlayoutDemo --textra-capi
# 通过下面命令找到容器，将 Documents/capi-comparison 拷贝到结果目录的 ios 子目录。
xcrun simctl get_app_container booted com.lynx.textra.TextlayoutDemo data
```

示例初始化位于 `AppDelegate.mm`，显示入口位于 `ViewController.mm`。
原生回调实现只在 AppDelegate 的编译单元中包含一次。离屏画布显式配置为
UIKit 的向下 Y 轴；比较图片可以直接显示，无需额外翻转。

## Skity

现有 macOS GLFW demo 支持：

```sh
gl_app_demo --capi                       # C API 离屏绘制后在原 C++ 窗口展示
gl_app_demo --capi-compare /tmp/textra-demo-parity/skity  # 软件画布逐像素比较
```

同一比较入口也提供独立的 `skity_compare.cc`，无需 GLFW 窗口。链接启用
HarfBuzz 的核心库和宿主的 Skity 库，例如：

```sh
clang++ -std=c++17 -mmacosx-version-min=15.0 -I. -Ipublic \
  -Fthird_party/skity_release/apple/skity.xcframework/macos-arm64_x86_64 \
  test/capi/skity_compare.cc \
  -Lout/rendering-backend-skity -Lout/mac \
  -llynxtextra -framework skity -framework CoreGraphics -framework CoreText \
  -framework Foundation -framework AppKit -framework Metal -framework OpenGL \
  -framework ImageIO -framework CoreServices \
  -Wl,-rpath,"$PWD/out/mac" -o /tmp/textra-skity-demo-compare
(cd out/rendering-backend-skity && /tmp/textra-skity-demo-compare /tmp/textra-demo-parity/skity)
```

Skity 输出原始 RGBA PAM；下面的检查脚本同时将其转换成 PNG。

## 汇总结果

将三组结果放在同一目录下的 `android`、`ios`、`skity` 子目录：

```sh
python3 test/capi/verify_results.py /tmp/textra-demo-parity
# 查看 /tmp/textra-demo-parity/index.html 和 summary.json
```

本次实测：Android 15/API 35 arm64 模拟器、iOS 26.5 arm64 模拟器、macOS
Skity/HarfBuzz 软件画布，各 8/8 通过，C++ 与 C API 像素及布局结果完全一致，
重复渲染一致。对照按平台分别进行，不要求不同系统字体的跨平台像素相同。
这些用例验证基础文字链路；Skity GPU 路径、真实设备以及全部高级绘图能力
仍需要额外覆盖。

Android 另外执行 81 项接口检查，覆盖全部 27 个 Canvas 平台回调：已有能力
比较 Android Canvas 像素；原后端未实现的能力按 I* 接口检查无操作或默认值，
回调错误不通过 I* 接口上报；这些对象没有直接操作的 C 入口。暂不补齐字体枚举、文件/字符查询、字体数据、glyph 映射、图片和原生
Canvas 空实现。结果位于 `files/capi-comparison/interfaces/results.json`。
详见 [Android 支持范围](../../test/capi/README.md#android-checks-and-benchmarks)。

同一进程对照还验证了缓存隔离：ShapeKey 现在携带字体集合资源代际的身份，
防止同一 RenderingBackend 下的 C++ 类型字体与 C 回调字体互相命中缓存。
同一字体集合的副本仍可复用缓存；不同集合和策略变更分别隔离。

## Android shaping 性能对照

`android_shaping_benchmark.cc` 分别通过原 C++ JavaFontManager 与 C 回调
FontManager 驱动同一个 JavaShaper。它只编入测试核心库副本，不进入正式核心
或 C ABI。`build_android_shaping_benchmark.py` 复用 GN 构建的原始对象及编译/
链接参数，附加 JNI 测量入口，保持原库与原对象不变。

```sh
python3 test/capi/build_android_shaping_benchmark.py \
  --core-build out/capi-demo-android --output /tmp/textra-android-shaping
python3 test/capi/build_android_demo.py \
  --sdk "$ANDROID_HOME" --ndk "$ANDROID_NDK" \
  --core-library /tmp/textra-android-shaping/liblynxtextra.so \
  --output /tmp/textra-android-shaping/app
python3 test/capi/run_android_shaping_benchmark.py \
  --adb "$ANDROID_HOME/platform-tools/adb" \
  --apk /tmp/textra-android-shaping/app/textra-capi-demo.apk \
  --output /tmp/textra-android-shaping/results --rotations 0 2 4 --rounds 21
```

同一模拟器安装不同测试输出目录的 APK 时，需要使用相同签名密钥；可复用该
打包脚本之前生成的 `demo.keystore`。基准在 Java 工作线程执行，避免阻塞 UI。

测量版本 3：每个进程先以全部语料和取整策略预热至少 2 秒，再测量 5 种文本（英文、中文、256 字混排、阿拉伯文、emoji），24px，
分别关闭/开启 AndroidMetricsRound。每个组合预热 2048 次；每轮交替执行
C++/C API 共 512 对，奇偶轮及奇偶调用交换先后顺序。21 轮 × 3 个独立进程，语料顺序分别轮换 0/2/4 位。
报告记录 APK/核心库/dex 哈希与独立进程 PID，保留原始样本；汇总按语料、
策略、模式匹配，百分比采用配对轮次均值之比的中位数，而非两个独立
中位数相除。模拟器调度/运行时预热仍使绝对时间有波动，不当作真机结论。

四种边界分别报告：

- `platform_shape`：JavaShaper::OnShapeText，包含 UTF 转换、字体解析/回调、
  JNI 与 Paint.getTextWidths、结果分配及拷贝；不是单独的系统排版函数时间。
- `engine_cache_miss`：TTShaper::ShapeText，显式禁用 ShapeCache，确保每次
  实际塑形；本节结果以此为主。
- `engine_cache_hit`：同一输入已预热的 ShapeCache 命中，不执行实际 shaping。
- `first_shape_fresh_manager`：已启动/预热的进程内，新建字体管理器与 shaper
  后第一次塑形。管理器创建耗时另列 `*_setup`；这不是冷进程或冷系统启动。

结果销毁时间在 `*_cleanup` 单独报告；计时不包含布局和绘制。
输出校验比较 glyph 数、advance 总和及 ascent/descent。

以下为收回后端扩展功能前的历史测量，不代表本次支持范围调整后的性能。
字体适配层重构后，Android 15/API 35 arm64 模拟器、Release `-Oz`、无 LTO，
沿用以上版本 2 协议的三个独立进程结果：

| 输入 | AndroidMetricsRound 关闭：C API 相对 C++ | 开启：C API 相对 C++ | 开启后额外 shaping 时间 |
| --- | ---: | ---: | ---: |
| 英文短文本 | +6.3% | +1.7% | +0.031 µs |
| 中文短文本 | +1.9% | +1.7% | +0.037 µs |
| 256 字混排 | +0.4% | +0.6% | +0.061 µs |
| 阿拉伯文 | +1.7% | +1.3% | +0.039 µs |
| emoji 混排 | +2.0% | +1.6% | +0.034 µs |

所有 40 组输入/模式在三个进程中均通过输出校验。关闭取整的英文短文本仍有
6.3% 差异，不能据此宣称所有模式性能相同。模拟器结果不代表真机性能。

重构前，同协议下开启取整的差距分别为 25.3%、31.3%、8.7%、12.2%、27.9%。
原路径每次创建 rounded Face、重新查询 font_style；中文还重复查询 fallback
metrics。现在字体适配层缓存 Java peer、fallback metrics 和按中文类别区分的
取整字体变体，且两个入口都复用不可变变体。该对照反映两入口差距缩小，
不能把差距缩小直接解释成 C API 的绝对加速比例。首次字体匹配及 fallback
初始化仍有成本，见 `first_shape_fresh_manager` 和 setup 样本。
缓存命中不执行实际 shaping，不能作为塑形性能。

## Android 字体适配层回归

测试版 core/APK 使用上述 benchmark 构建脚本，运行：

```sh
adb shell am start -S \
  -n com.lynx.textra.capidemo/com.lynx.textra.demo.CapiDemoActivity \
  --ez font_architecture_test true
adb exec-out run-as com.lynx.textra.capidemo cat files/font-architecture/results.json
```

当前原生默认实现共 38 项检查：22 项对照原 Java/C++ shaping 路径与
C API 平台默认路径，包括不同字号、取整模式、缓存和原生字体导入；另 16 项
比较 24px / 24.5px 的取整布局度量和像素。报告与 PNG 位于
`files/font-architecture`。常规 demo 运行 8 组绘制与重复绘制对照。

系统端渲染直接复用原 JavaCanvasHelper、JavaFontManager 和 JavaTypeface。
已移除独立平台 C 回调与 CapiNativeBridge；不存在 Java-only C API 管理器或
独立解码入口。字形、度量、匹配和未实现方法均遵循原 C++ I* 行为。

## Native platform defaults (ABI 1.0)

Android/iOS/macOS/OHOS now use their original C++ I* implementations inside the
engine. Native demos initialize platform managers/fonts/drawers with raw SDK
handles; no platform C callback implementation or CapiNativeBridge is needed.
The old independent Android C callback/probe files have been removed. Android's
font architecture runner now compares the original native Java path and the
C API native-default path (including fractional/rounded metrics, caches and
native font import). Earlier C-callback performance/check counts above describe
historical implementations and do not apply to this architecture.

Only Skia, Skity and Skity+CoreText use external self-render callback examples;
LVGL now uses its built-in C++ adapters through the same native factories. Runtime validation on Android and
macOS is separate from iOS/OHOS SDK compilation checks.

Native platform implementation headers are private under `src/ports/native`.
Both C++ and C demo paths obtain their native I* instances through PlatformHelper;
Android C++ drawing uses StartPaint/EndPaint to replay the original Java command
buffer inside the engine. No demo includes private platform implementation headers.
The Android shaping benchmark is version 6: it compares the default C++ manager
factory with the options factory used by C layout initialization. It measures
shared native shaping, not public TextLayout initialization. Public C owner
initialization is exercised by the rendering demo. Historical setup timings
use older construction paths and are not directly comparable.

LVGL native integration is covered by `test/rendering_backend_lvgl_test.cc`:
it uses only public Textra headers and host LVGL SDK headers. The dedicated
`demos/lvgl` sample also obtains its manager/canvas from PlatformHelper. Native
LVGL configuration and borrowed-resource lifetimes are documented in the public
C API README; no host LVGL callback implementation is required.
