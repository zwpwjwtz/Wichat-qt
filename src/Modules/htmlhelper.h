#ifndef HTMLHELPER_H
#define HTMLHELPER_H

#include <QString>


class HtmlHelper
{
public:
    HtmlHelper();

    static QString getHTMLHeader(int docType);
    static QString getHTMLFooter(int docType);
    static QString extractHTMLTag(const QString& rawHTML, QString tagName);
};

#endif // HTMLHELPER_H
