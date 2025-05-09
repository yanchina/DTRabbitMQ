// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

/** \file */

#ifndef RABBITMQ_C_SSL_SOCKET_H
#define RABBITMQ_C_SSL_SOCKET_H

#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/export.h>

AMQP_BEGIN_DECLS

/**
 * Create a new SSL/TLS socket object.
 *
 * The returned socket object is owned by the \ref amqp_connection_state_t
 * object and will be destroyed when the state object is destroyed or a new
 * socket object is created.
 *
 * If the socket object creation fails, the \ref amqp_connection_state_t object
 * will not be changed.
 *
 * The object returned by this function can be retrieved from the
 * amqp_connection_state_t object later using the amqp_get_socket() function.
 *
 * Calling this function may result in the underlying SSL library being
 * initialized.
 * \sa amqp_set_initialize_ssl_library()
 *
 * \param [in,out] state The connection object that owns the SSL/TLS socket
 * \return A new socket object or NULL if an error occurred.
 *
 * \since v0.4.0
 */
AMQP_EXPORT
amqp_socket_t *AMQP_CALL amqp_ssl_socket_new(amqp_connection_state_t state);

/**
 * Get the internal OpenSSL context. Caveat emptor.
 *
 * \param [in,out] self An SSL/TLS socket object.
 *
 * \return A pointer to the internal OpenSSL context. This should be cast to
 * <tt>SSL_CTX*</tt>.
 *
 * \since v0.9.0
 */
AMQP_EXPORT
void *AMQP_CALL amqp_ssl_socket_get_context(amqp_socket_t *self);

/**
 * Set the CA certificate.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] cacert Path to the CA cert file in PEM format.
 *
 * \return \ref AMQP_STATUS_OK on success an \ref amqp_status_enum value on
 *  failure.
 *
 * \since v0.4.0
 */
AMQP_EXPORT
int AMQP_CALL amqp_ssl_socket_set_cacert(amqp_socket_t *self,
                                         const char *cacert);

/**
 * Set the password of key in PEM format.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] passwd The password of key in PEM format.
 *
 * \since v0.11.0
 */
AMQP_EXPORT
void AMQP_CALL amqp_ssl_socket_set_key_passwd(amqp_socket_t *self,
                                              const char *passwd);

/**
 * Set the client key.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] cert Path to the client certificate in PEM foramt.
 * \param [in] key Path to the client key in PEM format.
 *
 * \return \ref AMQP_STATUS_OK on success an \ref amqp_status_enum value on
 *  failure.
 *
 * \since v0.4.0
 */
AMQP_EXPORT
int AMQP_CALL amqp_ssl_socket_set_key(amqp_socket_t *self, const char *cert,
                                      const char *key);

/**
 * Set the client key use the engine.
 *
 * This function requires amqp_set_ssl_engine() has been called.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] cert Path to the client certificate in PEM foramt.
 * \param [in] the key ID.
 *
 * \return \ref AMQP_STATUS_OK on success an \ref amqp_status_enum value on
 *  failure.
 *
 * \since v0.11.0
 */
AMQP_EXPORT
int AMQP_CALL amqp_ssl_socket_set_key_engine(amqp_socket_t *self,
                                             const char *cert, const char *key);

/**
 * Set the client key from a buffer.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] cert Path to the client certificate in PEM foramt.
 * \param [in] key A buffer containing client key in PEM format.
 * \param [in] n The length of the buffer.
 *
 * \return \ref AMQP_STATUS_OK on success an \ref amqp_status_enum value on
 *  failure.
 *
 * \since v0.4.0
 */
AMQP_EXPORT
int AMQP_CALL amqp_ssl_socket_set_key_buffer(amqp_socket_t *self,
                                             const char *cert, const void *key,
                                             size_t n);

/**
 * Enable or disable peer verification.
 *
 * \deprecated use \amqp_ssl_socket_set_verify_peer and
 * \amqp_ssl_socket_set_verify_hostname instead.
 *
 * If peer verification is enabled then the common name in the server
 * certificate must match the server name. Peer verification is enabled by
 * default.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] verify Enable or disable peer verification.
 *
 * \since v0.4.0
 */
AMQP_DEPRECATED_EXPORT void AMQP_CALL
    amqp_ssl_socket_set_verify(amqp_socket_t *self, amqp_boolean_t verify);

/**
 * Enable or disable peer verification.
 *
 * Peer verification validates the certificate chain that is sent by the broker.
 * Hostname validation is controlled by \amqp_ssl_socket_set_verify_peer.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] verify enable or disable peer validation
 *
 * \since v0.8.0
 */
AMQP_EXPORT
void AMQP_CALL amqp_ssl_socket_set_verify_peer(amqp_socket_t *self,
                                               amqp_boolean_t verify);

/**
 * Enable or disable hostname verification.
 *
 * Hostname verification checks the broker cert for a CN or SAN that matches the
 * hostname that amqp_socket_open() is presented. Peer verification is
 * controlled by \amqp_ssl_socket_set_verify_peer
 *
 * \since v0.8.0
 */
AMQP_EXPORT
void AMQP_CALL amqp_ssl_socket_set_verify_hostname(amqp_socket_t *self,
                                                   amqp_boolean_t verify);

typedef enum {
  AMQP_TLSv1 = 1,
  AMQP_TLSv1_1 = 2,
  AMQP_TLSv1_2 = 3,
  AMQP_TLSv1_3 = 4,
  AMQP_TLSvLATEST = 0xFFFF
} amqp_tls_version_t;

/**
 * Set min and max TLS versions.
 *
 * Set the oldest and newest acceptable TLS versions that are acceptable when
 * connecting to the broker. Set min == max to restrict to just that
 * version.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] min the minimum acceptable TLS version
 * \param [in] max the maxmium acceptable TLS version
 * \returns AMQP_STATUS_OK on success, AMQP_STATUS_UNSUPPORTED if OpenSSL does
 * not support the requested TLS version, AMQP_STATUS_INVALID_PARAMETER if an
 * invalid combination of parameters is passed.
 *
 * \since v0.8.0
 */
AMQP_EXPORT
int AMQP_CALL amqp_ssl_socket_set_ssl_versions(amqp_socket_t *self,
                                               amqp_tls_version_t min,
                                               amqp_tls_version_t max);

/**
 * Sets whether rabbitmq-c will initialize OpenSSL.
 *
 * \deprecated Since v0.13.0 this is a no-op. OpenSSL automatically manages
 *    library initialization and uninitialization.
 *
 * OpenSSL requires a one-time initialization across a whole program, this sets
 * whether or not rabbitmq-c will initialize the SSL library when the first call
 * to amqp_ssl_socket_new() is made. You should call this function with
 * do_init = 0 if the underlying SSL library is initialized somewhere else
 * the program.
 *
 * Failing to initialize or double initialization of the SSL library will
 * result in undefined behavior
 *
 * By default rabbitmq-c will initialize the underlying SSL library.
 *
 * NOTE: calling this function after the first socket has been opened with
 * amqp_open_socket() will not have any effect.
 *
 * \param [in] do_initialize If 0 rabbitmq-c will not initialize the SSL
 *                           library, otherwise rabbitmq-c will initialize the
 *                           SSL library
 *
 * \since v0.4.0
 */
AMQP_DEPRECATED_EXPORT
void AMQP_CALL amqp_set_initialize_ssl_library(amqp_boolean_t do_initialize);

/**
 * Initialize the underlying SSL/TLS library.
 *
 * \deprecated Since v0.13.0 this is a no-op. OpenSSL automatically manages
 *    library initialization and uninitialization.
 *
 * The OpenSSL library requires a one-time initialization across the whole
 * program.
 *
 * This function unconditionally initializes OpenSSL so that rabbitmq-c may
 * use it.
 *
 * This function is thread-safe, and may be called more than once.
 *
 * \return AMQP_STATUS_OK on success.
 *
 * \since v0.9.0
 */
AMQP_DEPRECATED_EXPORT
int AMQP_CALL amqp_initialize_ssl_library(void);

/**
 * Set the engine for underlying SSL/TLS library.
 *
 * This function is thread-safe, and may be called more than once.
 *
 * This function requires amqp_initialize_ssl_library() or amqp_ssl_socket_new()
 * has been called.
 *
 * \param [in] engine the engine ID
 * \return AMQP_STATUS_OK on success.
 *
 * \since v0.11.0
 */
AMQP_EXPORT
int amqp_set_ssl_engine(const char *engine);

/**
 * Uninitialize the underlying SSL/TLS library.
 *
 * \deprecated Since v0.13.0 this is a no-op. OpenSSL automatically manages
 *    library initialization and uninitialization.
 *
 * \return AMQP_STATUS_OK on success.
 *
 * \since v0.9.0
 */
AMQP_DEPRECATED_EXPORT
int AMQP_CALL amqp_uninitialize_ssl_library(void);

AMQP_END_DECLS

#endif /* RABBITMQ_C_SSL_SOCKET_H */
