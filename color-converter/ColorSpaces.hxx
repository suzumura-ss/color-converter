//
//  ColorSpaces.hxx
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/28.
//

#pragma once
#include <cv.hpp>


// 標準イルミナント D65(昼光色)
//static const GLTK::Vector3 StdIlluminant_D65_XYZ(95.04f, 100.0f, 108.88f);
// u' = 4X/(X+15Y+3Z), v' = 9Y/(X+15Y+3Z)
//static const GLTK::Vector3 StdIlluminant_D65_Yuv(95.04f, 0.19782690146122145f, 0.46834020232296747f);

// 標準イルミナント A(白熱電球)
//static const GLTK::Vector3 StdIlluminant_A_XYZ(109.85f, 100.0f, 35.58f);
//static const GLTK::Vector3 StdIlluminant_A_Yuv(109.85f, 0.25597259683442175f, 0.5242952597883013f);


// sRGB <=> linear RGB
// https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
namespace sRGB_linearRGB {
    namespace {
        static const float RGB_RGBTh = 0.0031308;
        static const float RGB_LowGain = 12.92;
        static const float RGB_HiGain = 1.055;
        static const float RGB_HiOffset = 0.055;
        static const float RGB_Gamma = 2.4;
        
        float _sRGB2linearRGB(float sRGB)
        {
            if(sRGB <= RGB_RGBTh * RGB_LowGain) {
                return sRGB / RGB_LowGain;
            } else {
                return pow(((sRGB + RGB_HiOffset) / RGB_HiGain), RGB_Gamma);
            }
        }
        
        float _linearRGB2sRGB(float lRGB)
        {
            if(lRGB <= RGB_RGBTh) {
                return RGB_LowGain * lRGB;
            } else {
                return RGB_HiGain * pow(lRGB, (1.0/RGB_Gamma)) - RGB_HiOffset;
            }
        }
        
        cv::Mat sRGB2linearRGB(const cv::Mat& sRGB)
        {
            return vec3(_sRGB2linearRGB(sRGB.r),
                        _sRGB2linearRGB(sRGB.g),
                        _sRGB2linearRGB(sRGB.b));
        }
        
        cv::Mat linearRGB2sRGB(const cv::Mat& lRGB)
        {
            return vec3(_linearRGB2sRGB(lRGB.r),
                        _linearRGB2sRGB(lRGB.g),
                        _linearRGB2sRGB(lRGB.b));
        }
    }
}



// YUV <=> sRGB
const highp mat3 YUVtoSRGB = mat3(1.000, 0.000, 1.402,   1.000, -0.344, -0.714,  1.000,  1.772,  0.000);
const highp mat3 SRGBtoYUV = mat3(0.299, 0.587, 0.114,  -0.169, -0.331, 0.5000,  0.500, -0.419, -0.081);

// linear RGB <=> XYZ
const highp mat3 LRGBtoXYZ = mat3( 0.4124,  0.3576,  0.1805,  0.2126,  0.7152,  0.0722,  0.0193,  0.1192,  0.9505);
const highp mat3 XYZtoLRGB = mat3( 3.2406, -1.5372, -0.4986, -0.9689,  1.8758,  0.0415,  0.0557, -0.2040,  1.0570);

// YUV <=> XYZ
struct XYZ_t {
    highp float X;
    highp float Y;
    highp float Z;
};
XYZ_t YUVtoXYZ(lowp vec3 YUV)
{
    highp vec3 sRGB = YUVtoSRGB * (YUV - vec3(0.0, 0.5, 0.5));
    highp vec3 xyz  = LRGBtoXYZ * sRGB2linearRGB(sRGB);
    return XYZ_t(xyz.r, xyz.g, xyz.b);
}

lowp vec3 XYZtoYUV(XYZ_t XYZ)
{
    highp vec3 lRGB = XYZtoLRGB * vec3(XYZ.X, XYZ.Y, XYZ.Z);
    highp vec3 sRGB = linearRGB2sRGB(lRGB);
    return SRGBtoYUV * sRGB + vec3(0.0, 0.5, 0.5);
}


// YUV <=> L*u*v*
struct Yuv_t {
    highp float Yn;  // Yn
    highp float udn; // u'n
    highp float vdn; // v'n
};
const Yuv_t StdIlluminant_D65 = Yuv_t(95.04, 0.19782690146122145, 0.46834020232296747);
struct Luv_t {
    highp float L; // L*
    highp float u; // u*
    highp float v; // v*
};

/* https://en.wikipedia.org/wiki/CIELUV
 ---- XYZ to L*u*v*
 u' = 4X/(X+15Y+3Z)
 v' = 9Y/(X+15Y+3Z)
 Yn, u'n, v'n = StdIlluminant_{D65|A}_Yuv
 L* = (29/3)^3 (Y/Yn)    , Y/Yn<=(6/29)^3
 116(Y/Yn)^1/3-16   , Y/Yn> (6/29)^3
 u* = 13L* (u'-u'n) = 13L* ( 4X/(X+15Y+3Z) - u'n )
 v* = 13L* (v'-v'n) = 13L* ( 9Y/(X+15Y+3Z) - v'n )
 */
Luv_t YUVtoLuv(lowp vec3 YUV)
{
    XYZ_t XYZ = YUVtoXYZ(YUV);
    Luv_t Luv;
    highp float yyn = XYZ.Y / StdIlluminant_D65.Yn;
    if (yyn <= 0.008856451679035631 /*=(6/29)^3*/) {
        Luv.L = 903.2962962962961 /*=(29/3)^3*/ * yyn;
    } else {
        Luv.L = 116.0 * pow(yyn, 1.0/3.0)-16.0;
    }
    highp float k = XYZ.X + 15.0*XYZ.Y + 3.0*XYZ.Z;
    Luv.u = 13.0*Luv.L*(4.0*XYZ.X / k - StdIlluminant_D65.udn);
    Luv.v = 13.0*Luv.L*(9.0*XYZ.Y / k - StdIlluminant_D65.vdn);
    return Luv;
}

/* https://en.wikipedia.org/wiki/CIELUV
 ---- L*u*v* to XYZ
 u' = (u* / L*)/13 + u'n
 v' = (v* / L*)/13 + v'n
 Y  = Yn L*(3/29)^3       , L*<=8
 Yn ((L* +16)/116)^3 , L*> 8
 X  = Y(9u')/4v'
 Z  = Y(12-3u'-20v')/4v'
 */
lowp vec3 LuvtoYUV(Luv_t Luv)
{
    highp float ud = (Luv.u / Luv.L)/13.0 + StdIlluminant_D65.udn;
    highp float vd = (Luv.v / Luv.L)/13.0 + StdIlluminant_D65.vdn;
    highp float k = 4.0*vd;
    XYZ_t XYZ;
    if (Luv.L <= 8.0) {
        XYZ.Y = StdIlluminant_D65.Yn * Luv.L * 0.0011070564598794539; //=(3/29)^3
    } else {
        XYZ.Y = StdIlluminant_D65.Yn * pow((Luv.L+16.0)/116.0, 3.0);
    }
    XYZ.X = XYZ.Y*9.0*ud/k;
    XYZ.Z = XYZ.Y*(12.0-3.0*ud-20.0*vd)/k;
    return XYZtoYUV(XYZ);
}
