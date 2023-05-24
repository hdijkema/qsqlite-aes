/*
 * Codec class for SQLite3 encryption codec.
 * (C) 2010 Olivier de Gaalon
 *
 * Distributed under the terms of the Botan license
 */

#include "codec.h"
#include <QDebug>
#include <QCryptographicHash>

//#define DEBUG

Codec::Codec(void *db)
{
    InitializeCodec(db);
}

Codec::Codec(const Codec *other, void *db)
{
    //Only used to copy main db key for an attached db
    InitializeCodec(db);
    _hasReadKey       = other->_hasReadKey;
    _hasWriteKey      = other->_hasWriteKey;
    _null_read_key    = other->_null_read_key;
    _null_write_key   = other->_null_write_key;
    _aes_writer_write = other->_aes_writer_write;
    _aes_writer_read  = other->_aes_writer_read;
    _aes_reader_read  = other->_aes_reader_read;
    _aes_reader_write = other->_aes_reader_write;
}

void Codec::InitializeCodec(void *db)
{
    _hasReadKey  = false;
    _hasWriteKey = false;
    _null_read_key = true;
    _null_write_key = true;

    _db = db;
    _aes_writer_write.SetParameters(256, 256);
    _aes_writer_read.SetParameters(256, 256);
    _aes_reader_write.SetParameters(256, 256);
    _aes_reader_read.SetParameters(256, 256);
    _blocksize = 256 / 8;
}

void Codec::GenerateWriteKey(const char *userPassword, int /*passwordLength*/)
{
#ifdef DEBUG
    qDebug() << __FUNCTION__ << userPassword;
#endif
    if (strcmp(userPassword, "null") == 0) {
        _null_write_key = true;
    } else {
        _null_write_key = false;
        auto rotr = [](aes_byte_t v, int shift) -> aes_byte_t  {
            aes_byte_t s =  (shift >=0) ? shift%8 : -((-shift)%8);
            return (v>>s) | (v<<(8-s));
        };

        QString _key(QString::fromUtf8(userPassword));
        QByteArray b = _key.toUtf8();
        QByteArray md5c = QCryptographicHash::hash(b, QCryptographicHash::Md5);
        QByteArray md5r;
        int i;
        for(i = md5c.size() - 1; i >= 0; i--) {
            md5r.append(rotr(md5c[i], 1));
        }
        md5c.append(md5r);

        _aes_writer_write.StartEncryption((aes_byte_t *) md5c.data());
        _aes_writer_read.StartDecryption((aes_byte_t *) md5c.data());
    }

    _hasWriteKey = true;
#ifdef DEBUG
    qDebug() << __FUNCTION__ << userPassword << _hasWriteKey << _hasReadKey << _null_write_key << _null_read_key;
#endif
}

void Codec::DropWriteKey()
{
    _hasWriteKey = false;
    _null_write_key = true;
}

void Codec::SetReadIsWrite()
{
    _hasReadKey = _hasWriteKey;
    _null_read_key = _null_write_key;
    _aes_reader_write = _aes_writer_write;
    _aes_reader_read = _aes_writer_read;
#ifdef DEBUG
    qDebug() << __FUNCTION__ << _hasWriteKey << _hasReadKey << _null_write_key << _null_read_key;
#endif
}

void Codec::SetWriteIsRead()
{
    _hasWriteKey = _hasReadKey;
    _null_write_key = _null_read_key;
    _aes_writer_write = _aes_reader_write;
    _aes_writer_read = _aes_reader_read;
#ifdef DEBUG
    qDebug() << __FUNCTION__ << _hasWriteKey << _hasReadKey << _null_write_key << _null_read_key;
#endif
}

unsigned char* Codec::Encrypt(int page, unsigned char *data, bool useWriteKey)
{
#ifdef DEBUG
    qDebug() << __FUNCTION__ << useWriteKey << _hasWriteKey << _hasReadKey << _null_write_key << _null_read_key;
#endif
    if (_null_write_key && useWriteKey) {
        memcpy(_page, data, _pageSize);
        return _page;
    } else if (_null_read_key && !useWriteKey) {
        return data;
    } else {
        int move = page % _pageSize;

        int i, k;
        for(i = 0, k = move;i < _pageSize; i++) {
            _mdata[k++] = data[i];
            if (k >= _pageSize) { k = 0; }
        }

        if (useWriteKey) {
            _aes_writer_write.Encrypt(_mdata, _page, _pageSize / _blocksize);
        } else {
            _aes_reader_write.Encrypt(_mdata, _page, _pageSize / _blocksize);
        }

        return _page; //return location of newly ciphered data
    }
}

void Codec::Decrypt(int page, unsigned char *data)
{
#ifdef DEBUG
    qDebug() << __FUNCTION__ << _hasWriteKey << _hasReadKey << _null_write_key << _null_read_key;
#endif
    if (_null_read_key) {
        // do nothing
    } else {
        _aes_reader_read.Decrypt(data, _mdata, _pageSize / _blocksize);

        int move = page % _pageSize;
        int i, k;
        for(i = 0, k = move; i < _pageSize; i++) {
            _page[i] = _mdata[k++];
            if (k >= _pageSize) { k = 0; }
        }

        memcpy(data, _page, _pageSize);
    }
}

void Codec::SetPageSize(int pageSize)
{
    _pageSize = pageSize;
    if ((_pageSize % _blocksize) != 0) {
        qCritical() << __FUNCTION__ << __LINE__ << "pageSize must be a multiple of blocksize!";
    }
}

bool Codec::HasReadKey()
{
    return _hasReadKey;
}

bool Codec::HasWriteKey()
{
    return _hasWriteKey;
}

void *Codec::GetDB()
{
    return _db;
}

const char* Codec::GetAndResetError()
{
    const char *message = 0;
    return message;
}


#include "codec_c_interface.h"

void* InitializeNewCodec(void *db) {
    return new Codec(db);
}
void* InitializeFromOtherCodec(const void *otherCodec, void *db) {
    return new Codec((Codec*)otherCodec, db);
}
void GenerateWriteKey(void *codec, const char *userPassword, int passwordLength) {
    ((Codec*)codec)->GenerateWriteKey(userPassword, passwordLength);
}
void DropWriteKey(void *codec) {
    ((Codec*)codec)->DropWriteKey();
}
void SetWriteIsRead(void *codec) {
    ((Codec*)codec)->SetWriteIsRead();
}
void SetReadIsWrite(void *codec) {
    ((Codec*)codec)->SetReadIsWrite();
}
unsigned char* Encrypt(void *codec, int page, unsigned char *data, Bool useWriteKey) {
    return ((Codec*)codec)->Encrypt(page, data, useWriteKey);
}
void Decrypt(void *codec, int page, unsigned char *data) {
    ((Codec*)codec)->Decrypt(page, data);
}
void SetPageSize(void *codec, int pageSize) {
    ((Codec*)codec)->SetPageSize(pageSize);
}
Bool HasReadKey(void *codec) {
    return ((Codec*)codec)->HasReadKey();
}
Bool HasWriteKey(void *codec) {
    return ((Codec*)codec)->HasWriteKey();
}
void* GetDB(void *codec) {
    return ((Codec*)codec)->GetDB();
}

const char* GetAndResetError(void *codec)
{
    return ((Codec*)codec)->GetAndResetError();
}

void DeleteCodec(void *codec) {
    Codec *deleteThisCodec = (Codec*)codec;
    delete deleteThisCodec;
}
