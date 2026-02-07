# MB-Mesh Test Suite

This directory contains tests for the mb-mesh tool, including simulated bathymetry data generators and test scripts.

## Test Files

- `test_data_generator.py` - Generates simulated bathymetry datasets
- `mb-mesh_test.py` - Test suite that runs mb-mesh with various inputs

## Generated Test Data

The test data generator creates the following simulated datasets:

### 1. Flat Seafloor (`flat_seafloor.txt`)
Simple flat bathymetry at constant depth for basic testing.

### 2. Seamount (`seamount.txt`)
Underwater mountain feature with:
- Base depth: -2000m
- Peak depth: -500m
- Gaussian-shaped profile

### 3. Ocean Trench (`trench.txt`)
Deep ocean trench feature with:
- Base depth: -1000m
- Trench depth: -3000m
- North-south orientation

### 4. Mid-Ocean Ridge (`ridge.txt`)
Underwater mountain range with:
- Base depth: -2000m
- Ridge peak: -1000m
- Linear N-S feature

### 5. Complex Canyon (`canyon.txt`)
Realistic canyon system with:
- Main diagonal canyon
- Side tributary
- Random roughness
- Depth variations

### 6. Noisy Data (`noisy.txt`)
Randomly distributed bathymetry points for testing interpolation.

### 7. Slope (`slope.txt`)
Simple sloping seafloor for gradient testing.

## Running Tests

### Generate Test Data

```bash
cd test/mb-mesh
python3 test_data_generator.py
```

This creates a `data/` directory with all test datasets.

### Run Test Suite

```bash
python3 mb-mesh_test.py
```

The test suite will:
1. Locate the mb-mesh executable
2. Generate test data if needed
3. Run mb-mesh with various parameters
4. Validate GLTF output files
5. Report results

## Test Cases

The test suite includes:

1. **Basic Tests**
   - Flat seafloor (baseline)
   - Default parameter validation

2. **Feature Tests**
   - Seamount rendering
   - Trench visualization
   - Ridge generation
   - Complex canyon

3. **Parameter Tests**
   - Vertical exaggeration (3x)
   - Grid spacing variations (0.5m to 5.0m)
   - Mesh decimation levels
   - Verbose output

4. **Stress Tests**
   - Noisy/sparse data
   - High-resolution meshes
   - Large datasets

## Expected Output

After successful test run:

```
test/mb-mesh/
├── data/
│   ├── flat_seafloor.txt
│   ├── seamount.txt
│   ├── trench.txt
│   ├── ridge.txt
│   ├── canyon.txt
│   ├── noisy.txt
│   └── slope.txt
└── output/
    ├── flat.gltf
    ├── seamount.gltf
    ├── seamount_exag.gltf
    ├── trench.gltf
    ├── ridge.gltf
    ├── canyon.gltf
    ├── noisy.gltf
    ├── slope_decimated.gltf
    ├── seamount_fine.gltf
    └── canyon_coarse.gltf
```

## Viewing Output Files

The generated GLTF files can be viewed with:

- **Online Viewers**: 
  - https://gltf-viewer.donmccurdy.com/
  - https://3dviewer.net/

- **Desktop Applications**:
  - Blender (File > Import > glTF 2.0)
  - MeshLab
  - ParaView

- **Web Frameworks**:
  - Three.js
  - Babylon.js
  - Cesium

## Validation

Tests check for:
- ✓ Successful execution (exit code 0)
- ✓ Output file creation
- ✓ Valid JSON structure
- ✓ GLTF 2.0 compliance
- ✓ Required GLTF fields
- ✓ Mesh data presence

## Adding New Tests

To add a new test case:

1. Add data generation function to `test_data_generator.py`
2. Add test case to `mb-mesh_test.py` tests array:

```python
{
    'name': 'My new test',
    'input': 'data/my_data.txt',
    'output': 'output/my_output.gltf',
    'args': ['-s', '1.0', '-e', '2.0']
}
```

## Requirements

- Python 3.6+
- mb-mesh executable (built from source)

Note: The test suite uses only Python's standard library - no external packages required.

## Troubleshooting

### "mb-mesh executable not found"
Build the project first:
```bash
cd ../../build
cmake ..
make mb-mesh
```

### "Test data directory not found"
Run the data generator:
```bash
python3 test_data_generator.py
```

### "Invalid GLTF file"
Check mb-mesh output for errors:
```bash
../../build/src/mb-mesh/mb-mesh -i data/test.txt -o output/test.gltf -v
```

## Integration with CMake

To add to the CMake test suite, add to `CMakeLists.txt`:

```cmake
add_test(NAME mb-mesh_test 
         COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/mb-mesh_test.py")
```

## License

See MB-System [COPYING.md](../../COPYING.md) for license information.
