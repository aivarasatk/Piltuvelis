#include "parser.h"

#include <QStringList>
#include <QDebug>
#include <QDate>

Parser::Parser()
{
}

void Parser::parseSheetInfo(QString sheetInfoLine, SheetInfo& sheetInfo){
    QStringList items = sheetInfoLine.split('-');

    sheetInfo.blockHeader = items[1].toInt();
    sheetInfo.dataIndex = items[2].toInt();
}

void Parser::parseBlockHeader(QString header, HeaderDate& headerDate){
    QStringList monthYear = header.split(' ');//atskirta tarpu excely
    std::map<QString, int> intToMonth;
    intToMonth.insert(std::pair<QString, int>("Sausis",   1));
    intToMonth.insert(std::pair<QString, int>("Vasaris",  2));
    intToMonth.insert(std::pair<QString, int>("Kovas",    3));
    intToMonth.insert(std::pair<QString, int>("Balandis", 4));
    intToMonth.insert(std::pair<QString, int>("Gegužė",   5));
    intToMonth.insert(std::pair<QString, int>("Birželis" ,6));
    intToMonth.insert(std::pair<QString, int>("Liepa",    7));
    intToMonth.insert(std::pair<QString, int>("Rugpjūtis",8));
    intToMonth.insert(std::pair<QString, int>("Rugsėjis", 9));
    intToMonth.insert(std::pair<QString, int>("Spalis",   10));
    intToMonth.insert(std::pair<QString, int>("Lapkritis",11));
    intToMonth.insert(std::pair<QString, int>("Gruodis",  12));

    for(auto& month : intToMonth){
        if(monthYear[0].contains(month.first)){
            headerDate.monthString = month.first;

            if(month.second < 10){
                headerDate.monthShortString = "0" + QString::number(month.second);
            }else{
                headerDate.monthShortString = QString::number(month.second);
            }

            headerDate.monthInt = month.second;
            break;
        }
    }
    headerDate.year = monthYear[1].trimmed().toInt();
}

bool Parser::correctPlanFormat(QString operationFile){
    if(!(QDate::fromString(operationFile, "PLANyyyyMMdd").isNull()) ||
       !(QDate::fromString(operationFile, "GEN_yyyy_MM-dd").isNull())){
        return true;
    }
    return false;
}


QString Parser::operationCode(const HeaderDate& headerDate, std::map<QString, QString>& idData){
    QString date = QString::number(headerDate.year) + headerDate.monthShortString + "01";
    for(auto& item : idData){
        if(item.first.contains("PLAN"+date)){
            return item.second;
        }
    }
    return "";
}

QString Parser::insertValueToExport(QString& dataRow, QString value){
    int startIndex = dataRow.indexOf('>') + 1;
    dataRow.insert(startIndex, value);
    return dataRow;
}
