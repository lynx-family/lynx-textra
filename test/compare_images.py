# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import os
import cv2
import numpy

# Copied over from DiffManager._diff_img() and removed unused code
# https://github.com/lynx-family/lynx/blob/develop/testing/integration_test/test_script/lib/test_runner/mixin/img_diff_mixin.py
class DiffManager:
    def diff_two_images(self, baseline_image_path, actual_image_path, mismatch_threshold):
        # Parse baseline image
        baseline_img = cv2.imread(baseline_image_path)
        baseline_img = cv2.cvtColor(baseline_img, cv2.COLOR_BGR2GRAY)
        height, width = baseline_img.shape
        # Parse actual image
        test_img = cv2.imread(actual_image_path)
        test_img = cv2.cvtColor(test_img, cv2.COLOR_BGR2GRAY)
        test_height, test_width = test_img.shape

        # Check image size
        if height != test_height or width != test_width:
            raise RuntimeError(f"User case [ {actual_image_path} ]: Image comparison failed, image size inconsistent, baseline_height: {height}, test_height: {test_height}, baseline_width: {width}, test_width: {test_width}")

        absdiff = cv2.absdiff(baseline_img, test_img)

        # Create the following directory structure
        # parent_dir/
        # ├── actual/             # Contains original test images
        # │   ├── image1.png
        # │   └── ...
        # └── diff/               # Contains the image diffs
        #     ├── image1_diff.png
        #     └── ...
        actual_dir = os.path.dirname(actual_image_path)
        diff_dir = os.path.join(os.path.dirname(actual_dir), 'diff')
        os.makedirs(diff_dir, exist_ok=True)
        diff_filename = os.path.basename(actual_image_path).replace(".png", '_diff.png')
        mismatch_path = os.path.join(diff_dir, diff_filename)
        
        test_grey_in_rgb_channel_img = cv2.cvtColor(test_img, cv2.COLOR_GRAY2BGR)
        mask = absdiff > int(0.1 * 255)
        mask = absdiff > 0
        # (B, G, R)
        test_grey_in_rgb_channel_img[mask] = (108, 96, 244)
        cv2.imwrite(mismatch_path, test_grey_in_rgb_channel_img)

        mismatch_rate = numpy.sum(mask) / (height * width)
        if mismatch_rate > mismatch_threshold:
            raise RuntimeError('User case [ %s ]: Image comparison fail! Mismatch rate: %s' % ( actual_image_path, str(mismatch_rate)))

def compare_images_in_directories(golden_dir, actual_dir, mismatch_threshold):
    '''
    Find all PNG files in actual_dir and compare them with corresponding files in golden_dir
    using DiffManager._diff_image_logic method.
    '''
    diff_manager = DiffManager()
    
    actual_files = [f for f in os.listdir(actual_dir) if f.lower().endswith('.png')]
    
    if not actual_files:
        print(f"No PNG files found in {actual_dir}")
        return
    
    print(f"Found {len(actual_files)} PNG files in {actual_dir}")
    
    success_count = 0
    fail_count = 0
    
    for file_name in actual_files:
        actual_path = os.path.join(actual_dir, file_name)
        golden_path = os.path.join(golden_dir, file_name)
        
        if not os.path.exists(golden_path):
            print(f"Warning: No corresponding golden image found for {file_name}")
            fail_count += 1
            continue
        
        try:
            diff_manager.diff_two_images(golden_path, actual_path, mismatch_threshold)
            print(f"✓ Comparison passed: {file_name}")
            success_count += 1
        except Exception as e:
            print(f"✗ Comparison failed: {file_name}, reason: {str(e)}")
            fail_count += 1
    
    print(f"\nComparison complete: {success_count} passed, {fail_count} failed")
    return success_count, fail_count

def __main__():
    parser = argparse.ArgumentParser(description='Compare PNG images between two directories and generate visual diffs')
    parser.add_argument('--golden-dir', help='Path to directory containing reference/golden PNG images that serve as the baseline for comparison', required=True)
    parser.add_argument('--actual-dir', help='Path to directory containing actual/test PNG images that will be compared against the golden images', required=True)
    parser.add_argument('--threshold', 
                      help='Maximum allowed mismatch rate between images (0.0-1.0). Lower values enforce stricter comparison. '
                           'When the mismatch rate exceeds this threshold, the comparison fails. (default: 0.001)', 
                      type=float, default=0.001)
    args = parser.parse_args()
    
    # Validate directories
    if not os.path.isdir(args.golden_dir):
        print(f"Error: Golden directory {args.golden_dir} does not exist")
        return 1
    
    if not os.path.isdir(args.actual_dir):
        print(f"Error: Actual directory {args.actual_dir} does not exist")
        return 1
    
    # Run comparison
    success_count, fail_count = compare_images_in_directories(args.golden_dir, args.actual_dir, args.threshold)
    
    # Return non-zero exit code if any comparison failed
    return 0 if fail_count == 0 else 1

if __name__ == '__main__':
    exit_code = __main__()
    exit(exit_code)