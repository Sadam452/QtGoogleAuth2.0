#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include<QtCore>
#include<QtXml>
//#include <QPainter>
#include <QJsonObject>
#include <QTextDocumentFragment>
#include <sstream>
#include <QVector>
#include <QThread>
#include <vector>
#include <QJsonValue>
#include <QGraphicsRectItem>
#include <QToolTip>
#include <QSyntaxHighlighter>
#ifdef __unix__
#include <unistd.h>
#endif
#include <QtNetworkAuth>
#include <QOAuth2AuthorizationCodeFlow>
#include <QCryptographicHash>
#include <QDesktopServices>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings settings("LoginExample", "Example");
    settings.beginGroup("login");
    QString email = settings.value("email").toString();
    settings.endGroup();
    if(!email.isEmpty()){
        ui->pushButton->setText("Logout");
        ui->textEdit->setHtml("You are already logged in as<br> <b>"+email+"</b><br>To Logout, click Logout button above.");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    if(ui->pushButton->text() == "Logout"){
        QSettings settings("LoginExample", "Example");
        //QDesktopServices::openUrl(QUrl("https://myaccount.google.com/permissions?continue=https%3A%2F%2Fmyaccount.google.com%2Fsecurity", QUrl::TolerantMode));
        settings.beginGroup("login");
        QString token = settings.value("token").toString();
        settings.endGroup();
        QProcess process;
        process.execute("curl -d -X -POST --header \"Content-type:application/x-www-form-urlencoded\" https://oauth2.googleapis.com/revoke?token="+token);
        settings.beginGroup("login");
        settings.remove("");
        settings.endGroup();
        ui->pushButton->setText("Login");
        ui->textEdit->setHtml("Sorry to see you go! <br>You have logged out successfully!");
    }
    else
        authenticate();
}
void MainWindow::authenticate() {
    googleAuth();
    this->google->grant();
}
void MainWindow::googleAuth()
{
    google = new QOAuth2AuthorizationCodeFlow;
    google->setScope("email");
    connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);

    const QUrl authUri("https://accounts.google.com/o/oauth2/auth");
    const QUrl tokenUri("https://oauth2.googleapis.com/token");
    const auto port = 8080;
    QString clientId = "Your client id here";
    QString clientSecret = "Your Client Secret Here";

    google->setAuthorizationUrl(authUri);
    google->setClientIdentifier(clientId);
    google->setAccessTokenUrl(tokenUri);
    google->setClientIdentifierSharedKey(clientSecret);

    google->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap* parameters) {
        // Percent-decode the "code" parameter so Google can match it
        if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            QByteArray code = parameters->value("code").toByteArray();
            (*parameters)["code"] = QUrl::fromPercentEncoding(code);
        }
    });

    QOAuthHttpServerReplyHandler* replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    google->setReplyHandler(replyHandler);
    connect(this->google, &QOAuth2AuthorizationCodeFlow::granted, [=](){
        const QString token = this->google->token();
        emit gotToken(token);

        auto reply = this->google->get(QUrl("https://www.googleapis.com/oauth2/v2/userinfo?access_token="+token));
        connect(reply, &QNetworkReply::finished, [reply, token, this](){
            const auto objectDetails = (QString)reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(objectDetails.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QString email = jsonObject["email"].toString();
            QString id = jsonObject["id"].toString();
            //save details in QSettings
            QSettings settings("LoginExample", "Example");
            settings.beginGroup("login");
            settings.setValue("token",token);
            settings.setValue("email",email);
            settings.setValue("id",id);
            settings.endGroup();
            ui->pushButton->setText("Logout");
            ui->textEdit->setHtml("You are successfully logged in as<br> <b>"+email+"</b><br>To Logout, click Logout button above.");
        });
    });

}
