#include "Data/Cloud/PocketBaseClient.h"
#include <QSettings>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDateTime>
#include <QDebug>

PocketBaseClient::PocketBaseClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    loadSettings();
}

PocketBaseClient::ServerMode PocketBaseClient::serverMode() const
{
    return m_serverMode;
}

void PocketBaseClient::setServerMode(ServerMode mode)
{
    if (m_serverMode != mode) {
        m_serverMode = mode;
        saveSettings();
        emit serverModeChanged(m_serverMode);
    }
}

QString PocketBaseClient::customServerUrl() const
{
    return m_customUrl;
}

void PocketBaseClient::setCustomServerUrl(const QString& url)
{
    m_customUrl = url.trimmed();
    while (m_customUrl.endsWith('/')) {
        m_customUrl.chop(1);
    }
    saveSettings();
}

QString PocketBaseClient::officialServerUrl() const
{
    return m_officialUrl;
}

void PocketBaseClient::setOfficialServerUrl(const QString& url)
{
    m_officialUrl = url.trimmed();
    while (m_officialUrl.endsWith('/')) {
        m_officialUrl.chop(1);
    }
    if (m_officialUrl.isEmpty() || m_officialUrl.contains("yourdomain.com")) {
        m_officialUrl = CLOUD_URL;
    }
    saveSettings();
}

QString PocketBaseClient::effectiveServerUrl() const
{
    if (m_serverMode == ServerMode::LocalOnly) {
        return QString();
    }
    if (m_serverMode == ServerMode::CustomServer) {
        return m_customUrl;
    }
    return m_officialUrl;
}

bool PocketBaseClient::isLoggedIn() const
{
    return m_serverMode != ServerMode::LocalOnly && !m_authToken.isEmpty();
}

QString PocketBaseClient::authToken() const
{
    return m_authToken;
}

QString PocketBaseClient::userId() const
{
    return m_userRecord.value("id").toString();
}

QString PocketBaseClient::userEmail() const
{
    return m_userRecord.value("email").toString();
}

QJsonObject PocketBaseClient::currentUserRecord() const
{
    return m_userRecord;
}

void PocketBaseClient::login(const QString& identity, const QString& password)
{
    const QString baseUrl = effectiveServerUrl();
    if (baseUrl.isEmpty()) {
        emit loginFailed(tr("Cloud sync is set to Local Only."));
        return;
    }

    QJsonObject payload;
    payload["identity"] = identity.trimmed();
    payload["password"] = password;

    QNetworkRequest request = createRequest("/api/collections/users/auth-with-password", false);
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();
            m_authToken = obj.value("token").toString();
            m_userRecord = obj.value("record").toObject();
            saveSettings();
            emit loginSucceeded(m_userRecord);
            emit authStateChanged(true);
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            emit loginFailed(err);
        }
    });
}

void PocketBaseClient::registerAccount(const QString& emailOrUsername, const QString& password, const QString& username, const QString& name)
{
    const QString baseUrl = effectiveServerUrl();
    if (baseUrl.isEmpty()) {
        emit registerFailed(tr("Cloud sync is set to Local Only."));
        return;
    }

    QString targetEmail = emailOrUsername.trimmed();
    QString targetUsername = username.trimmed();
    QString targetName = name.trimmed();

    if (!targetEmail.contains('@')) {
        if (targetUsername.isEmpty()) {
            targetUsername = targetEmail;
        }
        targetEmail = QString("%1@flow.local").arg(targetUsername.toLower());
    } else {
        if (targetUsername.isEmpty()) {
            targetUsername = targetEmail.section('@', 0, 0).trimmed();
            targetUsername.remove(QRegularExpression("[^a-zA-Z0-9_]"));
            if (targetUsername.length() < 3) {
                targetUsername = QString("user_%1").arg(QDateTime::currentMSecsSinceEpoch() % 100000);
            }
        }
    }

    if (targetName.isEmpty()) {
        targetName = targetUsername;
    }

    QJsonObject payload;
    payload["username"] = targetUsername;
    payload["email"] = targetEmail;
    payload["password"] = password;
    payload["passwordConfirm"] = password;
    payload["name"] = targetName;

    QNetworkRequest request = createRequest("/api/collections/users/records", false);
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            emit registerSucceeded();
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            emit registerFailed(err);
        }
    });
}

void PocketBaseClient::registerUser(const QString& email, const QString& password)
{
    registerAccount(email, password);
}

void PocketBaseClient::logout()
{
    m_authToken.clear();
    m_userRecord = QJsonObject();
    saveSettings();
    emit loggedOut();
    emit authStateChanged(false);
}

void PocketBaseClient::validateAuth()
{
    if (!isLoggedIn()) return;

    QNetworkRequest request = createRequest("/api/collections/users/auth-refresh", true);
    QNetworkReply* reply = m_networkManager->post(request, QByteArray());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();
            m_authToken = obj.value("token").toString();
            m_userRecord = obj.value("record").toObject();
            saveSettings();
            emit authStateChanged(true);
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
            // Token expired or invalid
            logout();
        }
    });
}

void PocketBaseClient::fetchRecords(const QString& collection, const QString& filter, FetchCallback callback)
{
    QString path = QString("/api/collections/%1/records?perPage=500").arg(collection);
    if (!filter.isEmpty()) {
        path += QString("&filter=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(filter)));
    }

    QNetworkRequest request = createRequest(path, true);
    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonArray items = doc.object().value("items").toArray();
            if (callback) callback(true, items, QString());
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            if (callback) callback(false, QJsonArray(), err);
        }
    });
}

void PocketBaseClient::createRecord(const QString& collection, const QJsonObject& data, RecordCallback callback)
{
    const QString path = QString("/api/collections/%1/records").arg(collection);
    QNetworkRequest request = createRequest(path, true);
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(data).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (callback) callback(true, doc.object(), QString());
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            if (callback) callback(false, QJsonObject(), err);
        }
    });
}

void PocketBaseClient::updateRecord(const QString& collection, const QString& recordId, const QJsonObject& data, RecordCallback callback)
{
    const QString path = QString("/api/collections/%1/records/%2").arg(collection, recordId);
    QNetworkRequest request = createRequest(path, true);
    QNetworkReply* reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(data).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (callback) callback(true, doc.object(), QString());
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            if (callback) callback(false, QJsonObject(), err);
        }
    });
}

void PocketBaseClient::deleteRecord(const QString& collection, const QString& recordId, DeleteCallback callback)
{
    const QString path = QString("/api/collections/%1/records/%2").arg(collection, recordId);
    QNetworkRequest request = createRequest(path, true);
    QNetworkReply* reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            if (callback) callback(true, QString());
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            if (callback) callback(false, err);
        }
    });
}

void PocketBaseClient::uploadAttachment(const QString& workspaceId,
                                        const QString& projectId,
                                        const QString& noteId,
                                        const QString& taskId,
                                        const QString& localFilePath,
                                        const QString& fileName,
                                        RecordCallback callback)
{
    const QString baseUrl = effectiveServerUrl();
    if (baseUrl.isEmpty() || !isLoggedIn()) {
        if (callback) callback(false, QJsonObject(), tr("Not logged in or local only."));
        return;
    }

    auto* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto addTextField = [multiPart](const QString& name, const QString& value) {
        if (value.isEmpty()) return;
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QString("form-data; name=\"%1\"").arg(name)));
        part.setBody(value.toUtf8());
        multiPart->append(part);
    };

    addTextField("workspace", workspaceId);
    addTextField("project", projectId);
    addTextField("note", noteId);
    addTextField("task", taskId);

    auto* file = new QFile(localFilePath, multiPart);
    if (!file->open(QIODevice::ReadOnly)) {
        delete multiPart;
        if (callback) callback(false, QJsonObject(), tr("Cannot read local file for upload: %1").arg(localFilePath));
        return;
    }

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    filePart.setBodyDevice(file);
    multiPart->append(filePart);

    const QString path = "/api/collections/attachments/records";
    const QUrl url(baseUrl + path);
    QNetworkRequest request(url);
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", m_authToken.toUtf8());
    }

    QNetworkReply* reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();
        const QByteArray response = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (callback) callback(true, doc.object(), QString());
        } else {
            const QString err = parseErrorMessage(response, reply->errorString());
            if (callback) callback(false, QJsonObject(), err);
        }
    });
}

void PocketBaseClient::loadSettings()
{
    QSettings settings;
    settings.beginGroup("CloudSync");
    m_serverMode = static_cast<ServerMode>(settings.value("serverMode", static_cast<int>(ServerMode::OfficialCloud)).toInt());
    m_officialUrl = settings.value("officialUrl", CLOUD_URL).toString();
    if (m_officialUrl.isEmpty() || m_officialUrl.contains("yourdomain.com")) {
        m_officialUrl = CLOUD_URL;
    }
    m_customUrl = settings.value("customUrl", LOCAL_URL).toString();
    if (m_customUrl.isEmpty() || m_customUrl.contains("yourdomain.com")) {
        m_customUrl = LOCAL_URL;
    }
    m_authToken = settings.value("authToken").toString();
    const QString userJson = settings.value("userRecord").toString();
    if (!userJson.isEmpty()) {
        m_userRecord = QJsonDocument::fromJson(userJson.toUtf8()).object();
    }
    settings.endGroup();
}

void PocketBaseClient::saveSettings()
{
    QSettings settings;
    settings.beginGroup("CloudSync");
    settings.setValue("serverMode", static_cast<int>(m_serverMode));
    settings.setValue("officialUrl", m_officialUrl);
    settings.setValue("customUrl", m_customUrl);
    settings.setValue("authToken", m_authToken);
    settings.setValue("userRecord", QString::fromUtf8(QJsonDocument(m_userRecord).toJson(QJsonDocument::Compact)));
    settings.endGroup();
}

QNetworkRequest PocketBaseClient::createRequest(const QString& path, bool requireAuth) const
{
    const QString baseUrl = effectiveServerUrl();
    const QString fullPath = path.startsWith('/') ? path : ("/" + path);
    const QUrl url(baseUrl + fullPath);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (requireAuth && !m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", m_authToken.toUtf8());
    }

    return request;
}

QString PocketBaseClient::parseErrorMessage(const QByteArray& responseData, const QString& fallback) const
{
    if (responseData.isEmpty()) {
        return fallback;
    }

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString message = obj.value("message").toString();
        QJsonObject data = obj.value("data").toObject();
        if (!data.isEmpty()) {
            QStringList fieldErrors;
            for (auto it = data.begin(); it != data.end(); ++it) {
                QJsonObject fieldObj = it.value().toObject();
                QString fieldMsg = fieldObj.value("message").toString();
                if (!fieldMsg.isEmpty()) {
                    fieldErrors << QString("%1: %2").arg(it.key(), fieldMsg);
                }
            }
            if (!fieldErrors.isEmpty()) {
                message += " (" + fieldErrors.join("; ") + ")";
            }
        }
        if (!message.isEmpty()) {
            return message;
        }
    }

    return fallback;
}
