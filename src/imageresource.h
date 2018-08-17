#ifndef IMAGERESOURCE_H
#define IMAGERESOURCE_H

#include <QString>


class ImageResource
{
public:
    ImageResource();

    static QString stateToImagePath(int stateNumber, bool displayHide = false);
    static QString getGroupImagePath(int groupType = 0);
};

#endif // IMAGERESOURCE_H
