class MbSystem < Formula
  desc "Processing and display of swath sonar bathymetry and backscatter imagery data"
  homepage "https://www.mbari.org/technology/mb-system/"
  url "https://github.com/dwcaress/MB-System.git",
      branch: "feature-newgui"
  version "5.8.2-newgui"
  license "GPL-3.0-or-later"
  head "https://github.com/dwcaress/MB-System.git", branch: "feature-newgui"

  depends_on "cmake" => :build
  depends_on "pkg-config" => :build

  depends_on "fftw"
  depends_on "gdal"
  depends_on "gmt"
  depends_on "netcdf"
  depends_on "openmotif"
  depends_on "proj"
  depends_on "qt@6"
  depends_on "vtk"
  depends_on "libx11"
  depends_on "libxt"

  def install
    # Set up build directory
    mkdir "build" do
      args = %W[
        -DCMAKE_INSTALL_PREFIX=#{prefix}
        -DCMAKE_BUILD_TYPE=Release
        -DbuildQt=1
      ]

      # Note: XQuartz GLX/OpenGL is broken on modern macOS
      # X11/Motif apps with OpenGL (mbgrdviz, mbeditviz) will not work
      # Users should use Qt versions (qt-mbgrdviz, etc.) instead

      # Configure Qt6 paths
      args << "-DQt6_DIR=#{Formula["qt@6"].opt_lib}/cmake/Qt6"

      # Configure VTK paths
      args << "-DVTK_DIR=#{Formula["vtk"].opt_lib}/cmake/vtk"

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
      MB-System has been installed.

      IMPORTANT: On modern macOS, XQuartz's OpenGL/GLX support is broken.
      X11/Motif apps with OpenGL (mbgrdviz, mbeditviz) will NOT work.

      ** Use the Qt versions instead: **
        - qt-mbgrdviz (instead of mbgrdviz)
        - qt-mbnavedit (instead of mbnavedit)
        - qt-mbeditviz (instead of mbeditviz)

      Qt versions use native macOS graphics and work reliably.

      For non-OpenGL X11 apps (MBedit, etc.), you need XQuartz:

        brew install --cask xquartz

      Then start XQuartz before running X11 apps:

        open -a XQuartz

      Add locale settings to your ~/.zshrc or ~/.bash_profile:

        export LC_ALL=en_US.UTF-8
        export LANG=en_US.UTF-8

      For more information:
        https://www.mbari.org/technology/mb-system/

      Discussion lists:
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
