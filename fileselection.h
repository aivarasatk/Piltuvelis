#ifndef FILESELECTION_H
#define FILESELECTION_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QObject>

class FileSelection: public QObject
{
    Q_OBJECT
public:
    explicit FileSelection(QObject *object= nullptr);
    ~FileSelection();

    QString getDataFileName() {return dataFile;}
    QString getIdentificationFileName() {return identificationFile;}
    QGridLayout* getFileSelectLayout() {return gridLayout;}

private slots:
    QString on_selectExcelFile_clicked();
signals:
    void dataFileSelected();

private:
    QGridLayout* gridLayout;

    QLabel* mainFileLabel;
    QLabel* secondaryFileLabel;

    QLineEdit* mainFilePath;
    QLineEdit* secondaryFilePath;

    QPushButton* selectMainFileButton;
    QPushButton* selectSecondaryFileButton;

    QString dataFile, identificationFile;
    QString currentDirectory;

    QString selectFile(QString format);//failo pasirinkimas

    void goToEmitSignal();

};

#endif // FILESELECTION_H
