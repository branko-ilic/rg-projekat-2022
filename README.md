# Computer Graphics project

## Instructions
1. `git clone https://github.com/Armapillow/rg-projekat-2022`
2. CLion -> Open -> path/to/my/project_base
3. Main is in src/main.cpp
4. ALT+SHIFT+F10 -> projekat -> run
5. Turn the cursor on/off `C` (on by default)
6. For movement use `W`, `A`, `S`, `D`
7. To speed up the camera use `UP`, to slow it down use `DOWN` arrows
8. For scrolling in/out use scroll on the mouse
9. Turn the spotlight(flashlight) on/off `F`
10. To switch between Anti Aliasing and Bloom use `B`
11. Switch the Bloom on/off  `SPACE`
12. Increase the exposure of the bloom `E`, decrease the exposure `Q`

* Unzip [objects.zip](https://drive.google.com/file/d/1E5Zn9Mm5aG44ah1jI6Ri56nznZUvHucG/view?usp=sharing) into the `resources/` directory.

## Additional implemented sections

Group A:
- [x] Anti aliasing (Radovan)
- [x] Cubemaps (Branko)

Group B:
- [x] Normal-plant/Parallax-books Mapping (Radovan)
- [x] HDR/Bloom (Branko)

## Assets:

* Pyramid coordinates borrowed from
  [Middle-Earth-Project](https://github.com/matf-rg-2020-showcase/Middle-Earth-Project/blob/main/src/main.cpp#L146)
* [Skybox textures](https://www.flickr.com/photos/gadl/393474308/)
* [Plant](https://sketchfab.com/3d-models/azalea-fae7c1ccc8d9405f859e4920787c1c08)
* [Sphere](https://sketchfab.com/3d-models/xxr-sphere-121319-7928b72a80d341cdae1d0b7cb09988bd)
* [Book](https://sketchfab.com/3d-models/the-hobbit-95198e3460b14c4db3749eb888a869b3)
* Program for reconstructing a height map from a normal map: [debump.c](https://stannum.io/blog/0IwyJ-)
  * `gcc ... -lm -lfftw3f`
  * `./debump deconv -ny <input> <output>`


## Authors

* Radovan Božić 172/2018
* Branko Ilić   216/2018
