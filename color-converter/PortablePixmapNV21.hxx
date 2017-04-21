//
//  PortablePixmapNV21.hxx
//  color-converter
//
//  Created by Toshiyuki Terashita on 2017/04/26.
//

#pragma once

#include "PortablePixmap.hxx"


class PortablePixmapNV21 : public PortablePixmap
{
protected:
    virtual void allocate(int width, int height)
    {
        _size = Vector2i(width, height);
        _bitmap.assign(bytesY()+bytesVU(), 0);
    }
    
    virtual void loadStream(std::istream& stream)
    {
        clear();
        stream >> std::noskipws;
        int width=0, height=0;
        if (parseHeader("N2", stream, width, height)) {
            allocate(width, height);
            stream.read((char*)&_bitmap.at(0), _bitmap.size());
            return;
        }
    }

public:
    PortablePixmapNV21():PortablePixmap() {}
    inline virtual ~PortablePixmapNV21() {}
    
    PortablePixmapNV21(int width, int height):PortablePixmap()
    {
        allocate(width, height);
    }
    
    PortablePixmapNV21(std::istream& stream):PortablePixmap()
    {
        loadStream(stream);
    }

    PortablePixmapNV21(const char* fileName):PortablePixmap()
    {
        loadFile(fileName);
    }
    
    virtual bool writeToStream(std::ostream& stream) const
    {
        stream << "N2 " << _size.width << " " << _size.height << " 255\n";
        stream.write((char*)&_bitmap.at(0), _bitmap.size());
        return true;
    }
    
    inline uint8_t* bitmapVU()
    {
        return &_bitmap.at(bytesY());
    }
    
    inline Vector2i sizeVU() const
    {
        return _size/2;
    }

    inline size_t bytesY() const
    {
        return _size.area();
    }
    inline size_t bytesVU() const
    {
        return sizeVU().area()*2;
    }
};
