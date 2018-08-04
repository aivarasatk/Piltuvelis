#include "fileselection.h"

#include <QDir>
#include <QDebug>
#include <QFileDialog>

FileSelection::FileSelection(QObject *object): QObject(object)
{
    currentDirectory = QDir::currentPath();

    gridLayout = new QGridLayout;
    mainFileLabel = new QLabel();
    secondaryFileLabel = new QLabel();
    mainFilePath = new QLineEdit();
    secondaryFilePath = new QLineEdit();
    selectMainFileButton = new QPushButton();
    selectSecondaryFileButton = new QPushButton();

    mainFileLabel->setText("Pasirinikite gamybos planą: ");
    secondaryFileLabel->setText("Pasirinkite operacijų numerių failą: ");
    mainFilePath->setEnabled(false);
    secondaryFilePath->setEnabled(false);

    selectMainFileButton->setText("Pasirinkti");
    QObject::connect(selectMainFileButton, &QPushButton::clicked,
           [this](){

        QString result = on_selectExcelFile_clicked();
        if(!result.isEmpty()){
        dataFile = result;
        mainFilePath->setText(dataFile);
        goToEmitSignal();
        }
    });

    selectSecondaryFileButton->setText("Pasirinkti");
    QObject::connect(selectSecondaryFileButton, &QPushButton::clicked,
            [this](){
        QString result = on_selectExcelFile_clicked();
        if(!result.isEmpty()){
        identificationFile = result;
        mainFilePath->setText(dataFile);
        secondaryFilePath->setText(identificationFile);
        goToEmitSignal();
        }

    });

    gridLayout->addWidget(mainFileLabel, 0,0);
    gridLayout->addWidget(selectMainFileButton, 0,1);
    gridLayout->addWidget(mainFilePath, 0,2);

    gridLayout->addWidget(secondaryFileLabel, 1,0);
    gridLayout->addWidget(selectSecondaryFileButton, 1,1);
    gridLayout->addWidget(secondaryFilePath, 1,2);


}

FileSelection::~FileSelection()
{

}


QString FileSelection::on_selectExcelFile_clicked()
{
    return selectFile("*.xlsx");
}

QString FileSelection::selectFile(QString format){
    QString selectedFile = QFileDialog::getOpenFileName(nullptr, "Atidarykit failą", currentDirectory, format);
    if(!selectedFile.isEmpty()) {
        QFileInfo fileLocation = QFileInfo(selectedFile);
        currentDirectory = fileLocation.absolutePath();
    }
    return selectedFile;
}

void FileSelection::goToEmitSignal(){
    emit dataFileSelected();
}


