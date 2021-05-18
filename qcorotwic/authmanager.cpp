#include "authmanager.h"
#include "api.h"
#include "utils/literals.h"

#include <qcoro/coro.h>

#include <QTcpServer>
#include <QUrlQuery>
#include <QDesktopServices>

using namespace QCoroTwic;
using namespace QCoroTwic::Literals;

namespace {

static constexpr uint16_t localPort = 61423;
static constexpr uint16_t fallbackLocalPort = 36355;

std::pair<QString, QString> parseCallbackResponse(const QByteArray &statusLine) {
    const auto uri = statusLine.mid(9, statusLine.lastIndexOf(' ') - 9).trimmed();
    const QUrl url{u"http://localhost/%1"_qs.arg(QString::fromUtf8(uri))};
    const QUrlQuery query{url};
    return {query.queryItemValue(u"oauth_token"_qs), query.queryItemValue(u"oauth_verifier"_qs)};
}

} // namespace

QCoro::Task<AccessToken> AuthManager::authenticate() {
    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHost, localPort)) {
        if (!server.listen(QHostAddress::LocalHost, fallbackLocalPort)) {
            qWarning("TCP server failed to start listening on local ports");
            co_return {}; // TODO Propagate error message
        }
    }
    const QString callbackUri = u"http://127.0.0.1:%1/oauth/callback"_qs.arg(server.serverPort());

    // Fetch initial token
    const auto tokenResponse = co_await Api::OAuth::request_token(callbackUri);
    if (tokenResponse.code != 200) {
        qWarning("Requesting token failed with HTTP error %d: %s", tokenResponse.code, qUtf8Printable(tokenResponse.body));
        co_return {}; // TODO: propagate error message
    }

    // Let user authorize the token
    const auto url = Api::OAuth::authorize_url(tokenResponse.data[u"oauth_token"_qs]);
    QDesktopServices::openUrl(url);


    auto *connection = co_await qCoro(server).waitForNewConnection(-1); // don't timeout
    if (!connection) {
        qWarning("Error waiting for incoming OAuth callback.");
        co_return {}; // TODO: propagate error message
    }

    const auto statusLine = co_await qCoro(connection).readLine();
    if (!statusLine.startsWith("GET ")) {
        qWarning("Invalid HTTP callback response: %s", qUtf8Printable(statusLine + co_await qCoro(connection).readAll()));
        co_return {}; // TODO: propagate error message
    }

    const auto [token, verifier] = parseCallbackResponse(statusLine);

    connection->close();

    const auto accessTokenResponse = co_await Api::OAuth::access_token(token, verifier);
    if (accessTokenResponse.code != 200) {
        qWarning("Requesting access token failed with HTTP error %d: %s", accessTokenResponse.code,
                 qUtf8Printable(accessTokenResponse.body));
        co_return {}; // TODO: propagate error message
    }

    co_return accessTokenResponse.data[u"oauth_token"_qs];
}
