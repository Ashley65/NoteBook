#include "UI/components/Dialog/CloudSyncDialog.h"
#include "Data/Cloud/PocketBaseClient.h"
#include "Data/Cloud/SyncManager.h"
#include "helpers/WasmClipboardHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>

CloudSyncDialog::CloudSyncDialog(PocketBaseClient* client,
                                 SyncManager* syncManager,
                                 QWidget* parent)
    : QDialog(parent)
    , m_client(client)
    , m_syncManager(syncManager)
{
    setWindowTitle(tr("Cloud Sync & Account"));
    setFixedSize(480, 560);

    initUi();
    setupStyling();
    updateUiState();

    connect(m_client, &PocketBaseClient::authStateChanged, this, &CloudSyncDialog::updateUiState);
    connect(m_client, &PocketBaseClient::loginFailed, this, [this](const QString& err) {
        m_authErrorLabel->setText(err);
        m_loginBtn->setEnabled(true);
        m_registerBtn->setEnabled(true);
    });
    connect(m_client, &PocketBaseClient::registerSucceeded, this, [this]() {
        m_authErrorLabel->setStyleSheet("color: #4ADE80; font-size: 12px;");
        m_authErrorLabel->setText(tr("Account created! Signing in..."));
        onLoginClicked();
    });
    connect(m_client, &PocketBaseClient::registerFailed, this, [this](const QString& err) {
        m_authErrorLabel->setText(err);
        m_loginBtn->setEnabled(true);
        m_registerBtn->setEnabled(true);
    });
    if (m_syncManager) {
        connect(m_syncManager, &SyncManager::lastSyncTimeChanged, this, [this](const QDateTime& dt) {
            m_lastSyncLabel->setText(tr("Last synced: %1").arg(dt.toString("yyyy-MM-dd hh:mm:ss")));
        });
        connect(m_syncManager, &SyncManager::syncFinished, this, [this](bool success, const QString& msg) {
            m_syncNowBtn->setEnabled(true);
            m_syncNowBtn->setText(tr("Sync Now"));
            if (!success) {
                m_authErrorLabel->setText(msg);
            }
        });
    }
}

void CloudSyncDialog::initUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(tr("Cloud & Account Settings"), this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #FFFFFF;");
    mainLayout->addWidget(titleLabel);

    // --- Provider Selection Group ---
    auto* providerGroup = new QGroupBox(tr("Storage Provider"), this);
    auto* providerLayout = new QVBoxLayout(providerGroup);
    providerLayout->setSpacing(10);

    m_radioLocal = new QRadioButton(tr("Local Only (Private browser / disk storage)"), providerGroup);
    m_radioOfficial = new QRadioButton(tr("Official Flow Cloud (Hosted)"), providerGroup);
    m_radioCustom = new QRadioButton(tr("Custom PocketBase Server (Self-Hosted)"), providerGroup);

    auto* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_radioLocal, 0);
    buttonGroup->addButton(m_radioOfficial, 1);
    buttonGroup->addButton(m_radioCustom, 2);

    providerLayout->addWidget(m_radioLocal);
    providerLayout->addWidget(m_radioOfficial);
    providerLayout->addWidget(m_radioCustom);

    // Custom Server URL Row
    m_customUrlWidget = new QWidget(providerGroup);
    auto* customUrlLayout = new QHBoxLayout(m_customUrlWidget);
    customUrlLayout->setContentsMargins(24, 0, 0, 0);
    customUrlLayout->setSpacing(8);

    m_customUrlEdit = new QLineEdit(m_customUrlWidget);
    m_customUrlEdit->setPlaceholderText("http://192.168.1.198:8090");
    m_customUrlEdit->setText(m_client ? m_client->customServerUrl() : "");

    auto* pasteUrlBtn = new QPushButton(tr("Paste"), m_customUrlWidget);
    pasteUrlBtn->setFixedWidth(60);
    connect(pasteUrlBtn, &QPushButton::clicked, this, [this]() {
        QString text = WasmClipboardHelper::getBrowserClipboardText();
        if (!text.isEmpty()) {
            m_customUrlEdit->setText(text.trimmed());
        }
    });

    m_saveUrlBtn = new QPushButton(tr("Save URL"), m_customUrlWidget);
    m_saveUrlBtn->setFixedWidth(90);

    customUrlLayout->addWidget(m_customUrlEdit);
    customUrlLayout->addWidget(pasteUrlBtn);
    customUrlLayout->addWidget(m_saveUrlBtn);
    providerLayout->addWidget(m_customUrlWidget);

    mainLayout->addWidget(providerGroup);

    // --- Stacked Widget for Auth States ---
    m_authStack = new QStackedWidget(this);

    // Page 0: Login / Register Form
    auto* authPage = new QWidget(m_authStack);
    auto* authLayout = new QVBoxLayout(authPage);
    authLayout->setContentsMargins(0, 0, 0, 0);
    authLayout->setSpacing(10);

    auto* accountGroup = new QGroupBox(tr("Sign In or Register"), authPage);
    auto* formLayout = new QVBoxLayout(accountGroup);
    formLayout->setSpacing(12);

    auto* emailRowLayout = new QHBoxLayout();
    emailRowLayout->setSpacing(8);
    m_emailEdit = new QLineEdit(accountGroup);
    m_emailEdit->setPlaceholderText(tr("Email or Username"));
    auto* pasteEmailBtn = new QPushButton(tr("Paste"), accountGroup);
    pasteEmailBtn->setFixedWidth(60);
    connect(pasteEmailBtn, &QPushButton::clicked, this, [this]() {
        QString text = WasmClipboardHelper::getBrowserClipboardText();
        if (!text.isEmpty()) {
            m_emailEdit->setText(text.trimmed());
        }
    });
    emailRowLayout->addWidget(m_emailEdit);
    emailRowLayout->addWidget(pasteEmailBtn);

    auto* passRowLayout = new QHBoxLayout();
    passRowLayout->setSpacing(8);
    m_passwordEdit = new QLineEdit(accountGroup);
    m_passwordEdit->setPlaceholderText(tr("Password"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    auto* pastePassBtn = new QPushButton(tr("Paste"), accountGroup);
    pastePassBtn->setFixedWidth(60);
    connect(pastePassBtn, &QPushButton::clicked, this, [this]() {
        QString text = WasmClipboardHelper::getBrowserClipboardText();
        if (!text.isEmpty()) {
            m_passwordEdit->setText(text);
        }
    });
    passRowLayout->addWidget(m_passwordEdit);
    passRowLayout->addWidget(pastePassBtn);

    m_authErrorLabel = new QLabel(accountGroup);
    m_authErrorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_authErrorLabel->setWordWrap(true);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_loginBtn = new QPushButton(tr("Sign In"), accountGroup);
    m_loginBtn->setStyleSheet("background-color: #4F46E5; color: #FFFFFF; font-weight: bold;");
    m_registerBtn = new QPushButton(tr("Create Account"), accountGroup);

    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);

    formLayout->addLayout(emailRowLayout);
    formLayout->addLayout(passRowLayout);
    formLayout->addWidget(m_authErrorLabel);
    formLayout->addLayout(btnLayout);

    authLayout->addWidget(accountGroup);
    m_authStack->addWidget(authPage);

    // Page 1: Logged In View
    auto* loggedInPage = new QWidget(m_authStack);
    auto* loggedInLayout = new QVBoxLayout(loggedInPage);
    loggedInLayout->setContentsMargins(0, 0, 0, 0);
    loggedInLayout->setSpacing(10);

    auto* statusGroup = new QGroupBox(tr("Account Status"), loggedInPage);
    auto* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(12);

    m_userInfoLabel = new QLabel(statusGroup);
    m_userInfoLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #F8FAFC;");

    m_lastSyncLabel = new QLabel(tr("Last synced: Never"), statusGroup);
    m_lastSyncLabel->setStyleSheet("color: #94A3B8; font-size: 12px;");

    auto* statusBtnLayout = new QHBoxLayout();
    statusBtnLayout->setSpacing(10);

    m_syncNowBtn = new QPushButton(tr("Sync Now"), statusGroup);
    m_syncNowBtn->setStyleSheet("background-color: #4F46E5; color: #FFFFFF; font-weight: bold;");
    m_logoutBtn = new QPushButton(tr("Log Out"), statusGroup);

    statusBtnLayout->addWidget(m_syncNowBtn);
    statusBtnLayout->addWidget(m_logoutBtn);

    statusLayout->addWidget(m_userInfoLabel);
    statusLayout->addWidget(m_lastSyncLabel);
    statusLayout->addLayout(statusBtnLayout);

    loggedInLayout->addWidget(statusGroup);
    m_authStack->addWidget(loggedInPage);

    // Page 2: Local Only info
    auto* localPage = new QWidget(m_authStack);
    auto* localLayout = new QVBoxLayout(localPage);
    localLayout->setContentsMargins(0, 0, 0, 0);

    auto* localGroup = new QGroupBox(tr("Local Storage Active"), localPage);
    auto* localGroupLayout = new QVBoxLayout(localGroup);
    m_localOnlyLabel = new QLabel(
        tr("Your data is stored completely offline and locally in your browser's IndexedDB / local disk. "
           "No data is sent to external servers."), localGroup);
    m_localOnlyLabel->setWordWrap(true);
    m_localOnlyLabel->setStyleSheet("color: #94A3B8; font-size: 13px; line-height: 1.4;");
    localGroupLayout->addWidget(m_localOnlyLabel);
    localLayout->addWidget(localGroup);

    m_authStack->addWidget(localPage);

    mainLayout->addWidget(m_authStack);

    // Bottom Close button
    auto* closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    // Signal connections
    connect(m_radioLocal, &QRadioButton::toggled, this, &CloudSyncDialog::onModeSelected);
    connect(m_radioOfficial, &QRadioButton::toggled, this, &CloudSyncDialog::onModeSelected);
    connect(m_radioCustom, &QRadioButton::toggled, this, &CloudSyncDialog::onModeSelected);

    connect(m_saveUrlBtn, &QPushButton::clicked, this, [this]() {
        if (m_client) {
            m_client->setCustomServerUrl(m_customUrlEdit->text());
            m_authErrorLabel->setStyleSheet("color: #4ADE80; font-size: 12px;");
            m_authErrorLabel->setText(tr("Server URL updated."));
        }
    });

    connect(m_loginBtn, &QPushButton::clicked, this, &CloudSyncDialog::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &CloudSyncDialog::onRegisterClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, &CloudSyncDialog::onLogoutClicked);
    connect(m_syncNowBtn, &QPushButton::clicked, this, &CloudSyncDialog::onSyncNowClicked);
}

void CloudSyncDialog::setupStyling()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #12141C;
            color: #E2E8F0;
        }
        QGroupBox {
            background-color: #1A1C26;
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 10px;
            margin-top: 18px;
            font-weight: bold;
            color: #818CF8;
            padding: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #A5B4FC;
        }
        QRadioButton {
            color: #E2E8F0;
            font-size: 13px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
        QLineEdit {
            background-color: #212433;
            border: 1px solid #2D3142;
            border-radius: 6px;
            padding: 8px 12px;
            color: #FFFFFF;
        }
        QLineEdit:focus {
            border: 1px solid #6366F1;
        }
        QPushButton {
            background-color: #1E202B;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 6px;
            padding: 8px 16px;
            color: #F8FAFC;
        }
        QPushButton:hover {
            background-color: #282C3D;
            border-color: #6366F1;
        }
    )");
}

void CloudSyncDialog::updateUiState()
{
    if (!m_client) return;

    // 1. Sync Radio buttons with client
    switch (m_client->serverMode()) {
    case PocketBaseClient::ServerMode::LocalOnly:
        m_radioLocal->setChecked(true);
        m_customUrlWidget->setVisible(false);
        m_authStack->setCurrentIndex(2); // Local only message
        break;
    case PocketBaseClient::ServerMode::OfficialCloud:
        m_radioOfficial->setChecked(true);
        m_customUrlWidget->setVisible(false);
        break;
    case PocketBaseClient::ServerMode::CustomServer:
        m_radioCustom->setChecked(true);
        m_customUrlWidget->setVisible(true);
        break;
    }

    // 2. Auth stack state for cloud modes
    if (m_client->serverMode() != PocketBaseClient::ServerMode::LocalOnly) {
        if (m_client->isLoggedIn()) {
            m_authStack->setCurrentIndex(1); // Logged in
            m_userInfoLabel->setText(tr("Logged in as:\n%1").arg(m_client->userEmail()));
            if (m_syncManager && !m_syncManager->lastSyncTime().isNull()) {
                m_lastSyncLabel->setText(tr("Last synced: %1").arg(m_syncManager->lastSyncTime().toString("yyyy-MM-dd hh:mm:ss")));
            }
        } else {
            m_authStack->setCurrentIndex(0); // Sign in / Register
            m_authErrorLabel->setText(QString());
        }
    }
}

void CloudSyncDialog::onModeSelected()
{
    if (!m_client) return;

    if (m_radioLocal->isChecked()) {
        m_client->setServerMode(PocketBaseClient::ServerMode::LocalOnly);
    } else if (m_radioOfficial->isChecked()) {
        m_client->setServerMode(PocketBaseClient::ServerMode::OfficialCloud);
    } else if (m_radioCustom->isChecked()) {
        m_client->setServerMode(PocketBaseClient::ServerMode::CustomServer);
    }
    updateUiState();
}

void CloudSyncDialog::onLoginClicked()
{
    if (!m_client) return;
    const QString email = m_emailEdit->text().trimmed();
    const QString pass = m_passwordEdit->text();

    if (email.isEmpty() || pass.isEmpty()) {
        m_authErrorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
        m_authErrorLabel->setText(tr("Please enter both email and password."));
        return;
    }

    m_loginBtn->setEnabled(false);
    m_registerBtn->setEnabled(false);
    m_authErrorLabel->setStyleSheet("color: #94A3B8; font-size: 12px;");
    m_authErrorLabel->setText(tr("Signing in..."));

    m_client->login(email, pass);
}

void CloudSyncDialog::onRegisterClicked()
{
    if (!m_client) return;
    const QString email = m_emailEdit->text().trimmed();
    const QString pass = m_passwordEdit->text();

    if (email.isEmpty() || pass.length() < 8) {
        m_authErrorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
        m_authErrorLabel->setText(tr("Email is required and password must be at least 8 characters."));
        return;
    }

    m_loginBtn->setEnabled(false);
    m_registerBtn->setEnabled(false);
    m_authErrorLabel->setStyleSheet("color: #94A3B8; font-size: 12px;");
    m_authErrorLabel->setText(tr("Creating account..."));

    m_client->registerAccount(email, pass);
}

void CloudSyncDialog::onLogoutClicked()
{
    if (m_client) {
        m_client->logout();
    }
    updateUiState();
}

void CloudSyncDialog::onSyncNowClicked()
{
    if (m_syncManager) {
        m_syncNowBtn->setEnabled(false);
        m_syncNowBtn->setText(tr("Syncing..."));
        m_syncManager->syncNow();
    }
}
