//
//  ColorSpaces.cpp
//  color-converter
//
//  Created by Toshiyuki Suzumura on 2017/04/28.
//

#include <cmath>
#include "ColorSpaces.hxx"


#pragma mark -- Constants.
// https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29

namespace {
    static const float RGB_RGBTh = 0.0031308;
    static const float RGB_LowGain = 12.92;
    static const float RGB_HiGain = 1.055;
    static const float RGB_HiOffset = 0.055;
    static const float RGB_Gamma = 2.4;
    static const Yuv_t StdIlluminant_D65(95.04, 0.19782690146122145, 0.46834020232296747);  // 標準イルミナント D65(昼光色)
    static const Yuv_t StdIlluminant_A(109.85, 0.25597259683442175, 0.5242952597883013);    // 標準イルミナント A(白熱電球)
    static const cv::Matx33f SRGB_to_YUV(0.299, 0.587, 0.114,  -0.169, -0.331, 0.5000,  0.500, -0.419, -0.081);
    static const cv::Matx33f LRGB_to_XYZ( 0.4124,  0.3576,  0.1805,  0.2126,  0.7152,  0.0722,  0.0193,  0.1192,  0.9505);
};



#pragma mark -- YUV_t

sRGB_t YUV_t::to_sRGB() const
{
    static cv::Matx33f YUV_to_SRGB = SRGB_to_YUV.inv();
    return YUV_to_SRGB * ((*this) - cv::Vec3f(0.0, 0.5, 0.5));
}



#pragma mark -- sRGB_t

lRGB_t sRGB_t::to_lRGB() const
{
    return map([](float sRGB)->float {
        if(sRGB <= RGB_RGBTh * RGB_LowGain) {
            return sRGB / RGB_LowGain;
        } else {
            return pow(((sRGB + RGB_HiOffset) / RGB_HiGain), RGB_Gamma);
        }
    });
}


YUV_t  sRGB_t::to_YUV() const
{
    return SRGB_to_YUV * (*this) + cv::Vec3f(0.0, 0.5, 0.5);
}



#pragma mark -- lRGB_t

sRGB_t lRGB_t::to_sRGB() const
{
    return map([](float lRGB)->float {
        if(lRGB <= RGB_RGBTh) {
            return RGB_LowGain * lRGB;
        } else {
            return RGB_HiGain * pow(lRGB, (1.0/RGB_Gamma)) - RGB_HiOffset;
        }
    });
}


XYZ_t  lRGB_t::to_XYZ() const
{
    return LRGB_to_XYZ * (*this);
}



#pragma mark -- XYZ_t

lRGB_t XYZ_t::to_lRGB() const
{
    static cv::Matx33f XYZ_to_LRGB = LRGB_to_XYZ.inv();
    return XYZ_to_LRGB * (*this);
}


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
Luv_t  XYZ_t::to_Luv() const
{
    float x15y3z = X()+15.0*Y()+3.0*Z();
    if (x15y3z==0.0) {
        return Luv_t(0.0f, 0.0f, 0.0f);
    }
    float L;
    float yyn = Y() / StdIlluminant_D65.Yn();
    if (yyn <= pow(6.0/29.0, 3.0)) {
        L = pow(29.0/3.0, 3.0) * yyn;
    } else {
        L = 116.0 * pow(yyn, 1.0/3.0) - 16.0;
    }
    float u = 13.0 * L *( 4.0*X()/x15y3z - StdIlluminant_D65.udn() );
    float v = 13.0 * L *( 9.0*Y()/x15y3z - StdIlluminant_D65.vdn() );
    return Luv_t(L, u, v);
}



#pragma mark -- Luv_t

/* https://en.wikipedia.org/wiki/CIELUV
 ---- L*u*v* to XYZ
 u' = (u* / L*)/13 + u'n
 v' = (v* / L*)/13 + v'n
 Y  = Yn L*(3/29)^3       , L*<=8
      Yn ((L* +16)/116)^3 , L*> 8
 X  = Y(9u')/4v'
 Z  = Y(12-3u'-20v')/4v'
 */
XYZ_t Luv_t::to_XYZ() const
{
    if (L()==0.0) {
        return XYZ_t(0.0f, 0.0f, 0.0f);
    }
    float ud = (u()/L())/13.0 + StdIlluminant_D65.udn();
    float vd = (v()/L())/13.0 + StdIlluminant_D65.vdn();
    float Y;
    if (L() <= 8.0) {
        Y = StdIlluminant_D65.Yn() * L() * pow(3.0/29.0, 3.0);
    } else {
        Y = StdIlluminant_D65.Yn() * pow(( L()+16.0 )/116.0, 3.0);
    }
    float X = Y * ( 9.0*ud              )/( 4.0*vd );
    float Z = Y * ( 12.0-3.0*ud-20.0*vd )/( 4.0*vd );
    return XYZ_t(X, Y, Z);
}
