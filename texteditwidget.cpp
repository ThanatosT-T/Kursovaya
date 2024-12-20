#include "texteditwidget.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QGridLayout>

TextEditWidget::TextEditWidget(const QString& fileName, QWidget *parent)
    : QWidget(parent), currentFileName(fileName)
{
    setWindowTitle("Текстовый редактор");
    resize(600, 400);

    textEdit = new QTextEdit(this);
    saveButton = new QPushButton("Сохранить и Закрыть", this);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(textEdit,0,0,1,1);
    layout->addWidget(saveButton,1,0,1,1);
    setLayout(layout);

    connect(saveButton, &QPushButton::clicked, this, &TextEditWidget::saveAndClose);

    loadTextFromFile();
}

TextEditWidget::~TextEditWidget()
{
}

void TextEditWidget::loadTextFromFile() {
    QFile file(currentFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Невозможно открыть файл для чтения");
        return;
    }
    QTextStream in(&file);
    textEdit->setText(in.readAll());
    file.close();
}

void TextEditWidget::saveAndClose()
{
    QFile file(currentFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Невозможно открыть файл для сохранения");
        return;
    }
    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();
    emit fileSaved();
    close();
}
void TextEditWidget::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}
