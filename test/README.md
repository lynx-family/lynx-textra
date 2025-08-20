# Testing

The core C++ logic is tested using two types of tests: 
- Unit test
- Image-based test

### Unit Tests

The C++ unit tests are written using Google Test and Google Mock. The source
code is located in the test/ folder. These tests run only on the Mac host
machine, but not on iOS, Android, or Harmony.

A subset of the tests consists of pure unit tests using Google Test and Google
Mock.

### Image-based Tests

The remaining tests are image-based. They use Skity to generate PNG images and
perform binary comparisons against corresponding golden images (located in
test/golden_images). 

To update the golden images, simply copy your output file to the
test/golden_images folder and check it into Git LFS.

## Building and Running Tests

Execute the following commands to build and run the tests:

    ```bash
    # Build tests
    cd <project_root>
    cmake -DCMAKE_BUILD_TYPE=Release -DSKITY_MTL_BACKEND=ON -G Ninja -S . -B build-test
    cmake --build build-test -t TextLayoutLiteTest

    # Run tests
    mkdir golden_check
    pushd golden_check
    # This step will execute pure unit tests and generate images for image-based tests (but will not compare with golden images)
    ../build-test/test/TextLayoutLiteTest
    popd

    # Compare newly generated images with golden images
    pip3 install opencv-python
    python3 test/compare_images.py --actual-dir=golden_check --golden-dir=test/golden_images
    # After running, a directory called diff/ will be created at the same level as --actual-dir, containing diff images
    ```