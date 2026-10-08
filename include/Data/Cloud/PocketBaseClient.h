#ifndef TASKHELPER_POCKETBASECLIENT_H
#define TASKHELPER_POCKETBASECLIENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <functional>

class PocketBaseClient : public QObject
{
    Q_OBJECT

public:
    enum class ServerMode
    {
        LocalOnly,
        OfficialCloud,
        CustomServer
    };
    Q_ENUM(ServerMode)

    explicit PocketBaseClient(QObject* parent = nullptr);
    ~PocketBaseClient() override = default;

    // Server URL Constants
    static inline const QString LOCAL_URL = QStringLiteral("http://127.0.0.1:8090");
    static inline const QString LAN_URL   = QStringLiteral("http://192.168.1.198:8090");
    static inline const QString CLOUD_URL = QStringLiteral("https://tnigelproject.co.uk");

    // Server Configuration
    ServerMode serverMode() const;
    void setServerMode(ServerMode mode);

    QString customServerUrl() const;
    void setCustomServerUrl(const QString& url);

    QString officialServerUrl() const;
    void setOfficialServerUrl(const QString& url);

    QString effectiveServerUrl() const;

    // Authentication State
    bool isLoggedIn() const;
    QString authToken() const;
    QString userId() const;
    QString userEmail() const;
    QJsonObject currentUserRecord() const;

    // Auth Actions
    void login(const QString& identity, const QString& password);
    void registerAccount(const QString& emailOrUsername, const QString& password, const QString& username = QString(), const QString& name = QString());
    void registerUser(const QString& email, const QString& password);
    void logout();
    void validateAuth();

    // Generic Collection REST operations
    using FetchCallback = std::function<void(bool success, const QJsonArray& items, const QString& error)>;
    using RecordCallback = std::function<void(bool success, const QJsonObject& record, const QString& error)>;
    using DeleteCallback = std::function<void(bool success, const QString& error)>;

    void fetchRecords(const QString& collection, const QString& filter, FetchCallback callback);
    void createRecord(const QString& collection, const QJsonObject& data, RecordCallback callback);
    void updateRecord(const QString& collection, const QString& recordId, const QJsonObject& data, RecordCallback callback);
    void deleteRecord(const QString& collection, const QString& recordId, DeleteCallback callback);

    // Attachment Upload
    void uploadAttachment(const QString& workspaceId,
                          const QString& projectId,
                          const QString& noteId,
                          const QString& taskId,
                          const QString& localFilePath,
                          const QString& fileName,
                          RecordCallback callback);

signals:
    void loginSucceeded(const QJsonObject& userRecord);
    void loginFailed(const QString& errorMessage);
    void registerSucceeded();
    void registerFailed(const QString& errorMessage);
    void loggedOut();
    void authStateChanged(bool loggedIn);
    void serverModeChanged(ServerMode mode);

private:
    void loadSettings();
    void saveSettings();

    QNetworkRequest createRequest(const QString& path, bool requireAuth = true) const;
    QString parseErrorMessage(const QByteArray& responseData, const QString& fallback) const;

    QNetworkAccessManager* m_networkManager{nullptr};
    ServerMode m_serverMode{ServerMode::OfficialCloud};
    QString m_officialUrl{CLOUD_URL};
    QString m_customUrl{LOCAL_URL};
    QString m_authToken;
    QJsonObject m_userRecord;
};

#endif // TASKHELPER_POCKETBASECLIENT_H
