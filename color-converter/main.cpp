//
//  main.cpp
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/20.
//

#include <string>
#include <vector>
#include "ColorConverter.hxx"



int main(int argc, const char * argv[])
{
    auto usage = [&]()->void {
        printf("usage1 : %s <-toRGB|-toYUV|-copy> <sourceName> <outputName>\n", argv[0]);
        printf("usage2 : %s <sourceName> <outputName>\n", argv[0]);
        printf("    .png, .jpg : RGB\n");
        printf("    .ppm       : YUV\n");
        printf("    .nv21      : YUV-NV21\n");
        printf("    'png:-' 'jpg:-' 'ppm:-' 'nv21:-' for pipe\n");
    };
    auto shiftArg = [&]()->void {
        argc--;
        argv++;
    };

    if (argc<3) {
        usage();
        return 1;
    }
    shiftArg();
    
    ColorConverter converter;
    if (argc>2 && strcmp(argv[0], "-toRGB")==0) {
        return converter.YUVtoRGB(argv[1], argv[2])? 0: 1;
    }
    if (argc>2 && strcmp(argv[0], "-toYUV")==0) {
        return converter.RGBtoYUV(argv[1], argv[2])? 0: 1;
    }
    if (argc>2 && strcmp(argv[0], "-copy")==0) {
        return converter.copy(argv[1], argv[2])? 0: 1;
    }
    return converter.convert(argv[0], argv[1])? 0: 1;
}
