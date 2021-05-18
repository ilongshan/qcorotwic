#include "api.h"
#include "utils/literals.h"
#include "secrets.h"

#include <qcoro/network.h>
#include <qcoro/coro.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator64>

#include <memory>

using namespace QCoroTwic;
using namespace QCoroTwic::Literals;

namespace {

class AuthBuilder {
public:
    AuthBuilder &setOAuthCallback(const QString &oauth_callback) {
        this->oauth_callback = oauth_callback;
        return *this;
    }

    AuthBuilder &setOAuthToken(const QString &oauth_token) {
        this->oauth_token = oauth_token;
        return *this;
    }

    AuthBuilder &setOAuthVerifier(const QString &oauth_verifier) {
        this->oauth_verifier = oauth_verifier;
        return *this;
    }

    QByteArray build(const QString &endpoint) const {
        const auto timestamp = QDateTime::currentSecsSinceEpoch();
        QMap<QByteArray, QByteArray> params = {
            {"oauth_consumer_key", Secrets::apiKey.toLatin1()},
            {"oauth_signature_method", "HMAC-SHA1"},
            {"oauth_timestamp", QByteArray::number(timestamp)},
            {"oauth_nonce", QByteArray::number(QRandomGenerator64::system()->generate())},
            {"oauth_version", "1.0"}
        };
        if (!oauth_token.isEmpty()) {
            params.insert("oauth_token", oauth_token.toLatin1());
        }
        if (!oauth_callback.isEmpty()) {
            params.insert("oauth_callback", QUrl::toPercentEncoding(oauth_callback));
        }
        if (!oauth_verifier.isEmpty()) {
            params.insert("oauth_verifier", oauth_verifier.toLatin1());
        }

        QByteArrayList list;
        std::transform(params.constKeyValueBegin(), params.constKeyValueEnd(), std::back_inserter(list),
                       [](const auto &it) { return it.first + "=" + it.second; });
        const QByteArray text = "GET&" + QUrl::toPercentEncoding(endpoint) + "&" +
            QUrl::toPercentEncoding(QString::fromLatin1(list.join('&')));

        const auto key = QUrl::toPercentEncoding(Secrets::apiSecret.toString()) + '&' + oauth_token_secret.toLatin1();
        const auto hash = QMessageAuthenticationCode::hash(text, key, QCryptographicHash::Sha1);

        params.insert("oauth_signature", QUrl::toPercentEncoding(QString::fromLatin1(hash.toBase64())));

        return "OAuth " + std::accumulate(params.constKeyValueBegin(), params.constKeyValueEnd(), QByteArray{},
                               [](QByteArray str, const auto it) {
                                if (!str.isEmpty()) {
                                    str += ", ";
                                }
                                return str + it.first + "=\"" + it.second + "\"";
                               });
    }

public:
    QString oauth_callback;
    QString oauth_token;
    QString oauth_token_secret;
    QString oauth_verifier;
};

QMap<QString, QString> parseResponse(const QByteArray &body) {
    QUrlQuery query(QString::fromUtf8(body));
    const auto items = query.queryItems();
    QMap<QString, QString> map;
    for (const auto &item : items) {
        map.insert(item.first, item.second);
    }
    return map;
}


QCoro::Task<Api::Response> authRequest(const QString &endpoint, AuthBuilder authBuilder) {
    QNetworkRequest request{u"https://api.twitter.com%1"_qs.arg(endpoint)};
    request.setRawHeader("Authorization", authBuilder.build(request.url().toString(QUrl::RemoveQuery | QUrl::RemovePort)));

    QNetworkAccessManager qnam;
    std::unique_ptr<QNetworkReply> reply{co_await qnam.get(request)};

    const auto body = co_await qCoro(reply.get()).readAll();
    co_return Api::Response {
        .code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
        .body = body,
        .data = parseResponse(body)
    };
}

} // namespace


QCoro::Task<Api::Response> Api::OAuth::request_token(const QString &oauth_callback) {
    return authRequest(u"/oauth/request_token"_qs, AuthBuilder{}.setOAuthCallback(oauth_callback));
}

QUrl Api::OAuth::authorize_url(const QString &oauth_token) {
    QUrl url(u"https://api.twitter.com/oauth/authorize"_qs);
    QUrlQuery query;
    query.addQueryItem(u"oauth_token"_qs, oauth_token);
    url.setQuery(query);
    return url;
}

QCoro::Task<Api::Response> Api::OAuth::access_token(const QString &oauth_token, const QString &oauth_verifier) {
    return authRequest(u"/oauth/access_token"_qs, AuthBuilder{}.setOAuthToken(oauth_token)
                                                                   .setOAuthVerifier(oauth_verifier));
}


