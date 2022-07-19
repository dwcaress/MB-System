# Running dockerized MB-System on Windows 11

**Note**:
These are rather basic instructions for a set of minimal tests on a Windows 11 machine.
In general, with a proper Docker environment and WSL, as well as an X11 server in place
on the target platform, the MB-System programs should run normally.
Please check with your sysadmin for assistance regarding these requirements.

The following was tested on a laptop with Windows 11 Version 21H2 (OS Build 22000.675).

## Key references and requirements

- "Run Linux GUI apps on the Windows Subsystem for Linux"
  : <https://docs.microsoft.com/en-us/windows/wsl/tutorials/gui-apps>
    - "You will need to be on Windows 11 Build 22000 or later to access this feature." 

- "Install Linux on Windows with WSL" 
  : <https://docs.microsoft.com/en-us/windows/wsl/install>
    - "must be running Windows 10 version 2004 and higher (Build 19041 and higher) or Windows 11."

- "Setting up Docker Desktop for Windows with WSL 2"
  – <https://docs.microsoft.com/en-us/windows/wsl/tutorials/wsl-containers>


## Base setup

In the Windows Features list, enable:
 
- "Windows System for Linux."
- "Virtual Machine Platform"

In a Powershell terminal, running as Administrator: 

```
wsl --install -d Ubuntu-20.04
```

Per <https://aka.ms/wsl2kernel>, download and install "Windows Subsystem for Linux Update." 

Per <https://docs.microsoft.com/en-us/windows/wsl/tutorials/wsl-containers>
install Docker Desktop.
- Ensure "that "Use the WSL 2 based engine" is checked in Settings > General."
- Select the Linux distribution (Ubuntu-20.04)
  "from your installed WSL 2 distributions which you want to enable Docker integration on
  by going to: Settings > Resources > WSL Integration."
- Click "Apply & Restart"
- Open Ubuntu-20.04 (via Start menu and selecting the entry showing up there)

```
$ docker --version
Docker version 20.10.17, build 100c701
```

As a basic test, run: `docker run hello-world`.

---
Per <https://docs.microsoft.com/en-us/windows/wsl/tutorials/gui-apps>,
install appropriate driver for vGPU as needed. 
In our test, we installed "Intel GPU driver for WSL"
<https://www.intel.com/content/www/us/en/download/19344/intel-graphics-windows-dch-drivers.html>.

> ...you can update to the latest [WSL] version that includes Linux GUI support by running
> the update command from an elevated command prompt.
> Select Start, type PowerShell, right-click Windows PowerShell, and then select Run as administrator.

```
wsl --update
wsl --shutdown
```
A reboot may be needed here.

Open an Ubuntu session (as done previously).

```
sudo apt update
```

As a basic test of a graphical application:

```
sudo apt install -y gedit
gedit
```

For X11 dependant applications:

```
sudo apt install x11-apps -y
```

Typical X11 applications like `xcalc`, `xclock`, and `xeyes` should open OK.

Since this base setup does not seem sufficient for the graphical MB-System applications,
per some references (including [this one](https://jack.kawell.us/posts/ros-windows-docker/)),
we installed VcXsrv from <https://sourceforge.net/projects/vcxsrv/>.

## MB-System

We can now install and run the MB-System image.

Download the launcher `mbsystem.sh` script from 
<https://raw.githubusercontent.com/dwcaress/MB-System/master/docker/user/mbsystem.sh>

Edit `mbsystem.sh` to indicate the concrete image to use.
In our case we used the "latest" tag, which corresponded to a build on June 26, 2022.

```
MBSYSTEM_IMAGE=mbari/mbsystem:latest
```

Set the `DISPLAY` environment variable:

```
export DISPLAY=host.docker.internal:0.0
```

We can now run the MB-System.

```
$ ./mbsystem.sh -L
::: OSTYPE=linux-gnu
::: Running as user 1000
::: Mounting /home/mbs as /opt/MBSWorkDir in container
::: Mounting /home/mbs/.mbsystem.bash_history as /opt/mbsystem.bash_history in container
::: Starting container 'mbsystem-bash'
::: Running: docker run -it --name mbsystem-bash --user 1000 -v /home/mbs:/opt/MBSWorkDir -v /home/mbs/.mbsystem.bash_history:/opt/mbsystem.bash_history -e DISPLAY --net=host mbari/mbsystem:latest bash

bash-4.25$ 
```
We are now at the bash prompt within the container, from where we can then run any of the MB-System programs.
