class MbSystem < Formula
  desc "Processing and display of swath sonar bathymetry and backscatter imagery data"
  homepage "https://www.mbari.org/technology/mb-system/"
  url "https://github.com/dwcaress/MB-System/archive/refs/tags/MB-System-5.8.2.tar.gz"
  sha256 ""  # You'll need to calculate this with: shasum -a 256 MB-System-5.8.2.tar.gz
  license "GPL-3.0-or-later"
  head "https://github.com/dwcaress/MB-System.git", branch: "master"

  bottle do
    # Bottle hashes will be added by Homebrew maintainers
  end

  depends_on "cmake" => :build
  depends_on "pkg-config" => :build
  
  depends_on "fftw"
  depends_on "gdal"
  depends_on "gmt"
  depends_on "netcdf"
  depends_on "openmotif"
  depends_on "proj"
  depends_on "qt@6"
  depends_on "vtk@9.5"
  depends_on "libx11"
  depends_on "libxt"
  depends_on "xquartz" => :cask

  def install
    # Set up build directory
    mkdir "build" do
      args = %W[
        -DCMAKE_INSTALL_PREFIX=#{prefix}
        -DCMAKE_BUILD_TYPE=Release
      ]

      # Configure Qt6 paths
      args << "-DQt6_DIR=#{Formula["qt@6"].opt_lib}/cmake/Qt6"
      
      # Configure VTK paths
      args << "-DVTK_DIR=#{Formula["vtk@9.5"].opt_lib}/cmake/vtk-9.5"
      
      # Enable VTK modules with Qt GUI support
      args << "-DVTK_QT_VERSION=6"
      args << "-DVTK_GROUP_ENABLE_Qt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_ViewsQt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=YES"
      
      # Configure GMT paths
      args << "-DGMT_DIR=#{Formula["gmt"].opt_prefix}"
      
      # Configure PROJ paths
      args << "-DPROJ_DIR=#{Formula["proj"].opt_prefix}"
      
      # Configure GDAL paths
      args << "-DGDAL_DIR=#{Formula["gdal"].opt_prefix}"
      
      # Configure NetCDF paths
      args << "-DNETCDF_DIR=#{Formula["netcdf"].opt_prefix}"
      
      # Configure FFTW paths
      args << "-DFFTW_DIR=#{Formula["fftw"].opt_prefix}"
      
      # Configure Motif paths
      args << "-DMOTIF_DIR=#{Formula["openmotif"].opt_prefix}"
      
      # Configure X11 paths
      args << "-DX11_INCLUDE_DIR=#{Formula["libx11"].opt_include}"
      args << "-DX11_LIBRARIES=#{Formula["libx11"].opt_lib}/libX11.dylib"
      args << "-DXt_INCLUDE_DIR=#{Formula["libxt"].opt_include}"
      args << "-DXt_LIBRARIES=#{Formula["libxt"].opt_lib}/libXt.dylib"

      system "cmake", "..", *args, *std_cmake_args
      system "make"
      system "make", "install"
    end
  end

  def caveats
    <<~EOS
      MB-System has been installed with XQuartz for X11 support.
      
      The graphical tools (MBedit, MBnavedit, MBvelocitytool, MBgrdviz, 
      MBeditviz) require X11, which has been installed via XQuartz.
      
      You may need to log out and back in for XQuartz to be fully configured.
      
      For more information and documentation, visit:
        https://www.mbari.org/technology/mb-system/
      
      To get help or report issues, use the MB-System discussion lists:
        http://listserver.mbari.org/sympa/info/mbsystem
    EOS
  end

  test do
    # Test that the main utilities are installed and can display version info
    system "#{bin}/mbformat", "-V"
    system "#{bin}/mbinfo", "--version"
    system "#{bin}/mbsystem", "-V"
  end
end
