/*
 * opensc-setup-custom-action.c: OpenSC setup custom action
 *
 * Copyright (C) 2015 vincent.letoux@mysmartlogon.com
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This module requires the WIX SDK to build.
 */

#include "config.h"
#ifdef ENABLE_MINIDRIVER

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <msiquery.h>

// WiX Header Files:
#include <wcautil.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
// only for VS 2015 or later
// WiX 3.10 was built for older versions of VS and needs this for compatibility
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#define X86onX64_SC_DATABASE TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Cryptography\\Calais\\SmartCards")
#define SC_DATABASE TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards")
#define BASE_CSP TEXT("OpenSC CSP")
#define BASE_KSP TEXT("Microsoft Smart Card Key Storage Provider")

typedef struct _MD_REGISTRATION
{
	TCHAR szName[256];
	BYTE pbAtr[256];
	DWORD dwAtrSize;
	BYTE pbAtrMask[256];
} MD_REGISTRATION, *PMD_REGISTRATION;

/* note: we could have added the minidriver registration data directly in OpenSC.wxs but coding it allows for more checks.
For example, do not uninstall the minidriver for a card if a middleware is already installed */

MD_REGISTRATION minidriver_registration[] = {
	{TEXT("ePass2003"),                       {0x3b,0x9f,0x95,0x81,0x31,0xfe,0x9f,0x00,0x66,0x46,0x53,0x05,0x01,0x00,0x11,0x71,0xdf,0x00,0x00,0x03,0x6a,0x82,0xf8},
                                          23, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("FTCOS/PK-01C"),                    {0x3b,0x9f,0x95,0x81,0x31,0xfe,0x9f,0x00,0x65,0x46,0x53,0x05,0x00,0x06,0x71,0xdf,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          23, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00}},
	{TEXT("SmartCard-HSM"),                   {0x3b,0xfe,0x18,0x00,0x00,0x81,0x31,0xfe,0x45,0x80,0x31,0x81,0x54,0x48,0x53,0x4d,0x31,0x73,0x80,0x21,0x40,0x81,0x07,0xfa},
                                          24, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("SmartCard-HSM-CL"),                {0x3B,0x8E,0x80,0x01,0x80,0x31,0x81,0x54,0x48,0x53,0x4D,0x31,0x73,0x80,0x21,0x40,0x81,0x07,0x18},
                                          19, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Contactless Smart Card"),          {0x3B,0x80,0x80,0x01,0x01},
                                           5, {0xff,0xff,0xff,0xff,0xff}},
	{TEXT("GoID"),                            {0x3B,0x84,0x80,0x01,0x47,0x6f,0x49,0x44,0x00},
                                           9, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00}},
	{TEXT("GoID (01)"),                       {0x3B,0x85,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00},
                                          10, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00}},
	{TEXT("GoID (02)"),                       {0x3B,0x86,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00},
                                          11, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00}},
	{TEXT("GoID (03)"),                       {0x3B,0x87,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00},
                                          12, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (04)"),                       {0x3B,0x88,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00},
                                          13, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (05)"),                       {0x3B,0x89,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00},
                                          14, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (06)"),                       {0x3B,0x8a,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          15, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (07)"),                       {0x3B,0x8b,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          16, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (08)"),                       {0x3B,0x8c,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          17, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (09)"),                       {0x3B,0x8d,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          18, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (10)"),                       {0x3B,0x8e,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          19, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("GoID (11)"),                       {0x3B,0x8f,0x80,0x01,0x47,0x6f,0x49,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                                          20, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{TEXT("CEV WESTCOS"),                     {0x3f,0x69,0x00,0x00,0x00,0x64,0x01,0x00,0x00,0x00,0x80,0x90,0x00},
                                          13, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0xf0,0xff,0xff}},
	/* from card-openpgp.c */
	{TEXT("OpenPGP card v1.x"),               {0x3b,0xfa,0x13,0x00,0xff,0x81,0x31,0x80,0x45,0x00,0x31,0xc1,0x73,0xc0,0x01,0x00,0x00,0x90,0x00,0xb1},
                                          20, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("OpenPGP card v2.x"),               {0x3b,0xda,0x18,0xff,0x81,0xb1,0xfe,0x75,0x1f,0x03,0x00,0x31,0xc5,0x73,0xc0,0x01,0x40,0x00,0x90,0x00,0x0c},
                                          21, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Gnuk v1.0.x (OpenPGP v2.0)"),      {0x3b,0xda,0x11,0xff,0x81,0xb1,0xfe,0x55,0x1f,0x03,0x00,0x31,0x84,0x73,0x80,0x01,0x80,0x00,0x90,0x00,0xe4},
                                          21, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0x00}},
	{TEXT("OpenPGP card v3.x"),               {0x3b,0xda,0x18,0xff,0x81,0xb1,0xfe,0x75,0x1f,0x03,0x00,0x31,0xf5,0x73,0xc0,0x01,0x60,0x00,0x90,0x00,0x1c},
                                          21, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	/* from card-masktech.c */
	/* note: the card name MUST be unique */
	{TEXT("MaskTech smart card (a)"),         {0x3b,0x89,0x80,0x01,0x4d,0x54,0x43,0x4f,0x53,0x70,0x02,0x00,0x04,0x31},
                                          14, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0xff,0xfc,0xf4,0xf5}},
	{TEXT("MaskTech smart card (b)"),         {0x3B,0x9D,0x13,0x81,0x31,0x60,0x35,0x80,0x31,0xC0,0x69,0x4D,0x54,0x43,0x4F,0x53,0x73,0x02,0x00,0x00,0x40},
                                          21, {0xff,0xff,0xff,0xff,0xff,0xff,0xfd,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0xf0,0xf0}},
	{TEXT("MaskTech smart card (c)"),         {0x3B,0x88,0x80,0x01,0x00,0x00,0x00,0x00,0x77,0x81,0x80,0x00,0x6E},
                                          13, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xee,0xff,0xee}},

	/* from card-cardos.c */
	{TEXT("CardOS 4.0"),                      {0x3b,0xe2,0x00,0xff,0xc1,0x10,0x31,0xfe,0x55,0xc8,0x02,0x9c},
                                          12, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Italian CNS (a)"),                 {0x3b,0xe9,0x00,0xff,0xc1,0x10,0x31,0xfe,0x55,0x00,0x64,0x05,0x00,0xc8,0x02,0x31,0x80,0x00,0x47},
                                          19, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Italian CNS (b)"),                 {0x3b,0xfb,0x98,0x00,0xff,0xc1,0x10,0x31,0xfe,0x55,0x00,0x64,0x05,0x20,0x47,0x03,0x31,0x80,0x00,0x90,0x00,0xf3},
                                          22, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Italian CNS (c)"),                 {0x3b,0xfc,0x98,0x00,0xff,0xc1,0x10,0x31,0xfe,0x55,0xc8,0x03,0x49,0x6e,0x66,0x6f,0x63,0x61,0x6d,0x65,0x72,0x65,0x28},
                                          23, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Italian CNS (d)"),                 {0x3b,0xff,0x18,0x00,0xff,0x81,0x31,0xfe,0x55,0x00,0x6b,0x02,0x09,0x03,0x03,0x01,0x01,0x01,0x43,0x4e,0x53,0x10,0x31,0x80,0x9d},
                                          25, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("CardOS 4.0 a"),                    {0x3b,0xf4,0x98,0x00,0xff,0xc1,0x10,0x31,0xfe,0x55,0x4d,0x34,0x63,0x76,0xb4},
                                          15, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("Cardos M4"),                       {0x3b,0xf2,0x18,0x00,0x02,0xc1,0x0a,0x31,0xfe,0x58,0xc8,0x08,0x74},
                                          13, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("CardOS 4.4"),                      {0x3b,0xd2,0x18,0x02,0xc1,0x0a,0x31,0xfe,0x58,0xc8,0x0d,0x51},
                                          12, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("CardOS v5.0"),                     {0x3b,0xd2,0x18,0x00,0x81,0x31,0xfe,0x58,0xc9,0x01,0x14},
                                          11, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("JPKI"),                            {0x3b,0xe0,0x00,0xff,0x81,0x31,0xfe,0x45,0x14},
                                          9,  {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},

	/* from card-starcos.c */
	{TEXT("STARCOS (a)"),                     {0x3B,0xB7,0x94,0x00,0xc0,0x24,0x31,0xfe,0x65,0x53,0x50,0x4b,0x32,0x33,0x90,0x00,0xb4},
                                          17, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS (b)"),                     {0x3B,0xB7,0x94,0x00,0x81,0x31,0xfe,0x65,0x53,0x50,0x4b,0x32,0x33,0x90,0x00,0xd1},
                                          16, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS (c)"),                     {0x3b,0xb7,0x18,0x00,0xc0,0x3e,0x31,0xfe,0x65,0x53,0x50,0x4b,0x32,0x34,0x90,0x00,0x25},
                                          17, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.4"),                     {0x3b,0xd8,0x18,0xff,0x81,0xb1,0xfe,0x45,0x1f,0x03,0x80,0x64,0x04,0x1a,0xb4,0x03,0x81,0x05,0x61},
                                          19, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (a)"),                 {0x3B,0x9B,0x96,0xC0,0x0A,0x31,0xFE,0x45,0x80,0x67,0x04,0x1E,0xB5,0x01,0x00,0x89,0x4C,0x81,0x05,0x45},
                                          20, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (b)"),                 {0x3B,0xDB,0x96,0xFF,0x81,0x31,0xFE,0x45,0x80,0x67,0x05,0x34,0xB5,0x02,0x01,0xC0,0xA1,0x81,0x05,0x3C},
                                          20, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (c)"),                 {0x3B,0xD9,0x96,0xFF,0x81,0x31,0xFE,0x45,0x80,0x31,0xB8,0x73,0x86,0x01,0xC0,0x81,0x05,0x02},
                                          18, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (d)"),                 {0x3B,0xDF,0x96,0xFF,0x81,0x31,0xFE,0x45,0x80,0x5B,0x44,0x45,0x2E,0x42,0x4E,0x4F,0x54,0x4B,0x31,0x31,0x31,0x81,0x05,0xA0},
                                          24, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (e)"),                 {0x3B,0xDF,0x96,0xFF,0x81,0x31,0xFE,0x45,0x80,0x5B,0x44,0x45,0x2E,0x42,0x4E,0x4F,0x54,0x4B,0x31,0x30,0x30,0x81,0x05,0xA0},
                                          24, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{TEXT("STARCOS 3.5 (f)"),                 {0x3B,0xD9,0x96,0xFF,0x81,0x31,0xFE,0x45,0x80,0x31,0xB8,0x73,0x86,0x01,0xE0,0x81,0x05,0x22},
                                          18, {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},

    /* from card-rtecp.c */
    {TEXT("Rutoken ECP"),                     {0x3B,0x8B,0x01,0x52,0x75,0x74,0x6F,0x6B,0x65,0x6E,0x20,0x45,0x43,0x50,0xA0},
                                          15, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},
    {TEXT("Rutoken ECP (DS)"),                {0x3B,0x8B,0x01,0x52,0x75,0x74,0x6F,0x6B,0x65,0x6E,0x20,0x44,0x53,0x20,0xC1},
                                          15, {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},
    {TEXT("Rutoken ECP SC"),                  {0x3B,0x9C,0x96,0x00,0x52,0x75,0x74,0x6F,0x6B,0x65,0x6E,0x45,0x43,0x50,0x73,0x63},
                                          16, {0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},
    {TEXT("Rutoken ECP SC"),                  {0x3B,0x9C,0x94,0x80,0x11,0x40,0x52,0x75,0x74,0x6F,0x6B,0x65,0x6E,0x45,0x43,0x50,0x73,0x63,0xC3},
                                          19, {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00}},
};


/* remove a card in the database if and only if the CSP match the OpenSC CSP
The program try to avoid any failure to not break the uninstall process */
VOID RemoveKey(PTSTR szSubKey)
{
	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hKey);

	if (lResult != ERROR_SUCCESS)
	{
		WcaLog(LOGMSG_STANDARD, "RegOpenKeyEx %S 0x%08X", szSubKey, lResult);
		return;
	}
	TCHAR szName[MAX_PATH];
	DWORD dwSize = MAX_PATH;
	FILETIME ftWrite;
	lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
							NULL, NULL, &ftWrite);

	if (lResult == ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		do {
			HKEY hTempKey = NULL;
			dwIndex++;
			lResult = RegOpenKeyEx (hKey, szName, 0, KEY_READ, &hTempKey);
			if (lResult == ERROR_SUCCESS)
			{
				TCHAR szCSP[MAX_PATH] = {0};
				dwSize = MAX_PATH;
				lResult = RegQueryValueEx(hTempKey, TEXT("Crypto Provider"), NULL, NULL, (PBYTE) szCSP, &dwSize);
				RegCloseKey(hTempKey);
				if (lResult == ERROR_SUCCESS)
				{
					if ( _tcsstr(szCSP, TEXT("OpenSC CSP")) != 0)
					{
						lResult = RegDeleteKey(hKey, szName);
						if (lResult != ERROR_SUCCESS)
						{
							WcaLog(LOGMSG_STANDARD, "RegDeleteKey %S 0x%08X", szName, lResult);
						}
						else
						{
							dwIndex--;
						}
					}
				}
				else
				{
					WcaLog(LOGMSG_STANDARD, "RegQueryValueEx %S 0x%08X", szName, lResult);
				}
			}
			dwSize = MAX_PATH;
			lResult = RegEnumKeyEx(hKey,dwIndex, szName, &dwSize, NULL,
									NULL, NULL, &ftWrite);

		} while (lResult == ERROR_SUCCESS);
	}
	RegCloseKey(hKey);
}

UINT WINAPI RemoveSmartCardConfiguration(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	hr = WcaInitialize(hInstall, "RemoveSmartCardConfiguration");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	/* clean a smart card database. As today the x64 setup doesn't handle x86 installation on x64 machine */
	RemoveKey(SC_DATABASE);
	/* when this happens, just uncomment the following line:
#ifdef _M_X64
	RemoveKey(X86onX64_SC_DATABASE);
#endif
	*/

	/* never fails or only if the msi uninstall didn't work. If the uninstall custom action trigger a failure, the user is unable to uninstall the software */
LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

/* note: szKey is here in case the card has to be registered in the Calais and WOW3264node\Calais databases */
void RegisterCardWithKey(PTSTR szKey, PTSTR szCard, PTSTR szPath, PBYTE pbATR, DWORD dwATRSize, PBYTE pbAtrMask)
{
	HKEY hKey = NULL;
	HKEY hTempKey = NULL;
	LONG lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

	if (lResult != ERROR_SUCCESS)
	{
		WcaLog(LOGMSG_STANDARD, "unable to open the calais database.");
		return;
	}
	lResult = RegCreateKeyEx(hKey, szCard, 0,NULL,0,KEY_WRITE, NULL,&hTempKey,NULL);
	if(!lResult)
	{
		RegSetValueEx( hTempKey,TEXT("Crypto Provider"),0, REG_SZ, (PBYTE)BASE_CSP,sizeof(BASE_CSP) - sizeof(TCHAR));
		RegSetValueEx( hTempKey,TEXT("Smart Card Key Storage Provider"),0, REG_SZ, (PBYTE)BASE_KSP,sizeof(BASE_KSP) - sizeof(TCHAR));
		RegSetValueEx( hTempKey,TEXT("80000001"),0, REG_SZ, (PBYTE)szPath,(DWORD) (sizeof(TCHAR) * _tcslen(szPath)));
		RegSetValueEx( hTempKey,TEXT("ATR"),0, REG_BINARY, (PBYTE)pbATR, dwATRSize);
		RegSetValueEx( hTempKey,TEXT("ATRMask"),0, REG_BINARY, (PBYTE)pbAtrMask, dwATRSize);
		RegCloseKey(hTempKey);
	}
	else
	{
		WcaLog(LOGMSG_STANDARD, "unable to create the card entry");
	}
	RegCloseKey(hKey);
}

VOID RegisterSmartCard(PMD_REGISTRATION registration)
{
	DWORD expanded_len = PATH_MAX;
	TCHAR expanded_val[PATH_MAX];
	PTSTR szPath = TEXT("C:\\Program Files\\OpenSC Project\\OpenSC\\minidriver\\opensc-minidriver.dll");

	/* cope with x86 installation on x64 */
	expanded_len = ExpandEnvironmentStrings(
			TEXT("%ProgramFiles%\\OpenSC Project\\OpenSC\\minidriver\\opensc-minidriver.dll"),
			expanded_val, expanded_len);
	if (0 < expanded_len && expanded_len < sizeof expanded_val)
		szPath = expanded_val;

	RegisterCardWithKey(SC_DATABASE, registration->szName, szPath, registration->pbAtr, registration->dwAtrSize, registration->pbAtrMask );
}

UINT WINAPI AddSmartCardConfiguration(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	int i ;

	hr = WcaInitialize(hInstall, "AddSmartCardConfiguration");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	for (i = 0; i < sizeof(minidriver_registration) / sizeof(MD_REGISTRATION); i++)
	{
		RegisterSmartCard(minidriver_registration + i);
	}

	/* never fails or only if the msi install functions didn't work. If the install custom action trigger a failure, the user is unable to install the software */
LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

// DllMain - Initialize and cleanup WiX custom action utils.
BOOL APIENTRY DllMain(
	__in HINSTANCE hInst,
	__in ULONG ulReason,
	__in LPVOID
	)
{
	switch(ulReason)
	{
	case DLL_PROCESS_ATTACH:
		WcaGlobalInitialize(hInst);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;
	}

	return TRUE;
}
#else

UINT WINAPI AddSmartCardConfiguration(unsigned long hInstall)
{
	return 0;
}

UINT WINAPI RemoveSmartCardConfiguration(unsigned long hInstall)
{
	return 0;
}
#endif
