//
//  ColorConverter.cpp
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/20.
//

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <istream>
#include <string>
#include "ColorConverter.hxx"
#include "PortablePixmap.hxx"
#include "PortablePixmapNV21.hxx"



ColorConverter::FORMAT ColorConverter::getFormat(const std::string& fileName, std::string& ext, bool& isStream)
{
    if (fileName.length()<5) {
        fprintf(stderr, "invalid file type - %s\n", fileName.c_str());
        return FORMAT::FT_NONE;
    }
    if (fileName.substr(fileName.length()-2, 2).compare(":-")==0) {
        isStream = true;
        ext = "." + fileName.substr(0, fileName.length()-2);
    } else {
        size_t p = fileName.rfind(".");
        if (p>0) {
            isStream = false;
            ext = fileName.substr(p);
        } else {
            fprintf(stderr, "invalid file type - %s\n", fileName.c_str());
            return FORMAT::FT_NONE;
        }
    }
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext.compare(".ppm")==0) {
        return FORMAT::FT_YUV;
    }
    if (ext.compare(".nv21")==0) {
        return FORMAT::FT_NV21;
    }
    return FORMAT::FT_RGB;
}



template<class MAT> cv::Mat loadPlane(const uint8_t* bitmap, const PortablePixmap::Vector2i& size, bool expand2x)
{
    cv::Mat r = MAT(size.height, size.width);
    memcpy(r.ptr(), bitmap, size.area()*r.elemSize());
    if (expand2x) {
        cv::Mat p2;
        cv::resize(r, p2, cv::Size(), 2, 2);
        return p2;
    }
    return r;
}

cv::Mat ColorConverter::decodeImage(const std::string& fileName, FORMAT& format)
{
    bool isStream = false;
    std::string ext;
    format = getFormat(fileName, ext, isStream);
    if (format == FORMAT::FT_NONE) return cv::Mat();
    
    auto loadNV21 = [&](std::istream& in)->cv::Mat {
        PortablePixmapNV21 bmp(in);
        cv::Mat rY  = loadPlane<cv::Mat1b>(bmp.bitmap(), bmp.size(), false);
        cv::Mat rVU = loadPlane<cv::Mat2b>(bmp.bitmapVU(), bmp.sizeVU(), true);
        std::vector<cv::Mat> rVUArray;
        cv::split(rVU, rVUArray);
        cv::Mat result;
        cv::merge(std::vector<cv::Mat>({rY, rVUArray.at(1), rVUArray.at(0)}), result);
        return result;
    };
    
    if (isStream) {
        if (ext.compare(".ppm")==0) {
            PortablePixmap bmp(std::cin);
            cv::Mat result = cv::Mat3b(bmp.size().height, bmp.size().width);
            memcpy(result.ptr(), bmp.bitmap(), bmp.size().area()*3);
            return result;
        }
        if (ext.compare(".nv21")==0) {
            return loadNV21(std::cin);
        }
        std::cin >> std::noskipws;
        std::vector<uchar> bitmap;
        std::copy(std::istream_iterator<uchar>(std::cin), std::istream_iterator<uchar>(), std::back_inserter(bitmap));
        return cv::imdecode(bitmap, CV_LOAD_IMAGE_COLOR);
    }
    if (ext.compare(".nv21")==0) {
        std::ifstream stream(fileName, std::ios::in);
        if (stream.is_open()) {
            return loadNV21(stream);
        }
        fprintf(stderr, "file(%s) open failed - %s\n", fileName.c_str(), strerror(errno));
        return cv::Mat();
    }
    cv::Mat result = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
    if (result.dims!=2) {
        fprintf(stderr, "file(%s) read failed - %s\n", fileName.c_str(), strerror(errno));
    }
    return result;
}



bool ColorConverter::encodeImage(const std::string& fileName, const cv::Mat& image)
{
    bool isStream = false;
    std::string ext;
    FORMAT format = getFormat(fileName, ext, isStream);
    if (format == FORMAT::FT_NONE) return false;
    
    auto saveNV21 = [&](std::ostream& out)->bool {
        PortablePixmapNV21 bmp(image.cols, image.rows);
        auto savePlane = [&](const cv::Mat& plane, uint8_t* p, bool reduceHalf)->void {
            if (reduceHalf) {
                cv::Mat p2(plane.rows/2, plane.cols/2, plane.type());
                cv::resize(plane, p2, p2.size(), cv::INTER_CUBIC);
                memcpy(p, p2.ptr(), p2.rows * p2.cols * p2.elemSize());
                return;
            }
            memcpy(p, plane.ptr(), plane.rows * plane.cols * plane.elemSize());
        };
        std::vector<cv::Mat> planes;
        cv::split(image, planes);
        cv::Mat rVU;
        cv::merge(std::vector<cv::Mat>({planes.at(2), planes.at(1)}), rVU);
        savePlane(planes.at(0), bmp.bitmap(), false);
        savePlane(rVU, bmp.bitmapVU(), true);
        bmp.writeToStream(out);
        return true;
    };

    if (isStream) {
        if (ext.compare(".ppm")==0) {
            PortablePixmap bmp(image.cols, image.rows);
            memcpy(bmp.bitmap(), image.ptr(), bmp.size().area()*3);
            bmp.writeToStream(std::cout);
            return true;
        }
        if (ext.compare(".nv21")==0) {
            return saveNV21(std::cout);
        }
        std::vector<uchar> bitmap;
        bool r = cv::imencode(ext, image, bitmap);
        if (r) {
            std::cout.write((const char*)&bitmap.at(0), bitmap.size());
        }
        return r;
    }
    try {
        if (ext.compare(".nv21")==0) {
            std::ofstream stream(fileName, std::ifstream::out|std::ifstream::binary);
            return saveNV21(stream);
        }
        if (!cv::imwrite(fileName, image)) {
            fprintf(stderr, "file(%s) write failed - %s\n", fileName.c_str(), strerror(errno));
            return false;
        }
    } catch (cv::Exception& e) {
        fprintf(stderr, "file(%s) write failed - %s\n", fileName.c_str(), e.what());
        return false;
    }
    return true;
}



bool ColorConverter::copy(const char* source, const char* output)
{
    ColorConverter::FORMAT format;
    cv::Mat sourceImage = decodeImage(source, format);
    if (!sourceImage.empty()) return false;
    return encodeImage(output, sourceImage);
}


bool ColorConverter::RGBtoYUV(const char* source, const char* output)
{
    ColorConverter::FORMAT format;
    cv::Mat sourceImage = decodeImage(source, format);
    if (!sourceImage.empty()) return false;
    cv::Mat outputImage;
    cvtColor(sourceImage, outputImage, cv::COLOR_RGB2YUV);
    return encodeImage(output, outputImage);
}


bool ColorConverter::YUVtoRGB(const char* source, const char* output)
{
    ColorConverter::FORMAT format;
    cv::Mat sourceImage = decodeImage(source, format);
    if (!sourceImage.empty()) return false;
    cv::Mat outputImage;
    cvtColor(sourceImage, outputImage, cv::COLOR_YUV2RGB);
    return encodeImage(output, outputImage);
}


bool ColorConverter::convert(const char* source, const char* output)
{
    ColorConverter::FORMAT format_in;
    cv::Mat sourceImage = decodeImage(source, format_in);
    if (sourceImage.empty()) return false;

    ColorConverter::FORMAT format_out = getFormat(output);
    if (format_out == FORMAT::FT_NONE) return false;
    if (isYUV(format_in) == isYUV(format_out)) {
        return encodeImage(output, sourceImage);
    }
    if (isYUV(format_in)) {
        cv::Mat outputImage;
        cvtColor(sourceImage, outputImage, cv::COLOR_YUV2RGB);
        return encodeImage(output, outputImage);
    } else {
        cv::Mat outputImage;
        cvtColor(sourceImage, outputImage, cv::COLOR_RGB2YUV);
        return encodeImage(output, outputImage);
    }
}

