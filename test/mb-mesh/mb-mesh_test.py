#!/usr/bin/env python3
"""
Test suite for mb-mesh

This script tests the mb-mesh tool with various simulated datasets.
"""

import subprocess
import os
import sys
import json


class Colors:
    """ANSI color codes for terminal output."""
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'
    BOLD = '\033[1m'


def print_header(text):
    """Print a formatted header."""
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'=' * 70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{text:^70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'=' * 70}{Colors.END}\n")


def print_test(test_name):
    """Print test name."""
    print(f"{Colors.BOLD}Test: {test_name}{Colors.END}")


def print_success(message):
    """Print success message."""
    print(f"{Colors.GREEN}✓ {message}{Colors.END}")


def print_error(message):
    """Print error message."""
    print(f"{Colors.RED}✗ {message}{Colors.END}")


def print_warning(message):
    """Print warning message."""
    print(f"{Colors.YELLOW}⚠ {message}{Colors.END}")


def find_mb_mesh_executable():
    """Find the mb-mesh executable."""
    # Check common locations
    possible_paths = [
        '../../build/src/mb-mesh/mb-mesh',
        '../build/src/mb-mesh/mb-mesh',
        './mb-mesh',
        'mb-mesh'
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            return path
    
    # Try which command
    try:
        result = subprocess.run(['which', 'mb-mesh'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
    except:
        pass
    
    return None


def check_gltf_validity(filepath):
    """Basic validation of GLTF file."""
    if not os.path.exists(filepath):
        return False, "File does not exist"
    
    file_size = os.path.getsize(filepath)
    if file_size == 0:
        return False, "File is empty"
    
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
        # Try to parse as JSON
        data = json.loads(content)
        
        # Check for required GLTF fields
        if 'asset' not in data:
            return False, "Missing 'asset' field"
        
        if 'version' not in data['asset']:
            return False, "Missing version in asset"
        
        # Check for mesh data
        if 'meshes' in data and len(data['meshes']) > 0:
            print_success(f"  Valid GLTF with {len(data['meshes'])} mesh(es)")
        
        return True, "Valid GLTF file"
        
    except json.JSONDecodeError as e:
        return False, f"Invalid JSON: {e}"
    except Exception as e:
        return False, f"Error validating: {e}"


def run_test(mb_mesh_path, test_name, input_file, output_file, extra_args=None):
    """Run a single test case."""
    print_test(test_name)
    
    if not os.path.exists(input_file):
        print_error(f"Input file not found: {input_file}")
        return False
    
    # Build command
    cmd = [mb_mesh_path, '-i', input_file, '-o', output_file]
    if extra_args:
        cmd.extend(extra_args)
    
    print(f"  Command: {' '.join(cmd)}")
    
    # Run mb-mesh
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        
        if result.returncode != 0:
            print_error(f"mb-mesh failed with return code {result.returncode}")
            if result.stderr:
                print(f"  Error output: {result.stderr}")
            return False
        
        # Validate output
        valid, message = check_gltf_validity(output_file)
        
        if valid:
            file_size = os.path.getsize(output_file)
            print_success(f"Test passed! Output: {output_file} ({file_size} bytes)")
            return True
        else:
            print_error(f"Invalid output: {message}")
            return False
            
    except subprocess.TimeoutExpired:
        print_error("Test timed out after 30 seconds")
        return False
    except Exception as e:
        print_error(f"Test failed with exception: {e}")
        return False


def main():
    """Run all tests."""
    print_header("MB-Mesh Test Suite")
    
    # Find executable
    mb_mesh = find_mb_mesh_executable()
    if not mb_mesh:
        print_error("mb-mesh executable not found!")
        print("Please build mb-mesh first:")
        print("  cd ../../build")
        print("  cmake ..")
        print("  make mb-mesh")
        sys.exit(1)
    
    print_success(f"Found mb-mesh: {mb_mesh}")
    
    # Check if test data exists
    data_dir = 'data'
    if not os.path.exists(data_dir):
        print_warning("Test data directory not found")
        print("Generating test data...")
        subprocess.run([sys.executable, 'test_data_generator.py'])
        print()
    
    # Create output directory
    output_dir = 'output'
    os.makedirs(output_dir, exist_ok=True)
    
    # Run tests
    tests_passed = 0
    tests_failed = 0
    
    tests = [
        {
            'name': 'Basic flat seafloor',
            'input': 'data/flat_seafloor.txt',
            'output': 'output/flat.gltf',
            'args': None
        },
        {
            'name': 'Seamount with default settings',
            'input': 'data/seamount.txt',
            'output': 'output/seamount.gltf',
            'args': None
        },
        {
            'name': 'Seamount with vertical exaggeration',
            'input': 'data/seamount.txt',
            'output': 'output/seamount_exag.gltf',
            'args': ['-e', '3.0']
        },
        {
            'name': 'Ocean trench',
            'input': 'data/trench.txt',
            'output': 'output/trench.gltf',
            'args': None
        },
        {
            'name': 'Mid-ocean ridge',
            'input': 'data/ridge.txt',
            'output': 'output/ridge.gltf',
            'args': ['-s', '2.0']
        },
        {
            'name': 'Complex canyon',
            'input': 'data/canyon.txt',
            'output': 'output/canyon.gltf',
            'args': ['-v']
        },
        {
            'name': 'Noisy data',
            'input': 'data/noisy.txt',
            'output': 'output/noisy.gltf',
            'args': ['-s', '3.0']
        },
        {
            'name': 'Slope with decimation',
            'input': 'data/slope.txt',
            'output': 'output/slope_decimated.gltf',
            'args': ['-d', '2']
        },
        {
            'name': 'Fine grid spacing',
            'input': 'data/seamount.txt',
            'output': 'output/seamount_fine.gltf',
            'args': ['-s', '0.5']
        },
        {
            'name': 'Coarse grid spacing',
            'input': 'data/canyon.txt',
            'output': 'output/canyon_coarse.gltf',
            'args': ['-s', '5.0']
        }
    ]
    
    for test in tests:
        if run_test(mb_mesh, test['name'], test['input'], 
                   test['output'], test['args']):
            tests_passed += 1
        else:
            tests_failed += 1
        print()
    
    # Print summary
    print_header("Test Summary")
    total = tests_passed + tests_failed
    
    print(f"Total tests: {total}")
    print_success(f"Passed: {tests_passed}")
    
    if tests_failed > 0:
        print_error(f"Failed: {tests_failed}")
    
    success_rate = (tests_passed / total * 100) if total > 0 else 0
    print(f"\nSuccess rate: {success_rate:.1f}%")
    
    if tests_failed == 0:
        print_success("\n🎉 All tests passed!")
        return 0
    else:
        print_error("\n⚠️  Some tests failed")
        return 1


if __name__ == '__main__':
    sys.exit(main())
