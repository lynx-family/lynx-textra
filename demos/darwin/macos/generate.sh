pushd ../../../
python3 tools/build_cmake_environment.py --gn-args "is_debug=true use_flutter_cxx=false"
popd