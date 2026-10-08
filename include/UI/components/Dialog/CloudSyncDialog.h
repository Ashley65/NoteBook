#ifndef TASKHELPER_CLOUDSYNCDIALOG_H
#define TASKHELPER_CLOUDSYNCDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>

class PocketBaseClient;
class SyncManager;

class CloudSyncDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CloudSyncDialog(PocketBaseClient* client,
                             SyncManager* syncManager,
                             QWidget* parent = nullptr);
    ~CloudSyncDialog() override = default;

private slots:
    void onModeSelected();
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onSyncNowClicked();
    void updateUiState();

private:
    void initUi();
    void setupStyling();

    PocketBaseClient* m_client{nullptr};
    SyncManager* m_syncManager{nullptr};

    // Mode selection
    QRadioButton* m_radioLocal{nullptr};
    QRadioButton* m_radioOfficial{nullptr};
    QRadioButton* m_radioCustom{nullptr};

    // Custom URL input
    QWidget* m_customUrlWidget{nullptr};
    QLineEdit* m_customUrlEdit{nullptr};
    QPushButton* m_saveUrlBtn{nullptr};

    // Auth forms
    QStackedWidget* m_authStack{nullptr};

    // Page 0: Login / Register
    QLineEdit* m_emailEdit{nullptr};
    QLineEdit* m_passwordEdit{nullptr};
    QPushButton* m_loginBtn{nullptr};
    QPushButton* m_registerBtn{nullptr};
    QLabel* m_authErrorLabel{nullptr};

    // Page 1: Logged in status
    QLabel* m_userInfoLabel{nullptr};
    QLabel* m_lastSyncLabel{nullptr};
    QPushButton* m_syncNowBtn{nullptr};
    QPushButton* m_logoutBtn{nullptr};

    // Page 2: Local Only message
    QLabel* m_localOnlyLabel{nullptr};
};

#endif // TASKHELPER_CLOUDSYNCDIALOG_H
