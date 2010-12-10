#include "tools.h"
#include "includes.h"
#include "aes.h"
#include "sha1.h"

//QString currentDir;
//QString cachePath = "./NUS_cache";
//QString nandPath = "./dump";

char ascii( char s ) {
    if ( s < 0x20 ) return '.';
    if ( s > 0x7E ) return '.';
    return s;
}

//using stderr just because qtcreator stows it correctly when mixed with qDebug(), qWarning(), etc
//otherwise it may not be shown in the correct spot in the output due to stdout/stderr caches
void hexdump( const void *d, int len ) {
    unsigned char *data;
    int i, off;
    data = (unsigned char*)d;
    fprintf( stderr, "\n");
    for ( off = 0; off < len; off += 16 ) {
	fprintf( stderr, "%08x  ", off );
	for ( i=0; i<16; i++ )
	{
	    if( ( i + 1 ) % 4 )
	    {
		if ( ( i + off ) >= len ) fprintf( stderr,"  ");
		else fprintf( stderr,"%02x",data[ off + i ]);
	    }
	    else
	    {
		if ( ( i + off ) >= len ) fprintf( stderr,"   ");
		else fprintf( stderr,"%02x ",data[ off + i ]);
	    }
	}

	fprintf( stderr, " " );
	for ( i = 0; i < 16; i++ )
	    if ( ( i + off) >= len ) fprintf( stderr," ");
	    else fprintf( stderr,"%c", ascii( data[ off + i ]));
	fprintf( stderr,"\n");
    }
    fflush( stderr );
}

void hexdump( const QByteArray &d, int from, int len )
{
    hexdump( d.data() + from, len == -1 ? d.size() : len );
}

void hexdump12( const void *d, int len ) {
    unsigned char *data;
    int i, off;
    data = (unsigned char*)d;
    fprintf( stderr, "\n");
    for ( off = 0; off < len; off += 12 ) {
	fprintf( stderr, "%08x  ", off );
	for ( i=0; i<12; i++ )
	{
	    if( ( i + 1 ) % 4 )
	    {
		if ( ( i + off ) >= len ) fprintf( stderr,"  ");
		else fprintf( stderr,"%02x",data[ off + i ]);
	    }
	    else
	    {
		if ( ( i + off ) >= len ) fprintf( stderr,"   ");
		else fprintf( stderr,"%02x ",data[ off + i ]);
	    }
	}

	fprintf( stderr, " " );
	for ( i = 0; i < 12; i++ )
	    if ( ( i + off) >= len ) fprintf( stderr," ");
	    else fprintf( stderr,"%c", ascii( data[ off + i ]));
	fprintf( stderr,"\n");
    }
    fflush( stderr );
}

void hexdump12( const QByteArray &d, int from, int len )
{
    hexdump12( d.data() + from, len == -1 ? d.size() : len );
}

QByteArray PaddedByteArray( const QByteArray &orig, quint32 padTo )
{
    QByteArray padding( RU( padTo, orig.size() ) - orig.size(), '\0' );
    //qDebug() << "padding with" << hex << RU( padTo, orig.size() ) << "bytes" <<
    return orig + padding;
}

QByteArray AesDecrypt( quint16 index, const QByteArray source )
{
	static quint8 iv[ 16 ];

	quint16 beidx = qFromBigEndian( index );
	memset( iv, 0, 16 );
	memcpy( iv, &beidx, 2 );
	QByteArray ret( source.size(), '\0' );
	aes_decrypt( iv, (const quint8*)source.data(), (quint8*)ret.data(), source.size() );
	return ret;
}

QByteArray AesEncrypt( quint16 index, const QByteArray source )
{
	static quint8 iv[ 16 ];

	quint16 beidx = qFromBigEndian( index );
	memset( iv, 0, 16 );
	memcpy( iv, &beidx, 2 );
	QByteArray ret( source.size(), '\0' );
	aes_encrypt( iv, (const quint8*)source.data(), (quint8*)ret.data(), source.size() );
	return ret;
}

void AesSetKey( const QByteArray key )
{
    aes_set_key( (const quint8*)key.data() );
}

QByteArray GetSha1( QByteArray stuff )
{
    SHA1Context sha;
    SHA1Reset( &sha );
    SHA1Input( &sha, (const unsigned char*)stuff.data(), stuff.size() );
    if( !SHA1Result( &sha ) )
    {
	qWarning() << "GetSha1 -> sha error";
	return QByteArray();
    }
    QByteArray ret( 20, '\0' );
    quint8 *p = (quint8 *)ret.data();
    for( int i = 0; i < 5 ; i++ )
    {
	quint32 part = qFromBigEndian( sha.Message_Digest[ i ] );
	memcpy( p + ( i * 4 ), &part, 4 );
    }
    //hexdump( ret );
    return ret;
}

QByteArray ReadFile( const QString &path )
{
    QFile file( path );
    if( !file.exists() || !file.open( QIODevice::ReadOnly ) )
    {
	qWarning() << "ReadFile -> can't open" << path;
	return QByteArray();
    }
    QByteArray ret = file.readAll();
    file.close();
    return ret;
}

bool WriteFile( const QString &path, const QByteArray ba )
{
    QFile file( path );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
	qWarning() << "WriteFile -> can't open" << path;
	return false;
    }
    if( file.write( ba ) != ba.size() )
    {
	file.close();
	qWarning() << "WriteFile -> can't write all the data to" << path;
	return false;
    }
    file.close();
    return true;
}

#define CERTS_DAT_SIZE 2560
const quint8 certs_dat[ CERTS_DAT_SIZE ] = {
0x00, 0x01, 0x00, 0x01, 0x7D, 0x9D, 0x5E, 0xBA, 0x52, 0x81, 0xDC, 0xA7, 0x06, 0x5D, 0x2F, 0x08,
0x68, 0xDB, 0x8A, 0xC7, 0x3A, 0xCE, 0x7E, 0xA9, 0x91, 0xF1, 0x96, 0x9F, 0xE1, 0xD0, 0xF2, 0xC1,
0x1F, 0xAE, 0xC0, 0xC3, 0xF0, 0x1A, 0xDC, 0xB4, 0x46, 0xAD, 0xE5, 0xCA, 0x03, 0xB6, 0x25, 0x21,
0x94, 0x62, 0xC6, 0xE1, 0x41, 0x0D, 0xB9, 0xE6, 0x3F, 0xDE, 0x98, 0xD1, 0xAF, 0x26, 0x3B, 0x4C,
0xB2, 0x87, 0x84, 0x27, 0x82, 0x72, 0xEF, 0x27, 0x13, 0x4B, 0x87, 0xC2, 0x58, 0xD6, 0x7B, 0x62,
0xF2, 0xB5, 0xBF, 0x9C, 0xB6, 0xBA, 0x8C, 0x89, 0x19, 0x2E, 0xC5, 0x06, 0x89, 0xAC, 0x74, 0x24,
0xA0, 0x22, 0x09, 0x40, 0x03, 0xEE, 0x98, 0xA4, 0xBD, 0x2F, 0x01, 0x3B, 0x59, 0x3F, 0xE5, 0x66,
0x6C, 0xD5, 0xEB, 0x5A, 0xD7, 0xA4, 0x93, 0x10, 0xF3, 0x4E, 0xFB, 0xB4, 0x3D, 0x46, 0xCB, 0xF1,
0xB5, 0x23, 0xCF, 0x82, 0xF6, 0x8E, 0xB5, 0x6D, 0xB9, 0x04, 0xA7, 0xC2, 0xA8, 0x2B, 0xE1, 0x1D,
0x78, 0xD3, 0x9B, 0xA2, 0x0D, 0x90, 0xD3, 0x07, 0x42, 0xDB, 0x5E, 0x7A, 0xC1, 0xEF, 0xF2, 0x21,
0x51, 0x09, 0x62, 0xCF, 0xA9, 0x14, 0xA8, 0x80, 0xDC, 0xF4, 0x17, 0xBA, 0x99, 0x93, 0x0A, 0xEE,
0x08, 0xB0, 0xB0, 0xE5, 0x1A, 0x3E, 0x9F, 0xAF, 0xCD, 0xC2, 0xD7, 0xE3, 0xCB, 0xA1, 0x2F, 0x3A,
0xC0, 0x07, 0x90, 0xDE, 0x44, 0x7A, 0xC3, 0xC5, 0x38, 0xA8, 0x67, 0x92, 0x38, 0x07, 0x8B, 0xD4,
0xC4, 0xB2, 0x45, 0xAC, 0x29, 0x16, 0x88, 0x6D, 0x2A, 0x0E, 0x59, 0x4E, 0xED, 0x5C, 0xC8, 0x35,
0x69, 0x8B, 0x4D, 0x62, 0x38, 0xDF, 0x05, 0x72, 0x4D, 0xCC, 0xF6, 0x81, 0x80, 0x8A, 0x70, 0x74,
0x06, 0x59, 0x30, 0xBF, 0xF8, 0x51, 0x41, 0x37, 0xE8, 0x15, 0xFA, 0xBA, 0xA1, 0x72, 0xB8, 0xE0,
0x69, 0x6C, 0x61, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x52, 0x6F, 0x6F, 0x74, 0x2D, 0x43, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x58, 0x53, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x33, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xF1, 0xB8, 0x9F, 0xD1, 0xAD, 0x07, 0xA9, 0x37, 0x8A, 0x7B, 0x10, 0x0C,
0x7D, 0xC7, 0x39, 0xBE, 0x9E, 0xDD, 0xB7, 0x32, 0x00, 0x89, 0xAB, 0x25, 0xB1, 0xF8, 0x71, 0xAF,
0x5A, 0xA9, 0xF4, 0x58, 0x9E, 0xD1, 0x83, 0x02, 0x32, 0x8E, 0x81, 0x1A, 0x1F, 0xEF, 0xD0, 0x09,
0xC8, 0x06, 0x36, 0x43, 0xF8, 0x54, 0xB9, 0xE1, 0x3B, 0xBB, 0x61, 0x3A, 0x7A, 0xCF, 0x87, 0x14,
0x85, 0x6B, 0xA4, 0x5B, 0xAA, 0xE7, 0xBB, 0xC6, 0x4E, 0xB2, 0xF7, 0x5D, 0x87, 0xEB, 0xF2, 0x67,
0xED, 0x0F, 0xA4, 0x41, 0xA9, 0x33, 0x66, 0x5E, 0x57, 0x7D, 0x5A, 0xDE, 0xAB, 0xFB, 0x46, 0x2E,
0x76, 0x00, 0xCA, 0x9C, 0xE9, 0x4D, 0xC4, 0xCB, 0x98, 0x39, 0x92, 0xAB, 0x7A, 0x2F, 0xB3, 0xA3,
0x9E, 0xA2, 0xBF, 0x9C, 0x53, 0xEC, 0xD0, 0xDC, 0xFA, 0x6B, 0x8B, 0x5E, 0xB2, 0xCB, 0xA4, 0x0F,
0xFA, 0x40, 0x75, 0xF8, 0xF2, 0xB2, 0xDE, 0x97, 0x38, 0x11, 0x87, 0x2D, 0xF5, 0xE2, 0xA6, 0xC3,
0x8B, 0x2F, 0xDC, 0x8E, 0x57, 0xDD, 0xBD, 0x5F, 0x46, 0xEB, 0x27, 0xD6, 0x19, 0x52, 0xF6, 0xAE,
0xF8, 0x62, 0xB7, 0xEE, 0x9A, 0xC6, 0x82, 0xA2, 0xB1, 0x9A, 0xA9, 0xB5, 0x58, 0xFB, 0xEB, 0xB3,
0x89, 0x2F, 0xBD, 0x50, 0xC9, 0xF5, 0xDC, 0x4A, 0x6E, 0x9C, 0x9B, 0xFE, 0x45, 0x80, 0x34, 0xA9,
0x42, 0x18, 0x2D, 0xDE, 0xB7, 0x5F, 0xE0, 0xD1, 0xB3, 0xDF, 0x0E, 0x97, 0xE3, 0x99, 0x80, 0x87,
0x70, 0x18, 0xC2, 0xB2, 0x83, 0xF1, 0x35, 0x75, 0x7C, 0x5A, 0x30, 0xFC, 0x3F, 0x30, 0x84, 0xA4,
0x9A, 0xAA, 0xC0, 0x1E, 0xE7, 0x06, 0x69, 0x4F, 0x8E, 0x14, 0x48, 0xDA, 0x12, 0x3A, 0xCC, 0x4F,
0xFA, 0x26, 0xAA, 0x38, 0xF7, 0xEF, 0xBF, 0x27, 0x8F, 0x36, 0x97, 0x79, 0x77, 0x5D, 0xB7, 0xC5,
0xAD, 0xC7, 0x89, 0x91, 0xDC, 0xF8, 0x43, 0x8D, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x00, 0x00, 0xB3, 0xAD, 0xB3, 0x22, 0x6B, 0x3C, 0x3D, 0xFF, 0x1B, 0x4B, 0x40, 0x77,
0x16, 0xFF, 0x4F, 0x7A, 0xD7, 0x64, 0x86, 0xC8, 0x95, 0xAC, 0x56, 0x2D, 0x21, 0xF1, 0x06, 0x01,
0xD4, 0xF6, 0x64, 0x28, 0x19, 0x1C, 0x07, 0x76, 0x8F, 0xDF, 0x1A, 0xE2, 0xCE, 0x7B, 0x27, 0xC9,
0x0F, 0xBC, 0x0A, 0xD0, 0x31, 0x25, 0x78, 0xEC, 0x07, 0x79, 0xB6, 0x57, 0xD4, 0x37, 0x24, 0x13,
0xA7, 0xF8, 0x6F, 0x0C, 0x14, 0xC0, 0xEF, 0x6E, 0x09, 0x41, 0xED, 0x2B, 0x05, 0xEC, 0x39, 0x57,
0x36, 0x07, 0x89, 0x00, 0x4A, 0x87, 0x8D, 0x2E, 0x9D, 0xF8, 0xC7, 0xA5, 0xA9, 0xF8, 0xCA, 0xB3,
0x11, 0xB1, 0x18, 0x79, 0x57, 0xBB, 0xF8, 0x98, 0xE2, 0xA2, 0x54, 0x02, 0xCF, 0x54, 0x39, 0xCF,
0x2B, 0xBF, 0xA0, 0xE1, 0xF8, 0x5C, 0x06, 0x6E, 0x83, 0x9A, 0xE0, 0x94, 0xCA, 0x47, 0xE0, 0x15,
0x58, 0xF5, 0x6E, 0x6F, 0x34, 0xE9, 0x2A, 0xA2, 0xDC, 0x38, 0x93, 0x7E, 0x37, 0xCD, 0x8C, 0x5C,
0x4D, 0xFD, 0x2F, 0x11, 0x4F, 0xE8, 0x68, 0xC9, 0xA8, 0xD9, 0xFE, 0xD8, 0x6E, 0x0C, 0x21, 0x75,
0xA2, 0xBD, 0x7E, 0x89, 0xB9, 0xC7, 0xB5, 0x13, 0xF4, 0x1A, 0x79, 0x61, 0x44, 0x39, 0x10, 0xEF,
0xF9, 0xD7, 0xFE, 0x57, 0x22, 0x18, 0xD5, 0x6D, 0xFB, 0x7F, 0x49, 0x7A, 0xA4, 0xCB, 0x90, 0xD4,
0xF1, 0xAE, 0xB1, 0x76, 0xE4, 0x68, 0x5D, 0xA7, 0x94, 0x40, 0x60, 0x98, 0x2F, 0x04, 0x48, 0x40,
0x1F, 0xCF, 0xC6, 0xBA, 0xEB, 0xDA, 0x16, 0x30, 0xB4, 0x73, 0xB4, 0x15, 0x23, 0x35, 0x08, 0x07,
0x0A, 0x9F, 0x4F, 0x89, 0x78, 0xE6, 0x2C, 0xEC, 0x5E, 0x92, 0x46, 0xA5, 0xA8, 0xBD, 0xA0, 0x85,
0x78, 0x68, 0x75, 0x0C, 0x3A, 0x11, 0x2F, 0xAF, 0x95, 0xE8, 0x38, 0xC8, 0x99, 0x0E, 0x87, 0xB1,
0x62, 0xCD, 0x10, 0xDA, 0xB3, 0x31, 0x96, 0x65, 0xEF, 0x88, 0x9B, 0x54, 0x1B, 0xB3, 0x36, 0xBB,
0x67, 0x53, 0x9F, 0xAF, 0xC2, 0xAE, 0x2D, 0x0A, 0x2E, 0x75, 0xC0, 0x23, 0x74, 0xEA, 0x4E, 0xAC,
0x8D, 0x99, 0x50, 0x7F, 0x59, 0xB9, 0x53, 0x77, 0x30, 0x5F, 0x26, 0x35, 0xC6, 0x08, 0xA9, 0x90,
0x93, 0xAC, 0x8F, 0xC6, 0xDE, 0x23, 0xB9, 0x7A, 0xEA, 0x70, 0xB4, 0xC4, 0xCF, 0x66, 0xB3, 0x0E,
0x58, 0x32, 0x0E, 0xC5, 0xB6, 0x72, 0x04, 0x48, 0xCE, 0x3B, 0xB1, 0x1C, 0x53, 0x1F, 0xCB, 0x70,
0x28, 0x7C, 0xB5, 0xC2, 0x7C, 0x67, 0x4F, 0xBB, 0xFD, 0x8C, 0x7F, 0xC9, 0x42, 0x20, 0xA4, 0x73,
0x23, 0x1D, 0x58, 0x7E, 0x5A, 0x1A, 0x1A, 0x82, 0xE3, 0x75, 0x79, 0xA1, 0xBB, 0x82, 0x6E, 0xCE,
0x01, 0x71, 0xC9, 0x75, 0x63, 0x47, 0x4B, 0x1D, 0x46, 0xE6, 0x79, 0xB2, 0x82, 0x37, 0x62, 0x11,
0xCD, 0xC7, 0x00, 0x2F, 0x46, 0x87, 0xC2, 0x3C, 0x6D, 0xC0, 0xD5, 0xB5, 0x78, 0x6E, 0xE1, 0xF2,
0x73, 0xFF, 0x01, 0x92, 0x50, 0x0F, 0xF4, 0xC7, 0x50, 0x6A, 0xEE, 0x72, 0xB6, 0xF4, 0x3D, 0xF6,
0x08, 0xFE, 0xA5, 0x83, 0xA1, 0xF9, 0x86, 0x0F, 0x87, 0xAF, 0x52, 0x44, 0x54, 0xBB, 0x47, 0xC3,
0x06, 0x0C, 0x94, 0xE9, 0x9B, 0xF7, 0xD6, 0x32, 0xA7, 0xC8, 0xAB, 0x4B, 0x4F, 0xF5, 0x35, 0x21,
0x1F, 0xC1, 0x80, 0x47, 0xBB, 0x7A, 0xFA, 0x5A, 0x2B, 0xD7, 0xB8, 0x84, 0xAD, 0x8E, 0x56, 0x4F,
0x5B, 0x89, 0xFF, 0x37, 0x97, 0x37, 0xF1, 0xF5, 0x01, 0x3B, 0x1F, 0x9E, 0xC4, 0x18, 0x6F, 0x92,
0x2A, 0xD5, 0xC4, 0xB3, 0xC0, 0xD5, 0x87, 0x0B, 0x9C, 0x04, 0xAF, 0x1A, 0xB5, 0xF3, 0xBC, 0x6D,
0x0A, 0xF1, 0x7D, 0x47, 0x08, 0xE4, 0x43, 0xE9, 0x73, 0xF7, 0xB7, 0x70, 0x77, 0x54, 0xBA, 0xF3,
0xEC, 0xD2, 0xAC, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x52, 0x6F, 0x6F, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x43, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x5B, 0xFA, 0x7D, 0x5C, 0xB2, 0x79, 0xC9, 0xE2, 0xEE, 0xE1, 0x21, 0xC6,
0xEA, 0xF4, 0x4F, 0xF6, 0x39, 0xF8, 0x8F, 0x07, 0x8B, 0x4B, 0x77, 0xED, 0x9F, 0x95, 0x60, 0xB0,
0x35, 0x82, 0x81, 0xB5, 0x0E, 0x55, 0xAB, 0x72, 0x11, 0x15, 0xA1, 0x77, 0x70, 0x3C, 0x7A, 0x30,
0xFE, 0x3A, 0xE9, 0xEF, 0x1C, 0x60, 0xBC, 0x1D, 0x97, 0x46, 0x76, 0xB2, 0x3A, 0x68, 0xCC, 0x04,
0xB1, 0x98, 0x52, 0x5B, 0xC9, 0x68, 0xF1, 0x1D, 0xE2, 0xDB, 0x50, 0xE4, 0xD9, 0xE7, 0xF0, 0x71,
0xE5, 0x62, 0xDA, 0xE2, 0x09, 0x22, 0x33, 0xE9, 0xD3, 0x63, 0xF6, 0x1D, 0xD7, 0xC1, 0x9F, 0xF3,
0xA4, 0xA9, 0x1E, 0x8F, 0x65, 0x53, 0xD4, 0x71, 0xDD, 0x7B, 0x84, 0xB9, 0xF1, 0xB8, 0xCE, 0x73,
0x35, 0xF0, 0xF5, 0x54, 0x05, 0x63, 0xA1, 0xEA, 0xB8, 0x39, 0x63, 0xE0, 0x9B, 0xE9, 0x01, 0x01,
0x1F, 0x99, 0x54, 0x63, 0x61, 0x28, 0x70, 0x20, 0xE9, 0xCC, 0x0D, 0xAB, 0x48, 0x7F, 0x14, 0x0D,
0x66, 0x26, 0xA1, 0x83, 0x6D, 0x27, 0x11, 0x1F, 0x20, 0x68, 0xDE, 0x47, 0x72, 0x14, 0x91, 0x51,
0xCF, 0x69, 0xC6, 0x1B, 0xA6, 0x0E, 0xF9, 0xD9, 0x49, 0xA0, 0xF7, 0x1F, 0x54, 0x99, 0xF2, 0xD3,
0x9A, 0xD2, 0x8C, 0x70, 0x05, 0x34, 0x82, 0x93, 0xC4, 0x31, 0xFF, 0xBD, 0x33, 0xF6, 0xBC, 0xA6,
0x0D, 0xC7, 0x19, 0x5E, 0xA2, 0xBC, 0xC5, 0x6D, 0x20, 0x0B, 0xAF, 0x6D, 0x06, 0xD0, 0x9C, 0x41,
0xDB, 0x8D, 0xE9, 0xC7, 0x20, 0x15, 0x4C, 0xA4, 0x83, 0x2B, 0x69, 0xC0, 0x8C, 0x69, 0xCD, 0x3B,
0x07, 0x3A, 0x00, 0x63, 0x60, 0x2F, 0x46, 0x2D, 0x33, 0x80, 0x61, 0xA5, 0xEA, 0x6C, 0x91, 0x5C,
0xD5, 0x62, 0x35, 0x79, 0xC3, 0xEB, 0x64, 0xCE, 0x44, 0xEF, 0x58, 0x6D, 0x14, 0xBA, 0xAA, 0x88,
0x34, 0x01, 0x9B, 0x3E, 0xEB, 0xEE, 0xD3, 0x79, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x00, 0x01, 0x4E, 0x00, 0x5F, 0xF1, 0x3F, 0x86, 0x75, 0x8D, 0xB6, 0x9C, 0x45, 0x63,
0x0F, 0xD4, 0x9B, 0xF4, 0xCC, 0x5D, 0x54, 0xCF, 0xCC, 0x22, 0x34, 0x72, 0x57, 0xAB, 0xA4, 0xBA,
0x53, 0xD2, 0xB3, 0x3D, 0xE6, 0xEC, 0x9E, 0xA1, 0x57, 0x54, 0x53, 0xAE, 0x5F, 0x93, 0x3D, 0x96,
0xBF, 0xF7, 0xCC, 0x7A, 0x79, 0x56, 0x6E, 0x84, 0x7B, 0x1B, 0x60, 0x77, 0xC2, 0xA9, 0x38, 0x71,
0x30, 0x1A, 0x8C, 0xD3, 0xC9, 0x3D, 0x4D, 0xB3, 0x26, 0xE9, 0x87, 0x92, 0x66, 0xE9, 0xD3, 0xBA,
0x9F, 0x79, 0xBC, 0x46, 0x38, 0xFA, 0x2D, 0x20, 0xA0, 0x3A, 0x70, 0x67, 0xA4, 0x11, 0xA7, 0xA0,
0xB7, 0xD9, 0x12, 0xAD, 0x11, 0x6A, 0x3A, 0xC4, 0x6E, 0x32, 0x42, 0x47, 0xC2, 0x08, 0xBA, 0xB4,
0x94, 0x9C, 0xC5, 0x2E, 0xD0, 0x2F, 0x19, 0xF6, 0x51, 0xE0, 0xDF, 0x2E, 0x36, 0x53, 0xAA, 0xAF,
0x97, 0xA6, 0x92, 0xBB, 0xA9, 0x1D, 0xD8, 0x6E, 0x24, 0x2E, 0xB3, 0x08, 0x77, 0x55, 0x11, 0xCE,
0x98, 0xF6, 0xA2, 0xF4, 0x26, 0xC9, 0x27, 0x04, 0xD0, 0xFC, 0x8D, 0xD4, 0x80, 0x9E, 0xD7, 0x61,
0xBD, 0x11, 0xB7, 0x85, 0x94, 0x8C, 0xD6, 0xD0, 0x7A, 0xDB, 0xA4, 0x08, 0xD0, 0xF0, 0x86, 0xF6,
0x5A, 0xAE, 0x19, 0x14, 0xB2, 0x88, 0x9A, 0xA8, 0xAE, 0x4A, 0xA2, 0xAA, 0xC7, 0x61, 0xA9, 0x0D,
0x41, 0x2C, 0xB1, 0x50, 0x09, 0xAB, 0x3E, 0x93, 0xFC, 0xA9, 0x24, 0xDE, 0xCE, 0x4F, 0x7C, 0x06,
0xAB, 0xDC, 0x2E, 0x60, 0x9D, 0x68, 0xBE, 0x00, 0x73, 0xFA, 0x80, 0x57, 0x6A, 0x14, 0x5E, 0xED,
0xC4, 0x8B, 0x74, 0x32, 0x87, 0x07, 0x93, 0xC8, 0xFC, 0xA6, 0xD8, 0x3E, 0x09, 0x6E, 0xC5, 0xF2,
0xA9, 0xC4, 0x21, 0xE7, 0x48, 0xB3, 0x73, 0x40, 0x5B, 0xE2, 0xFA, 0x8A, 0xE1, 0x58, 0x78, 0xE9,
0xD5, 0x23, 0x88, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x52, 0x6F, 0x6F, 0x74, 0x2D, 0x43, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x43, 0x50, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x34, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xF1, 0xB8, 0xA0, 0x64, 0xC1, 0x6D, 0xF3, 0x83, 0x29, 0x55, 0xC3, 0x29,
0x5B, 0x72, 0xF0, 0x33, 0x2E, 0x97, 0xEF, 0x14, 0x84, 0x8A, 0x68, 0x04, 0x9C, 0xA6, 0x8E, 0xAC,
0xDE, 0x14, 0x50, 0x33, 0xB8, 0x6C, 0x10, 0x8D, 0x48, 0x33, 0x5C, 0x5D, 0x0C, 0xAB, 0x77, 0x04,
0x62, 0x54, 0x47, 0x55, 0x45, 0x2A, 0x90, 0x00, 0x70, 0xB1, 0x56, 0x92, 0x5C, 0x17, 0x86, 0xE2,
0xCD, 0x20, 0x6D, 0xCC, 0xDC, 0x2C, 0x2E, 0x37, 0x6E, 0x27, 0xFC, 0xB4, 0x20, 0x66, 0xCC, 0x0A,
0x8C, 0xE9, 0xFE, 0xE8, 0x57, 0x04, 0xE6, 0xCA, 0x63, 0x1A, 0x2E, 0x7E, 0x91, 0x7E, 0x94, 0x7C,
0x39, 0x91, 0x77, 0x36, 0x29, 0xD1, 0x55, 0x61, 0x85, 0xBB, 0xD7, 0xB7, 0x73, 0xCA, 0x37, 0x47,
0x9E, 0x5F, 0xAA, 0xA3, 0xB6, 0x05, 0xE0, 0x01, 0xE1, 0xAC, 0xE5, 0x8D, 0xD8, 0xF8, 0x47, 0x82,
0xD6, 0x45, 0xFC, 0xE3, 0xA1, 0xCD, 0x03, 0xAB, 0x36, 0xF0, 0xF3, 0x86, 0xB1, 0xA2, 0xD1, 0x37,
0x40, 0xA1, 0x94, 0x8A, 0x53, 0xBA, 0x1B, 0x0D, 0x8C, 0x48, 0x63, 0xCD, 0x6B, 0x2C, 0x2E, 0x20,
0x64, 0x94, 0x80, 0x4C, 0x62, 0xFA, 0xA9, 0x3A, 0x7E, 0x33, 0xA9, 0xEA, 0x78, 0x6B, 0x59, 0xCA,
0xE3, 0xAB, 0x36, 0x45, 0xF4, 0xCB, 0x8F, 0xD7, 0x90, 0x6B, 0x82, 0x68, 0xCD, 0xAC, 0xF1, 0x7B,
0x3A, 0xEC, 0x46, 0x83, 0x1B, 0x91, 0xF6, 0xDE, 0x18, 0x61, 0x83, 0xBC, 0x4B, 0x32, 0x67, 0x93,
0xC7, 0x2E, 0x50, 0xD9, 0x1E, 0x36, 0xA0, 0xDC, 0xE2, 0xB9, 0x7D, 0xA0, 0x21, 0x3E, 0x46, 0x96,
0x02, 0x1F, 0x33, 0x1C, 0xBE, 0xAE, 0x8D, 0xFC, 0x92, 0x87, 0x32, 0xAA, 0x44, 0xDC, 0x78, 0xE7,
0x19, 0x9A, 0x3D, 0xDD, 0x57, 0x22, 0x7E, 0x9E, 0x77, 0xDE, 0x32, 0x63, 0x86, 0x93, 0x6C, 0x11,
0xAC, 0xA7, 0x0F, 0x81, 0x19, 0xD3, 0x3A, 0x99, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// public root cert
const quint8 root_dat[] = {
0xF8, 0x24, 0x6C, 0x58, 0xBA, 0xE7, 0x50, 0x03, 0x01, 0xFB, 0xB7, 0xC2, 0xEB, 0xE0, 0x01, 0x05,
0x71, 0xDA, 0x92, 0x23, 0x78, 0xF0, 0x51, 0x4E, 0xC0, 0x03, 0x1D, 0xD0, 0xD2, 0x1E, 0xD3, 0xD0,
0x7E, 0xFC, 0x85, 0x20, 0x69, 0xB5, 0xDE, 0x9B, 0xB9, 0x51, 0xA8, 0xBC, 0x90, 0xA2, 0x44, 0x92,
0x6D, 0x37, 0x92, 0x95, 0xAE, 0x94, 0x36, 0xAA, 0xA6, 0xA3, 0x02, 0x51, 0x0C, 0x7B, 0x1D, 0xED,
0xD5, 0xFB, 0x20, 0x86, 0x9D, 0x7F, 0x30, 0x16, 0xF6, 0xBE, 0x65, 0xD3, 0x83, 0xA1, 0x6D, 0xB3,
0x32, 0x1B, 0x95, 0x35, 0x18, 0x90, 0xB1, 0x70, 0x02, 0x93, 0x7E, 0xE1, 0x93, 0xF5, 0x7E, 0x99,
0xA2, 0x47, 0x4E, 0x9D, 0x38, 0x24, 0xC7, 0xAE, 0xE3, 0x85, 0x41, 0xF5, 0x67, 0xE7, 0x51, 0x8C,
0x7A, 0x0E, 0x38, 0xE7, 0xEB, 0xAF, 0x41, 0x19, 0x1B, 0xCF, 0xF1, 0x7B, 0x42, 0xA6, 0xB4, 0xED,
0xE6, 0xCE, 0x8D, 0xE7, 0x31, 0x8F, 0x7F, 0x52, 0x04, 0xB3, 0x99, 0x0E, 0x22, 0x67, 0x45, 0xAF,
0xD4, 0x85, 0xB2, 0x44, 0x93, 0x00, 0x8B, 0x08, 0xC7, 0xF6, 0xB7, 0xE5, 0x6B, 0x02, 0xB3, 0xE8,
0xFE, 0x0C, 0x9D, 0x85, 0x9C, 0xB8, 0xB6, 0x82, 0x23, 0xB8, 0xAB, 0x27, 0xEE, 0x5F, 0x65, 0x38,
0x07, 0x8B, 0x2D, 0xB9, 0x1E, 0x2A, 0x15, 0x3E, 0x85, 0x81, 0x80, 0x72, 0xA2, 0x3B, 0x6D, 0xD9,
0x32, 0x81, 0x05, 0x4F, 0x6F, 0xB0, 0xF6, 0xF5, 0xAD, 0x28, 0x3E, 0xCA, 0x0B, 0x7A, 0xF3, 0x54,
0x55, 0xE0, 0x3D, 0xA7, 0xB6, 0x83, 0x26, 0xF3, 0xEC, 0x83, 0x4A, 0xF3, 0x14, 0x04, 0x8A, 0xC6,
0xDF, 0x20, 0xD2, 0x85, 0x08, 0x67, 0x3C, 0xAB, 0x62, 0xA2, 0xC7, 0xBC, 0x13, 0x1A, 0x53, 0x3E,
0x0B, 0x66, 0x80, 0x6B, 0x1C, 0x30, 0x66, 0x4B, 0x37, 0x23, 0x31, 0xBD, 0xC4, 0xB0, 0xCA, 0xD8,
0xD1, 0x1E, 0xE7, 0xBB, 0xD9, 0x28, 0x55, 0x48, 0xAA, 0xEC, 0x1F, 0x66, 0xE8, 0x21, 0xB3, 0xC8,
0xA0, 0x47, 0x69, 0x00, 0xC5, 0xE6, 0x88, 0xE8, 0x0C, 0xCE, 0x3C, 0x61, 0xD6, 0x9C, 0xBB, 0xA1,
0x37, 0xC6, 0x60, 0x4F, 0x7A, 0x72, 0xDD, 0x8C, 0x7B, 0x3E, 0x3D, 0x51, 0x29, 0x0D, 0xAA, 0x6A,
0x59, 0x7B, 0x08, 0x1F, 0x9D, 0x36, 0x33, 0xA3, 0x46, 0x7A, 0x35, 0x61, 0x09, 0xAC, 0xA7, 0xDD,
0x7D, 0x2E, 0x2F, 0xB2, 0xC1, 0xAE, 0xB8, 0xE2, 0x0F, 0x48, 0x92, 0xD8, 0xB9, 0xF8, 0xB4, 0x6F,
0x4E, 0x3C, 0x11, 0xF4, 0xF4, 0x7D, 0x8B, 0x75, 0x7D, 0xFE, 0xFE, 0xA3, 0x89, 0x9C, 0x33, 0x59,
0x5C, 0x5E, 0xFD, 0xEB, 0xCB, 0xAB, 0xE8, 0x41, 0x3E, 0x3A, 0x9A, 0x80, 0x3C, 0x69, 0x35, 0x6E,
0xB2, 0xB2, 0xAD, 0x5C, 0xC4, 0xC8, 0x58, 0x45, 0x5E, 0xF5, 0xF7, 0xB3, 0x06, 0x44, 0xB4, 0x7C,
0x64, 0x06, 0x8C, 0xDF, 0x80, 0x9F, 0x76, 0x02, 0x5A, 0x2D, 0xB4, 0x46, 0xE0, 0x3D, 0x7C, 0xF6,
0x2F, 0x34, 0xE7, 0x02, 0x45, 0x7B, 0x02, 0xA4, 0xCF, 0x5D, 0x9D, 0xD5, 0x3C, 0xA5, 0x3A, 0x7C,
0xA6, 0x29, 0x78, 0x8C, 0x67, 0xCA, 0x08, 0xBF, 0xEC, 0xCA, 0x43, 0xA9, 0x57, 0xAD, 0x16, 0xC9,
0x4E, 0x1C, 0xD8, 0x75, 0xCA, 0x10, 0x7D, 0xCE, 0x7E, 0x01, 0x18, 0xF0, 0xDF, 0x6B, 0xFE, 0xE5,
0x1D, 0xDB, 0xD9, 0x91, 0xC2, 0x6E, 0x60, 0xCD, 0x48, 0x58, 0xAA, 0x59, 0x2C, 0x82, 0x00, 0x75,
0xF2, 0x9F, 0x52, 0x6C, 0x91, 0x7C, 0x6F, 0xE5, 0x40, 0x3E, 0xA7, 0xD4, 0xA5, 0x0C, 0xEC, 0x3B,
0x73, 0x84, 0xDE, 0x88, 0x6E, 0x82, 0xD2, 0xEB, 0x4D, 0x4E, 0x42, 0xB5, 0xF2, 0xB1, 0x49, 0xA8,
0x1E, 0xA7, 0xCE, 0x71, 0x44, 0xDC, 0x29, 0x94, 0xCF, 0xC4, 0x4E, 0x1F, 0x91, 0xCB, 0xD4, 0x95,
0x00, 0x01, 0x00, 0x01
};
