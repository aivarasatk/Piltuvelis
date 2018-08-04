#ifndef STUCTURES_H
#define STUCTURES_H

#include <QString>
#include <QStandardItem>
#include <map>
#include <QDebug>
struct SheetInfo{
    int blockHeader;
    int codeColumn;
    int nameColumn;
    int makerColumn;
    int dataIndex;
};

struct NamingScheme{
    QString excelNaming;
    int index;

    bool isValid(){
        if(!excelNaming.isEmpty() && index != -1){
            return true;
        }
        return false;
    }

    void operator=(const NamingScheme& rhs){
        excelNaming = rhs.excelNaming;
        index = rhs.index;
    }
};

struct DataRowPart{
    enum class SymbolMap : char{MAKER = 'G', CODE = 'K', NAME = 'P'};
    int elements = 3;
};

struct CellData{
    QColor color;
    QString data;

    ~CellData(){}

    void operator=(const CellData& rhs){
        color = rhs.color;
        data = rhs.data;
    }
};

struct HeaderDate{
  QString defaultDate = "";
  QString monthString;
  QString monthShortString;
  int monthInt;
  int year;



   bool operator <(const HeaderDate& right) const{
      if(this->year < right.year){
          return true;
      }else if(this->year == right.year &&
               this->monthInt < right.monthInt){
          return true;
      }
      return false;
   }
   bool operator==(const HeaderDate& right)const{
       if(this->monthInt == right.monthInt &&
          this->monthString == right.monthString &&
               this->year == right.year &&
               this->defaultDate == right.defaultDate){
           return true;
       }
       return false;
   }
};

struct DataRow{
    enum class Input{PRODUCTCODE, PRODUCTNAME, PRODUCTMAKER, PRODUCTAMOUNT1, PRODUCTAMOUNT2,
         PRODUCTDATE, PRODUCTCOMMENTS};
    enum class Result{PRODUCTCODE, PRODUCTNAME, PRODUCTMAKER, PRODUCTAMOUNT,
            PRODUCTDATE, PRODUCTCOMMENTS};
};

struct ExcelMonthErrors{
    std::vector<std::vector<QString>> error;
    HeaderDate headerDate;
};

struct IdentificationColumns{
  uint STARTROW = 3;
  uint OPERATIONCODE = 1;
  uint FILENAME = 3;
};

struct Duplicate{
    std::pair<HeaderDate, std::vector<QString> > sheet1;
    std::pair<HeaderDate, std::vector<QString> > sheet2;
};

struct DataFromSheet{
    std::map<HeaderDate, std::vector<std::vector<QString> > > allDataBlock;
    std::map<HeaderDate, std::vector<std::vector<QString> > > newDataBlock;
};

struct TreeViewMonthItem{
  bool parentChecked;
  std::vector<int> block;
};

struct TreeViewSheetItem{
    bool parentChecked;
    std::vector<TreeViewMonthItem> months;
};

struct Options{
    QStandardItem* optionAll;
    QStandardItem* optionNew;

    bool operator <(const Options& rhs) const{
        if(optionAll < rhs.optionAll){
            return true;
        }
        return false;
    }
};
struct MonthSelectionCounters{
    bool monthChecked = false;
    int allCounter = 0;
    int newCounter = 0;
};
struct SheetSelection{
    int monthCounter = 0;
    std::map<QModelIndex, MonthSelectionCounters> monthCounters;
};

#endif // STUCTURES_H
