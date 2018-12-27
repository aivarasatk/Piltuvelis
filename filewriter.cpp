#include "filewriter.h"
#include "parser.h"

#include <QTextStream>
#include <QDebug>
FileWriter::FileWriter(std::vector<QString> eipTemplateV,
                       std::map<QString, QString> identificationData):
    eipTemplate(eipTemplateV), idData(identificationData)
{

}

QString FileWriter::writeDataToFile(std::vector<DataFromSheet>& sheetData,
                                    const QStandardItemModel *treeModel, QString fileName, QString genCode){
    if(genCode != nullptr){
        writeToGenExport = true;
        genExportCode = genCode;
    }else{
        writeToGenExport = false;
        genExportCode = "KLAIDA";
    }
    file.setFileName(fileName);
    if(file.open(QIODevice::WriteOnly)){
        QString failedSheetMonths = "";
        QTextStream out(&file);
        out << "<I06>\n";
        out.flush();
        for(uint i = 0; i < sheetData.size(); ++i){
            if(treeModel->item(i)->checkState() == Qt::Checked){//irasysim visus sheete pazymetas menesio opcijas
                QString sheetWriteResult = writeSheetData(sheetData[i], treeModel->item(i));

                if(!sheetWriteResult.isEmpty()){
                    failedSheetMonths += treeModel->item(i)->text() + ":\n";
                    failedSheetMonths += sheetWriteResult;
                }
            }else{
                if(treeModel->item(i)->hasChildren()){

                    QString sheetWriteResult = analyzeSheetData(sheetData[i], treeModel->item(i));

                    if(!sheetWriteResult.isEmpty()){
                        failedSheetMonths += treeModel->item(i)->text( )+ ":\n";
                        failedSheetMonths += sheetWriteResult;
                    }
                }
            }
        }
        out << "</I06>";
        out.flush();

        file.close();
        return failedSheetMonths;
    }else {
        return "-1";
    }
}

QString FileWriter::analyzeSheetData(DataFromSheet& sheetData,
                                  QStandardItem* root){
    QString failedMonths = "";
    QStandardItem* monthItem;
    for(int i = 0; (monthItem = root->child(i)) != nullptr; ++i){//einam per menesius
        if(monthItem->checkState() == Qt::Checked){//irasysim visa menesi
            QStandardItem* option;
            for(int j = 0; (option = monthItem->child(j)) != nullptr; ++j){//einam per pasirinkimus
                if(option->checkState() == Qt::Checked){
                    QString writeResult = "";
                    if(j == Options::ALL){//irasom visus menesio irasus
                        auto data = getMonthPair(sheetData.allDataBlock, i);
                        writeResult = writeMonthData(data, option);
                    }else if(j == Options::NEW){//irasom naujus menesio irasus
                        auto data = getMonthPair(sheetData.newDataBlock, i);
                        writeResult = writeMonthData(data, option);
                    }
                    if(!writeResult.isEmpty()){
                        failedMonths += writeResult;
                    }
                    break;
                }
            }
        }else if(monthItem->checkState() == Qt::PartiallyChecked){//irasom dalinai pazymeta menesi
            QStandardItem* option;
            QString writeResult = "";
            for(int j = 0; (option = monthItem->child(j)) != nullptr; ++j){
                if(option->checkState() != Qt::Unchecked && j == Options::ALL){
                    auto dataPair = getMonthPair(sheetData.allDataBlock, i);
                    writeResult = analyzeMonthData(dataPair, option);
                }else if (option->checkState() != Qt::Unchecked && j == Options::NEW){
                    auto dataPair = getMonthPair(sheetData.newDataBlock, i);
                    writeResult = analyzeMonthData(dataPair, option);
                }
            }
            if(!writeResult.isEmpty()){
                failedMonths += writeResult;
            }
        }
    }
    return failedMonths;
}

QString FileWriter::analyzeMonthData(std::pair<const HeaderDate, std::vector<std::vector<QString> > >& monthItem,
                                  QStandardItem* monthTreeItem){

    QString operationCode = Parser::operationCode(monthItem.first, idData);
   // qDebug() << operationCode;
    for(uint h = 0; h < monthItem.second.size(); ++h){
        if(monthTreeItem->child(h)->checkState() == Qt::Checked){//rasom viena irasa

            if(!operationCode.isEmpty()){
                writeItemData(monthItem.second[h], operationCode, monthTreeItem, h);
            }else{
                return QString(monthItem.first.monthString + QString::number(monthItem.first.year) + '\n');
            }
        }
    }
    return "";

}

QString FileWriter::writeSheetData(DataFromSheet& singleSheetData,
                                QStandardItem* root){
    QString failedMonths = "";
    QStandardItem* monthItem;
    for(int i = 0; (monthItem = root->child(i)) != nullptr; ++i){
        if(monthItem->checkState() == Qt::Checked){
            QStandardItem* option;
            for(int j = 0; (option = monthItem->child(j)) != nullptr; ++j){
                if(option->checkState() == Qt::Checked){
                    QString monthWriteResult = "";
                    if(j == Options::ALL){
                        auto data = getMonthPair(singleSheetData.allDataBlock, i);
                        monthWriteResult = writeMonthData(data, option);
                    }else if(j == Options::NEW){
                        auto data = getMonthPair(singleSheetData.newDataBlock, i);
                        monthWriteResult = writeMonthData(data, option);
                    }
                    if(!monthWriteResult.isEmpty()){
                          failedMonths += monthWriteResult;
                    }
                    break;
                }
            }
        }
    }

    return failedMonths;
}

QString FileWriter::writeMonthData(std::pair<const HeaderDate, std::vector<std::vector<QString> > >& monthItem,
                                QStandardItem* month){
    QString operationCode = Parser::operationCode(monthItem.first, idData);
    if(!operationCode.isEmpty()){
        int index = 0;
        for(auto& dataRow : monthItem.second){
            writeItemData(dataRow, operationCode, month, index);
            ++index;
        }
        return "";
    }else{
        return QString(monthItem.first.monthString + QString::number(monthItem.first.year) + '\n');
    }
}

void FileWriter::writeItemData(std::vector<QString> rowData, QString operationCode, QStandardItem* item, int child){
    QString itemBlock = "";

    itemBlock += "<I07>\n";//iraso pradzia

    if(!writeToGenExport){
        appendDataBlock(itemBlock, eipTemplate[EipID::OPERATIONCODE], operationCode);
    }else{
        appendDataBlock(itemBlock, eipTemplate[EipID::OPERATIONCODE], genExportCode);
    }

    appendDataBlock(itemBlock, eipTemplate[EipID::CODE], rowData[(int)DataRow::Result::PRODUCTCODE]);
    appendDataBlock(itemBlock, eipTemplate[EipID::AMOUNT], rowData[(int)DataRow::Result::PRODUCTAMOUNT]);
    appendDataBlock(itemBlock, eipTemplate[EipID::MAKER], rowData[(int)DataRow::Result::PRODUCTMAKER]);

    appendDataBlock(itemBlock, eipTemplate[EipID::DATE],
            item->child(child,(int)DataRow::Result::PRODUCTDATE + 1)->text());
    appendDataBlock(itemBlock, eipTemplate[EipID::DETAILS],
            item->child(child,(int)DataRow::Result::PRODUCTCOMMENTS + 1)->text());

    QString pillCount = isPill(rowData[(int)DataRow::Result::PRODUCTNAME]);
    if(!pillCount.isEmpty()){
        appendDataBlock(itemBlock, eipTemplate[EipID::COST], pillCount);
    }
    itemBlock += "</I07>\n";//iraso pabaiga

    QTextStream out(&file);
    out << itemBlock;
    out.flush();
}

void FileWriter::appendDataBlock(QString& itemBlock,QString exportRow,const QString& value){
    itemBlock += Parser::insertValueToExport(exportRow, value);
    itemBlock += '\n';
}

QString FileWriter::isPill(QString name){
    int searchIndex = 0;
    int index;
    QString pillCount = "";
    while((index = name.indexOf('N', searchIndex)) != -1){
        while(name[index+1] >= '0' && name[index+1] <= '9'){
            pillCount += name[index+1];
            index++;
        }
        if(!pillCount.isEmpty()){
            break;
        }
        searchIndex = index + 1;
    }
    return pillCount;
}

std::pair<const HeaderDate, std::vector<std::vector<QString>>> FileWriter::getMonthPair(std::map<HeaderDate, std::vector<std::vector<QString>>>& dataBlock,
                                                                                  int i){
    int counter = 0;
    for(auto& blockPair : dataBlock){
        if(counter == i){
            return blockPair;
        }
        ++counter;
    }
    return std::pair<const HeaderDate, std::vector<std::vector<QString>>>();
}

QStringList FileWriter::getGenSelection(){
    QStringList list;
    for(auto& pair: idData){
        if(pair.first.contains("GEN")){
            list << pair.first;
        }
    }
    return list;
}

QStringList FileWriter::getGenCodes(){
    QStringList list;
    for(auto& pair: idData){
        if(pair.first.contains("GEN")){
            list << pair.second;
        }
    }
    return list;
}
