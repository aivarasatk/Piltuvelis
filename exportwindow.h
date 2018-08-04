#ifndef EXPORTWINDOW_H
#define EXPORTWINDOW_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
class ExportWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ExportWindow(QStringList genOptions, QWidget *parent = nullptr);
    ~ExportWindow();

    void reset();

signals:
    void passControlToMain();
private slots:
    void changeFileSelectVisability(int state);
    void selectFile();
    void continuePressed();

private:
    int genSelectionIndex = -1;
    bool performFileCheck = false;
    QString selectedFile;
    //pagrindinis isdestymas
    QVBoxLayout* mainLayout;

    //gen failo opcijos UI
    QString text1 = "Pasirinkite į kurį GEN failą kelti: ";
    QLabel* genSelectionText;
    QComboBox* genSelection;

    //eip failo pasirinkimo UI
    QString text2 = "Vykdyti tikrinimą prieš keliant";
    QCheckBox* optionCheckFiles;
    QString text3 = "Pasirinkite mėnesinių + GEN failą:";
    QLabel* eipFileSelectionText;
    QLineEdit* selectedFileLine;
    QString text4 = "Pasirinkti";
    QPushButton* fileSelectButton;

    //testi UI
    QString text5 = "Tęsti";
    QPushButton* continueButton;

    void createLayout();
    void cleanLayout(QLayout* layout);

};

#endif // EXPORTWINDOW_H
