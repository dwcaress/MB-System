#!/usr/bin/env python3
"""
Test Data Generator for mb-mesh

This script generates simulated bathymetry data for testing the mb-mesh tool.
It creates various seafloor features like seamounts, trenches, and ridges.
Uses only Python standard library (no numpy required).
"""

import math
import random
import os


def linspace(start, stop, num):
    """Create evenly spaced values (numpy replacement)."""
    if num == 1:
        return [start]
    step = (stop - start) / (num - 1)
    return [start + step * i for i in range(num)]


def generate_flat_seafloor(output_file, width=100, height=100, depth=-1000):
    """Generate a simple flat seafloor."""
    print(f"Generating flat seafloor: {output_file}")
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lat in linspace(36.0, 36.0 + height/1000, 20):
            for lon in linspace(-122.0, -122.0 + width/1000, 20):
                f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {20*20} points at depth {depth}m")


def generate_seamount(output_file, center_lon=-122.05, center_lat=36.05, 
                     base_depth=-2000, peak_depth=-500, radius=0.05):
    """Generate a seamount (underwater mountain)."""
    print(f"Generating seamount: {output_file}")
    
    points = []
    grid_size = 50
    
    for lat in linspace(36.0, 36.1, grid_size):
        for lon in linspace(-122.1, -122.0, grid_size):
            # Calculate distance from center
            dist = math.sqrt((lon - center_lon)**2 + (lat - center_lat)**2)
            
            # Calculate depth using Gaussian-like profile
            if dist < radius:
                elevation = (base_depth - peak_depth) * math.exp(-(dist/radius*3)**2)
                depth = base_depth - elevation
            else:
                depth = base_depth
            
            points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {len(points)} points with seamount")


def generate_trench(output_file, center_lon=-122.05, center_lat=36.05,
                   base_depth=-1000, trench_depth=-3000, width=0.02):
    """Generate an ocean trench."""
    print(f"Generating ocean trench: {output_file}")
    
    points = []
    grid_size = 50
    
    for lat in linspace(36.0, 36.1, grid_size):
        for lon in linspace(-122.1, -122.0, grid_size):
            # Calculate distance from trench center line (running N-S)
            dist = abs(lon - center_lon)
            
            # Calculate depth using inverted Gaussian
            if dist < width:
                depression = (trench_depth - base_depth) * math.exp(-(dist/width*3)**2)
                depth = base_depth + depression
            else:
                depth = base_depth
            
            points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {len(points)} points with trench")


def generate_ridge(output_file, center_lon=-122.05, base_depth=-2000,
                  ridge_depth=-1000, width=0.03):
    """Generate a mid-ocean ridge."""
    print(f"Generating mid-ocean ridge: {output_file}")
    
    points = []
    grid_size = 50
    
    for lat in linspace(36.0, 36.1, grid_size):
        for lon in linspace(-122.1, -122.0, grid_size):
            # Calculate distance from ridge center line (running N-S)
            dist = abs(lon - center_lon)
            
            # Calculate depth using Gaussian
            if dist < width * 2:
                elevation = (base_depth - ridge_depth) * math.exp(-(dist/width*2)**2)
                depth = base_depth - elevation
            else:
                depth = base_depth
            
            points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {len(points)} points with ridge")


def generate_complex_canyon(output_file):
    """Generate a complex underwater canyon system."""
    print(f"Generating complex canyon: {output_file}")
    
    points = []
    grid_size = 60
    
    base_depth = -1500
    random.seed(42)  # For reproducibility
    
    for lat in linspace(36.0, 36.1, grid_size):
        for lon in linspace(-122.1, -122.0, grid_size):
            depth = base_depth
            
            # Main canyon running diagonally
            canyon_center = -122.05 + (lat - 36.05) * 0.5
            dist_main = abs(lon - canyon_center)
            
            if dist_main < 0.015:
                canyon_depth = 500 * math.exp(-(dist_main/0.01)**2)
                depth -= canyon_depth
            
            # Side tributary
            if lat > 36.04 and lat < 36.06:
                dist_trib = abs(lon - (-122.03))
                if dist_trib < 0.008:
                    trib_depth = 300 * math.exp(-(dist_trib/0.005)**2)
                    depth -= trib_depth
            
            # Add some roughness
            roughness = random.uniform(-50, 50)
            depth += roughness
            
            points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {len(points)} points with complex canyon")


def generate_noisy_data(output_file, num_points=500):
    """Generate randomly distributed bathymetry points."""
    print(f"Generating noisy data: {output_file}")
    
    random.seed(42)  # For reproducibility
    
    points = []
    for _ in range(num_points):
        lon = random.uniform(-122.1, -122.0)
        lat = random.uniform(36.0, 36.1)
        depth = random.uniform(-2000, -500)
        points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {num_points} random points")


def generate_slope(output_file):
    """Generate a simple sloping seafloor."""
    print(f"Generating sloping seafloor: {output_file}")
    
    points = []
    grid_size = 40
    
    for lat in linspace(36.0, 36.1, grid_size):
        for lon in linspace(-122.1, -122.0, grid_size):
            # Depth increases linearly with longitude (west to east)
            depth = -500 - (lon + 122.1) * 10000
            
            points.append((lon, lat, depth))
    
    with open(output_file, 'w') as f:
        f.write("lon lat depth\n")
        for lon, lat, depth in points:
            f.write(f"{lon:.6f} {lat:.6f} {depth:.2f}\n")
    
    print(f"  Created {len(points)} points with slope")


def main():
    """Generate all test datasets."""
    print("=" * 70)
    print("MB-Mesh Test Data Generator")
    print("=" * 70)
    print()
    
    # Create data directory if it doesn't exist
    os.makedirs('data', exist_ok=True)
    
    # Generate various test datasets
    generate_flat_seafloor('data/flat_seafloor.txt')
    print()
    
    generate_seamount('data/seamount.txt')
    print()
    
    generate_trench('data/trench.txt')
    print()
    
    generate_ridge('data/ridge.txt')
    print()
    
    generate_complex_canyon('data/canyon.txt')
    print()
    
    generate_noisy_data('data/noisy.txt')
    print()
    
    generate_slope('data/slope.txt')
    print()
    
    print("=" * 70)
    print("Test data generation complete!")
    print("Data files created in ./data/ directory")
    print("=" * 70)


if __name__ == '__main__':
    main()
