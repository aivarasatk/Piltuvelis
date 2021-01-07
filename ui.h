#ifndef UI_H
#define UI_H

#include <QMainWindow>
#include "filereader.h"
#include "fileselection.h"
#include "filewriter.h"
#include "exportwindow.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QTreeView>
#include <QStandardItemModel>
#include <set>
#include <QFutureWatcher>
#include <QThread>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadFiles();
    void showSheetSelection();
    void addContinueButton();
    void searchButtonPressed();
    void exportData();
    void checkItemChildren(QStandardItem* item);
    void resetLayout();
    void fitContent(const QModelIndex& index);

signals:
    void filesLoaded();
    void treeViewGenerated();

private:
    QVBoxLayout* vLayout = nullptr;

    //issaugoti pasirinktus failus
    QPushButton* saveSelectedFiles;

    //sheetu pasirinkimas
    QVBoxLayout* sheetDisplayLayout = nullptr;
    QHBoxLayout* sheetCheckBoxLayout = nullptr;
    QLabel* sheetDisplayLabel = nullptr;
    QString sheetDisplayLabelText;
    std::vector<QCheckBox*> sheetCheckBoxes;

    //vykdymo mygtukai
    QHBoxLayout* actionButtonLayout = nullptr;
    QPushButton* searchButton = nullptr;
    QPushButton* exportButton = nullptr;

    ExportWindow* exportWindow;

    //file related
    FileSelection fileSelection;
    std::vector<DataFromSheet> sheetData;
    std::vector<QString> eipTemplate;
    FileReader fileReader;
    FileWriter* fileWriter;

    QTreeView* treeView;
    QStandardItemModel* treeModel;
    bool treeCompleted = false;
    QFutureWatcher<int> futureWatcher;
    QString treeCreationDataError;
    enum TreeCreateError{NO_SHEETS_SELECTED, FAILED_TO_READ_GEN_DATA, BAD_PRODUCT_AMOUNT, DUPLICATE_EXISTANCE, ALL_GOOD};
    bool lockMonth = false;//pasirinkus naujus/visus opcija ieitume i begaline rekursija
    bool uncheckParent = false;

    //new
    bool lockOption = false;
    bool lockChildren = false;
    bool onlyChangeParent = false;
    bool disableChangeParent = false;

    enum CheckCode{CHECK = 0, UNCHECK = 1, IDLE = 2};

    std::vector<std::set<Options>> optionSelectors;

    std::vector<SheetSelection> sheetSelectionCounters;
    enum OptionID{ALL = 0, NEW = 1};

    void loadFileSelection();
    void addFileSaveButton();
    void loadDefaultView();

    void cleanLayout(QLayout* layout);
    void cleanSheetLayoutCheckBoxes();
    void cleanContinueButtons();

    bool checkSheetData(std::vector<std::vector<Duplicate> > &duplicates);
    bool checkForDuplicates(std::map<HeaderDate, std::vector<std::vector<QString> > > &sheet1,
                            std::map<HeaderDate, std::vector<std::vector<QString> > > &sheet2,
                            std::vector<Duplicate> &duplicate);
    QString terminateSearchDueToDuplicate(std::vector<std::vector<Duplicate>>& duplicates);
    bool bothEmpty(std::vector<QString> blockData1,std::vector<QString> blockData2);

    void createTreeView(std::vector<int> selectedSheetsIndexes);
    QStandardItem *addMonthOption(QStandardItem* monthItem, QString text, int& childCount);
    void fillMonthItemWithChildren(std::vector<std::vector<QString> > &blockData, QStandardItem* monthItem,
                                   const HeaderDate &headerDate);
    void colorRow(QStandardItem* item, QString rowItem, int colorDif, int itemIndex);

    void formAmountErrorMessage(const std::vector<ExcelMonthErrors>& amountErrors, QString& amountError);

    std::vector<std::vector<QString> > *newData(std::map<HeaderDate, std::vector<std::vector<QString> > > newSheetData, const HeaderDate& headerDate);

    void expandTreeRoot();
    void addNewOptionItems(std::map<HeaderDate, std::vector<std::vector<QString>>>& newDataBlock,
                                       HeaderDate headerDate, QStandardItem *monthOption);
    void updateOptionSelector(QStandardItem* option);
    void setCheckStateForOptionItems(QStandardItem* item, bool &uncheckParent);
    bool noOptionSelected(QStandardItem* item);
    bool isOption(QStandardItem* item);
    void checkDefaultOptions(QStandardItem* root);
    int getOptionSheetIndex(QStandardItem* item);
    void uncheckOption(QStandardItem* option);

    void createSheetLayout();
    void showContinueButton();

    void setupRootItem(QStandardItem* rootItem, int mapIndex);

    //new tree system
    void uncheckOtherOptions(QStandardItem* item);
    bool isOptionAll(QStandardItem* option);
    void changeCounter(int& counter, bool increase);
    int adjustSelectionCounters(QModelIndex item, int increase, bool optionAll);
    void setParentsPartiallyChecked(QStandardItem* child);
    void changeOption(QStandardItem* option);
    int changeMonthCounter(QStandardItem* monthItem);
    void changeMonth(QStandardItem* monthItem);
    int parentChangeCode(int allRowCount, int currentRowCount);
    void changeParent(int code, QStandardItem* item);
    int countCheckedMonths(std::map<QModelIndex, MonthSelectionCounters> &monthCounters);
    bool allChildrenUnchecked(QStandardItem* parent);
    void changeRoot(QStandardItem* item);

    int visualizeData(QString &treeCreationDataError);
    void parseLogMessage(int message);

    QString exportFileToGen(QString selectedFile, QString extension);
};

#endif // UI_H
