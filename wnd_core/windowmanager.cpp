#include "windowmanager.h"
#include "../mwc_wallet/wallet.h"
#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include "../mwc_windows/connect2server.h"
#include "../mwc_windows/nodemanually.h"
#include "../mwc_windows/nodestatus.h"
#include "../mwc_windows/choosewallet.h"
#include "../mwc_windows/newwallet.h"
#include "../mwc_windows/passwordforseed.h"
#include "../mwc_windows/newseed.h"
#include "../mwc_windows/confirmseed.h"
#include "../mwc_windows/enterseed.h"
#include "../mwc_windows/fromseedfile.h"

WindowManager::WindowManager(WalletData & walData, Wallet * wallet, QWidget  * mainWnd ) :
    walletData(walData), mainWindow(mainWnd), mwcWallet(wallet)
{
    Q_ASSERT(mainWindow);
    Q_ASSERT(mwcWallet);
}


void WindowManager::processNextStep( WalletWindowAction action ) {
    Q_ASSERT(currentWnd);
    if (currentWnd==nullptr)
        return;

    if (action == WalletWindowAction::CANCEL) {
        if ( QMessageBox::question(currentWnd, "MWC Wallet", "Are you sure you want to exit MWC Wallet?") ==
             QMessageBox::Yes) {
            // exiting
            QApplication::quit();
        }
        return;
    }

    if (!currentWnd->validateData()){
        qDebug() << "Unable to validate the data for dialog";
        return;
    }

    switch ( currentWnd->getWindowType() ) {
    case WalletWindowType::CONNECT_TO_SERVER: {
        if (walletData.getNodeConnection()->getNode() == NodeConnection::SELECTED) {
            // specify the node by host:port
            switchToWindow( new NodeManually(WalletWindowType::NODE_MANUALLY, mainWindow, walletData.getNodeConnection() ));
            break;
        }
    }
    [[clang::fallthrough]];
    case WalletWindowType::NODE_MANUALLY: {
        switchToWindow( new NodeStatus(WalletWindowType::NODE_STATUS, mainWindow, walletData.getNodeConnection() ));
        break;
    }
    case WalletWindowType::NODE_STATUS: {
        switchToWindow( new PasswordForSeed(WalletWindowType::NEW_PASSWORD, mainWindow, walletData.getSessionPswd() ));
        break;

    }
    case WalletWindowType::NEW_PASSWORD: {
        switchToWindow( new ChooseWallet(WalletWindowType::SELECT_WALLET,mainWindow));
        break;
    }

    case WalletWindowType::SELECT_WALLET: {
        switchToWindow( new NewWallet(WalletWindowType::NEW_WALLET, mainWindow ));
        break;
    }
    case WalletWindowType::NEW_WALLET: {
        NewWallet * wnd = (NewWallet *) currentWnd;
        switch (wnd->getChoice() ){
            case NewWallet::NewWalletChoice::NEW_SEED: {
                // Generate a new seed from the wallet
                //auto seed = mwcWallet->createNewSeed();
                auto seed = QVector<QString>{"local", "donor", "often", "upon", "copper", "minimum",
                                        "message", "gossip", "vendor", "route", "rival", "brick",
                                        "suffer", "gravity", "mom", "daring", "else", "exile",
                                        "brush", "mansion", "shift", "load", "harbor", "close"};
                switchToWindow( new NewSeed(WalletWindowType::NEW_SEED, mainWindow, seed ));
                break;
            }
            case NewWallet::NewWalletChoice::HAVE_SEED: {
                switchToWindow( new EnterSeed(WalletWindowType::ENTER_SEED, mainWindow));
                break;
            }
            case NewWallet::NewWalletChoice::SEED_FILE: {
                switchToWindow( new FromSeedFile(WalletWindowType::FROM_SEED_FILE, mainWindow));
                break;
            }
        }
        break;
    }
    case WalletWindowType::NEW_SEED: {
        auto seed = ((NewSeed*)currentWnd)->getSeed();
        switchToWindow( new ConfirmSeed(WalletWindowType::CONFIRM_SEED, mainWindow, seed ));
        break;
    }
    case WalletWindowType::ENTER_SEED: {
        auto seed = ((EnterSeed*)currentWnd)->getSeed();
        // Done
    }
    case WalletWindowType::CONFIRM_SEED:
    case WalletWindowType::FROM_SEED_FILE:
    default: {
        //Q_ASSERT(false);
        QMessageBox::critical(nullptr, "NOT Implemented!!!", "You are done with wallet setup. The rest is not implemented yet!");
        QApplication::quit();
    }

    }
}

void WindowManager::start() {
    // Normally we shouls go through the data, or start the chain.
    // Let's start the chanin
    switchToWindow( new ConnectToServer(WalletWindowType::CONNECT_TO_SERVER, mainWindow, walletData.getNodeConnection() ));

}

void WindowManager::switchToWindow( WalletWindow * newWindow ) {
    if (currentWnd==newWindow)
        return;

    if (currentWnd!=nullptr) {
        currentWnd->close();
        currentWnd = nullptr;
    }
    if (newWindow==nullptr)
        return;

    currentWnd = newWindow;
    mainWindow->layout()->addWidget(currentWnd);
    currentWnd->show();
}

