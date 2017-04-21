//
//  PortablePixmap.hxx
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/20.
//

#pragma once

#include <istream>
#include <ostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <errno.h>



class PortablePixmap
{
public:
    union Vector2i {
        struct {
            int x;
            int y;
        };
        struct {
            int width;
            int height;
        };
        int area() const {
            return x * y;
        }
        Vector2i operator / (int value) const
        {
            return Vector2i(x/value, y/value);
        }
        Vector2i(int x_=0, int y_=0) {
            x = x_;
            y = y_;
        }
    };
    
protected:
    Vector2i _size;
    std::vector<uint8_t> _bitmap;
    
    static bool getHeader(std::istream& stream, std::string& magic, int& width, int& height, int& depth)
    {
        auto getToken = [&](std::string& tok)->bool {
            tok.clear();
            char c;
            while (stream.get(c)) {
                if (isspace(c)) {
                    return true;
                }
                tok += c;
            }
            return false;
        };
        auto getValue = [&](int& value)->bool {
            value = 0;
            std::string tok;
            if (!getToken(tok)) return false;
            value = atoi(tok.c_str());
            return true;
        };
        if (!getToken(magic)) return false;
        if (!getValue(width)) return false;
        if (!getValue(height)) return false;
        if (!getValue(depth)) return false;
        return true;
    }
    
    static bool parseHeader(const char* expect_magic, std::istream& stream, int& width, int& height)
    {
        std::string magic;
        int depth;
        if (!getHeader(stream, magic, width, height, depth)) {
            fprintf(stderr, "PortablePixmap: invalid header - expect `%s width height depth`\n", expect_magic);
            return false;
        }
        if (magic.compare(expect_magic)!=0) {
            fprintf(stderr, "PortablePixmap: invalid header - expect `%s`\n", expect_magic);
            return false;
        }
        if (width<=0 || height<=0) {
            fprintf(stderr, "PortablePixmap invalid header - width:%d, height:%d\n", width, height);
            return false;
        }
        if (depth!=255) {
            fprintf(stderr, "PortablePixmap: invalid header - expect depth=255\n");
            return false;
        }
        return true;
    };
    
    virtual void clear()
    {
        _bitmap.clear();
        _size = Vector2i(0, 0);
    }
    
    virtual void allocate(int width, int height)
    {
        _size = Vector2i(width, height);
        _bitmap.assign(_size.area()*3, 0);
    }
    
    virtual void loadStream(std::istream& stream)
    {
        clear();
        stream >> std::noskipws;
        int width=0, height=0;
        if (parseHeader("P6", stream, width, height)) {
            allocate(width, height);
            stream.read((char*)&_bitmap.at(0), _bitmap.size());
            return;
        }
    }

    virtual void loadFile(const char* fileName)
    {
        std::ifstream stream(fileName, std::ios::in|std::ios::binary);
        loadStream(stream);
    }

public:
    PortablePixmap()
    {
        clear();
    }
    
    inline virtual ~PortablePixmap() {}
    
    PortablePixmap(int width, int height)
    {
        allocate(width, height);
    }
    
    PortablePixmap(std::istream& stream)
    {
        loadStream(stream);
    }

    PortablePixmap(const char* fileName)
    {
        loadFile(fileName);
    }
    
    inline virtual uint8_t* bitmap()
    {
        return &_bitmap.at(0);
    }
    
    inline virtual Vector2i size() const
    {
        return _size;
    }
    
    virtual bool writeToStream(std::ostream& stream) const
    {
        stream << "P6\n" << _size.width << " " << _size.height << "\n255\n";
        stream.write((char*)&_bitmap.at(0), _bitmap.size());
        return true;
    }

    virtual bool writeToFile(const char* fileName) const
    {
        std::ofstream stream(fileName, std::ifstream::out|std::ifstream::binary);
        return writeToStream(stream);
    }
};
