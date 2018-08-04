#include "exportwindow.h"

#include <QHBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

ExportWindow::ExportWindow(QStringList genOptions, QWidget *parent) : QDialog(parent)
{

    //gen failo opcijos UI
    genSelectionText = new QLabel(text1);
    genSelection = new QComboBox();
    genSelection->addItems(genOptions);

    //eip failo pasirinkimo UI
    optionCheckFiles = new QCheckBox(text2);
    connect(optionCheckFiles, SIGNAL(stateChanged(int)), this, SLOT(changeFileSelectVisability(int)));

    //testi UI
    continueButton = new QPushButton(text5);
    connect(continueButton, SIGNAL(clicked(bool)), this, SLOT(continuePressed()));

    //kuriam layouta
    mainLayout = new QVBoxLayout();
    mainLayout->stretch(0);
    setLayout(mainLayout);

    createLayout();
}

ExportWindow::~ExportWindow(){
    cleanLayout(mainLayout);
}

void ExportWindow::createLayout(){
    QHBoxLayout* genOptionLayout = new QHBoxLayout;
    genOptionLayout->addWidget(genSelectionText);
    genOptionLayout->addWidget(genSelection);

    mainLayout->addLayout(genOptionLayout);

    mainLayout->addWidget(optionCheckFiles);
    mainLayout->addWidget(continueButton, 0, Qt::AlignCenter);
}

void ExportWindow::changeFileSelectVisability(int state){
    if(state == Qt::Checked){//sukuriam pasirinkima
        eipFileSelectionText = new QLabel(text3);
        selectedFileLine = new QLineEdit();
        selectedFileLine->setEnabled(false);
        fileSelectButton = new QPushButton(text4);
        connect(fileSelectButton, SIGNAL(clicked(bool)), this, SLOT(selectFile()));

        QHBoxLayout* fileSelectionLayout = new QHBoxLayout;
        fileSelectionLayout->addWidget(eipFileSelectionText);
        fileSelectionLayout->addWidget(selectedFileLine);
        fileSelectionLayout->addWidget(fileSelectButton);

        mainLayout->insertLayout(mainLayout->indexOf(optionCheckFiles) + 1, fileSelectionLayout, 0);
    }else{//sunaikinam pasirinkima
        delete eipFileSelectionText;
        eipFileSelectionText = nullptr;
        delete selectedFileLine;
        selectedFileLine = nullptr;
        delete fileSelectButton;
        fileSelectButton = nullptr;
        delete mainLayout->itemAt(mainLayout->indexOf(optionCheckFiles) + 1);
    }
}

void ExportWindow::reset(){
    optionCheckFiles->setCheckState(Qt::Unchecked);
    genSelection->setCurrentIndex(0);
}

void ExportWindow::cleanLayout(QLayout* layout){
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

void ExportWindow::selectFile(){
    selectedFile = QFileDialog::getOpenFileName(this, "Išsaugoti failą",QDir::currentPath(), "*.eip");
    if(!selectedFile.isEmpty()){
        selectedFileLine->setText(selectedFile);
    }
}

void ExportWindow::continuePressed(){
    if(optionCheckFiles->checkState() == Qt::Checked && !selectedFileLine->text().isEmpty()){
        genSelectionIndex = genSelection->currentIndex();
        performFileCheck = true;
        emit passControlToMain();
        close();
    }else if(optionCheckFiles->checkState() == Qt::Checked && selectedFileLine->text().isEmpty()){
        QMessageBox::information(this, "Klaida", "Nepasirinkta failas");
    }else if(optionCheckFiles->checkState() == Qt::Unchecked){
        genSelectionIndex = genSelection->currentIndex();
        emit passControlToMain();
        close();
    }
}
