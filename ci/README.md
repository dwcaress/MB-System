# Continuous Integration Testing Matrix

## Linux

### Build dependencies docker images

These images are built and pushed to hub.docker.com as monthly cron jobs.

| Name                                       | PROJ | GDAL  | GMT  |
|--------------------------------------------|------|-------|------|
| mbsystem-deps:ubuntu-focal                 | 6.2.1| 2.4.3 | 6.0.0|
| mbsystem-deps:ubuntu-focal-proj6.3-gmt6.0.0| 6.3  | 3.0.3 | 6.0.0|
| mbsystem-deps:ubuntu-disco                 | 5.2.0| 2.4.0 | 5.4.5|
| mbsystem-deps:ubuntu-bionic                | 4.9.3| 2.2.3 | 5.4.3|
| mbsystem-deps:ubuntu-xenial                | 4.9.2| 1.11.3| 6.0.0|
| mbsystem-deps:centos-7                     | 4.8.0| 1.11.4| 5.4.4|
| OSX                                        | 4.8.0| 1.11.4| 5.4.4|


### MB-System build tasks

Distribution Compatabilty builds test MB-System code base against distributions

| Name              | Image                      | Build Trigger | CFLAGS | MBTRN | MBTNAV | GSF |
|-------------------|----------------------------|---------------|--------|:-----:|:------:|:---:|
| Ubuntu Focal      | mbsystem-deps:ubuntu-focal |  master       |        |   Y   |   Y    |  Y  |
| Ubuntu Disco      | mbsystem-deps:ubuntu-disco |  master       |        |   Y   |   Y    |  Y  |
| Ubuntu Bionic     | mbsystem-deps:ubuntu-bionic|  master       |        |   Y   |   Y    |  Y  |
| Ubuntu Xenial     | mbsystem-deps:ubuntu-xenial|  master       |        |   Y   |   Y    |  Y  |
| CentOS 7          | mbsystem-deps:centos-7     |  master       |        |   Y   |   Y    |  Y  |


Development builds

| Name              | Image                                       | Build Trigger | CFLAGS         | MBTRN | MBTNAV | GSF |
|-------------------|---------------------------------------------|---------------|----------------|:-----:|:------:|:---:|
| Latest PROJ/GMT   | mbsystem-deps:ubuntu-focal-proj6.3-gmt6.0.0 |  master       |                |   Y   |   Y    |  Y  |
| clang compiler    | mbsystem-deps:ubuntu-focal                  |  master       |                |   Y   |   Y    |  Y  |
| debug build       | mbsystem-deps:ubuntu-focal                  |  master       | -g -Og         |   Y   |   Y    |  Y  |
| release build     | mbsystem-deps:ubuntu-focal                  |  master       | -g0 -O3 -NDEBUG|   N   |   N    |  N  |

## OSX

OSX build dependencies are installed using homebrew.

- PROJ: 6.2.1
- GDAL: 2.4.2
- GMT: 6.0.0

Distribution Compatabilty builds test MB-System code base against distributions

| Name              | OSX Version     | XCODE    | Build Trigger | CFLAGS             | MBTRN | MBTNAV | GSF |
|-------------------|-----------------|----------|---------------|--------------------|:-----:|:------:|:---:|
| OSX               | 10.13 (default) | 9.4.1    |  master       | -I/opt/X11/include |   Y   |   Y    |  Y  |