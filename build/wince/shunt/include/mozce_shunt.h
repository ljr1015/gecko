/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is MOZCE Lib.
 *
 * The Initial Developer of the Original Code is Doug Turner <dougt@meer.net>.

 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  John Wolfe <wolfe@lobo.us>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef MOZCE_SHUNT_H
#define MOZCE_SHUNT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifdef __cplusplus
}   //extern "C" 
#endif

#ifdef MOZ_MEMORY

#ifdef __cplusplus
#define _NEW_
void * operator new(size_t _Size);
void operator delete(void * ptr);
void *operator new[](size_t size);
void operator delete[](void *ptr);

extern "C" {
#endif

#undef _strdup
#undef strdup
#undef _strndup
#undef strndup
#undef _wcsdup
#undef wcsdup
#undef _wcsndup
#undef wcsndup

char * __cdecl
_strdup(const char*);

wchar_t * __cdecl
_wcsdup(const wchar_t *);

char * __cdecl
_strndup(const char *, unsigned int);

wchar_t * __cdecl
_wcsndup(const wchar_t *, unsigned int);
  
#ifdef __cplusplus
}   //extern "C" 
#endif

#endif

#define strdup  _strdup
#define strndup _strndup
#define wcsdup _wcsdup
#define wcsndup _wcsndup


#define strcmpi _stricmp
#define stricmp _stricmp
#define wgetcwd _wgetcwd
#define vsnprintf _vsnprintf

#define SHGetSpecialFolderPathW SHGetSpecialFolderPath
#define SHGetPathFromIDListW    SHGetPathFromIDList
#define FONTENUMPROCW           FONTENUMPROC

#ifdef __cplusplus
extern "C" {
#endif

/* errno and family */
extern int errno;
char* strerror(int);

/* abort */
void abort(void);
  
/* Environment stuff */
char* getenv(const char* inName);
int putenv(const char *a);
char SetEnvironmentVariableW(const unsigned short * name, const unsigned short * value );
char GetEnvironmentVariableW(const unsigned short * lpName, unsigned short* lpBuffer, unsigned long nSize);
  
unsigned int ExpandEnvironmentStringsW(const unsigned short* lpSrc,
				       unsigned short* lpDst,
				       unsigned int nSize);

/* File system stuff */
unsigned short * _wgetcwd(unsigned short* dir, unsigned long size);
unsigned short *_wfullpath( unsigned short *absPath, const unsigned short *relPath, unsigned long maxLength );
int _unlink(const char *filename );
  
/* The time stuff should be defined here, but it can't be because it
   is already defined in time.h.
  
 size_t strftime(char *, size_t, const char *, const struct tm *)
 struct tm* localtime(const time_t* inTimeT)
 struct tm* mozce_gmtime_r(const time_t* inTimeT, struct tm* outRetval)
 struct tm* gmtime(const time_t* inTimeT)
 time_t mktime(struct tm* inTM)
 time_t time(time_t *)
 clock_t clock() 
  
*/
  
/* Locale Stuff */
  
/* The locale stuff should be defined here, but it can't be because it
   is already defined in locale.h.
  
 struct lconv * localeconv(void)
  
*/


unsigned short* mozce_GetEnvironmentCL();

  /* square root of 1/2, missing from math.h */ 
#define M_SQRT1_2  0.707106781186547524401

#ifdef __cplusplus
};
#endif

#endif //MOZCE_SHUNT_H
