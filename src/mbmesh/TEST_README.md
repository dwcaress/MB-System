# MBMesh Tests

Unit tests for `mbmesh.cpp` XYZ point cloud generation.

## Building

```bash
cd build
cmake -DENABLE_TESTING=ON ..
make test_mbmesh
```

## Running

```bash
# Run all tests
./src/mbmesh/test_mbmesh

# Run specific test class
./src/mbmesh/test_mbmesh --gtest_filter="XYZFileFormatTest*"

# Verbose output
./src/mbmesh/test_mbmesh -v

# With CTest
ctest --test-dir . --verbose
```

## Test Coverage (29 tests)

- **XYZFileFormatTest** (5) - File structure, header, precision validation
- **DataAccuracyTest** (4) - Data ordering, large datasets, edge cases
- **GeographicVariationTest** (3) - Regional bathymetry, global coordinates
- **BoundsFilteringTest** (5) - Geographic filtering and boundaries
- **NumericStabilityTest** (3) - Extreme values, precision maintenance
- **FileIOTest** (4) - Robustness, error handling

## Precision Specifications

- Longitude/Latitude: 8 decimal places (±1.1mm)
- Depth: 3 decimal places (±1mm)
- Datasets: 1 to 10,000+ points
- Coverage: Global (-180°/-90° to +180°/+90°), -11,000m depths
