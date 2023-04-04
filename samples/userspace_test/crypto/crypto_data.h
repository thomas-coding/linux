/*
 * Copyright (c) 2021-2031, Jinping Wu (wunekky@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CRYPTO_DATA_H_
#define CRYPTO_DATA_H_
#include <stdint.h>

extern uint8_t aes_ecb_128_key[16];
extern uint8_t aes_ecb_128_plaintext[16];
extern uint8_t aes_ecb_128_ciphertext[16];

extern uint8_t aes_ecb_192_key[24];
extern uint8_t aes_ecb_192_plaintext[16];
extern uint8_t aes_ecb_192_ciphertext[16];

extern uint8_t aes_ecb_256_key[32];
extern uint8_t aes_ecb_256_plaintext[16];
extern uint8_t aes_ecb_256_ciphertext[16];

extern uint8_t aes_cbc_128_key[16];
extern uint8_t aes_cbc_128_iv[16];
extern uint8_t aes_cbc_128_plaintext[16];
extern uint8_t aes_cbc_128_ciphertext[16];

extern uint8_t aes_cbc_192_key[24];
extern uint8_t aes_cbc_192_iv[16];
extern uint8_t aes_cbc_192_plaintext[16];
extern uint8_t aes_cbc_192_ciphertext[16];

extern uint8_t aes_cbc_256_key[32];
extern uint8_t aes_cbc_256_iv[16];
extern uint8_t aes_cbc_256_plaintext[16];
extern uint8_t aes_cbc_256_ciphertext[16];

extern uint8_t aes_ctr_128_key[16];
extern uint8_t aes_ctr_128_iv[16];
extern uint8_t aes_ctr_128_plaintext[16];
extern uint8_t aes_ctr_128_ciphertext[16];

extern uint8_t aes_xts_128_key1[16];
extern uint8_t aes_xts_128_key2[16];
extern uint8_t aes_xts_128_iv[16];
extern uint8_t aes_xts_128_plaintext[16];
extern uint8_t aes_xts_128_ciphertext[16];

extern uint8_t aes_xts_256_key1[32];
extern uint8_t aes_xts_256_key2[32];
extern uint8_t aes_xts_256_iv[16];
extern uint8_t aes_xts_256_plaintext[32];
extern uint8_t aes_xts_256_ciphertext[32];

extern uint8_t aes_gcm_128_key[16];
extern uint8_t aes_gcm_128_iv[12];
extern uint8_t aes_gcm_128_aad[16];
extern uint8_t aes_gcm_128_plaintext[16];
extern uint8_t aes_gcm_128_ciphertext[16];
extern uint8_t aes_gcm_128_tag[16];

extern uint8_t aes_ccm_128_key[16];
extern uint8_t aes_ccm_128_nonce[13];
extern uint8_t aes_ccm_128_aad[32];
extern uint8_t aes_ccm_128_plaintext[24];
extern uint8_t aes_ccm_128_ciphertext[24];
extern uint8_t aes_ccm_128_tag[4];

extern uint8_t aes_cmac_128_key[16];
extern uint8_t aes_cmac_128_msg[32];
extern uint8_t aes_cmac_128_mac[16];

extern uint8_t sha1_msg[4];
extern uint8_t sha1_digest[20];

extern uint8_t sha224_msg[4];
extern uint8_t sha224_digest[28];

extern uint8_t sha256_msg[4];
extern uint8_t sha256_digest[32];

extern uint8_t sha384_msg[4];
extern uint8_t sha384_digest[48];

extern uint8_t sha512_msg[4];
extern uint8_t sha512_digest[64];

extern uint8_t hmac_sha1_key[10];
extern uint8_t hmac_sha1_msg[128];
extern uint8_t hmac_sha1_mac[20];

extern uint8_t hmac_sha2_256_key[40];
extern uint8_t hmac_sha2_256_msg[128];
extern uint8_t hmac_sha2_256_mac[32];

extern unsigned char mypublic_pem[];
extern unsigned int mypublic_pem_len;
extern unsigned char myprivate_pem[];
extern unsigned int myprivate_pem_len;

extern uint8_t rsaes_oaep_rsa_modulus[128];
extern uint8_t rsaes_oaep_rsa_priv_exponent[128];
extern uint8_t rsaes_oaep_rsa_pub_exponent[3];
extern uint8_t rsaes_oaep_message[28];

extern uint8_t rsaes_pkcs1v15_rsa_modulus[128];
extern uint8_t rsaes_pkcs1v15_rsa_priv_exponent[128];
extern uint8_t rsaes_pkcs1v15_rsa_pub_exponent[3];
extern uint8_t rsaes_pkcs1v15_message[28];

extern uint8_t rsassa_pkcs1v15_rsa_modulus[128];
extern uint8_t rsassa_pkcs1v15_rsa_priv_exponent[128];
extern uint8_t rsassa_pkcs1v15_rsa_pub_exponent[3];
extern uint8_t rsassa_pkcs1v15_message[217];
extern uint8_t rsassa_pkcs1v15_signature[128];

extern uint8_t rsassa_pss_rsa_modulus[128];
extern uint8_t rsassa_pss_rsa_priv_exponent[128];
extern uint8_t rsassa_pss_rsa_pub_exponent[3];
extern uint8_t rsassa_pss_message[217];

extern uint8_t ecdsa_p256_d[32];
extern uint8_t ecdsa_p256_qx[32];
extern uint8_t ecdsa_p256_qy[32];
extern uint8_t ecdsa_message[128];
#endif /*CRYPTO_DATA_H_*/