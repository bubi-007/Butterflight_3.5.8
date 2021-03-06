/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Author: jflyper@github.com
 */

#include "common/utils.h"
#include "common/printf.h"

#include "emfat.h"
#include "emfat_file.h"

#include "io/flashfs.h"

#define FILESYSTEM_SIZE_MB 256

#define USE_EMFAT_AUTORUN
#define USE_EMFAT_ICON
//#define USE_EMFAT_README

#ifdef USE_EMFAT_AUTORUN
static const char autorun_file[] =
    "[autorun]\r\n"
    "icon=icon.ico\r\n"
    "label=ButterFlight Onboard Flash\r\n" ;
#define AUTORUN_SIZE (sizeof(autorun_file) - 1)
#define EMFAT_INCR_AUTORUN 1
#else
#define EMFAT_INCR_AUTORUN 0
#endif

#ifdef USE_EMFAT_README
static const char readme_file[] =
    "This is readme file\r\n";
#define README_SIZE  (sizeof(readme_file) - 1)
#define EMFAT_INCR_README 1
#else
#define EMFAT_INCR_README 0
#endif

#ifdef USE_EMFAT_ICON
static const char icon_file[] =
{
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x18, 0x18, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x28, 0x09,
    0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x30, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xfc,
    0xfc, 0xde, 0xfc, 0xfc, 0xfc, 0xee, 0xfb, 0xfb, 0xfb, 0xee, 0xfb, 0xfb, 0xfb, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfe, 0xfe, 0xfe, 0xee, 0xfe, 0xfe, 0xfe, 0xee, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff,
    0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xfe, 0xfe, 0xfe, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfb, 0xfb, 0xfb, 0xee, 0xfb, 0xfb, 0xfb, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfc, 0xfc, 0xfc, 0xde, 0xfb, 0xfb,
    0xfb, 0xf4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbb, 0xde,
    0xfc, 0xff, 0x79, 0xbe, 0xfa, 0xff, 0x54, 0xad, 0xf9, 0xff, 0x42, 0xa4, 0xf8, 0xff, 0x42, 0xa4,
    0xf8, 0xff, 0x53, 0xac, 0xf9, 0xff, 0x77, 0xbd, 0xfa, 0xff, 0xb7, 0xdb, 0xfc, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc, 0xfc, 0xf4, 0xfd, 0xfd,
    0xfd, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xfe,
    0xfe, 0xff, 0xf3, 0xeb, 0xe2, 0xff, 0xac, 0xda, 0xfc, 0xff, 0x34, 0x9e, 0xf9, 0xff, 0x00, 0x7e,
    0xf6, 0xff, 0x00, 0x72, 0xf4, 0xff, 0x00, 0x72, 0xf5, 0xff, 0x00, 0x77, 0xf7, 0xff, 0x00, 0x77,
    0xf7, 0xff, 0x00, 0x72, 0xf5, 0xff, 0x00, 0x6e, 0xf4, 0xff, 0x00, 0x6e, 0xf4, 0xff, 0x28, 0x98,
    0xf8, 0xff, 0xad, 0xda, 0xfc, 0xff, 0xf1, 0xe8, 0xe0, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xfe,
    0xfe, 0xff, 0xfd, 0xfd, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfc, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x68, 0xc1, 0xff, 0xff, 0x00, 0x7c, 0xf8, 0xff, 0x00, 0x72, 0xf4, 0xff, 0x12, 0x92,
    0xff, 0xff, 0x6c, 0xc9, 0xff, 0xff, 0xa2, 0xe6, 0xff, 0xff, 0xb2, 0xe6, 0xff, 0xff, 0xb2, 0xe6,
    0xff, 0xff, 0x9d, 0xe4, 0xff, 0xff, 0x8f, 0xd4, 0xff, 0xff, 0xad, 0xd8, 0xfc, 0xff, 0x00, 0x81,
    0xf5, 0xff, 0x00, 0x73, 0xf7, 0xff, 0x61, 0xbc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfd,
    0xfd, 0xff, 0xfd, 0xfd, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfb, 0xfb,
    0xfb, 0xee, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xf2, 0xee, 0xeb, 0xff, 0x44, 0xae,
    0xff, 0xff, 0x00, 0x73, 0xf5, 0xff, 0x00, 0x81, 0xf8, 0xff, 0x7c, 0xd0, 0xff, 0xff, 0xda, 0xef,
    0xff, 0xff, 0xc5, 0xbb, 0xb2, 0xff, 0xa3, 0x8f, 0x7e, 0xff, 0x8b, 0x76, 0x64, 0xff, 0x8a, 0x75,
    0x63, 0xff, 0x98, 0x84, 0x74, 0xff, 0xee, 0xde, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa6, 0xe5,
    0xff, 0xff, 0x00, 0x7a, 0xf8, 0xff, 0x00, 0x73, 0xf5, 0xff, 0x3d, 0xaa, 0xff, 0xff, 0xf4, 0xf1,
    0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc, 0xfc, 0xee, 0xfa, 0xfa,
    0xfa, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x56, 0xba, 0xff, 0xff, 0x00, 0x6f,
    0xf4, 0xff, 0x0a, 0x8c, 0xfa, 0xff, 0xcb, 0xf6, 0xff, 0xff, 0xe6, 0xcf, 0xb9, 0xff, 0x54, 0x46,
    0x39, 0xff, 0x0f, 0x0d, 0x0b, 0xff, 0x0e, 0x0d, 0x0d, 0xff, 0x18, 0x18, 0x18, 0xff, 0x19, 0x19,
    0x19, 0xff, 0x00, 0x00, 0x00, 0xff, 0x57, 0x57, 0x56, 0xff, 0xcb, 0xcb, 0xcb, 0xff, 0xcc, 0xbc,
    0xac, 0xff, 0xd2, 0xf8, 0xff, 0xff, 0x0d, 0x8e, 0xfb, 0xff, 0x00, 0x6f, 0xf4, 0xff, 0x4d, 0xb5,
    0xff, 0xff, 0xfd, 0xfc, 0xfb, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xf3, 0xf0, 0xec, 0xff, 0xa2, 0xd9, 0xfc, 0xff, 0x00, 0x7c, 0xf9, 0xff, 0x00, 0x84,
    0xf9, 0xff, 0xd7, 0xff, 0xff, 0xff, 0xd3, 0xb4, 0x99, 0xff, 0x08, 0x05, 0x03, 0xff, 0x1a, 0x1a,
    0x1b, 0xff, 0x35, 0x35, 0x35, 0xff, 0x34, 0x34, 0x34, 0xff, 0x35, 0x35, 0x35, 0xff, 0x2f, 0x2f,
    0x2f, 0xff, 0x0b, 0x0b, 0x0b, 0xff, 0x40, 0x40, 0x40, 0xff, 0x8f, 0x8f, 0x8f, 0xff, 0x00, 0x00,
    0x00, 0xff, 0xaf, 0x90, 0x75, 0xff, 0xdf, 0xff, 0xff, 0xff, 0x03, 0x87, 0xf9, 0xff, 0x00, 0x79,
    0xf8, 0xff, 0xa7, 0xdb, 0xfc, 0xff, 0xf8, 0xf6, 0xf6, 0xff, 0xff, 0xff, 0xff, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xf5, 0xf8, 0xfd, 0xff, 0x24, 0x9f, 0xfd, 0xff, 0x00, 0x70, 0xf4, 0xff, 0x99, 0xe3,
    0xff, 0xff, 0xec, 0xd0, 0xb8, 0xff, 0x03, 0x01, 0x00, 0xff, 0x2a, 0x2a, 0x2b, 0xff, 0x37, 0x37,
    0x37, 0xff, 0x33, 0x33, 0x33, 0xff, 0x1f, 0x1f, 0x1f, 0xff, 0x0a, 0x0a, 0x0a, 0xff, 0x1f, 0x1f,
    0x1f, 0xff, 0x56, 0x56, 0x56, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc4, 0xc4, 0xc4, 0xff, 0x84, 0x85,
    0x85, 0xff, 0x38, 0x37, 0x36, 0xff, 0xcf, 0xb2, 0x9a, 0xff, 0xa1, 0xe8, 0xff, 0xff, 0x00, 0x71,
    0xf4, 0xff, 0x1e, 0x9a, 0xfd, 0xff, 0xf0, 0xf5, 0xf7, 0xff, 0xf7, 0xf7, 0xf7, 0xee, 0xf9, 0xf7,
    0xf5, 0xee, 0xa2, 0xd6, 0xff, 0xff, 0x00, 0x7c, 0xfb, 0xff, 0x27, 0xa0, 0xfe, 0xff, 0xed, 0xf4,
    0xf4, 0xff, 0x4c, 0x3d, 0x31, 0xff, 0x1d, 0x1e, 0x1f, 0xff, 0x2a, 0x2a, 0x2a, 0xff, 0x0e, 0x0e,
    0x0e, 0xff, 0x14, 0x14, 0x14, 0xff, 0x46, 0x46, 0x46, 0xff, 0x8e, 0x8e, 0x8e, 0xff, 0xda, 0xda,
    0xda, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd7, 0xd7, 0xd7, 0xff, 0xca, 0xca,
    0xca, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7b, 0x6d, 0x62, 0xff, 0xcb, 0xcf, 0xd0, 0xff, 0x2e, 0xa6,
    0xff, 0xff, 0x00, 0x7a, 0xfa, 0xff, 0x9d, 0xd4, 0xff, 0xff, 0xee, 0xec, 0xeb, 0xee, 0xfd, 0xfb,
    0xf9, 0xee, 0x68, 0xbd, 0xfd, 0xff, 0x00, 0x6d, 0xf3, 0xff, 0x93, 0xe1, 0xff, 0xff, 0xc5, 0xb1,
    0xa3, 0xff, 0x00, 0x00, 0x00, 0xff, 0x14, 0x14, 0x14, 0xff, 0x46, 0x46, 0x46, 0xff, 0x7a, 0x7a,
    0x7a, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb1, 0xb1,
    0xb1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb0, 0xb0, 0xb1, 0xff, 0xa2, 0x8d, 0x7c, 0xff, 0x96, 0xe1,
    0xff, 0xff, 0x00, 0x6d, 0xf3, 0xff, 0x62, 0xba, 0xfd, 0xff, 0xfa, 0xf8, 0xf7, 0xee, 0xef, 0xee,
    0xee, 0xee, 0x4c, 0xb3, 0xff, 0xff, 0x00, 0x77, 0xf7, 0xff, 0xb3, 0xe7, 0xfd, 0xff, 0x96, 0x80,
    0x6c, 0xff, 0x4b, 0x4c, 0x4c, 0xff, 0x9f, 0x9f, 0x9f, 0xff, 0xaf, 0xaf, 0xaf, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xf7, 0xf7, 0xff, 0x79, 0x79,
    0x79, 0xff, 0x83, 0x83, 0x83, 0xff, 0xfc, 0xfc, 0xfc, 0xff, 0xc8, 0xb1, 0x9e, 0xff, 0xa1, 0xd1,
    0xf6, 0xff, 0x00, 0x79, 0xf8, 0xff, 0x45, 0xb0, 0xff, 0xff, 0xe8, 0xe8, 0xe8, 0xee, 0xfc, 0xfe,
    0xff, 0xee, 0x28, 0x98, 0xf9, 0xff, 0x00, 0x82, 0xfb, 0xff, 0x9b, 0xc3, 0xe9, 0xff, 0xd5, 0xc0,
    0xaf, 0xff, 0xf4, 0xf4, 0xf4, 0xff, 0xad, 0xad, 0xad, 0xff, 0xb3, 0xb3, 0xb3, 0xff, 0xf2, 0xf2,
    0xf2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xc7, 0xc7, 0xc7, 0xff, 0x97, 0x97, 0x97, 0xff, 0x92, 0x92, 0x92, 0xff, 0xaf, 0xaf,
    0xaf, 0xff, 0xb7, 0xb7, 0xb7, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xb4, 0xa0, 0x90, 0xff, 0xa7, 0xcc,
    0xef, 0xff, 0x00, 0x86, 0xfd, 0xff, 0x24, 0x98, 0xf9, 0xff, 0xee, 0xf4, 0xfa, 0xee, 0xec, 0xf0,
    0xf1, 0xee, 0x38, 0xa8, 0xfe, 0xff, 0x00, 0x80, 0xfb, 0xff, 0xad, 0xd6, 0xf9, 0xff, 0x98, 0x84,
    0x72, 0xff, 0xda, 0xda, 0xda, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xdb, 0xdb, 0xff, 0xa5, 0xa5, 0xa5, 0xff, 0x8f, 0x8f,
    0x8f, 0xff, 0x96, 0x96, 0x96, 0xff, 0xc9, 0xc9, 0xc9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x73, 0x73, 0x73, 0xff, 0x64, 0x51, 0x40, 0xff, 0xbf, 0xe5,
    0xff, 0xff, 0x00, 0x84, 0xfc, 0xff, 0x34, 0xa7, 0xfe, 0xff, 0xdc, 0xe1, 0xe6, 0xee, 0xfb, 0xfb,
    0xfb, 0xee, 0x44, 0xa9, 0xfa, 0xff, 0x00, 0x74, 0xf6, 0xff, 0xb5, 0xec, 0xff, 0xff, 0x79, 0x62,
    0x4f, 0xff, 0x9c, 0x9d, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xff, 0xb1, 0xb1,
    0xb1, 0xff, 0x92, 0x92, 0x92, 0xff, 0x92, 0x92, 0x92, 0xff, 0xb6, 0xb6, 0xb6, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xae, 0xae, 0xae, 0xff, 0x60, 0x60,
    0x60, 0xff, 0x24, 0x24, 0x24, 0xff, 0x00, 0x00, 0x00, 0xff, 0x9a, 0x83, 0x6f, 0xff, 0xba, 0xef,
    0xff, 0xff, 0x00, 0x76, 0xf6, 0xff, 0x3d, 0xa5, 0xfa, 0xff, 0xf9, 0xf9, 0xf9, 0xee, 0xf7, 0xf2,
    0xee, 0xee, 0x7d, 0xcc, 0xff, 0xff, 0x00, 0x6d, 0xf4, 0xff, 0x7f, 0xd6, 0xff, 0xff, 0xb7, 0xa6,
    0x9b, 0xff, 0x37, 0x37, 0x36, 0xff, 0xc7, 0xc7, 0xc7, 0xff, 0xa5, 0xa5, 0xa5, 0xff, 0xa6, 0xa6,
    0xa6, 0xff, 0xeb, 0xeb, 0xeb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc,
    0xcc, 0xff, 0x79, 0x79, 0x79, 0xff, 0x32, 0x32, 0x32, 0xff, 0x0d, 0x0d, 0x0d, 0xff, 0x15, 0x15,
    0x15, 0xff, 0x30, 0x30, 0x30, 0xff, 0x0a, 0x0a, 0x09, 0xff, 0xcf, 0xbc, 0xb1, 0xff, 0x86, 0xd9,
    0xff, 0xff, 0x00, 0x6d, 0xf3, 0xff, 0x78, 0xca, 0xff, 0xff, 0xf3, 0xee, 0xea, 0xee, 0xfa, 0xf9,
    0xf8, 0xee, 0xae, 0xd7, 0xf7, 0xff, 0x00, 0x85, 0xfd, 0xff, 0x13, 0x92, 0xfc, 0xff, 0xef, 0xff,
    0xff, 0xff, 0x49, 0x35, 0x25, 0xff, 0x18, 0x1a, 0x1b, 0xff, 0xdc, 0xdc, 0xdc, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xeb, 0xeb, 0xeb, 0xff, 0x8f, 0x8f, 0x8f, 0xff, 0x47, 0x47, 0x47, 0xff, 0x15, 0x15,
    0x15, 0xff, 0x0e, 0x0e, 0x0e, 0xff, 0x28, 0x28, 0x28, 0xff, 0x35, 0x35, 0x35, 0xff, 0x37, 0x37,
    0x37, 0xff, 0x11, 0x13, 0x14, 0xff, 0x62, 0x4f, 0x40, 0xff, 0xed, 0xfc, 0xfc, 0xff, 0x18, 0x96,
    0xfd, 0xff, 0x00, 0x82, 0xfc, 0xff, 0xa9, 0xd6, 0xf9, 0xff, 0xf8, 0xf7, 0xf5, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xec, 0xec, 0xed, 0xff, 0x3b, 0xaa, 0xfe, 0xff, 0x00, 0x6f, 0xf4, 0xff, 0x73, 0xce,
    0xff, 0xff, 0xff, 0xf2, 0xda, 0xff, 0x08, 0x01, 0x00, 0xff, 0x0f, 0x10, 0x11, 0xff, 0x4f, 0x4f,
    0x4f, 0xff, 0x2b, 0x2b, 0x2b, 0xff, 0x0b, 0x0b, 0x0b, 0xff, 0x1f, 0x1f, 0x1f, 0xff, 0x33, 0x33,
    0x33, 0xff, 0x34, 0x34, 0x34, 0xff, 0x33, 0x33, 0x33, 0xff, 0x37, 0x37, 0x37, 0xff, 0x1d, 0x1e,
    0x1f, 0xff, 0x16, 0x10, 0x0c, 0xff, 0xff, 0xea, 0xd1, 0xff, 0x7b, 0xd2, 0xff, 0xff, 0x00, 0x70,
    0xf4, 0xff, 0x34, 0xa7, 0xff, 0xff, 0xea, 0xea, 0xed, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xe0, 0xe0, 0xe0, 0xff, 0xb7, 0xdc, 0xfb, 0xff, 0x00, 0x7a, 0xf5, 0xff, 0x00, 0x7a,
    0xf7, 0xff, 0xa7, 0xea, 0xff, 0xff, 0xff, 0xe2, 0xc4, 0xff, 0x23, 0x1a, 0x12, 0xff, 0x00, 0x00,
    0x00, 0xff, 0x27, 0x27, 0x27, 0xff, 0x36, 0x36, 0x36, 0xff, 0x34, 0x34, 0x34, 0xff, 0x33, 0x33,
    0x33, 0xff, 0x35, 0x35, 0x35, 0xff, 0x2f, 0x2f, 0x2f, 0xff, 0x0a, 0x0b, 0x0b, 0xff, 0x25, 0x1c,
    0x15, 0xff, 0xfb, 0xdb, 0xbd, 0xff, 0xaf, 0xed, 0xff, 0xff, 0x00, 0x7b, 0xf7, 0xff, 0x00, 0x79,
    0xf5, 0xff, 0xad, 0xd6, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc, 0xfc, 0xee, 0xfb, 0xfb,
    0xfb, 0xee, 0xf7, 0xf8, 0xf8, 0xff, 0xf0, 0xf2, 0xf2, 0xff, 0x76, 0xbe, 0xfa, 0xff, 0x00, 0x74,
    0xf4, 0xff, 0x00, 0x7d, 0xf7, 0xff, 0x95, 0xdc, 0xff, 0xff, 0xf9, 0xef, 0xe3, 0xff, 0x89, 0x74,
    0x62, 0xff, 0x2c, 0x25, 0x1e, 0xff, 0x0e, 0x0d, 0x0b, 0xff, 0x0f, 0x0f, 0x0e, 0xff, 0x0f, 0x0f,
    0x0f, 0xff, 0x0e, 0x0d, 0x0b, 0xff, 0x2b, 0x23, 0x1d, 0xff, 0x86, 0x71, 0x5f, 0xff, 0xf7, 0xec,
    0xe0, 0xff, 0x9b, 0xe0, 0xff, 0xff, 0x00, 0x7f, 0xf7, 0xff, 0x00, 0x74, 0xf4, 0xff, 0x6a, 0xb9,
    0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xfb, 0xfb, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfe, 0xfe, 0xff, 0xfb, 0xfb, 0xfc, 0xff, 0xfa, 0xfb, 0xfc, 0xff, 0x63, 0xb3,
    0xf9, 0xff, 0x00, 0x76, 0xf4, 0xff, 0x00, 0x76, 0xf5, 0xff, 0x48, 0xb2, 0xff, 0xff, 0xc3, 0xf0,
    0xff, 0xff, 0xd0, 0xd7, 0xdd, 0xff, 0xbb, 0xaf, 0xa6, 0xff, 0xaf, 0x9c, 0x8d, 0xff, 0xaf, 0x9c,
    0x8c, 0xff, 0xba, 0xaf, 0xa5, 0xff, 0xcf, 0xd6, 0xdb, 0xff, 0xc5, 0xf1, 0xff, 0xff, 0x4c, 0xb4,
    0xff, 0xff, 0x00, 0x77, 0xf6, 0xff, 0x00, 0x75, 0xf5, 0xff, 0x58, 0xae, 0xf9, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfb, 0xfd, 0xfd, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x84, 0xc3, 0xfb, 0xff, 0x03, 0x85, 0xf6, 0xff, 0x00, 0x72, 0xf4, 0xff, 0x00, 0x7f,
    0xf8, 0xff, 0x37, 0xa9, 0xff, 0xff, 0x75, 0xd0, 0xff, 0xff, 0x8f, 0xdc, 0xff, 0xff, 0x8f, 0xdc,
    0xff, 0xff, 0x76, 0xd1, 0xff, 0xff, 0x39, 0xab, 0xff, 0xff, 0x00, 0x80, 0xf8, 0xff, 0x00, 0x72,
    0xf5, 0xff, 0x01, 0x84, 0xf6, 0xff, 0x7e, 0xc0, 0xfa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe,
    0xfe, 0xff, 0xfc, 0xfc, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd8, 0xed, 0xfd, 0xff, 0x5e, 0xb1, 0xf9, 0xff, 0x12, 0x8c,
    0xf6, 0xff, 0x00, 0x79, 0xf5, 0xff, 0x00, 0x71, 0xf5, 0xff, 0x00, 0x70, 0xf4, 0xff, 0x00, 0x70,
    0xf4, 0xff, 0x00, 0x71, 0xf5, 0xff, 0x00, 0x79, 0xf5, 0xff, 0x10, 0x8c, 0xf7, 0xff, 0x5b, 0xb0,
    0xf9, 0xff, 0xd4, 0xeb, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xfd, 0xfd, 0xee, 0xfb, 0xfb,
    0xfb, 0xf4, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec, 0xf6,
    0xfe, 0xff, 0xaa, 0xd5, 0xfc, 0xff, 0x83, 0xc3, 0xfa, 0xff, 0x6f, 0xba, 0xfa, 0xff, 0x6f, 0xba,
    0xfa, 0xff, 0x82, 0xc3, 0xfa, 0xff, 0xa8, 0xd5, 0xfc, 0xff, 0xeb, 0xf5, 0xfe, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xfb, 0xfb, 0xf4, 0xfd, 0xfd,
    0xfd, 0xde, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfc, 0xfc, 0xfc, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfe, 0xfe,
    0xfe, 0xee, 0xfe, 0xfe, 0xfe, 0xee, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff,
    0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xfe, 0xfe, 0xfe, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfb, 0xfb, 0xfb, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd,
    0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xee, 0xfd, 0xfd, 0xfd, 0xde,
};

#define ICON_SIZE    (sizeof(icon_file))
#define EMFAT_INCR_ICON 1
#else
#define EMFAT_INCR_ICON 0
#endif

#define CMA_TIME EMFAT_ENCODE_CMA_TIME(1,1,2018, 13,0,0)
#define CMA { CMA_TIME, CMA_TIME, CMA_TIME }

static void memory_read_proc(uint8_t *dest, int size, uint32_t offset, emfat_entry_t *entry)
{
    int len;

    if (offset > entry->curr_size) {
        return;
    }

    if (offset + size > entry->curr_size) {
        len = entry->curr_size - offset;
    } else {
        len = size;
    }

    memcpy(dest, &((char *)entry->user_data)[offset], len);
}

static void bblog_read_proc(uint8_t *dest, int size, uint32_t offset, emfat_entry_t *entry)
{
    UNUSED(entry);

    flashfsReadAbs(offset, dest, size);
}

static const emfat_entry_t entriesPredefined[] =
{
    // name           dir    attr         lvl offset  size             max_size        user                time  read               write
    { "",             true,  0,           0,  0,      0,               0,              0,                  CMA,  NULL,              NULL, { 0 } },
#ifdef USE_EMFAT_AUTORUN
    { "autorun.inf",  false, ATTR_HIDDEN, 1,  0,      AUTORUN_SIZE,    AUTORUN_SIZE,   (long)autorun_file, CMA,  memory_read_proc,  NULL, { 0 } },
#endif
#ifdef USE_EMFAT_ICON
    { "icon.ico",     false, ATTR_HIDDEN, 1,  0,      ICON_SIZE,       ICON_SIZE,      (long)icon_file,    CMA,  memory_read_proc,  NULL, { 0 } },
#endif
#ifdef USE_EMFAT_README
    { "readme.txt",   false, 0,           1,  0,      README_SIZE,     1024*1024,      (long)readme_file,  CMA,  memory_read_proc,  NULL, { 0 } },
#endif
    { "BTTR_ALL.BBL", 0,     0,           1,  0,      0,               0,              0,                  CMA,  bblog_read_proc,   NULL, { 0 } },

    { "PADDING.TXT",  0,     ATTR_HIDDEN, 1,  0,      0,               0,              0,                  CMA,  NULL,              NULL, { 0 } },
};

#define PREDEFINED_ENTRY_COUNT (1 + EMFAT_INCR_AUTORUN + EMFAT_INCR_ICON + EMFAT_INCR_README)
#define APPENDED_ENTRY_COUNT 2

#define EMFAT_MAX_LOG_ENTRY 100
#define EMFAT_MAX_ENTRY (PREDEFINED_ENTRY_COUNT + EMFAT_MAX_LOG_ENTRY + APPENDED_ENTRY_COUNT)

static emfat_entry_t entries[EMFAT_MAX_ENTRY];
static char logNames[EMFAT_MAX_LOG_ENTRY][8+3];

emfat_t emfat;

static void emfat_add_log(emfat_entry_t *entry, int number, uint32_t offset, uint32_t size)
{
    tfp_sprintf(logNames[number], "BTTR_%03d.BBL", number + 1);

    entry->name = logNames[number];
    entry->level = 1;
    entry->offset = offset;
    entry->curr_size = size;
    entry->max_size = entry->curr_size;
    entry->cma_time[0] = CMA_TIME;
    entry->cma_time[1] = CMA_TIME;
    entry->cma_time[2] = CMA_TIME;
    entry->readcb = bblog_read_proc;
}

static int emfat_find_log(emfat_entry_t *entry, int maxCount)
{
    uint32_t limit  = flashfsIdentifyStartOfFreeSpace();
    uint32_t lastOffset = 0;
    uint32_t currOffset = 0;
    int fileNumber = 0;
    uint8_t buffer[18];
    int logCount = 0;

    for ( ; currOffset < limit ; currOffset += 2048) { // XXX 2048 = FREE_BLOCK_SIZE in io/flashfs.c

        flashfsReadAbs(currOffset, buffer, 18);

        if (strncmp((char *)buffer, "H Product:Blackbox", 18)) {
            continue;
        }

        if (lastOffset != currOffset) {
            emfat_add_log(entry, fileNumber, lastOffset, currOffset - lastOffset);

            ++fileNumber;
            ++logCount;
            if (fileNumber == maxCount) {
                break;
            }
            ++entry;
        }

        lastOffset = currOffset;
    }

    if (fileNumber != maxCount && lastOffset != currOffset) {
        emfat_add_log(entry, fileNumber, lastOffset, currOffset - lastOffset);
        ++logCount;
    }
    return logCount;
}

void emfat_init_files(void)
{
    emfat_entry_t *entry;
    memset(entries, 0, sizeof(entries));

    for (size_t i = 0 ; i < PREDEFINED_ENTRY_COUNT ; i++) {
        entries[i] = entriesPredefined[i];
    }

    // Detect and create entries for each individual log
    const int logCount = emfat_find_log(&entries[PREDEFINED_ENTRY_COUNT], EMFAT_MAX_LOG_ENTRY);

    int entryIndex = PREDEFINED_ENTRY_COUNT + logCount;

    if (logCount > 0) {
        // Create the all logs entry that represents all used flash space to
        // allow downloading the entire log in one file
        entries[entryIndex] = entriesPredefined[PREDEFINED_ENTRY_COUNT];
        entry = &entries[entryIndex];
        entry->curr_size = flashfsIdentifyStartOfFreeSpace();
        entry->max_size = entry->curr_size;
        ++entryIndex;
    }

    // Padding file to fill out the filesystem size to FILESYSTEM_SIZE_MB
    entries[entryIndex] = entriesPredefined[PREDEFINED_ENTRY_COUNT + 1];
    entry = &entries[entryIndex];
    // used space is doubled because of the individual files plus the single complete file
    entry->curr_size = (FILESYSTEM_SIZE_MB * 1024 * 1024) - (flashfsIdentifyStartOfFreeSpace() * 2);
    entry->max_size = entry->curr_size;

    emfat_init(&emfat, "BUTTERF", entries);

}
