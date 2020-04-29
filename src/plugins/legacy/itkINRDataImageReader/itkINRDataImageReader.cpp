/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 All rights reserved.
 See LICENSE.txt for details in the root of the sources or:
 https://github.com/medInria/medInria-public/blob/master/LICENSE.txt

 This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////

#include "itkINRDataImageReader.h"

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>

#include <medInrimageImageIO.h>

// /////////////////////////////////////////////////////////////////
// itkINRDataImageReaderPrivate
// /////////////////////////////////////////////////////////////////

class itkINRDataImageReaderPrivate
{
public:
};

// /////////////////////////////////////////////////////////////////
// itkINRDataImageReader
// /////////////////////////////////////////////////////////////////
const char itkINRDataImageReader::ID[] = "itkINRDataImageReader";

itkINRDataImageReader::itkINRDataImageReader(void) :  itkDataImageReaderBase(), d(new itkINRDataImageReaderPrivate)
{
    this->io = InrimageImageIO::New();
}

itkINRDataImageReader::~itkINRDataImageReader(void)
{

}

bool itkINRDataImageReader::registered(void)
{
    return medAbstractDataFactory::instance()->registerDataReaderType(ID,s_handled(),createItkINRDataImageReader);
}

QString itkINRDataImageReader::description(void) const
{
    return "itkINRDataImageReader";
}

QStringList itkINRDataImageReader::handled() const {
    return s_handled();
}

QStringList itkINRDataImageReader::s_handled() {
    return QStringList()  << "itkDataImageChar3" << "itkDataImageChar4"
            << "itkDataImageUChar3" << "itkDataImageUChar4"
            << "itkDataImageShort3" << "itkDataImageShort4"
            << "itkDataImageUShort3" << "itkDataImageUShort4"
            << "itkDataImageInt3" << "itkDataImageInt4"
            << "itkDataImageUInt3" << "itkDataImageUInt4"
            << "itkDataImageLong3" << "itkDataImageLong4"
            << "itkDataImageULong3" << "itkDataImageULong4"
            << "itkDataImageFloat3" << "itkDataImageFloat4"
            << "itkDataImageDouble3" << "itkDataImageDouble4"
            << "itkDataImageRGB3" << "itkDataImageRGBA3";
}

QString itkINRDataImageReader::identifier() const {
    return ID;
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataReader *createItkINRDataImageReader(void)
{
    return new itkINRDataImageReader;
}
