/*
 * p11test_case_common.h: Functions shared between test cases.
 *
 * Copyright (C) 2016, 2017 Red Hat, Inc.
 *
 * Author: Jakub Jelen <jjelen@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef P11TEST_CASE_COMMON_H
#define P11TEST_CASE_COMMON_H

#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
# include <openssl/core_names.h>
# include <openssl/param_build.h>
#endif
#include "p11test_common.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct {
	unsigned char	*key_id;
	CK_ULONG	key_id_size;
	char		*id_str;
	X509		*x509;
	int		 type;
	EVP_PKEY	*key;
	CK_OBJECT_HANDLE private_handle;
	CK_OBJECT_HANDLE public_handle;
	CK_BBOOL	sign;
	CK_BBOOL	decrypt;
	CK_BBOOL	verify;
	CK_BBOOL	encrypt;
	CK_BBOOL	wrap;
	CK_BBOOL	unwrap;
	CK_BBOOL	derive_priv;
	CK_BBOOL	derive_pub;
	CK_KEY_TYPE	key_type;
	CK_BBOOL	always_auth;
	CK_BBOOL	extractable;
	char		*label;
	CK_ULONG 	 bits;
	char 		*value;
	int			verify_public;
	test_mech_t	mechs[MAX_PSS_MECHS];
	size_t			num_mechs;
} test_cert_t;

typedef struct {
	unsigned int alloc_count;
	unsigned int count;
	test_cert_t *data;
} test_certs_t;

void test_certs_init(test_certs_t *objects);

void always_authenticate(test_cert_t *o, token_info_t *info);

int search_objects(test_certs_t *objects, token_info_t *info,
	CK_ATTRIBUTE filter[], CK_LONG filter_size, CK_ATTRIBUTE template[], CK_LONG template_size,
	int (*callback)(test_certs_t *, CK_ATTRIBUTE[], unsigned long, CK_OBJECT_HANDLE));
void search_for_all_objects(test_certs_t *objects, token_info_t *info);
void clean_all_objects(test_certs_t *objects);

const char *get_mechanism_name(unsigned long mech_id);
const char *get_mgf_name(unsigned long mech_id);
const char *get_mechanism_flag_name(unsigned long flag_id);
const char *get_mechanism_all_flag_name(unsigned long flag_id);
const char *get_key_type(test_cert_t *key);
char *convert_byte_string(unsigned char *id, unsigned long length);

int is_pss_mechanism(CK_MECHANISM_TYPE mech);

CK_RV destroy_tmp_object(token_info_t *info, CK_OBJECT_HANDLE o);

// TODO sanitize inputs

#define P11TEST_START(info) if (info->log.fd) { \
	if (info->log.in_test) \
		fprintf(info->log.fd, ",\n\t\"result\": \"unknown\"\n}"); \
	fprintf(info->log.fd, "%s\n{\n\t\"test_id\": \"%s\"", \
			info->log.first ? "" : ",", __func__); \
	info->log.in_test = 1; \
	info->log.first = 0; \
	info->log.in_data = 0; \
	} else {}

#define _P11TEST_FINALIZE(info, result) if (info->log.fd) {\
	if (info->log.in_data) {\
		fprintf(info->log.fd, "]"); \
	} \
	if (info->log.in_test) { \
		fprintf(info->log.fd, ",\n\t\"result\": \"" result "\"\n}"); \
		info->log.in_test = 0; \
	} \
	} else {}

#define P11TEST_SKIP(info) do { _P11TEST_FINALIZE(info, "skip"); skip(); return; } while(0);

#define P11TEST_PASS(info) do { _P11TEST_FINALIZE(info, "pass"); } while(0);

#define P11TEST_FAIL(info, msg, ...) do { \
		if (info->log.fd && info->log.in_test) { \
			fprintf(info->log.fd, ",\n\t\"fail_reason\": \"" msg "\"", ##__VA_ARGS__); \
		} \
		_P11TEST_FINALIZE(info, "fail") \
		fail_msg(msg, ##__VA_ARGS__); \
		exit(1); \
	} while (0);

#define P11TEST_DATA_ROW(info, cols, ...) if (info->log.fd) { \
	if (info->log.in_test == 0) {\
		fail_msg("Can't add data outside of the test");\
		exit(1); \
	} \
	if (info->log.in_data == 0) {\
		fprintf(info->log.fd, ",\n\t\"data\": [");\
		info->log.in_data = 1;\
	} else { \
		fprintf(info->log.fd, ",");\
	} \
	write_data_row(info, cols, ##__VA_ARGS__); \
	} else {}

void write_data_row(token_info_t *info, int cols, ...);

#endif /* P11TEST_CASE_COMMON_H */
