#include "htmlhelper.h"

HtmlHelper::HtmlHelper()
{
}

QString HtmlHelper::getHTMLHeader(int docType)
{
    // docType: 0=Content Window; 1=Input Window
    QString buffer("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
                   "<head>"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                   "<title></title>");
    switch (docType)
    {
        case 0:
            buffer.append("<style>"
                          "body{width:360px;overflow:hidden}"
                          "div,span,ul,li{marin:0px;padding:0px;list-style-type:none;font-size:15px}"
                          "p{text-align:left}"
                          "img{border:0px;max-width:100%;height:auto}"
                          ".s{color:#339966}"
                          ".r{color:#0066CC}.a{color:#000000}"
                          ".c{margin:0px 0px 15px 5px;width:350px;word-wrap:break-word}"
                          "</style></head><body>");
            break;
        default:
            buffer.append("<style>"
                          "body{width:98%;margin:2px;overflow:hidden}"
                          "div,span,ul,li{marin:0px;padding:0px;list-style-type:none}"
                          "p{text-align:left;}"
                          "</style></head><body>");
    }
    return buffer;
}

QString HtmlHelper::getHTMLFooter(int docType)
{
    QString buffer;
    switch (docType)
    {
        case 0:
            buffer.append("</body></html>");
            break;
        default:
            buffer.append("</body></html>");
    }
    return buffer;
}

QString HtmlHelper::extractHTMLTag(const QString& rawHTML, QString tagName)
{
    int p1, p2, p3;
    QString tagBegin(tagName), tagEnd(tagName);
    tagBegin.prepend('<');
    tagEnd.prepend("</").append('>');

    p1 = rawHTML.indexOf(tagBegin, 0, Qt::CaseInsensitive);
    p2 = rawHTML.indexOf('>', p1);
    p3 = rawHTML.indexOf(tagEnd, 0, Qt::CaseInsensitive);
    if (p1 >= 0 && p2 > p1 && p3 > p2)
        return rawHTML.mid(p2 + 1, p3 - p2 - 1);
    else
        return "";
}
