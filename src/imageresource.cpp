#include "imageresource.h"
#include "Modules/account.h"


ImageResource::ImageResource()
{
}

QString ImageResource::stateToImagePath(int stateNumber, bool displayHide)
{
    QString path = ":/Icons/";
    switch (Account::OnlineState(stateNumber))
    {
        case Account::OnlineState::Online:
            path.append("online");
            break;
        case Account::OnlineState::Busy:
            path.append("busy");
            break;
        case Account::OnlineState::Hide:
            if (displayHide)
                path.append("invisible");
            else
                path.append("offline");
            break;
        case Account::OnlineState::Offline:
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
