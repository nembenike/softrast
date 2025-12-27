How to build?
```sh
cc -o nob nob.c # you only need to this once, nob rebuilts itself upon changes
./nob
```
To build with assets packed (important)
```sh
cc -o nob nob.c
./nob assets
```