#!/usr/bin/env python3
"""Package the existing Android C API demo Activity without Gradle dependencies.

The normal app includes this Activity too. This focused runner uses the same
Activity, JNI translation unit and Java SDK sources, plus a caller-built core.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[2]

def run(command, **kwargs):
    subprocess.run([str(arg) for arg in command], check=True, **kwargs)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--ndk', type=Path, required=True)
    parser.add_argument('--core-library', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--platform', default='android-35')
    parser.add_argument('--build-tools', default='35.0.0')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    tools = args.sdk / 'build-tools' / args.build_tools
    android = args.sdk / 'platforms' / args.platform / 'android.jar'
    jdk = Path(shutil.which('javac')).resolve().parents[1]
    env = dict(os.environ, JAVA_HOME=str(jdk))
    toolchain = args.ndk / 'toolchains/llvm/prebuilt/darwin-x86_64'
    host_library = output / 'libandroid_test.so'
    run([toolchain / 'bin/aarch64-linux-android24-clang++', '-std=c++17',
         '-fPIC', '-shared', '-fno-exceptions', '-I'+str(ROOT/'public'), '-I'+str(ROOT),
         '-I'+str(ROOT/'src/ports'), '-I'+str(ROOT/'src/textlayout'),
         '-I'+str(ROOT/'third_party/icu/source/common'),
         ROOT/'demos/android/app/src/main/jni/capi_demo.cc',
         args.core_library.resolve(), '-llog', '-Wl,-z,max-page-size=16384',
         '-o', host_library])
    sources = list((ROOT/'platform/android/lynxtextra/src/main/java').rglob('*.java'))
    sources += [ROOT/'demos/android/app/src/main/java/com/lynx/textra/demo/CapiDemoActivity.java',
                ROOT/'test/capi/android/com/lynx/textra/demo/CapiFontArchitectureTest.java']
    classes = output / 'classes'
    if classes.exists():
        shutil.rmtree(classes)
    classes.mkdir()
    run([jdk/'bin/javac', '-source', '8', '-target', '8', '-classpath', android,
         '-d', classes, *sources], env=env)
    jar = output / 'classes.jar'
    with zipfile.ZipFile(jar, 'w') as archive:
        for file in classes.rglob('*.class'):
            archive.write(file, file.relative_to(classes))
    dex = output / 'dex'
    dex.mkdir(exist_ok=True)
    run([tools/'d8', '--min-api', '24', '--lib', android, '--output', dex, jar], env=env)
    manifest = output / 'AndroidManifest.xml'
    manifest.write_text('''<manifest xmlns:android="http://schemas.android.com/apk/res/android"
package="com.lynx.textra.capidemo" android:versionCode="1" android:versionName="1.0">
<uses-sdk android:minSdkVersion="24" android:targetSdkVersion="35"/>
<application android:label="Textra C API comparison" android:debuggable="true"
android:theme="@android:style/Theme.Material.Light.NoActionBar">
<activity android:name="com.lynx.textra.demo.CapiDemoActivity" android:exported="true">
<intent-filter><action android:name="android.intent.action.MAIN"/>
<category android:name="android.intent.category.LAUNCHER"/></intent-filter>
</activity></application></manifest>''')
    unsigned = output / 'unsigned.apk'
    run([tools/'aapt2', 'link', '-I', android, '--manifest', manifest, '-o', unsigned])
    with zipfile.ZipFile(unsigned, 'a') as archive:
        archive.write(dex/'classes.dex', 'classes.dex')
        for file, name in [(args.core_library, 'liblynxtextra.so'),
                           (host_library, 'libandroid_test.so'),
                           (toolchain/'sysroot/usr/lib/aarch64-linux-android/libc++_shared.so', 'libc++_shared.so')]:
            archive.write(file, 'lib/arm64-v8a/'+name)
    aligned = output / 'aligned.apk'
    run([tools/'zipalign', '-f', '-P', '16', '4', unsigned, aligned])
    key = output / 'demo.keystore'
    if not key.exists():
        run([jdk/'bin/keytool', '-genkeypair', '-keystore', key, '-storepass', 'android',
             '-keypass', 'android', '-alias', 'demo', '-dname', 'CN=Textra Demo',
             '-keyalg', 'RSA', '-validity', '3650'], env=env)
    apk = output / 'textra-capi-demo.apk'
    run([tools/'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android',
         '--out', apk, aligned], env=env)
    print(apk)

if __name__ == '__main__':
    main()
