#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QStandardItemModel>
#include "structures.h"
class Parser
{
public:
    Parser();
    static void parseSheetInfo(QString sheetInfoLine, SheetInfo &sheetInfo);
    static void parseBlockHeader(QString header, HeaderDate& headerDate);
    static bool correctPlanFormat(QString operationFile);

    static QString operationCode(const HeaderDate& headerDate, std::map<QString, QString>& idData);
    static QString insertValueToExport(QString &dataRow, QString value);
};

#endif // PARSER_H
