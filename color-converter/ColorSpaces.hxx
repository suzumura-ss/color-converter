//
//  ColorSpaces.hxx
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/28.
//

#pragma once
#include <cv.hpp>
#include <string>


#define DEFINE_INITIALIZER(KLASS) \
    inline KLASS(): Vec3fE(0.0f, 0.0f, 0.0f) {} \
    inline KLASS(float _0_, float _1_, float _2_): Vec3fE(_0_, _1_, _2_) {} \
    inline KLASS(const Vec3fE& src): Vec3fE(src) {} \
    inline KLASS(const cv::Vec3f& src): Vec3fE(Vec3fE(src)) {}

#define DEFINE_UINT8_INITIALIZER(KLASS) \
    inline KLASS(int _0_, int _1_, int _2_): Vec3fE(_0_/255.0, _1_/255.0, _2_/255.0) {} \
    inline KLASS(const cv::Vec3b& src): Vec3fE(Vec3fE(src)/255.0) {}

#define DEFINE_ACCESSOR(_0_,_1_,_2_) \
    protected:\
    virtual const char* name0() const { return #_0_; } \
    virtual const char* name1() const { return #_1_; } \
    virtual const char* name2() const { return #_2_; } \
    public: \
    inline float _0_() const { return this->operator[](0); } \
    inline float _1_() const { return this->operator[](1); } \
    inline float _2_() const { return this->operator[](2); } \
    inline float& _0_() { return this->operator[](0); } \
    inline float& _1_() { return this->operator[](1); } \
    inline float& _2_() { return this->operator[](2); }


class Vec3fE : public cv::Vec3f
{
protected:
    virtual const char* name0() const { return "x"; }
    virtual const char* name1() const { return "y"; }
    virtual const char* name2() const { return "z"; }
    
public:
    inline Vec3fE(float x=0, float y=0, float z=0): cv::Vec3f(x, y, z) {}
    inline Vec3fE(const cv::Vec3f& src): cv::Vec3f(src) {}
    inline Vec3fE map(std::function<float(float)> proc) const {
        Vec3fE result;
        result[0] = proc(this->operator[](0));
        result[1] = proc(this->operator[](1));
        result[2] = proc(this->operator[](2));
        return result;
    }
    inline operator cv::Vec3b() const {
        auto result = map([](float v)->float {
            if (v<0.0) return 0.0;
            if (v>1.0) return 255.0;
            return v*255.0;
        });
        return cv::Vec3b(result[0], result[1], result[2]);
    }
    std::string to_s() const
    {
        std::stringstream s;
        s << "{"  << name0() << ":" << this->operator[](0);
        s << ", " << name1() << ":" << this->operator[](1);
        s << ", " << name2() << ":" << this->operator[](2) << "}";
        return s.str();
    }
};

class sRGB_t;
class lRGB_t;
class XYZ_t;
class Luv_t;



class YUV_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(YUV_t);
    DEFINE_UINT8_INITIALIZER(YUV_t);
    DEFINE_ACCESSOR(y, u, v);
    sRGB_t to_sRGB() const;
};


class sRGB_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(sRGB_t);
    DEFINE_UINT8_INITIALIZER(sRGB_t);
    DEFINE_ACCESSOR(r, g, b);
    lRGB_t to_lRGB() const;
    YUV_t  to_YUV() const;
};


class lRGB_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(lRGB_t);
    DEFINE_ACCESSOR(r, g, b);
    sRGB_t to_sRGB() const;
    XYZ_t  to_XYZ() const;
};


class XYZ_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(XYZ_t);
    DEFINE_ACCESSOR(X, Y, Z);
    lRGB_t to_lRGB() const;
    Luv_t  to_Luv() const;
};


class Luv_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(Luv_t);
    DEFINE_ACCESSOR(L, u, v);
    XYZ_t to_XYZ() const;
};


class Yuv_t : public Vec3fE
{
public:
    DEFINE_INITIALIZER(Yuv_t);
    DEFINE_ACCESSOR(Yn, udn, vdn);
    inline cv::Vec2f uv() const { return cv::Vec2f(udn(), vdn()); }
};
