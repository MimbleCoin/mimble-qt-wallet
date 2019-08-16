// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core/mainwindow.h"
#include <QApplication>
#include "core/windowmanager.h"
#include "wallet/mwc713.h"
#include "state/state.h"
#include "state/statemachine.h"
#include "core/appcontext.h"
#include "util/ioutils.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "core/global.h"
#include "util/ioutils.h"
#include "util/Log.h"
#include "core/Config.h"
#include "util/ConfigReader.h"
#include <QFileDevice>
#include <QJsonDocument>
#include <QJsonObject>
#include "util/address.h"
#include "dialogs/helpdlg.h"

// Very first run - init everything
bool deployFilesFromResources() {
    QString confPath = ioutils::getAppDataPath();

    QString mwc713conf = confPath + "/wallet713v2.toml";
    QString mwcGuiWalletConf = confPath + "/mwc-gui-wallet-v2.conf";

    bool ok = true;

    if ( !QFile::exists(mwc713conf)) {
        ok = ok && QFile::copy(mwc::MWC713_DEFAULT_CONFIG, mwc713conf);
        if (ok)
            QFile::setPermissions(mwc713conf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    if ( !QFile::exists(mwcGuiWalletConf)) {
        ok = ok && QFile::copy(":/resource/mwc-gui-wallet.conf", mwcGuiWalletConf);
        if (ok)
            QFile::setPermissions(mwcGuiWalletConf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    // Set default values
    config::setMwc713conf(mwc713conf);
    config::setMwcGuiWalletConf(mwcGuiWalletConf);

    return ok;
}

// Read configs
bool readConfig(QApplication & app) {
    QCoreApplication::setApplicationName("mwc-gui-wallet");
    QCoreApplication::setApplicationVersion("v0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("GUI wallet for MWC (MimbleWimbleCoin) https://www.mwc.mw/");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
                              {{"c", "config"},
                                      "Path to the mwc-gui-wallet config ",
                                      "mwc713 path",
                                      ""},
                      });

    parser.process(app);

    QString config = parser.value("config");
    if (config.isEmpty()) {
        config = config::getMwcGuiWalletConf();
    }
    else {
        config::setMwcGuiWalletConf(config);
    }

    util::ConfigReader reader;
    if ( !reader.readConfig(config) ) {
        qDebug() << "Failed to read config file " << config;
        return false;
    }

    QString mwc_path = reader.getString("mwc_path");
    QString wallet713_path = reader.getString("wallet713_path");
    QString main_style_sheet = reader.getString("main_style_sheet");
    QString dialogs_style_sheet = reader.getString("dialogs_style_sheet");
    QString airdropUrl = reader.getString("airdrop_url");

    QString logoutTimeoutStr = reader.getString("logoutTimeout");
    bool logoutTimeoutOk = false;
    int     logoutTimeout = reader.getString("logoutTimeout").toInt(&logoutTimeoutOk);
    if (!logoutTimeoutOk || logoutTimeoutStr.isEmpty() )
        logoutTimeout = 15*60;


    if ( mwc_path.isEmpty() || wallet713_path.isEmpty() || main_style_sheet.isEmpty() || dialogs_style_sheet.isEmpty() || airdropUrl.isEmpty() ) {
        qDebug() << "Failed to read all expected data from config file " << config;
        return false;
    }

    if (wallet713_path == "build in") {
        wallet713_path = QCoreApplication::applicationDirPath() + "/" + "mwc713";
#ifdef Q_OS_WIN
        wallet713_path += ".exe";
#endif
    }

    if (mwc_path == "build in") {
        mwc_path = QCoreApplication::applicationDirPath() + "/" + "mwc";
#ifdef Q_OS_WIN
        mwc_path += ".exe";
#endif
    }

    config::setConfigData( mwc_path, wallet713_path, main_style_sheet, dialogs_style_sheet, airdropUrl, logoutTimeout*1000L );
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // -------------------------------------------
    // Check envoronment variables
/*    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QStringList kk = env.keys();
        QString reportStr = "Envirinment variable for this process at starting point:\n";
        for ( const auto & k : kk ) {
            QString val = env.value( k );
            reportStr +=  "'" + k + "' => '" + val + "'\n";
        }
        QMessageBox::information(nullptr, "Environment variables", reportStr);
    }*/
    // -------------------------------------------


    logger::initLogger();

    if (!deployFilesFromResources() ) {
        QMessageBox::critical(nullptr, "Error", "Unable to provision or verify resource files during the first run");
        return 1;
    }

    if (!readConfig(app) ) {
        QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read configuration");
        return 1;
    }

    qDebug().noquote() << "Starting mwc-gui-wallet with config:\n" << config::toString();


    { // Apply style sheet
        QFile file( config::getMainStyleSheetPath() );
        if (file.open(QFile::ReadOnly | QFile::Text)) {
               QTextStream ts(&file);
               app.setStyleSheet(ts.readAll());
        }
        else {
            QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read the stylesheet.");
            return 1;
        }
    }

    core::AppContext appContext;

    //main window has delete on close flag. That is why need to
    // create dynamically. Window will be deleted on close
    core::MainWindow * mainWnd = new core::MainWindow(nullptr);

    mwc::setApplication(&app, mainWnd);

    wallet::MWC713 wallet( config::getWallet713path(), config::getMwc713conf(), &appContext );

    core::WindowManager wndManager( mainWnd->getMainWindow() );

    mainWnd->show();

    state::StateContext context( &appContext, &wallet, &wndManager, mainWnd );

    state::StateMachine machine(&context);
    mainWnd->setAppEnvironment(&machine, &wallet);
    machine.start();

    app.exec();
  }

