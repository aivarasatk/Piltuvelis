#include "ui.h"
#include "fileselection.h"
#include "parser.h"

#include <QDebug>
#include <QMessageBox>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QStandardItem>
#include <QDir>
#include <QFileDialog>

#include <QThread>
#include <QtConcurrent>
#include <qtconcurrentrun.h>


#include <chrono>
#include <iostream>
using Clock=std::chrono::high_resolution_clock;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), vLayout(new QVBoxLayout)
{
    saveSelectedFiles = new QPushButton("Išsaugoti pasirinkimą");
    sheetDisplayLabelText = "Pasirinkite, kuriuos lapus skaityti:";
    treeView = new QTreeView;
    treeView->setExpandsOnDoubleClick(true);
    treeModel = new QStandardItemModel;
    uncheckParent = false;
    treeView->setModel(treeModel);


    //lygiuojam viska vienas paskui kita
    vLayout->setAlignment(Qt::AlignTop);

    //jei nepavyko nuskaityt eip_template failo, baigiam programa
    if(!fileReader.readEipTemplate("eip_template.txt", eipTemplate)){
        QMessageBox::critical(this, "Klaida", "Nerasta .eip šablonas!");
        exit(1);
    }

    connect(treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(fitContent(QModelIndex)));

    connect(&fileSelection, SIGNAL(dataFileSelected()), this, SLOT(resetLayout()));//pakeitus duomenu faila, isvalom view

    connect(saveSelectedFiles, SIGNAL(clicked(bool)),//nuspaudus mygtuka nuskaitomi keli failai
            this, SLOT(loadFiles()));

    connect(this, filesLoaded,
            [this](){searchButton->setEnabled(true);});

    connect(this, SIGNAL(filesLoaded()),//issaugojus failu pasirinkima rodomi sheet pasitrinkimai
            this, SLOT(showSheetSelection()));

    connect(treeModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(checkItemChildren(QStandardItem*)));

    /*connect(&futureWatcher, &QFutureWatcher<int>::finished, [this](){//sujungiam worker thread pabaiga su erroru spaudinimu
        if(futureWatcher.result() == TreeCreateError::NO_SHEETS_SELECTED){
            QMessageBox::information(this, "Klaida", "Nepasirinkti lapai");
        }else if(futureWatcher.result() == TreeCreateError::FAILED_TO_READ_GEN_DATA){
            QMessageBox::warning(this, "Klaida", "Nepavyko nuskaityt gamybos plano");
        }else if(futureWatcher.result() == TreeCreateError::BAD_PRODUCT_AMOUNT){
            resetLayout();
            QMessageBox::information(this, "Klaida", treeCreationDataError, "Baigti");
        }else if(futureWatcher.result() == TreeCreateError::DUPLICATE_EXISTANCE){
            QMessageBox::critical(this, "Klaida", treeCreationDataError);
            resetLayout();
        }else if(futureWatcher.result() == TreeCreateError::ALL_GOOD){
            exportButton->setEnabled(true);
        }
    });*/

    setWindowTitle("Piltuvėlis");
    resize(1280,720);

    QWidget* widget = new QWidget;
    widget->setLayout(vLayout);
    setCentralWidget(widget);

    loadDefaultView();
}

MainWindow::~MainWindow()
{

}

void MainWindow::loadFileSelection(){
    QGridLayout* fileSelectionLayout = fileSelection.getFileSelectLayout();
    vLayout->addLayout(fileSelectionLayout, 0);
}

void MainWindow::addFileSaveButton(){
    vLayout->addWidget(saveSelectedFiles, 0, Qt::AlignLeft);
}

void MainWindow::loadFiles(){
    resetLayout();
    if(!fileSelection.getDataFileName().isEmpty() &&
       !fileSelection.getIdentificationFileName().isEmpty()){
        //auto t1 = Clock::now();
        std::map<QString, QString> identificationData;
        if(!fileReader.readIdentificationFile(fileSelection.getIdentificationFileName(), identificationData)){
            QMessageBox::warning(this, "Klaida", "Nepavyko nuskaityt operacijų kodų failo");
            return;
        }
        fileWriter = new FileWriter(eipTemplate, identificationData);
        showSheetSelection();
        showContinueButton();
        //qDebug() << "OverAll:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t1).count();
        emit filesLoaded();
    }else{
        QMessageBox::information(this, "Klaida", "Nepasirinkti failai!");
    }

}

void MainWindow::createSheetLayout(){
    sheetDisplayLayout = new QVBoxLayout;
    sheetCheckBoxLayout = new QHBoxLayout;
    sheetCheckBoxLayout->setAlignment(Qt::AlignLeft);

    sheetDisplayLabel = new QLabel(sheetDisplayLabelText);
    sheetDisplayLayout->addWidget(sheetDisplayLabel);
    sheetDisplayLabel->setVisible(false);

    vLayout->insertLayout(2,sheetDisplayLayout);
}

void MainWindow::showSheetSelection(){
    cleanSheetLayoutCheckBoxes();

    fileReader.setExcelFileName(fileSelection.getDataFileName());
    auto& file = fileReader.getExcel();
    QStringList sheetList = file.sheetNames();


    for(int i = 0; i < sheetList.size(); ++i){
        sheetCheckBoxes.push_back(new QCheckBox(sheetList[i]));
        sheetCheckBoxLayout->addWidget(sheetCheckBoxes.back());
    }
    sheetDisplayLayout->addLayout(sheetCheckBoxLayout, 0);

    sheetDisplayLabel->setVisible(true);

}

void MainWindow::cleanLayout(QLayout* layout){
    if(layout == nullptr){
        return;
    }
    QLayoutItem *child;
    QLayout * sublayout;
    QWidget * widget;
    while ((child = layout->takeAt(0))) {
        if ((sublayout = child->layout()) != nullptr) { cleanLayout(sublayout);}
        else if ((widget = child->widget()) != nullptr) {delete widget;}
        else {delete child;}
        delete child;
    }
    layout = nullptr;
}
void MainWindow::cleanSheetLayoutCheckBoxes(){
    //delete sheetDisplayLabel;
    for(auto& item: sheetCheckBoxes){
        delete item;
    }
    sheetCheckBoxes.clear();
    //sheetDisplayLabel = nullptr;
}

void MainWindow::addContinueButton(){
    /*cleanContinueButtons();
    cleanLayout(actionButtonLayout);*/

    actionButtonLayout = new QHBoxLayout;
    actionButtonLayout->setAlignment(Qt::AlignLeft);

    searchButton = new QPushButton("Atvaizduoti");
    searchButton->setEnabled(false);

    connect(searchButton, SIGNAL(clicked(bool)),
            this, SLOT(searchButtonPressed()) );

    exportButton = new QPushButton("Eksportuoti");
    exportButton->setEnabled(false);

    connect(exportButton, SIGNAL(clicked(bool)),
            this, SLOT(exportData()) );

    actionButtonLayout->addWidget(searchButton,0,Qt::AlignLeft);
    actionButtonLayout->addWidget(exportButton,0,Qt::AlignLeft);

    searchButton->setVisible(false);
    exportButton->setVisible(false);

    vLayout->insertLayout(3,actionButtonLayout);
}

void MainWindow::showContinueButton(){
    searchButton->setVisible(true);
    exportButton->setVisible(true);
}

void MainWindow::searchButtonPressed(){
    treeCreationDataError = "";
    //QFuture<int> future = QtConcurrent::run(this, &visualizeData, treeCreationDataError);
    //futureWatcher.setFuture(future);
    int result = visualizeData(treeCreationDataError);
    parseLogMessage(result);
}
int MainWindow::visualizeData(QString& treeCreationDataError){
    //auto t1 = Clock::now();
    sheetData.clear();
    treeModel->clear();
    optionSelectors.clear();
    std::vector<int> selectedSheetsIndexes;
    for(uint i = 0; i < sheetCheckBoxes.size(); ++i){
        if(sheetCheckBoxes[i]->isChecked()){
            selectedSheetsIndexes.push_back(i);
        }
    }
    if(selectedSheetsIndexes.size() == 0){
        return TreeCreateError::NO_SHEETS_SELECTED;
    }
    for(uint i = 0; i < selectedSheetsIndexes.size(); ++i){
        DataFromSheet dataFromSheet;
        if(fileReader.readExcel(fileSelection.getDataFileName(),
                                 dataFromSheet.allDataBlock,
                                 dataFromSheet.newDataBlock,
                                 selectedSheetsIndexes[i])){

            sheetData.push_back(dataFromSheet);
            auto& amountErrors = fileReader.getExcelErrors();
            if(amountErrors.size() != 0){
                formAmountErrorMessage(amountErrors, treeCreationDataError);
            }
        }else{
            sheetData.clear();
            return TreeCreateError::FAILED_TO_READ_GEN_DATA;
        }
        //qDebug() << "PreTreView"<<i<<":" <<std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t1).count();
    }

    if(!treeCreationDataError.isEmpty()){
        QString treeCreationDataError = "Produktai, kuriuose rasta ne skaitinės reikšmės:\n"
                            + treeCreationDataError;
        return TreeCreateError::BAD_PRODUCT_AMOUNT;
    }

    std::vector<std::vector<Duplicate>> duplicates;
    if(!checkSheetData(duplicates)){
        treeCreationDataError = terminateSearch(duplicates);
        return TreeCreateError::DUPLICATE_EXISTANCE;
    }else{
        createTreeView(selectedSheetsIndexes);
        treeCompleted = true;
        return TreeCreateError::ALL_GOOD;
    }
}

void MainWindow::exportData(){
    QString extension = ".eip";
    QString format = "*" + extension;
    QString selectedFile = QFileDialog::getSaveFileName(this, "Išsaugoti failą",QDir::currentPath(), format);

    if(!selectedFile.isEmpty()){
        QStringList genSelection = fileWriter->getGenSelection();
        exportWindow = new ExportWindow(genSelection, this);
        exportWindow->exec();//baigem nustatymus

        QString sheetWriteResult = fileWriter->writeSelectedDataToFile(sheetData, treeModel, selectedFile);
        if(sheetWriteResult.isEmpty()){
            QMessageBox::information(this, "", "Eksportas baigtas");
        }else if(sheetWriteResult == "-1"){
            QMessageBox::critical(this, "Klaida", "Nepavyko sukurti failo");
        }else if(!sheetWriteResult.isEmpty()){
            QMessageBox::warning(this, "Klaida", "Neįrašyti mėnesiai:\n" + sheetWriteResult + "Nėra operacijos kodo");
        }
        delete exportWindow;
    }

}

bool MainWindow::checkSheetData(std::vector<std::vector<Duplicate>>& duplicates){
    if(sheetData.size() == 1){
        return true;
    }

    for(auto singleSheetData = sheetData.begin(); singleSheetData != sheetData.end(); ++singleSheetData){//einam per visus sheet
        for(auto otherSingleSheetData = singleSheetData+1; otherSingleSheetData != sheetData.end(); ++otherSingleSheetData){//tikrinam ar skiritinguose lapuose nera sutampanciu kodu
            if(otherSingleSheetData != singleSheetData){//netikrinam 'saves'
                std::vector<Duplicate> duplicate;
                if(checkForDuplicates(singleSheetData->allDataBlock, otherSingleSheetData->allDataBlock, duplicate)){
                    duplicates.push_back(duplicate);
                }
            }
        }
    }
    if(!duplicates.empty()){
        return false;
    }
    return true;
}

bool MainWindow::checkForDuplicates(std::map<HeaderDate, std::vector<std::vector<QString> > >& sheet1,
                                std::map<HeaderDate, std::vector<std::vector<QString> > >& sheet2,
                                std::vector<Duplicate>& duplicate){

    for(auto& sheetBlock1 : sheet1){
        for(auto& sheetBlock2: sheet2){
            for(auto& blockData: sheetBlock1.second){
                for(auto& blockData2: sheetBlock2.second){//tikrinam bloku kodus
                    if(blockData[(int)DataRow::Result::PRODUCTCODE] == blockData2[(int)DataRow::Result::PRODUCTCODE]){
                        Duplicate tempStruct;

                        std::pair<HeaderDate, std::vector<QString> > tempData, tempData2;
                        tempData = std::make_pair(sheetBlock1.first, blockData);//uzpildom pirma duomeni
                        tempStruct.sheet1 = tempData;

                        tempData2 = std::make_pair(sheetBlock2.first, blockData2);
                        tempStruct.sheet2 = tempData;

                        duplicate.push_back(tempStruct);//irasom i vektoriu
                    }
                }
            }
        }
    }

    if(duplicate.empty()){
        return false;
    }
    return true;
}

QString MainWindow::terminateSearch(std::vector<std::vector<Duplicate>>& duplicates){
    QString errorMessage = "";
    for(auto& duplicate: duplicates){
        for(auto& item: duplicate){
            errorMessage += QString::number(item.sheet1.first.year) + " " + item.sheet1.first.monthString + " ";
            for(auto& string: item.sheet1.second){
                errorMessage += string + " ";
            }
            errorMessage += '\n';

            errorMessage += QString::number(item.sheet2.first.year) + " " + item.sheet2.first.monthString + " ";
            for(auto& string: item.sheet2.second){
                errorMessage += string + " ";
            }
            errorMessage += '\n';
        }
        errorMessage += '\n';
    }
    return errorMessage;
}

void MainWindow::createTreeView(std::vector<int> selectedSheetsIndexes){
    QStringList headerList;
    headerList << "Skyrius" << "Kodas" << "Pavadinimas" << "Padalinys" << "Kiekis" << "Data" << "Pastabos";
    treeModel->setHorizontalHeaderLabels(headerList);

    //std::vector<QFuture<void>> workerSheets;
    for(uint i = 0; i < selectedSheetsIndexes.size(); ++i){
        QStandardItem* root = treeModel->invisibleRootItem();
        QStandardItem* rootItem = new QStandardItem(sheetCheckBoxes[selectedSheetsIndexes[i]]->text());
        root->appendRow(rootItem);//sukuriam sheet pavadinimo gija
        setupRootItem(rootItem, i);
    }
    expandTreeRoot();
    treeView->resizeColumnToContents(0);
}

void MainWindow::loadDefaultView(){
    loadFileSelection();
    addFileSaveButton();
    vLayout->addWidget(treeView);
    createSheetLayout();
    addContinueButton();
}
void MainWindow::checkItemChildren(QStandardItem* item){
    if(!treeCompleted){
        return;
    }
    if(item->isCheckable() && !item->hasChildren() && item->parent() != nullptr){
        if(item->checkState() == Qt::Checked){//atztmim, kitas opcijas
            uncheckOtherOptions(item->parent());//paduodam opcijos adresa.
            //setParentsPartiallyChecked(item);
            int checkedItems = adjustSelectionCounters(item->parent()->parent()->index(), 1, isOptionAll(item->parent()));
            if(!lockChildren){
                changeParent(parentChangeCode(item->parent()->rowCount(), checkedItems), item);
            }

        }else if(item->checkState() == Qt::Unchecked){
            int checkedItems = adjustSelectionCounters(item->parent()->parent()->index(), -1, isOptionAll(item->parent()));
            if(!lockChildren){
                changeParent(parentChangeCode(item->parent()->rowCount(), checkedItems), item);
                /*if(checkedItems != 0){
                    setParentsPartiallyChecked(item);
                }*/
            }

        }
    }else if(item->isCheckable() && isOption(item) && !lockOption){
        changeOption(item);
    }else if(item->isCheckable() && isOption(item->child(0))){//month item
        changeMonth(item);
    }else if(item->isCheckable() && item->parent() == 0){
        changeRoot(item);
    }

    lockMonth = false;
    uncheckParent = false;

    //new method
    lockOption = false;
    lockChildren = false;
    onlyChangeParent = false;
    //disableChangeParent = false;
}

void MainWindow::fillMonthItemWithChildren(std::vector<std::vector<QString>>& blockData, QStandardItem* monthItem,
                                       const HeaderDate& headerDate){
    int colorDif = 0;
    for(auto& blockRows : blockData){
        QList<QStandardItem*> rowItems;
        QStandardItem* checkBox = new QStandardItem("");
        checkBox->setCheckable(true);
        checkBox->setEditable(false);
        rowItems << checkBox;

        int itemIndex = 0;
        for(auto& rowItem : blockRows){
            QStandardItem* item = new QStandardItem();
            colorRow(item, rowItem, colorDif, itemIndex);

            if(itemIndex == (int)DataRow::Result::PRODUCTDATE &&
                    rowItem.isEmpty()){
                rowItem = QString::number(headerDate.year) + "-" + headerDate.monthShortString + "-01";
            }
            if(itemIndex != (int)DataRow::Result::PRODUCTDATE &&
               itemIndex != (int)DataRow::Result::PRODUCTCOMMENTS){
               item->setEditable(false);
            }
            item->setText(rowItem);

            rowItems << item;
            ++itemIndex;
        }
        monthItem->appendRow(rowItems);
        ++colorDif;
    }
}

void MainWindow::colorRow(QStandardItem* item,QString rowItem, int colorDif, int itemIndex){
    QFont font;
    font.setPixelSize(15);
    if(colorDif % 2 == 1){
        QBrush backgroundColor(QColor(211,211,211));
        item->setBackground(backgroundColor);
    }
    if(itemIndex == (int)DataRow::Result::PRODUCTDATE &&
            rowItem.isEmpty()){
        QBrush backgroundColor(QColor(200,0,0));
        item->setBackground(backgroundColor);
    }
    item->setFont(font);
}

void MainWindow::resetLayout(){
    optionSelectors.clear();

    cleanSheetLayoutCheckBoxes();

    cleanContinueButtons();
    sheetDisplayLabel->setVisible(false);

    fileReader.unsetExcelFileName();

    treeModel->clear();
    treeCompleted= false;
}

void MainWindow::cleanContinueButtons(){
    searchButton->setVisible(false);
    searchButton->setEnabled(false);

    exportButton->setVisible(false);
    exportButton->setEnabled(false);
}

void MainWindow::formAmountErrorMessage(const std::vector<ExcelMonthErrors>& amountErrors, QString& amountError){
    for(auto& monthErrors : amountErrors){
        amountError += monthErrors.headerDate.monthString + " " +
                QString::number(monthErrors.headerDate.year) + '\n';
        for(auto& month : monthErrors.error){
            for(QString field : month){
                amountError += field + ' ';
            }
            amountError += '\n';
        }
        amountError += '\n';
    }
}

QStandardItem* MainWindow::addMonthOption(QStandardItem* monthItem, QString text, int& childCount){
    QStandardItem* item = new QStandardItem(text);
    item->setCheckable(true);
    item->setEditable(false);
    monthItem->appendRow(item);
    ++childCount;

    return item;
}

std::vector<std::vector<QString>>* MainWindow::newData(std::map<HeaderDate, std::vector<std::vector<QString>>> newSheetData, const HeaderDate& headerDate){

    for(auto& newMonthItems: newSheetData){
        if(newMonthItems.first == headerDate){
            return &newMonthItems.second;
        }
    }
    return nullptr;
}

void MainWindow::expandTreeRoot(){
    for(int i = 0; i <  treeModel->rowCount(); ++i){
        treeView->expand(treeModel->index(i,0));

    }
}

void MainWindow::addNewOptionItems(std::map<HeaderDate, std::vector<std::vector<QString> > > &newDataBlock,
                                   HeaderDate headerDate, QStandardItem* monthOption){
    std::vector<std::vector<QString>>* newBlockData = nullptr;
    for(auto& newMonthItems: newDataBlock){
        if(newMonthItems.first == headerDate){
            newBlockData = &newMonthItems.second;
        }
    }
    if(newBlockData != nullptr){
        fillMonthItemWithChildren(*newBlockData, monthOption, headerDate);
    }else{
        QStandardItem* item = new QStandardItem("");
        monthOption->appendRow(item);
    }
}
void MainWindow::updateOptionSelector(QStandardItem* option){
    QStandardItem* parent = option->parent();
    QStandardItem* child;
    if(option->checkState() == Qt::Checked){
        for(int index = 0; (child = parent->child(index)) != 0 ; ++index){
            if(child != option){
                child->setCheckState(Qt::Unchecked);
            }
        }
    }else{//atzyvim. viskas nepazymeta
        option->parent()->setCheckState(Qt::Unchecked);//atzymim ir menesi.
    }
}
void MainWindow::fitContent(const QModelIndex& index){
    QStandardItem* item = treeModel->itemFromIndex(index);
    if(item->child(0) != 0 && item->child(0)->child(0) != 0){
        if(item->child(0)->child(0) != 0){
            treeView->resizeColumnToContents(0);
        }else{
            treeView->resizeColumnToContents((int)DataRow::Result::PRODUCTNAME + 1);
        }
    }

}

void MainWindow::setCheckStateForOptionItems(QStandardItem* item, bool& uncheckParent){
    QStandardItem* child;
    bool optionSelected = false;
    for(int i = 0; (child = item->child(i)) != 0; ++i){
        if(child->checkState() == Qt::Checked){//surandam pasirinkta opcija
            for(int j = 0; child->child(j) != 0; ++j){//(at)zymim visus elementus
                child->child(j)->setCheckState(item->checkState());
                optionSelected = true;
            }
        }
    }
    if(!optionSelected && item->checkState() == Qt::Checked){//pazymim visus by default
        for(auto& options: optionSelectors[getOptionSheetIndex(item->child(0))]){
            if(options.optionAll == item->child(0) ||
                    options.optionNew == item->child(0)){//surandam, kuri opcija
                options.optionAll->setCheckState(Qt::Checked);
                for(int j = 0; options.optionAll->child(j) != 0; ++j){//zymim visus elementus
                    options.optionAll->child(j)->setCheckState(Qt::Checked);
                }
            }
        }
    }else if(item->checkState() == Qt::Unchecked){//atzymejus menesi, atzymim teva
        uncheckParent = true;
        item->parent()->setCheckState(Qt::Unchecked);
    }
}

bool MainWindow::noOptionSelected(QStandardItem* item){
    for(int i = 0; item->child(i) != 0; ++i){//iteruojam per menesio elementus
        for(int j = 0; item->child(i)->child(j) != 0; ++j){//iteruojam per menesio opcijas
            if(item->child(i)->child(j)->checkState() == Qt::Checked){
                return false;
            }
        }
    }
    return true;
}

bool MainWindow::isOption(QStandardItem* item){
    for(auto& sheetOptions: optionSelectors){
        for(auto& option: sheetOptions){
            if(option.optionAll == item || option.optionNew == item){
                return true;
            }
        }
    }

    return false;
}

void MainWindow::checkDefaultOptions(QStandardItem* root){
    for(auto& options: optionSelectors[root->index().row()]){
        if(options.optionNew->checkState() == Qt::Unchecked){
            options.optionAll->setCheckState(Qt::Checked);
        }
    }

    QStandardItem* monthItem;
    for(int i = 0; (monthItem = root->child(i)) != 0; ++i){
        monthItem->setCheckState(Qt::Checked);
    }
}

int MainWindow::getOptionSheetIndex(QStandardItem* item){
    for(uint i = 0; i < optionSelectors.size(); ++i){
        for(auto& options: optionSelectors[i]){
            if(options.optionAll == item || options.optionNew == item){
                return i;
            }
        }
    }
}

void MainWindow::uncheckOption(QStandardItem* option){
    option->setCheckState(Qt::Unchecked);

    for(int i = 0; option->child(i) != 0; ++i){//atzymim opcijos vaikus
        option->child(i)->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::setupRootItem(QStandardItem* rootItem, int mapIndex){
    rootItem->setCheckable(true);
    std::set<Options> options;
    SheetSelection sheetSelection;
    auto sheetDataIterator = sheetData.begin() + mapIndex;//rodykle i map struktura
    for(auto& blockData : sheetDataIterator->allDataBlock){
        //qDebug() << mapIndex;
        QStandardItem* monthItem = new QStandardItem(blockData.first.monthString + " " +
                          QString::number(blockData.first.year));
        monthItem->setCheckable(true);
        monthItem->setEditable(false);

        int monthChildren = 0;
        Options option;// rodyt visus, rodyt naujus
        option.optionAll = addMonthOption(monthItem, "Pasirinkti visus", monthChildren);//nustatom rodykle i opcija visus
        fillMonthItemWithChildren(blockData.second, monthItem->child(monthChildren-1), blockData.first);

        if(sheetDataIterator->newDataBlock.size() != 0){//nepridedam opcijos, jei neturim nauju irasu
            option.optionNew = addMonthOption(monthItem, "Pasirinkti naujus", monthChildren);//nustatom rodykle i opcija nauji
            addNewOptionItems(sheetDataIterator->newDataBlock, blockData.first,monthItem->child(monthChildren-1));
        }
        rootItem->appendRow(monthItem);//pridedam menesi i medi
        sheetSelection.monthCounters.insert(std::pair<QModelIndex, MonthSelectionCounters>(monthItem->index(), MonthSelectionCounters()));//menesio skaitikli ivedam
        options.insert(option);//menesiui suteikiam opcijas
    }
    sheetSelectionCounters.push_back(sheetSelection);
    optionSelectors.push_back(options);
}

//*******
void MainWindow::uncheckOtherOptions(QStandardItem* item){
    QStandardItem* parent = item->parent();
    QStandardItem* child;
    for(int i = 0; (child = parent->child(i)) != 0; ++i){
        if(child != item){
            child->setCheckState(Qt::Unchecked);
        }
    }
}

bool MainWindow::isOptionAll(QStandardItem* option){
    if(option->row() == (int)OptionID::ALL){
        return true;
    }
    return false;
}

int MainWindow::adjustSelectionCounters(QModelIndex monthIndex, int increase, bool optionAll){
    for(auto& sheet : sheetSelectionCounters){
        for(auto& monthCounters: sheet.monthCounters){
            if(monthCounters.first == monthIndex){
                if(optionAll){//keiciam 'pasirinkti visus' opcijos counterius
                    if(increase == 1 || increase == -1){
                        monthCounters.second.allCounter += increase;
                    }else if(increase > 1){//pazymeti visi opcijos elementai
                        monthCounters.second.allCounter = increase;
                    }else if (increase < 0){
                        monthCounters.second.allCounter = 0;
                    }
                    return monthCounters.second.allCounter;
                }else{
                    if(increase == 1 || increase == -1){
                        monthCounters.second.newCounter += increase;
                    }else if(increase > 1){//pazymeti visi opcijos elementai
                        monthCounters.second.newCounter = increase;
                    }else if (increase < 0){
                        monthCounters.second.newCounter = 0;
                    }
                    return monthCounters.second.newCounter;
                }
            }
        }
    }
}

void MainWindow::setParentsPartiallyChecked(QStandardItem* child){
    QStandardItem* parent = child;
    for(int i = 0; (parent = parent->parent()) != 0; ++i){
        parent->setCheckState(Qt::PartiallyChecked);
    }
}

void MainWindow::changeOption(QStandardItem* option){
    QStandardItem* child;
    int code = CheckCode::IDLE;
    if(onlyChangeParent){//pakeiciam opcijos teva, nes vaikai visi pazymeti / nei vienas
        if(option->checkState() == Qt::Checked){
            if(isOptionAll(option)){
                code = CheckCode::CHECK;
            }else{
                code = CheckCode::IDLE;
            }

        }else if(option->checkState() == Qt::Unchecked){
            code = CheckCode::UNCHECK;
        }

    }else if(option->checkState() == Qt::Checked){
        uncheckOtherOptions(option);
        for(int i = 0; (child = option->child(i)) != 0; ++i){//check current items
            lockChildren = true;
            child->setCheckState(Qt::Checked);
        }
        if(isOptionAll(option)){
            code = CheckCode::CHECK;
        }else{
            code = CheckCode::IDLE;
        }

    }else if(option->checkState() == Qt::Unchecked){
        for(int i = 0; (child = option->child(i)) != 0; ++i){
            lockChildren = true;
            child->setCheckState(Qt::Unchecked);
        }
        code = CheckCode::UNCHECK;

    }else if(option->checkState() ==  Qt::PartiallyChecked){
        code = CheckCode::IDLE;
    }
    changeParent(code, option);
}

int MainWindow::changeMonthCounter(QStandardItem* monthItem){
    for(auto& sheet : sheetSelectionCounters){
        for(auto& monthOptionCounter : sheet.monthCounters){
            if(monthOptionCounter.first == monthItem->index()){
                if(monthItem->checkState() == Qt::Checked){
                    monthOptionCounter.second.monthChecked = true;
                }else{
                    monthOptionCounter.second.monthChecked = false;
                }
                int counter = countCheckedMonths(sheet.monthCounters);
                sheet.monthCounter = counter;
                return counter;
            }
        }
    }
}

void MainWindow::changeMonth(QStandardItem* monthItem){

    if(onlyChangeParent){//keisim tik menesio counteri
        int amount = -1;
        if(monthItem->checkState() == Qt::Checked){//didinam pazymetu menesiu skaiciu
            amount = changeMonthCounter(monthItem);

        }else if (monthItem->checkState() == Qt::Unchecked || monthItem->checkState() == Qt::PartiallyChecked){//mazinam pazymetu menesiu skaiciu

            amount = changeMonthCounter(monthItem);
            if(monthItem->checkState() == Qt::PartiallyChecked){//nors amount = 0, vis tiek teva reikia partially zymet
                changeParent(CheckCode::IDLE, monthItem);
                return;
            }
        }
        changeParent(parentChangeCode(monthItem->parent()->rowCount(), amount), monthItem);
    }else if(monthItem->checkState() == Qt::Unchecked ||
            monthItem->checkState() == Qt::Checked){//nekreipiam demesio i partially checked (neduoda informacijos)

        for(auto& options: optionSelectors[getOptionSheetIndex(monthItem->child(0))]){
            if(options.optionAll == monthItem->child(0) ||
                    options.optionNew == monthItem->child(0)){//surandam, kuri opcija
                options.optionAll->setCheckState(monthItem->checkState());
                if(monthItem->checkState() == Qt::Checked){
                    int amount = changeMonthCounter(monthItem);//padidinam pazymetu menesiu counteri
                    changeParent(parentChangeCode(monthItem->parent()->rowCount(), amount), monthItem);
                }else{
                    int amount = changeMonthCounter(monthItem);//padidinam pazymetu menesiu counteri
                    changeParent(parentChangeCode(monthItem->parent()->rowCount(), amount), monthItem);
                }
                break;
            }
        }

    }else if(monthItem->checkState() ==  Qt::PartiallyChecked){
        changeMonthCounter(monthItem);
        changeParent(CheckCode::IDLE, monthItem);
    }
}

int MainWindow::parentChangeCode(int allRowCount, int currentRowCount){
    if(currentRowCount == 0){//jei viskas atzymeta
        return CheckCode::UNCHECK;
    }else if(currentRowCount != 0 && allRowCount == currentRowCount){//jei viskas pilnai pazymeta
        return CheckCode::CHECK;
    }
    return CheckCode::IDLE;
}

void MainWindow::changeParent(int code, QStandardItem* item){
    if(disableChangeParent){
        return;
    }

    if(code == CheckCode::CHECK){
        onlyChangeParent = true;
        item->parent()->setCheckState(Qt::Checked);
    }else if(code == CheckCode::UNCHECK && allChildrenUnchecked(item->parent())){
        onlyChangeParent = true;
        item->parent()->setCheckState(Qt::Unchecked);
    }else if(code == CheckCode::IDLE){
        item->parent()->setCheckState(Qt::PartiallyChecked);
    }
}

int MainWindow::countCheckedMonths(std::map<QModelIndex, MonthSelectionCounters>& monthCounters){
    int checkedMonths = 0;
    for(auto& month: monthCounters){
        if(month.second.monthChecked){
            checkedMonths += 1;
        }
    }
    return checkedMonths;
}

bool MainWindow::allChildrenUnchecked(QStandardItem* parent){
    QStandardItem* child;
    for(int i = 0; (child = parent->child(i)); ++i){
        if(child->checkState() == Qt::Checked ||
                child->checkState() == Qt::PartiallyChecked){
            return false;
        }
    }
    return true;
}

void MainWindow::changeRoot(QStandardItem* item){
    if((item->checkState() == Qt::Checked ||
            item->checkState() == Qt::Unchecked) && !onlyChangeParent){
        QStandardItem* child;
        disableChangeParent = true;
        for(int i = 0; (child = item->child(i)) != 0; ++i){
            child->setCheckState(item->checkState());
        }
        disableChangeParent = false;
    }
}

void MainWindow::parseLogMessage(int message){
    if(message == TreeCreateError::NO_SHEETS_SELECTED){
        QMessageBox::information(this, "Klaida", "Nepasirinkti lapai");
    }else if(message== TreeCreateError::FAILED_TO_READ_GEN_DATA){
        QMessageBox::warning(this, "Klaida", "Nepavyko nuskaityt gamybos plano");
    }else if(message == TreeCreateError::BAD_PRODUCT_AMOUNT){
        resetLayout();
        QMessageBox::information(this, "Klaida", treeCreationDataError, "Baigti");
    }else if(message == TreeCreateError::DUPLICATE_EXISTANCE){
        QMessageBox::critical(this, "Klaida", treeCreationDataError);
        resetLayout();
    }else if(message == TreeCreateError::ALL_GOOD){
        exportButton->setEnabled(true);
    }
}
