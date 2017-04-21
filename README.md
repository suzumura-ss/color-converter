# color-converter
RGB/YUV/NV21 Color converter

## How to use

### build

Open color-converter.xcodeproj with XCode, or

```
$ make
```

### convert

* RGB to YUV
```
$ color-converter input.png output.ppm
```
```
$ color-converter -toYUV input.png output.png
#=> output[R] for Y, [G] for U, [B] for V.
```

* RGB to NV21(YUV420,SemiPlanar,Y+VU)
```
$ color-converter input.png output.nv21
```

* YUV to RGB
```
$ color-converter input.ppm output.png
```
```
$ color-converter -toRGB input.png output.png
#=> input[R] for Y, [G] for U, [B] for V.
```

* YUV to NV21
```
$ color-converter input.ppm output.nv21
```

* NV21 to RGB
```
$ color-converter input.nv21 output.png
```

* NV21 to YUV
```
$ color-converter input.nv21 output.ppm
```


### without color convert

```
$ color-converter -copy input.nv21 output.png
```


### with pipe

* example 1: push NV21 image to Android with adb.
```
$ color-converter input.png nv21:-| gzip -9 | adb shell 'gzip -d > /path/to/file.nv21'
```
* example 2: pull NV21 image from Android with adb.
```
$ adb shell 'cat /path/to/file.nv21 | gzip -5' | color-converter nv21:- output.png
```
