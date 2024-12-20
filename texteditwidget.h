#ifndef TEXTEDITWIDGET_H
#define TEXTEDITWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QString>
#include <QCloseEvent>

class TextEditWidget : public QWidget
{
    Q_OBJECT

public:
    TextEditWidget(const QString& fileName, QWidget *parent = nullptr);
    ~TextEditWidget() override;

signals:
    void fileSaved();

private slots:
    void saveAndClose();

private:
    QTextEdit *textEdit;
    QPushButton *saveButton;
    QString currentFileName;
    void closeEvent(QCloseEvent *event) override;
    void loadTextFromFile();
};

#endif // TEXTEDITWIDGET_H
