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

#ifndef MWC_QT_WALLET_SEND_H
#define MWC_QT_WALLET_SEND_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../util/address.h"
#include <QSet>

namespace wnd {
class SendStarting;
class SendOnline;
class SendOffline;
}

namespace state {

struct SendEventInfo {
    QString address;
    int64_t txid = -1;
    QString slate;
    int64_t timestamp = 0;

    bool send = false;
    bool respond = false;

    void setData(QString address, int64_t txid,  QString slate, bool send, bool respond);

    // was send and was never respond
    bool isStaleTransaction() const {return send && !respond;}
};

class Send  : public QObject, public State {
    Q_OBJECT
public:
    Send( StateContext * context);
    virtual ~Send() override;

    void destroyOnlineOfflineWnd(wnd::SendStarting * w) { if (w==onlineOfflineWnd) onlineOfflineWnd = nullptr;}
    void destroyOnlineWnd(wnd::SendOnline * w) { if (w==onlineWnd) onlineWnd = nullptr;}
    void destroyOfflineWnd(wnd::SendOffline * w) { if (w==offlineWnd) offlineWnd = nullptr;}

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const core::SendCoinsParams  & params );

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    // onlineOffline => Next step
    void processSendRequest( bool isOnline, const wallet::AccountInfo & selectedAccount, int64_t amount );

    // Request for MWC to send
    void sendMwcOnline( const wallet::AccountInfo & account, util::ADDRESS_TYPE type, QString address, int64_t mwcNano,
            QString message, QString apiSecret, const QStringList & outputs, int changeOutputs, bool fluff );

    void sendMwcOffline(  const wallet::AccountInfo & account, int64_t amount, QString message, QString fileName,
            const QStringList & outputs, int changeOutputs );

    QString getWalletPasswordHash();

    void switchAccount(const QString & accountName);

    bool isNodeHealthy() const {return nodeIsHealthy;}
protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "send.html";}

private slots:
    void onWalletBalanceUpdated();
    void sendRespond( bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc );

    void respSendFile( bool success, QStringList errors, QString fileName );

    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

private:
    void switchToStartingWindow();


private:
    wnd::SendStarting  * onlineOfflineWnd = nullptr;
    wnd::SendOnline         * onlineWnd = nullptr;
    wnd::SendOffline  * offlineWnd = nullptr;

    bool nodeIsHealthy = false;
};

}

#endif
