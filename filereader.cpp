#include "filereader.h"
#include "parser.h"

#include <fstream>
#include <QDebug>
#include <QDate>

#include <chrono>
using Clock=std::chrono::high_resolution_clock;
FileReader::FileReader()
{
}

bool FileReader::readEipTemplate(QString fileName, std::vector<QString> &eipTemplate){
    std::ifstream read;
    read.open(fileName.toStdString());

    if(read.is_open()){
        while(read){
            std::string stdString;
            getline(read, stdString);
            eipTemplate.push_back(QString::fromStdString(stdString));
        }
        read.close();
        return true;
    }else{
        return false;
    }
}

SheetInfo FileReader::readSheetInfo(QXlsx::Document& file){
    SheetInfo sheetInfo;
    DataRowPart dtp;
    int readItemCount = 0;
    int currentColumn = 1;
    while(1){
        QString data = file.read(1, currentColumn).toString().trimmed();
        if(!data.isEmpty()){
            if(data.contains( (char)DataRowPart::SymbolMap::CODE) ){
                sheetInfo.codeColumn = currentColumn;
                Parser::parseSheetInfo(data,sheetInfo);
            }
            else if(data.contains( (char)DataRowPart::SymbolMap::NAME) ){
                sheetInfo.nameColumn = currentColumn;
            }else if(data.contains( (char)DataRowPart::SymbolMap::MAKER) ){
                sheetInfo.makerColumn = currentColumn;
            }


            ++readItemCount;
            if(readItemCount == dtp.elements){
                break;
            }
        }
        if(data.toLower().contains(endDelimiter)){

        }
        ++currentColumn;
    }
    return sheetInfo;
}

bool FileReader::readExcel(QString fileName,
                           std::map<HeaderDate, std::vector<std::vector<QString>>>& allItems,
                           std::map<HeaderDate, std::vector<std::vector<QString>>>& newItems,
                           int sheetIndex){
    excelErrors.clear();
    if(!excelSet){
        file = new QXlsx::Document(fileName);
        excelSet = true;
    }


    QStringList sheetNames = file->sheetNames();

    if(!file->selectSheet(sheetNames[sheetIndex])){
        return false;
    }

    int currentRow = 1;
    int currentColumn = findStartColumn(*file, currentRow);
    if(currentColumn == -1){
        return false;
    }

    readBlocks(allItems, newItems, currentColumn, *file);

    //iterate through months (threads?)
    return true;
}

int FileReader::findStartColumn(QXlsx::Document& file, int currentRow){
    int currentColumn = 1;
    QString data;
    do{
        data = file.read(currentRow,currentColumn).toString().trimmed();
        if(data.contains('#')){
            break;
        }
        ++currentColumn;
    }while(!data.toLower().contains(endDelimiter));

    if(data == endDelimiter){
        return -1;
    }
    return currentColumn;

}

void FileReader::readBlocks(std::map<HeaderDate, std::vector<std::vector<QString>>>& allItems,
                            std::map<HeaderDate, std::vector<std::vector<QString> > > &newItems,
                            int currentColumn, QXlsx::Document& file){

    SheetInfo sheetInfo = readSheetInfo(file);
    std::vector<NamingScheme> namingScheme = getNamingScheme("naming_scheme.txt");
    QString data;

    for(int j = currentColumn;
        !(data = file.read(1, j).toString()).toLower().contains(endDelimiter);
        j += namingScheme.size()){//einam per menesiu blokus
        QString data;
        while((data = file.read(1, j).toString()).isEmpty()){
            ++j;
        }
        if(data.toLower().toLower().contains(endDelimiter)){
                break;
        }
        std::vector<std::vector<QString>> allBlockRows;//duomenu vektorius
        std::vector<std::vector<QString>> newBlockRows;//duomenu vektorius
        excelErrors.push_back(ExcelMonthErrors());
        int additionalColumns = readBlockRows(sheetInfo, j, allBlockRows, newBlockRows, file, namingScheme);

        HeaderDate headerDate;
        Parser::parseBlockHeader(file.read(sheetInfo.blockHeader, j).toString().trimmed(), headerDate);
        if(!allBlockRows.empty()){
            formDataBlock(allItems, allBlockRows, headerDate);
        }
        if(!newBlockRows.empty()){
            formDataBlock(newItems, newBlockRows, headerDate);
        }

        if(excelErrors.back().error.size() > 0){
            ExcelMonthErrors& temp = excelErrors.back();
            temp.headerDate = headerDate;
        }else{
            excelErrors.pop_back();
        }
        j += additionalColumns;
    }
}

int FileReader::readBlockRows(const SheetInfo& sheetInfo, int currentColumn,
                               std::vector<std::vector<QString>>& allBlockRows,
                               std::vector<std::vector<QString>>& newBlockRows, QXlsx::Document& file,
                               std::vector<NamingScheme>& namingScheme){//nuskaitomi menesio bloko duomenys

    int columnsRead = 0;
    ExcelMonthErrors& currentMonthErrors = excelErrors.back();
    bool countColumns = true;
    for(int i = sheetInfo.dataIndex;//iteruojam, stulpeli, kol nestuscias produktas
        !file.read(i, sheetInfo.codeColumn).toString().isEmpty() ||
        !file.read(i, sheetInfo.nameColumn).toString().isEmpty();
        ++i){

        std::vector<CellData*> cellData1;

        readCell(file, i, sheetInfo.codeColumn, cellData1);
        readCell(file, i, sheetInfo.nameColumn, cellData1);
        readCell(file, i, sheetInfo.makerColumn, cellData1);

        uint index = 0;
        std::vector<CellData*> cellData2;
        std::vector<NamingScheme> anchorScheme;
        for(uint j = currentColumn; index < namingScheme.size(); ++j){//einam per menesiu blokus
            QString infoField = file.read(1, j).toString().trimmed();//nuskaitom header eilute
            NamingScheme validator = dataFromScheme(infoField, index, namingScheme);
            if(!infoField.isEmpty() && validator.isValid()){
                anchorScheme.push_back(validator);
                readCell(file, i, j, cellData2);
                ++index;
            }
            if(infoField.toLower().contains(endDelimiter)){
                break;
            }
            if(countColumns){
                ++columnsRead;
            }
        }
        countColumns = false;
        sortByNamingScheme(namingScheme, anchorScheme, cellData2);
        mergeRowData(cellData1, cellData2);
        if(!cellData1[(int)DataRow::Input::PRODUCTAMOUNT1]->data.isEmpty() ||
           !cellData1[(int)DataRow::Input::PRODUCTAMOUNT2]->data.isEmpty()){//jei netusti kiekio stulpeliai
            std::vector<QString> rowData;
            setDataFromCell(cellData1, rowData);//extractinam stringus is cell
            includeDimDate(rowData);
            setDefaultDate(rowData,file.read(sheetInfo.blockHeader, currentColumn).toString().trimmed());
            removeAmountField(rowData, currentMonthErrors);
            allBlockRows.push_back(rowData);
            if(cellData1[(int)DataRow::Input::PRODUCTAMOUNT1]->color == QColor(0,112,192) ||
                    cellData1[(int)DataRow::Input::PRODUCTAMOUNT2]->color == QColor(0,112,192)){
                newBlockRows.push_back(rowData);
            }
        }
        releaseCellDataMemory(cellData1);
        cellData1.clear();
        cellData2.clear();
    }
    if(newBlockRows.empty()){//sutavrko problema, kai kazkuriam menesy nera nauju elementu ir neiterpiama eilute.
        std::vector<QString> empty;
        for(int i = 0; i < 7; ++i){//7 DataRow::Input dydis
            empty.push_back("");
        }
        newBlockRows.push_back(empty);
    }else{
        sortDataByName(newBlockRows);
    }
    if(allBlockRows.empty()){
        std::vector<QString> empty;
        for(int i = 0; i < 7; ++i){//7 DataRow::Input dydis
            empty.push_back("");
        }
        allBlockRows.push_back(empty);
    }else{
        sortDataByName(allBlockRows);

    }


    return columnsRead - namingScheme.size();
}

void FileReader::formDataBlock(std::map<HeaderDate, std::vector<std::vector<QString>>>& dataBlock,
                               std::vector<std::vector<QString>>& blockRows,
                               HeaderDate headerDate){//sukuriam bloko data is header datos ir duomenu eiluciu

    dataBlock.insert(std::pair<HeaderDate, std::vector<std::vector<QString>>>(headerDate,blockRows));

}

bool FileReader::readIdentificationFile(QString fileName, std::map<QString, QString>& identificationData){
    IdentificationColumns idColumns;
    uint currentRow = idColumns.STARTROW;

    QXlsx::Document file(fileName);

    QString operationFileName;
    QString operationCode;
    while(!(operationFileName = file.read(currentRow, idColumns.FILENAME).toString()).isEmpty()){
        if(Parser::correctPlanFormat(operationFileName)){
            operationCode = file.read(currentRow, idColumns.OPERATIONCODE).toString();
            identificationData.insert(std::pair<QString, QString>(operationFileName.trimmed(), operationCode.trimmed()));
        }
        ++currentRow;
    }
    return true;
}

void FileReader::includeDimDate(std::vector<QString> &rowData){
    if(rowData[(int)DataRow::Input::PRODUCTDATE].isEmpty())
        rowData.push_back("");
    else
    {
        QString formattedDate = rowData[(int)DataRow::Input::PRODUCTDATE].replace('.', '-');

        QDate date = QDate::fromString(formattedDate, "yyyy-MM-dd");
        if(!date.isValid() || date.year() == 1900)//default invalid date case
        {
            QString rowValues = "";
            for(auto& v: rowData)
                rowValues.append(v).append('\n');
            throw QString("Klaida nustatant DIM data. Eilutes informacija: \n%1").arg(rowValues);
        }

        rowData.push_back(formattedDate);
    }
}

void FileReader::setDefaultDate(std::vector<QString>& rowData, QString header){
    HeaderDate headerDate;
    Parser::parseBlockHeader(header, headerDate);
    if(!rowData[(int)DataRow::Input::PRODUCTAMOUNT1].isEmpty()){
        rowData[(int)DataRow::Input::PRODUCTDATE] = QString::number(headerDate.year) + "-" + headerDate.monthShortString + "-07";
        if(!rowData[(int)DataRow::Input::PRODUCTAMOUNT2].isEmpty()){
            rowData[(int)DataRow::Input::PRODUCTDATE] = QString::number(headerDate.year) + "-" + headerDate.monthShortString + "-15";
        }
    }else if(!rowData[(int)DataRow::Input::PRODUCTAMOUNT2].isEmpty()){
        rowData[(int)DataRow::Input::PRODUCTDATE] = QString::number(headerDate.year) + "-" + headerDate.monthShortString + "-22";
    }

}
void FileReader::removeAmountField(std::vector<QString>& rowData, ExcelMonthErrors& errors){
    double fullAmount = 0;
    bool* ok = new bool;
    if(!rowData[(int)DataRow::Input::PRODUCTAMOUNT1].isEmpty()){
        fullAmount = rowData[(int)DataRow::Input::PRODUCTAMOUNT1].toDouble(ok);
        if(*ok == false){
            errors.error.push_back(rowData);
        }
    }
    if(!rowData[(int)DataRow::Input::PRODUCTAMOUNT2].isEmpty()){
        fullAmount += rowData[(int)DataRow::Input::PRODUCTAMOUNT2].toDouble(ok);
        if(*ok == false){
            errors.error.push_back(rowData);
        }
    }
    rowData.erase(rowData.begin() + (int)DataRow::Input::PRODUCTAMOUNT2);
    rowData[(int)DataRow::Input::PRODUCTAMOUNT1] = QString::number(fullAmount);
}

std::vector<NamingScheme> FileReader::getNamingScheme(QString fileName){
    std::fstream reader(fileName.toStdString());
    std::vector<NamingScheme> namingVector;
    if(reader.is_open()){
        std::string stdString;
        getline(reader, stdString);
        QString dataLine = QString::fromStdString(stdString);
        QStringList namingList = dataLine.split(';');
        for(auto& item : namingList){
            QStringList data = item.split('-');
            NamingScheme namingScheme{data[0], data[1].toInt()};
            namingVector.push_back(namingScheme);
        }
    }
    return namingVector;
}

void FileReader::sortByNamingScheme(std::vector<NamingScheme> namingScheme, std::vector<NamingScheme> anchorScheme, std::vector<CellData*>& data){
    for(uint i = 0; i < namingScheme.size(); ++i){
        for(uint j = 0; j < namingScheme.size(); ++j){
            if(namingScheme[i].excelNaming == anchorScheme[j].excelNaming &&
               namingScheme[i].index != anchorScheme[j].index){
                swapNamingScheme(namingScheme[i], anchorScheme[j], anchorScheme[namingScheme[i].index - 1]);

                CellData temp = *data[namingScheme[i].index - 1];
                *data[namingScheme[i].index - 1] = *data[j];
                *data[j] = temp;
                break;
            }
        }
    }
}

NamingScheme FileReader::dataFromScheme(QString data,int index, std::vector<NamingScheme>& namingScheme){
    NamingScheme temp = {"", -1};
    for(auto& item : namingScheme){
        if(item.excelNaming == data){
            temp.excelNaming = data;
            temp.index = index + 1;//naming scheme nuo 1

        }
    }
    return temp;
}

void FileReader::mergeRowData(std::vector<CellData*>& rowData01, std::vector<CellData*>& rowData02){
    for(auto& item : rowData02){
        rowData01.push_back(item);
    }
}

void FileReader::setDataFromCell(std::vector<CellData*>& cellData1, std::vector<QString>& rowData){
    for(auto& cell : cellData1){
        rowData.push_back(cell->data);
    }
}
void FileReader::readCell(QXlsx::Document& file, int row, int column, std::vector<CellData*>& cellData){
    QXlsx::Cell* cell = file.cellAt(row, column);

    CellData* tempCellData = new CellData;
    if(cell != nullptr){
        tempCellData->color = cell->format().patternBackgroundColor();
        tempCellData->data = cell->value().toString().trimmed();
    }else{
        tempCellData->data = "";
    }
    cellData.push_back(tempCellData);


}

void FileReader::releaseCellDataMemory(std::vector<CellData*>& cellData){
    for(auto& item : cellData){
        delete item;
        item = nullptr;
    }
}

void FileReader::swapNamingScheme(NamingScheme& namingScheme, NamingScheme& anchorSchemeMatch, NamingScheme& anchorSchemeOther){
    NamingScheme temp = anchorSchemeOther;
    int index = anchorSchemeMatch.index;

    anchorSchemeOther = anchorSchemeMatch;//swap places
    anchorSchemeOther.index = namingScheme.index;//pakeiciam indeksa sarase

    anchorSchemeMatch = temp;//swap 2
    anchorSchemeMatch.index = index;// sugrazinam issaugota indeksa
}

void FileReader::sortDataByName(std::vector<std::vector<QString>>& data){
    for(uint i = 0; i < data.size(); ++i){
        for(uint j = i; j < data.size(); ++j){
            if(data[i][(int)DataRow::Result::PRODUCTNAME] > data[j][(int)DataRow::Result::PRODUCTNAME]){
                auto temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }
}

void FileReader::setExcelFileName(QString fileName){
    file = new QXlsx::Document(fileName);
    excelSet = true;
}

