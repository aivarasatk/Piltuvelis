#ifndef FILEREADER_H
#define FILEREADER_H

#include <QString>
#include "xlsxdocument.h"
#include "xlsxcell.h"

#include "structures.h"


class FileReader
{
public:
    FileReader();

    bool readEipTemplate(QString fileName, std::vector<QString>& eipTemplate);
    bool readIdentificationFile(QString fileName, std::map<QString, QString>& identificationData);
    bool readExcel(QString fileName,
                   std::map<HeaderDate, std::vector<std::vector<QString> > > &allItems,
                   std::map<HeaderDate, std::vector<std::vector<QString>>>& newItems,
                   int sheetIndex);
    void setExcelFileName(QString fileName);
    QXlsx::Document& getExcel(){return *file;}
    void unsetExcelFileName() {excelSet = false;}
    std::vector<ExcelMonthErrors>& getExcelErrors(){return excelErrors;}
private:
    bool excelSet = false;
    QXlsx::Document* file;
    QString endDelimiter = "end";
    std::vector<ExcelMonthErrors> excelErrors;

    SheetInfo readSheetInfo(QXlsx::Document& file);
    int findStartColumn(QXlsx::Document& file, int currentRow);
    void readBlocks(std::map<HeaderDate, std::vector<std::vector<QString>>> &allItems,
                    std::map<HeaderDate, std::vector<std::vector<QString>>> &newItems,
                    int currentColumn, QXlsx::Document& file);

    int readBlockRows(const SheetInfo& sheetInfo, int currentColumn,
                       std::vector<std::vector<QString>>& allBlockRows,
                       std::vector<std::vector<QString> > &newBlockRows, QXlsx::Document& file,
                       std::vector<NamingScheme> &namingScheme);

    void formDataBlock(std::map<HeaderDate, std::vector<std::vector<QString>>> &dataBlock,
                                   std::vector<std::vector<QString>> &blockRows,
                                   HeaderDate headerDate);



    void includeDimDate(std::vector<QString>& rowData);
    void setDefaultDate(std::vector<QString>& rowData, QString header);
    void removeAmountField(std::vector<QString>& rowData, ExcelMonthErrors &errors);
    std::vector<NamingScheme> getNamingScheme(QString fileName);

    void sortByNamingScheme(std::vector<NamingScheme> namingScheme, std::vector<NamingScheme> anchorScheme, std::vector<CellData *> &data);
    NamingScheme dataFromScheme(QString data, int index, std::vector<NamingScheme>& namingScheme);
    void mergeRowData(std::vector<CellData *> &rowData01, std::vector<CellData *> &rowData02);
    void setDataFromCell(std::vector<CellData *> &cellData1, std::vector<QString>& rowData);
    void readCell(QXlsx::Document &file, int row, int column, std::vector<CellData *> &cellData);

    void releaseCellDataMemory(std::vector<CellData*>& cellData);

    void swapNamingScheme(NamingScheme& namingScheme, NamingScheme& anchorSchemeMatch, NamingScheme &anchorSchemeOther);
    void sortDataByName(std::vector<std::vector<QString> > &data);
};

#endif // FILEREADER_H
