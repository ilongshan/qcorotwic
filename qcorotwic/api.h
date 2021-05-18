#include <qcoro/task.h>

#include <QMap>

namespace QCoroTwic::Api {

struct Response {
    int code;
    QByteArray body;
    QMap<QString, QString> data;
};

namespace OAuth {

QCoro::Task<Response> request_token(const QString &oauth_callback);

QUrl authorize_url(const QString &oauth_token);

QCoro::Task<Response> access_token(const QString &oauth_token, const QString &oauth_verifier);

} // namespace OAuth

}
