# Exercise 1

## Windows

### Dependencies

We previously need [Git](http://git-scm.com/install/windows), [CMake](https://cmake.org/download/) (make sure to check the **Add CMake to the system path** option) and [Visual Studio](https://visualstudio.microsoft.com/es/vs/community/) (make sure to include **Desktop development with C++** in the Visual Studio installation) installed in our PC.
Go to C:\Users\<Your user name>, and open PowerShell from the searchbar. We install the required dependencies via PowerShell:
```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
Integrate it with Visual Studio/CMake and install the required packages:
```powershell
.\vcpkg integrate install
.\vcpkg install glew freeglut glm freeimage
```

Install assimp for optional part
```powershell
.\vcpkg install assimp
```

### Building
```powershell
cd <project_root_folder>
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/<Your user name>/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug --parallel 10
```

### Running

We need to declare a temporal environment variable to be able to run the application (this variable will only exists in that PowerShell session):
```powershell
cd <project_root_folder>/build
cd p1GLSL\Debug
$env:Path += ";C:\Users\<Your user name>\vcpkg\installed\x64-windows\bin"
.\p1GLSL
```

## Linux (Ubuntu)

### Dependencies
```
sudo apt install build-essential cmake cmake-curses-gui git libxmu-dev libxi-dev libgl-dev  libglew-dev libfreeimage-dev freeglut3-dev
```

### Building
```bash
cd <project_root_folder>
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j10
```

### Running

Make sure the shader files and images are accesible to the executables copying the related files into: `<project_root_folder>/build`. After you should have an structure like this:
```
- build
-- img -> contaings images
-- p1GLSL -> contains project executable
-- shaders_p1 -> contains shaders for exercise 1
```

Then, go to the building folder:

```bash
cd <project_root_folder>/build
./p1GLSL/p1GLSL
```
