# VKDynamicRendering

## To configure
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
## To build 
```
cmake --build .
or
MSBuild VKGame.vcxproj -t:Rebuild -p:Configuration=Release
```


## Plans
- [x] Phong lighting
- [x] Loading multiple models 
- [ ] Make seperate materials struct
- [ ] Begin skinning/animations
