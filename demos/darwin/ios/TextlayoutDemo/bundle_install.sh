pushd ../../../../
python3 tools/ios_tools/generate_ios_podspec_by_gn.py --target LynxTextra json_parser
popd
bundle exec pod install
