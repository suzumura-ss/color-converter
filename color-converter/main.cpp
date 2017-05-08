//
//  main.cpp
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/20.
//

#include <string>
#include <vector>
#include "ColorConverter.hxx"
#include "ColorSpaces.hxx"
#include <iostream>


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


int main_(int argc, const char * argv[])
{
    Luv_t luvMax(0.0f, 0.0f, 0.0f);
    Luv_t luvMin(1.0f*(1<<10), 0.0f, 0.0f);
    sRGB_t L_Max, u_Max, v_Max, L_Min, u_Min, v_Min;

    auto update = [](float v, const sRGB_t& tag, float& regMax, float& regMin, sRGB_t& tagMax, sRGB_t& tagMin)->void {
        if (v < regMin) {
            regMin = v;
            tagMin = tag;
        }
        if (v > regMax) {
            regMax = v;
            tagMax = tag;
        }
    };

    auto calc = [&](float r, float g, float b, float vmax)->void {
        sRGB_t srgb(r/vmax, g/vmax, b/vmax);
        Luv_t  luv = srgb.to_lRGB().to_XYZ().to_Luv();
        update(luv.L(), srgb, luvMax.L(), luvMin.L(), L_Max, L_Min);
        update(luv.u(), srgb, luvMax.u(), luvMin.u(), u_Max, u_Min);
        update(luv.v(), srgb, luvMax.v(), luvMin.v(), v_Max, v_Min);
    };

    int vmax = 100;
    if (true) {
        for (int r = 0; r<=vmax; ++r) {
            calc(r, r, r, vmax);
        }
    } else {
        for (int r = 0; r<=vmax; ++r) {
            for (int g = 0; g<=vmax; ++g) {
                for (int b = 0; b<=vmax; ++b) {
                    calc(r, g, b, vmax);
                }
            }
        }
    }
    std::cout << "max: " << luvMax.to_s() << std::endl;
    std::cout << "\t" << "L:" << L_Max << " u:" << u_Max << " v:" << v_Max << std::endl;
    std::cout << "min: " << luvMin.to_s() << std::endl;
    std::cout << "\t" << "L:" << L_Min << " u:" << u_Min << " v:" << v_Min << std::endl;
    return 0;
}
