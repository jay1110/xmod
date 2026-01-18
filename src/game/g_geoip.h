/*
 * g_geoip.h
 *
 * GeoIP country flag support for xmod
 * Based on ET Legacy implementation
 */

#ifndef GAME_G_GEOIP_H
#define GAME_G_GEOIP_H

///////////////////////////////////////////////////////////////////////////////

#define MAX_COUNTRY_NUM 256

typedef struct GeoIPTag {
    fileHandle_t GeoIPDatabase;
    unsigned char *cache;
    unsigned int memsize;
} GeoIP;

extern GeoIP *gidb;
extern const char *country_name[MAX_COUNTRY_NUM];

// GeoIP functions
unsigned long GeoIP_addr_to_num(const char *addr);
unsigned int GeoIP_seek_record(GeoIP *gi, unsigned long ipnum);
void GeoIP_open(void);
void GeoIP_close(void);

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_GEOIP_H
