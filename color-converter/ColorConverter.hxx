//
//  ColorConverter.hxx
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/20.
//

#pragma once
#include <cv.hpp>


class ColorConverter
{
public:
    enum FORMAT {
        FT_NONE = 0,
        FT_RGB,
        FT_YUV,
        FT_NV21,
    };
    inline static bool isYUV(FORMAT f)
    {
        return (f == FT_YUV) || (f == FT_NV21);
    }
    inline static const char* to_str(FORMAT f)
    {
        static const char* names[] = {
            "NONE", "RGB", "YUV", "NV21"
        };
        return names[f];
    }
    FORMAT getFormat(const std::string& fileName, std::string& ext, bool& isStream);
    FORMAT getFormat(const std::string& fileName) {
        std::string ext;
        bool isStream;
        return getFormat(fileName, ext, isStream);
    }
    cv::Mat decodeImage(const std::string& fileName, FORMAT& format);
    bool encodeImage(const std::string& fileName, const cv::Mat& image);
    
public:
    inline ColorConverter() {}
    inline virtual ~ColorConverter() {}
    bool copy(const char* source, const char* output);
    bool RGBtoYUV(const char* source, const char* output);
    bool YUVtoRGB(const char* source, const char* output);
    bool convert(const char* source, const char* output);
};
