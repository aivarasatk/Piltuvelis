#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QString>
#include <map>
#include <QStandardItemModel>
#include <QFile>

#include "structures.h"

class FileWriter
{
public:
    FileWriter(std::vector<QString> eipTemplateV,
               std::map<QString, QString> identificationData);
    QString writeDataToFile(std::vector<DataFromSheet>& sheetData,
                            const QStandardItemModel *treeModel, QString fileName,
                            QString genCode = nullptr);
    QString analyzeSheetData(DataFromSheet &sheetData,
                                      QStandardItem* root);
    QString analyzeMonthData(std::pair<const HeaderDate, std::vector<std::vector<QString> > > &monthItem,
                                      QStandardItem* monthTreeItem);

    QString writeSheetData(DataFromSheet &singleSheetData, QStandardItem *root);
    QString writeMonthData(std::pair<const HeaderDate, std::vector<std::vector<QString> > > &monthItem, QStandardItem *month);
    void writeItemData(std::vector<QString> rowData, QString operationCode, QStandardItem *item, int child);
    void appendDataBlock(QString& itemBlock, QString exportRow, const QString &value);
    QString isPill(QString name);
    std::pair<const HeaderDate, std::vector<std::vector<QString> > > getMonthPair(std::map<HeaderDate, std::vector<std::vector<QString>>>& dataBlock,
                                                                                      int i);
    QStringList getGenSelection();
    QStringList getGenCodes();

    bool defaultGeneratedDate(QString date);

private:
    std::vector<QString> eipTemplate;
    std::map<QString, QString> idData;
    QFile file;
    bool writeToGenExport = false;
    QString genExportCode = "";

    enum EipID{OPERATIONCODE = 0, CODE = 1, AMOUNT = 2, MAKER = 3, DATE = 4, DETAILS = 5, COST = 6, DIMDATE = 7};
    enum Options{ALL = 0, NEW = 1};
};

#endif // FILEWRITER_H
