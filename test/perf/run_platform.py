#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
"""Build/deploy paired hosts using the same environments as Lynx CI."""
import argparse
import json
import os
import platform
import plistlib
import re
import shutil
import subprocess
import sys
import uuid
import zipfile
from pathlib import Path

from gate import benchmark_digest, digest, run
from fetch_fonts import fetch
from platform_build import HERE, artifacts, build, checkout, cleanup, execute, write_metadata

VERSIONS = ('baseline', 'candidate')


def ios_simulator(requested):
    devices = json.loads(execute(['xcrun','simctl','list','devices','available','--json']))['devices']
    if requested:
        if not any(d['udid']==requested for entries in devices.values() for d in entries):
            raise ValueError('requested iOS simulator is unavailable')
        return requested, False
    runtimes = json.loads(execute(['xcrun','simctl','list','runtimes','--json']))['runtimes']
    available = sorted((r for r in runtimes if r.get('isAvailable') and r['name'].startswith('iOS')),
                       key=lambda r:tuple(map(int,r['version'].split('.'))), reverse=True)
    for runtime in available:
        phones = [d for d in devices.get(runtime['identifier'],[]) if 'iPhone' in d['name']]
        if phones and phones[0].get('deviceTypeIdentifier'):
            device = execute(['xcrun','simctl','create','Textra Perf '+uuid.uuid4().hex[:8],
                              phones[0]['deviceTypeIdentifier'],runtime['identifier']])
            return device, True
    raise ValueError('no available iOS simulator runtime/device type')


def ios_app(executable, metadata, package, destination):
    destination.mkdir()
    shutil.copy2(executable,destination/'perf_host')
    shutil.copy2(metadata,destination/'metadata.json')
    info = dict(CFBundleIdentifier=package,CFBundleExecutable='perf_host',
                CFBundleName='TextraPerf',CFBundlePackageType='APPL',
                CFBundleVersion='1',CFBundleShortVersionString='1.0',
                MinimumOSVersion='13.0',LSRequiresIPhoneOS=True,
                UILaunchScreen={},UIDeviceFamily=[1,2],
                UIApplicationSceneManifest=dict(UIApplicationSupportsMultipleScenes=False,
                    UISceneConfigurations={'UIWindowSceneSessionRoleApplication': [dict(
                        UISceneConfigurationName='Textra Perf',
                        UISceneDelegateClassName='TextraPerfSceneDelegate')]}))
    (destination/'Info.plist').write_bytes(plistlib.dumps(info))
    subprocess.run(['codesign','--force','--sign','-',str(destination)],check=True)
    archive = destination.with_suffix('.zip')
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as out:
        for path in destination.rglob('*'):
            if path.is_file(): out.write(path,str(path.relative_to(destination.parent)))
    return archive


def android_tools():
    sdk = Path(os.environ.get('ANDROID_HOME',os.environ.get('ANDROID_SDK_ROOT','')))
    if not sdk.is_dir(): raise ValueError('ANDROID_HOME is required')
    platforms = sorted((p for p in (sdk/'platforms').glob('android-*') if (p/'android.jar').exists()),
                       key=lambda p:tuple(int(x) for x in re.findall(r'\d+',p.name)))
    tools = sorted((sdk/'build-tools').iterdir(),key=lambda p:tuple(int(x) for x in re.findall(r'\d+',p.name)))
    ndks = sorted((sdk/'ndk').iterdir(),key=lambda p:tuple(int(x) for x in re.findall(r'\d+',p.name)))
    if not platforms or not tools or not ndks: raise ValueError('Android SDK/build-tools/NDK not installed')
    # Match the NDK revision used by this repository when available.
    ndk = next((p for p in ndks if p.name=='21.1.6352462'),ndks[-1])
    os.environ['ANDROID_NDK'] = str(ndk)
    return sdk,next((p for p in platforms if p.name=='android-35'),platforms[-1]),next((p for p in tools if p.name=='35.0.0'),tools[-1]),ndk


def android_apk(tree, native, metadata, package, output, abi, tools):
    sdk,android_platform,build_tools,ndk = tools
    directory = output/(package+'.apk-build')
    directory.mkdir()
    manifest = directory/'AndroidManifest.xml'
    manifest.write_text('''<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="'''+package+'''">
<uses-sdk android:minSdkVersion="28" android:targetSdkVersion="28"/>
<application android:label="TextraPerf" android:debuggable="true" android:extractNativeLibs="true">
<activity android:name="com.lynx.textra.perf.PerfActivity" android:exported="true" android:launchMode="singleTask"/>
</application></manifest>''')
    classes,dex = directory/'classes',directory/'dex'
    classes.mkdir();dex.mkdir()
    java = list((tree/'platform/android/lynxtextra/src/main/java').rglob('*.java'))
    subprocess.run(['javac','-source','8','-target','8','-encoding','UTF-8','-classpath',
                    str(android_platform/'android.jar'),'-d',str(classes)]+
                   [str(p) for p in java]+[str(HERE/'hosts/PerfActivity.java')],check=True)
    jar = directory/'classes.jar'
    subprocess.run(['jar','cf',str(jar),'-C',str(classes),'.'],check=True)
    subprocess.run([str(build_tools/'d8'),'--lib',str(android_platform/'android.jar'),
                    '--min-api','28','--output',str(dex),str(jar)],check=True)
    assets = directory/'assets';assets.mkdir();shutil.copy2(metadata,assets/'metadata.json')
    unsigned = directory/'unsigned.apk'
    subprocess.run([str(build_tools/'aapt'),'package','-f','-M',str(manifest),'-I',
                    str(android_platform/'android.jar'),'-A',str(assets),'-F',str(unsigned)],check=True)
    triplet = {'x86':'i686-linux-android','x86_64':'x86_64-linux-android',
               'arm64-v8a':'aarch64-linux-android','armeabi-v7a':'arm-linux-androideabi'}[abi]
    runtime = next((ndk/'toolchains/llvm/prebuilt').glob('*/sysroot/usr/lib/'+triplet+'/libc++_shared.so'),None)
    if runtime is None:
        runtime = ndk/'sources/cxx-stl/llvm-libc++/libs'/abi/'libc++_shared.so'
    with zipfile.ZipFile(unsigned,'a') as archive:
        archive.write(dex/'classes.dex','classes.dex')
        archive.write(native,'lib/'+abi+'/libperf_host.so')
        archive.write(runtime,'lib/'+abi+'/libc++_shared.so')
    aligned = directory/'aligned.apk'
    subprocess.run([str(build_tools/'zipalign'),'-f','4',str(unsigned),str(aligned)],check=True)
    key = output/'debug.keystore'
    if not key.exists():
        subprocess.run(['keytool','-genkeypair','-keystore',str(key),'-storepass','android',
                        '-keypass','android','-alias','androiddebugkey','-keyalg','RSA',
                        '-dname','CN=Textra Performance','-validity','30'],check=True)
    apk = output/(package+'.apk')
    subprocess.run([str(build_tools/'apksigner'),'sign','--ks',str(key),
                    '--ks-pass','pass:android','--out',str(apk),str(aligned)],check=True)
    return apk


def windows_skity(source, output):
    root = output/'skity'
    cmake = shutil.which('cmake')
    if not cmake: raise ValueError('system CMake with VS 2022 support is required')
    subprocess.run([cmake,'-S',str(source/'third_party/skity'),'-B',str(root),
        '-G','Visual Studio 17 2022','-A','x64','-DCMAKE_BUILD_TYPE=Release','-DSKITY_GL_BACKEND=OFF',
        '-DSKITY_MTL_BACKEND=OFF','-DSKITY_CODEC_MODULE=OFF','-DSKITY_IO_MODULE=OFF'],check=True)
    subprocess.run([cmake,'--build',str(root),'--config','Release','--target','skity'],check=True)
    library = next(root.rglob('skity.lib'))
    dll = next(root.rglob('skity.dll'))
    shutil.copy2(dll,output/'skity.dll')
    os.environ['TEXTRA_PERF_SKITY_LIBRARY'] = library.as_posix()
    fonts = [source/f for f in ['fonts/Inter/Inter-VariableFont_opsz,wght.ttf',
        'fonts/Noto_Sans_SC/NotoSansSC-VariableFont_wght.ttf',
        'fonts/Noto_Kufi_Arabic/NotoKufiArabic-VariableFont_wght.ttf',
        'fonts/Noto_Sans_Thai/NotoSansThai-VariableFont_wdth,wght.ttf',
        'fonts/Noto_Color_Emoji/NotoColorEmoji-Regular.ttf',
        'fonts/Noto_Sans_Hebrew/NotoSansHebrew-VariableFont_wdth,wght.ttf']]
    fonts += fetch(output/'fonts')
    for font in fonts:
        if not font.is_file(): raise ValueError('fixed Windows font unavailable: '+str(font))
    return [output/'skity.dll',library],fonts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform',choices=['ios','android','windows','harmony'],required=True)
    parser.add_argument('--baseline',required=True)
    parser.add_argument('--candidate',required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--source',type=Path,default=HERE.parents[1])
    parser.add_argument('--device',help='optional existing simulator; CI auto-selects/creates one')
    args = parser.parse_args()
    source,output = args.source.resolve(),args.output.resolve()
    if not all(re.fullmatch('[0-9a-f]{40}',x) for x in [args.baseline,args.candidate]):
        parser.error('full commit SHAs are required')
    if execute(['git','-C',source,'rev-parse','HEAD'])!=args.candidate:
        parser.error('candidate does not match checkout')
    if execute(['git','-C',source,'diff','HEAD','--','src','public','platform','config.gni','BUILD.gn']):
        parser.error('candidate engine has uncommitted changes')
    output.mkdir(parents=True,exist_ok=False)
    trees = []
    namespace = 'com.lynx.textra.perf.r'+uuid.uuid4().hex[:8]
    packages = {v:namespace+'.'+v for v in VERSIONS}
    device,owned,adb = None,False,'adb'
    config = dict(schema=1,platform=args.platform,backend=args.platform,fonts=[],
                  benchmark_sha256=benchmark_digest(HERE),timeout=60)
    arguments = dict(is_debug=False,enable_lto=True,dynamic_load_harfbuzz=False)
    shared,extra = [],dict(hosts={str(f.relative_to(HERE)):digest(f)
                                 for f in (HERE/'hosts').rglob('*') if f.is_file()},
                           gn_target=digest(HERE/'BUILD.gn'))
    try:
        if args.platform=='ios':
            device,owned = ios_simulator(args.device)
            (output/'device.json').write_text(json.dumps(dict(platform='ios',device=device,owned=owned)))
            state = json.loads(execute(['xcrun','simctl','list','devices','--json']))['devices']
            if not any(d['udid']==device and d['state']=='Booted' for ds in state.values() for d in ds):
                subprocess.run(['xcrun','simctl','boot',device],check=True)
                subprocess.run(['xcrun','simctl','bootstatus',device,'-b'],check=True,timeout=300)
            arguments.update(target_os='ios',target_cpu='arm64' if platform.machine()=='arm64' else 'x64',
                use_xcode=True,use_flutter_cxx=True,use_ios_simulator=True,ios_deployment_target='13.0')
            extra['sdk'] = execute(['xcrun','--sdk','iphonesimulator','--show-sdk-version'])
        elif args.platform=='android':
            tools = android_tools()
            adb = str(tools[0]/'platform-tools/adb')
            devices = [line.split()[0] for line in execute([adb,'devices']).splitlines()[1:]
                       if len(line.split())==2 and line.split()[1]=='device' and line.startswith('emulator-')]
            device = args.device or (devices[0] if devices else None)
            if not device or not device.startswith('emulator-'):
                raise ValueError('start the Lynx CI AVD before invoking the Android gate')
            abi = execute([adb,'-s',device,'shell','getprop','ro.product.cpu.abi'])
            cpu = {'x86':'x86','x86_64':'x64','arm64-v8a':'arm64','armeabi-v7a':'arm'}[abi]
            arguments.update(target_os='android',target_cpu=cpu,use_flutter_cxx=False,use_ndk_cxx=True,
                enable_skshaper=False,
                android_sdk_root=str(tools[0]),android_sdk_version=tools[1].name.split('-',1)[1],
                android_sdk_build_tools_version=tools[2].name)
            extra.update(ndk=digest(tools[3]/'source.properties'),sdk=tools[1].name)
        elif args.platform=='windows':
            shared,fonts = windows_skity(source,output)
            config.update(backend='skity',fonts=[str(f) for f in fonts])
            arguments.update(target_os='win',target_cpu='x64',use_flutter_cxx=False,is_clang=True)
            extra['skity'] = digest(shared[0])
        else:
            arguments.update(target_os='harmony',target_cpu='arm64',harmony_sdk_version='default',
                             enable_skshaper=False,
                             use_flutter_cxx=False)
        for version,revision in [('baseline',args.baseline),('candidate',args.candidate)]:
            tree = output/(version+'-source');trees.append(tree)
            checkout(source,revision,tree)
            build_dir = output/(version+'-build')
            executable = build(source,tree,build_dir,arguments)
            metadata = write_metadata(source,revision,arguments,output/(version+'.metadata.json'),
                                      extra,config['fonts'])
            package = packages[version]
            if args.platform=='ios':
                app = output/(version+'.app')
                archive = ios_app(executable,metadata,package,app)
                subprocess.run(['xcrun','simctl','install',device,str(app)],check=True)
                measured = [archive,metadata]
            elif args.platform=='android':
                native = build_dir/'libperf_host.so'
                apk = android_apk(tree,native,metadata,package,output,abi,tools)
                subprocess.run([adb,'-s',device,'install','--no-incremental','-r',str(apk)],check=True)
                measured = [apk,metadata]
            elif args.platform=='windows':
                # Windows resolves DLLs next to the executable, not next to the
                # parent output directory or the JSON configuration file.
                runtime = build_dir/'skity.dll'
                shutil.copy2(shared[0],runtime)
                measured = [executable,metadata,runtime]+shared+config['fonts']
            else:
                continue  # Build-only: no Harmony device pool exists in the referenced Lynx CI.
            if device:
                cmd = [sys.executable,str(HERE/'app_adapter.py'),'--platform',args.platform,
                       '--device',device,'--package',package]
                if args.platform=='android': cmd += ['--adb',adb]
            else:
                cmd = [str(executable),str(metadata),'--server']
            config[version] = dict(revision=revision,protocol='jsonl',command=cmd,artifacts=artifacts(measured))
        if args.platform=='harmony':
            (output/'build-only.json').write_text(json.dumps(dict(built=True,performance_measured=False,
                 reason='Harmony SDK container has no device execution pool'),indent=2))
            return 0
        (output/'config.json').write_text(json.dumps(config,indent=2))
        config['cases_sha256'] = digest(HERE/'cases.json')
        report = run(config,json.loads((HERE/'cases.json').read_text())['cases'],output/'results')
        return 0 if report['passed'] else 1
    finally:
        if device:
            for package in packages.values():
                cmd = ([adb,'-s',device,'shell','am','force-stop',package] if args.platform=='android'
                       else ['xcrun','simctl','terminate',device,package])
                subprocess.run(cmd,capture_output=True,timeout=15,check=False)
                uninstall = ([adb,'-s',device,'uninstall',package] if args.platform=='android'
                             else ['xcrun','simctl','uninstall',device,package])
                subprocess.run(uninstall,capture_output=True,timeout=20,check=False)
            if owned:
                subprocess.run(['xcrun','simctl','delete',device],capture_output=True,timeout=30,check=False)
        for tree in trees:
            if tree.exists(): cleanup(source,tree)


if __name__=='__main__':
    raise SystemExit(main())
