# This script checks that the VTK at specified location meets minimum
# version requiremnts.
# 
BEGIN {
    if (ARGC < 3) {
        print "usage minMajor minMinor vtkVersionHeaderFile";
        usageError = 1;
        exit;
    }
    minMajor = ARGV[1];
    minMinor = ARGV[2];
    ARGV[1] = "";
    ARGV[2] = "";
}

/define VTK_MAJOR_VERSION/ {
    major = $3;
}
/define VTK_MINOR_VERSION/ {
    minor = $3;
}


END {
    if (usageError) {
        exit 1;
    }
    # printf "major: %d  minor: %d\n", major, minor;
    # printf "minMajor: %d  minMinor: %d\n", minMajor, minMinor;

    ok = 0;
    printf "%d.%d ", major, minor;

    if (major > minMajor) {
        ok = 1;
    }
    else if (major == minMajor && minor >= minMinor) {
        ok = 1;
    }
    if (ok) {
        print "OK";
        exit 0;
    }
    else {
        printf "too old: require at least VTK %d.%d\n", minMajor, minMinor;
        exit 1;
    }
}

