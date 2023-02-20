#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qoauth2authorizationcodeflow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    void gotToken(const QString& token);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    void authenticate();
    void googleAuth();
    QOAuth2AuthorizationCodeFlow * google;
};
#endif // MAINWINDOW_H
