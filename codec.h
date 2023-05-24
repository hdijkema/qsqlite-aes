/*
 * Codec class for SQLite3 encryption codec.
 * (C) 2018 Hans Dijkema
 *
 */

#ifndef _CODEC_H_
#define _CODEC_H_

#include <string>
#include <memory>
#include "AES.h"

using namespace std;

//This is definited in sqlite.h and very unlikely to change
#define SQLITE_MAX_PAGE_SIZE 32768

class Codec
{
public:
    Codec(void *db);
    Codec(const Codec* other, void *db);

    void GenerateWriteKey(const char *userPassword, int passwordLength);
    void DropWriteKey();
    void SetWriteIsRead();
    void SetReadIsWrite();

    unsigned char* Encrypt(int page, unsigned char *data, bool useWriteKey);
    void           Decrypt(int page, unsigned char *data);

    void SetPageSize(int pageSize);

    bool        HasReadKey();
    bool        HasWriteKey();
    void       *GetDB();
    const char *GetAndResetError();

private:
    bool _hasReadKey;
    bool _hasWriteKey;
    bool _null_write_key;
    bool _null_read_key;

    AES _aes_writer_read;
    AES _aes_writer_write;
    AES _aes_reader_read;
    AES _aes_reader_write;

    int             _pageSize;
    unsigned char   _page[SQLITE_MAX_PAGE_SIZE];
    unsigned char   _mdata[SQLITE_MAX_PAGE_SIZE];
    int             _blocksize;
    void           *_db;

    void InitializeCodec(void *db);
};

#endif
