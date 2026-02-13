class MbSystem < Formula
  desc "Processing and display of swath sonar bathymetry and backscatter imagery data"
  homepage "https://www.mbari.org/technology/mb-system/"
  url "https://github.com/dwcaress/MB-System.git",
      branch: "feature-newgui"
  version "5.8.2-newgui"
  license "GPL-3.0-or-later"
  head "https://github.com/dwcaress/MB-System.git", branch: "feature-newgui"

  depends_on "cmake" => :build
  depends_on "googletest" => :build
  depends_on "pkg-config" => :build

  depends_on "fftw"
  depends_on "gdal"
  depends_on "gmt"
  depends_on "libx11"
  depends_on "libxt"
  depends_on "mesa"
  depends_on "mesa-glu"
  depends_on "netcdf"
  depends_on "opencv"
  depends_on "openmotif"
  depends_on "proj"
  depends_on "qt@6"
  depends_on "vtk"

  def install
    # Set up build directory
    mkdir "build" do
      args = %W[
        -DCMAKE_INSTALL_PREFIX=#{prefix}
        -DCMAKE_BUILD_TYPE=Release
        -DbuildQt=1
        -DbuildTests=1
      ]

      # NOTE: XQuartz GLX/OpenGL is broken on modern macOS
      # X11/Motif apps with OpenGL (mbgrdviz, mbeditviz) will not work
      # Users should use Qt versions (qt-mbgrdviz, etc.) instead

      # Configure Qt6 paths
      ENV["Qt6_DIR"] = "#{Formula["qt@6"].opt_lib}/cmake/Qt6"

      # Configure VTK - let CMake find it via CMAKE_PREFIX_PATH
      args << "-DVTK_DIR=#{Formula["vtk"].opt_lib}/cmake/vtk-9.3"

      # Enable VTK modules with Qt GUI support
      args << "-DVTK_QT_VERSION=6"
      args << "-DVTK_GROUP_ENABLE_Qt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_ViewsQt=YES"
      args << "-DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=YES"

      system "cmake", "..", *args, *std_cmake_args
      system "make"
      system "make", "install"
      
      # 1. Install C++ Test Apps from test/mbio
      # This assumes the build process outputs binaries into the
      # build/test/mbio directory
      cd "build/test/mbio" do
        # Binaries usually don't have extensions; this grabs all executable
        # files
        bin.install Dir["*"].select { |f| File.executable?(f) && !File.directory?(f) }
      end

    # 2. Install Python Utilities from test/utilities
    # These are typically in the source tree, not the build tree
      cd "../test/utilities" do
        bin.install Dir["*.py"]
      end
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
