/*
 * g_geoip.cpp
 *
 * GeoIP country flag support for xmod
 * Based on ET Legacy implementation
 */

#include <bgame/impl.h>
#include "g_geoip.h"
#include "g_local.h"

// GeoIP database handle
GeoIP *gidb = NULL;

// Country names for flag display
const char *country_name[MAX_COUNTRY_NUM] = {
    "N/A",                                    // 0
    "Asia/Pacific Region",                    // 1
    "Europe",                                 // 2
    "Andorra",                                // 3
    "United Arab Emirates",                   // 4
    "Afghanistan",                            // 5
    "Antigua and Barbuda",                    // 6
    "Anguilla",                               // 7
    "Albania",                                // 8
    "Armenia",                                // 9
    "Netherlands Antilles",                   // 10
    "Angola",                                 // 11
    "Antarctica",                             // 12
    "Argentina",                              // 13
    "American Samoa",                         // 14
    "Austria",                                // 15
    "Australia",                              // 16
    "Aruba",                                  // 17
    "Azerbaijan",                             // 18
    "Bosnia and Herzegovina",                 // 19
    "Barbados",                               // 20
    "Bangladesh",                             // 21
    "Belgium",                                // 22
    "Burkina Faso",                           // 23
    "Bulgaria",                               // 24
    "Bahrain",                                // 25
    "Burundi",                                // 26
    "Benin",                                  // 27
    "Bermuda",                                // 28
    "Brunei Darussalam",                      // 29
    "Bolivia",                                // 30
    "Brazil",                                 // 31
    "Bahamas",                                // 32
    "Bhutan",                                 // 33
    "Bouvet Island",                          // 34
    "Botswana",                               // 35
    "Belarus",                                // 36
    "Belize",                                 // 37
    "Canada",                                 // 38
    "Cocos (Keeling) Islands",                // 39
    "Congo, The Democratic Republic of the",  // 40
    "Central African Republic",               // 41
    "Congo",                                  // 42
    "Switzerland",                            // 43
    "Cote D'Ivoire",                          // 44
    "Cook Islands",                           // 45
    "Chile",                                  // 46
    "Cameroon",                               // 47
    "China",                                  // 48
    "Colombia",                               // 49
    "Costa Rica",                             // 50
    "Serbia and Montenegro",                  // 51
    "Cuba",                                   // 52
    "Cape Verde",                             // 53
    "Christmas Island",                       // 54
    "Cyprus",                                 // 55
    "Czech Republic",                         // 56
    "Germany",                                // 57
    "Djibouti",                               // 58
    "Denmark",                                // 59
    "Dominica",                               // 60
    "Dominican Republic",                     // 61
    "Algeria",                                // 62
    "Ecuador",                                // 63
    "Estonia",                                // 64
    "Egypt",                                  // 65
    "Western Sahara",                         // 66
    "Eritrea",                                // 67
    "Spain",                                  // 68
    "Ethiopia",                               // 69
    "Finland",                                // 70
    "Fiji",                                   // 71
    "Falkland Islands (Malvinas)",            // 72
    "Micronesia, Federated States of",        // 73
    "Faroe Islands",                          // 74
    "France",                                 // 75
    "France, Metropolitan",                   // 76
    "Gabon",                                  // 77
    "United Kingdom",                         // 78
    "Grenada",                                // 79
    "Georgia",                                // 80
    "French Guiana",                          // 81
    "Ghana",                                  // 82
    "Gibraltar",                              // 83
    "Greenland",                              // 84
    "Gambia",                                 // 85
    "Guinea",                                 // 86
    "Guadeloupe",                             // 87
    "Equatorial Guinea",                      // 88
    "Greece",                                 // 89
    "South Georgia and the South Sandwich Islands", // 90
    "Guatemala",                              // 91
    "Guam",                                   // 92
    "Guinea-Bissau",                          // 93
    "Guyana",                                 // 94
    "Hong Kong",                              // 95
    "Heard Island and McDonald Islands",      // 96
    "Honduras",                               // 97
    "Croatia",                                // 98
    "Haiti",                                  // 99
    "Hungary",                                // 100
    "Indonesia",                              // 101
    "Ireland",                                // 102
    "Israel",                                 // 103
    "India",                                  // 104
    "British Indian Ocean Territory",         // 105
    "Iraq",                                   // 106
    "Iran, Islamic Republic of",              // 107
    "Iceland",                                // 108
    "Italy",                                  // 109
    "Jamaica",                                // 110
    "Jordan",                                 // 111
    "Japan",                                  // 112
    "Kenya",                                  // 113
    "Kyrgyzstan",                             // 114
    "Cambodia",                               // 115
    "Kiribati",                               // 116
    "Comoros",                                // 117
    "Saint Kitts and Nevis",                  // 118
    "Korea, Democratic People's Republic of", // 119
    "Korea, Republic of",                     // 120
    "Kuwait",                                 // 121
    "Cayman Islands",                         // 122
    "Kazakhstan",                             // 123
    "Lao People's Democratic Republic",       // 124
    "Lebanon",                                // 125
    "Saint Lucia",                            // 126
    "Liechtenstein",                          // 127
    "Sri Lanka",                              // 128
    "Liberia",                                // 129
    "Lesotho",                                // 130
    "Lithuania",                              // 131
    "Luxembourg",                             // 132
    "Latvia",                                 // 133
    "Libyan Arab Jamahiriya",                 // 134
    "Morocco",                                // 135
    "Monaco",                                 // 136
    "Moldova, Republic of",                   // 137
    "Madagascar",                             // 138
    "Marshall Islands",                       // 139
    "Macedonia",                              // 140
    "Mali",                                   // 141
    "Myanmar",                                // 142
    "Mongolia",                               // 143
    "Macau",                                  // 144
    "Northern Mariana Islands",               // 145
    "Martinique",                             // 146
    "Mauritania",                             // 147
    "Montserrat",                             // 148
    "Malta",                                  // 149
    "Mauritius",                              // 150
    "Maldives",                               // 151
    "Malawi",                                 // 152
    "Mexico",                                 // 153
    "Malaysia",                               // 154
    "Mozambique",                             // 155
    "Namibia",                                // 156
    "New Caledonia",                          // 157
    "Niger",                                  // 158
    "Norfolk Island",                         // 159
    "Nigeria",                                // 160
    "Nicaragua",                              // 161
    "Netherlands",                            // 162
    "Norway",                                 // 163
    "Nepal",                                  // 164
    "Nauru",                                  // 165
    "Niue",                                   // 166
    "New Zealand",                            // 167
    "Oman",                                   // 168
    "Panama",                                 // 169
    "Peru",                                   // 170
    "French Polynesia",                       // 171
    "Papua New Guinea",                       // 172
    "Philippines",                            // 173
    "Pakistan",                               // 174
    "Poland",                                 // 175
    "Saint Pierre and Miquelon",              // 176
    "Pitcairn Islands",                       // 177
    "Puerto Rico",                            // 178
    "Palestinian Territory",                  // 179
    "Portugal",                               // 180
    "Palau",                                  // 181
    "Paraguay",                               // 182
    "Qatar",                                  // 183
    "Reunion",                                // 184
    "Romania",                                // 185
    "Russian Federation",                     // 186
    "Rwanda",                                 // 187
    "Saudi Arabia",                           // 188
    "Solomon Islands",                        // 189
    "Seychelles",                             // 190
    "Sudan",                                  // 191
    "Sweden",                                 // 192
    "Singapore",                              // 193
    "Saint Helena",                           // 194
    "Slovenia",                               // 195
    "Svalbard and Jan Mayen",                 // 196
    "Slovakia",                               // 197
    "Sierra Leone",                           // 198
    "San Marino",                             // 199
    "Senegal",                                // 200
    "Somalia",                                // 201
    "Suriname",                               // 202
    "Sao Tome and Principe",                  // 203
    "El Salvador",                            // 204
    "Syrian Arab Republic",                   // 205
    "Swaziland",                              // 206
    "Turks and Caicos Islands",               // 207
    "Chad",                                   // 208
    "French Southern Territories",            // 209
    "Togo",                                   // 210
    "Thailand",                               // 211
    "Tajikistan",                             // 212
    "Tokelau",                                // 213
    "Turkmenistan",                           // 214
    "Tunisia",                                // 215
    "Tonga",                                  // 216
    "Timor-Leste",                            // 217
    "Turkey",                                 // 218
    "Trinidad and Tobago",                    // 219
    "Tuvalu",                                 // 220
    "Taiwan",                                 // 221
    "Tanzania, United Republic of",           // 222
    "Ukraine",                                // 223
    "Uganda",                                 // 224
    "United States Minor Outlying Islands",   // 225
    "United States",                          // 226
    "Uruguay",                                // 227
    "Uzbekistan",                             // 228
    "Holy See (Vatican City State)",          // 229
    "Saint Vincent and the Grenadines",       // 230
    "Venezuela",                              // 231
    "Virgin Islands, British",                // 232
    "Virgin Islands, U.S.",                   // 233
    "Vietnam",                                // 234
    "Vanuatu",                                // 235
    "Wallis and Futuna",                      // 236
    "Samoa",                                  // 237
    "Yemen",                                  // 238
    "Mayotte",                                // 239
    "Serbia",                                 // 240
    "South Africa",                           // 241
    "Zambia",                                 // 242
    "Montenegro",                             // 243
    "Zimbabwe",                               // 244
    "Anonymous Proxy",                        // 245
    "Satellite Provider",                     // 246
    "Other",                                  // 247
    "Aland Islands",                          // 248
    "Guernsey",                               // 249
    "Isle of Man",                            // 250
    "Jersey",                                 // 251
    "Saint Barthelemy",                       // 252
    "Saint Martin",                           // 253
    "Localhost",                              // 254
    "Unknown"                                 // 255
};

#define GEOIP_COUNTRY_BEGIN 16776960
#define GEOIP_STANDARD      0
#define GEOIP_MEMORY_CACHE  1
#define GEOIP_SEEK_SET      0

/*
=================
GeoIP_addr_to_num

Converts an IP address string to a long integer
=================
*/
unsigned long GeoIP_addr_to_num(const char *addr) {
    unsigned int    c, octet, t;
    unsigned long   ipnum;
    int             i = 3;

    octet = ipnum = 0;
    while ((c = *addr++)) {
        if (c == '.') {
            if (octet > 255) {
                return 0;
            }
            ipnum <<= 8;
            ipnum += octet;
            i--;
            octet = 0;
        } else {
            t = octet;
            octet <<= 3;
            octet += t;
            octet += t;
            c -= '0';
            if (c > 9) {
                return 0;
            }
            octet += c;
        }
    }
    if ((octet > 255) || (i != 0)) {
        return 0;
    }
    ipnum <<= 8;
    return ipnum + octet;
}

/*
=================
GeoIP_seek_record

Binary search in GeoIP database for country code
Requires the database to be loaded into memory cache

GeoIP database is a binary tree where each node is 6 bytes:
- Bytes 0-2: pointer to left child (when IP bit is 0)
- Bytes 3-5: pointer to right child (when IP bit is 1)
=================
*/
unsigned int GeoIP_seek_record(GeoIP *gi, unsigned long ipnum) {
    int             depth;
    unsigned int    x = 0;
    unsigned int    step;
    const unsigned char *buf;

    if (gi == NULL || gi->cache == NULL) {
        return 0;
    }

    for (depth = 31; depth >= 0; depth--) {
        step = 6 * x;

        if (step + 6 > gi->memsize) {
            return 255;
        }

        buf = gi->cache + step;

        if (ipnum & (1 << depth)) {
            // Right branch: read bytes 3-5
            x = (buf[3] << 0) + (buf[4] << 8) + (buf[5] << 16);
        } else {
            // Left branch: read bytes 0-2
            x = (buf[0] << 0) + (buf[1] << 8) + (buf[2] << 16);
        }

        if (x >= GEOIP_COUNTRY_BEGIN) {
            return x - GEOIP_COUNTRY_BEGIN;
        }
    }

    return 255;
}

/*
=================
GeoIP_open

Open and cache the GeoIP database into memory
=================
*/
void GeoIP_open(void) {
    int len;
    fileHandle_t f;

    if (gidb != NULL) {
        return;
    }

    gidb = (GeoIP *)malloc(sizeof(GeoIP));
    if (gidb == NULL) {
        G_Printf("GeoIP: Memory allocation error for GeoIP struct\n");
        return;
    }

    gidb->cache = NULL;
    gidb->memsize = 0;
    gidb->GeoIPDatabase = 0;

    len = trap_FS_FOpenFile("GeoIP.dat", &f, FS_READ);
    if (len < 0) {
        G_Printf("GeoIP: GeoIP.dat not found. Country flags will not be available.\n");
        free(gidb);
        gidb = NULL;
        return;
    }

    gidb->cache = (unsigned char *)malloc((size_t)len);
    if (gidb->cache != NULL) {
        gidb->memsize = (unsigned int)len;
        trap_FS_Read(gidb->cache, len, f);
        trap_FS_FCloseFile(f);
        G_Printf("GeoIP: Loaded GeoIP.dat (%d bytes) into memory.\n", len);
    } else {
        trap_FS_FCloseFile(f);
        G_Printf("GeoIP: Memory allocation error. Country flags will not be available.\n");
        free(gidb);
        gidb = NULL;
    }
}

/*
=================
GeoIP_close

Close and free the GeoIP database
=================
*/
void GeoIP_close(void) {
    if (gidb == NULL) {
        return;
    }

    if (gidb->GeoIPDatabase) {
        trap_FS_FCloseFile(gidb->GeoIPDatabase);
    }
    if (gidb->cache != NULL) {
        free(gidb->cache);
    }
    free(gidb);
    gidb = NULL;
}
