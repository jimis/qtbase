/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTLSBACKEND_P_H
#define QTLSBACKEND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtnetworkglobal_p.h>

#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslerror.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qssl.h>

#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

#include <vector>
#include <memory>

QT_REQUIRE_CONFIG(ssl);

QT_BEGIN_NAMESPACE

class QByteArray;
class QIODevice;

namespace QSsl {

// Encapsulates key's data or backend-specific
// data-structure, like RSA/DSA/DH structs in OpenSSL.
class TlsKey;

// Abstraction above OpenSSL's X509, or our generic
// 'derData'-based code.
class X509Certificate;

// X509-related auxiliary functions, previously static
// member-functions in different classes.
using X509ChainVerifyPtr = QList<QSslError> (*)(const QList<QSslCertificate> &chain,
                                                const QString &hostName);
using X509PemReaderPtr = QList<QSslCertificate> (*)(const QByteArray &pem, int count);
using X509DerReaderPtr = X509PemReaderPtr;
using X509Pkcs12ReaderPtr = bool (*)(QIODevice *device, QSslKey *key, QSslCertificate *cert,
                                     QList<QSslCertificate> *caCertificates,
                                     const QByteArray &passPhrase);

// TLS over TCP. Handshake, encryption/decryption.
class TlsCryptograph;

// TLS over UDP. Handshake, encryption/decryption.
class DtlsCryptograph;

// DTLS cookie: generation and verification.
class DtlsCookieVerifier;

} // namespace QSsl

// Factory, creating back-end specific implementations of
// different entities QSslSocket is using.
// TLSTODO: consider merging with ... it's own factory
// below, no real benefit in having this split.
class Q_NETWORK_EXPORT QTlsBackend : public QObject
{
    Q_OBJECT
public:
    QTlsBackend();
    ~QTlsBackend() override;

    virtual QString backendName() const;

    // X509 and keys:
    virtual QSsl::TlsKey *createKey() const;
    virtual QSsl::X509Certificate *createCertificate() const;

    // TLS and DTLS:
    virtual QSsl::TlsCryptograph *createTlsCryptograph() const;
    virtual QSsl::DtlsCryptograph *createDtlsCryptograph() const;
    virtual QSsl::DtlsCookieVerifier *createDtlsCookieVerifier() const;

    // X509 machinery:
    virtual QSsl::X509ChainVerifyPtr X509Verifier() const;
    virtual QSsl::X509PemReaderPtr X509PemReader() const;
    virtual QSsl::X509DerReaderPtr X509DerReader() const;
    virtual QSsl::X509Pkcs12ReaderPtr X509Pkcs12Reader() const;

    Q_DISABLE_COPY_MOVE(QTlsBackend)
};

// Factory for a backend.
class Q_NETWORK_EXPORT QTlsBackendFactory : public QObject
{
    Q_OBJECT
public:
    QTlsBackendFactory();
    ~QTlsBackendFactory() override;

    virtual QString backendName() const = 0;
    virtual QTlsBackend *create() const = 0;
    virtual QList<QSsl::SslProtocol> supportedProtocols() const = 0;
    virtual QList<QSsl::SupportedFeature> supportedFeatures() const = 0;
    virtual QList<QSsl::ImplementedClass> implementedClasses() const = 0;

    static QList<QString> availableBackendNames();
    static QString defaultBackendName();
    static QTlsBackend *create(const QString &backendName);

    static QList<QSsl::SslProtocol> supportedProtocols(const QString &backendName);
    static QList<QSsl::SupportedFeature> supportedFeatures(const QString &backendName);
    static QList<QSsl::ImplementedClass> implementedClasses(const QString &backendName);

    // Built-in, this is what Qt provides out of the box (depending on OS):
    static constexpr const int nameIndexSchannel = 0;
    static constexpr const int nameIndexSecureTransport = 1;
    static constexpr const int nameIndexOpenSSL = 2;

    static const QString builtinBackendNames[];

    Q_DISABLE_COPY_MOVE(QTlsBackendFactory)
};

#define QTlsBackendFactory_iid "org.qt-project.Qt.QTlsBackendFactory"
Q_DECLARE_INTERFACE(QTlsBackendFactory, QTlsBackendFactory_iid);


QT_END_NAMESPACE

#endif // QTLSBACKEND_P_H
