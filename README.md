# Unreal Engine plugin for desaturating what otherwise would have been blurred

## How to build

1. Clone the repository to your machine
2. Find `RunUAT.bat` (for Windows) or `RunUAT.sh` (for Linux/MacOS): `<Unreal engine path>/Engine/Build/BatchFiles`
3. Build the plugin using `RunUAT`:

! Important Note: Close your editor before building

For Windows:
```powershell
./RunUAT.bat BuildPlugin -plugin='<path to plugins repo>/FullScreenPass.uplugin' -package='<path to Unreal Engine project>/Plugins/FullScreenPass/'
```
For Linux/MacoOS:
```powershell
./RunUAT.sh BuildPlugin -plugin='<path to plugins repo>/FullScreenPass.uplugin' -package='<path to Unreal Engine project>/Plugins/FullScreenPass/'
```