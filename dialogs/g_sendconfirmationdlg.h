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

#ifndef SENDCONFIRMATIONDLG_H
#define SENDCONFIRMATIONDLG_H

#include "../control/mwcdialog.h"

namespace Ui {
class SendConfirmationDlg;
}

namespace dlg {

// SendConfirmationDlg is based off of MessageBox but it is more specific and
// intended to handle confirmations for sending or finalizing a transaction.
// This dialog includes a checkbox which allows users to configure whether
// the fluff-phase of the dandelion protocol should be used to sending
// transactions.
// This dialog is very specific about the use of Decline and Confirm buttons.
class SendConfirmationDlg: public control::MwcDialog
{
    Q_OBJECT
public:
    enum class RETURN_CODE {DECLINE, CONFIRM};

    // widthScale - Horizontal scale for the dialog. Sometimes we need it wider.
    explicit SendConfirmationDlg(QWidget *parent, QString title, QString message, double widthScale, QString passwordHash, bool fluffTxn );
    ~SendConfirmationDlg();

public:
    RETURN_CODE getRetCode() const {return retCode;}
    bool getFluffSetting() { return newFluffSetting; }

signals:
    void saveFluffSetting(const bool fluffSetting);

private slots:
    void on_passwordEdit_textChanged(const QString &str);
    void on_declineButton_clicked();
    void on_confirmButton_clicked();

private:

    Ui::SendConfirmationDlg *ui;
    RETURN_CODE retCode = RETURN_CODE::DECLINE;
    QString blockingPasswordHash;

    bool origFluffSetting = false;
    bool newFluffSetting = false;
};

}

#endif // SENDCONFIRMATIONDLG_H