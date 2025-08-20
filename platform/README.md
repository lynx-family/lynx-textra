# Libraries

## iOS Library

The iOS library is provided as a Cocoapods podspec. You can declare dependency on it in your iOS project Podfile.

    ```ruby
    pod 'LynxTextra', :path => '../../../../'
    ```

Then run `pod install` to install the library. See [TextlayoutDemo/Podfile](../demos/darwin/ios/TextlayoutDemo/Podfile) as an example.

## Android Library

To build the AAR, run:

    ```bash
    source tools/envsetup.sh
    tools/hab sync . -f
    pushd platform/android
    ./gradlew assembleRelease
    popd
    ```

## OpenHarmony Library

TODO: Support building the OpenHarmony library.
