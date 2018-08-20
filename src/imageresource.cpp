#include "imageresource.h"
#include "Modules/onlinestate.h"


ImageResource::ImageResource()
{
}

QString ImageResource::stateToImagePath(int stateNumber, bool displayHide)
{
    QString path = ":/Icons/";
    switch (stateNumber)
    {
        case OnlineState::Online:
            path.append("online");
            break;
        case OnlineState::Busy:
            path.append("busy");
            break;
        case OnlineState::Hide:
            if (displayHide)
                path.append("invisible");
            else
                path.append("offline");
            break;
        case OnlineState::Offline:
        default:
            path.append("offline");
    }
    path.append(".png");
    return path;
}

QString ImageResource::getGroupImagePath(int groupType)
{
    switch (groupType)
    {
        case 0:
        default:
            return ":/Icons/group.png";
    }
}
