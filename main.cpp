//////////////
//	main.cpp
//

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>

void oem_vprint(LPCTSTR lpszFormat,va_list args){
	LPTSTR lpszBuffer;
	LPSTR lpOEMBuffer;
	int len,size;
	//
	len			= _vsctprintf(lpszFormat,args);
	size		= (len + 1) * sizeof(_TCHAR);
	lpszBuffer	= (LPTSTR)malloc(size);
	lpOEMBuffer	= (LPSTR)malloc(len + 1);
	_vstprintf_s(lpszBuffer,len + 1,lpszFormat,args);
#if defined(_UNICODE)
	sprintf_s(lpOEMBuffer,len + 1,"%ws",lpszBuffer);
	::CharToOemA(lpOEMBuffer,lpOEMBuffer);
#else
	::CharToOemBuff(lpszBuffer,lpOEMBuffer,len);
#endif
	printf_s("%.*s",len,lpOEMBuffer);
	free(lpOEMBuffer);
	free(lpszBuffer);
}
void oem_print(LPCTSTR lpszFormat,...){
	va_list args;
	va_start(args,lpszFormat);
	oem_vprint(lpszFormat,args);
	va_end(args);
}
void lc_print(LPCTSTR lpszLocale,LPCTSTR lpszFormat,...){
	va_list args;
	//
	va_start(args,lpszFormat);
#if defined(_UNICODE)
	LPCTSTR lpszSvLc	= _tsetlocale(LC_ALL,NULL);
	_tsetlocale(LC_ALL,lpszLocale);
	_vtprintf(lpszFormat,args);
	_tsetlocale(LC_ALL,lpszSvLc);
#else
	oem_vprint(lpszFormat,args);
#endif
	va_end(args);
}
int usage(void){
	lc_print(
		_T(".866"),
		_T("vc - счетчик версий\r\n")
		_T("\r\nинструкция:\r\n")
		_T("vc <опции> <параметры> <вход> <выход>\r\n\r\n")
		_T("\r\nопции:\r\n")
		_T("  -a - <шаблон> и <выход> в кодировке ANSI\r\n")
		_T("  -U - <шаблон> и <выход> в кодировке UNICODE\r\n")
		_T("  -u - <шаблон> и <выход> в кодировке UTF-8\r\n")
		_T("  -e - <шаблон> и <выход> в кодировке UTF-16LE\r\n")
		_T("  -r - не изменять значение счетчика\r\n")
		_T("\r\nвход:\r\n")
		_T("  имя файла-шаблона\r\n")
		_T("  это может быть любой текстовый файл\r\n")
		_T("  содержимое этого файла транслируется в файл <выход>,\r\n")
		_T("  согласно следующей спецификации:\r\n")
		_T("    %%      - символ escape-последовательности\r\n")
		_T("              которая всегда заменяется в выходном файле\r\n")
		_T("              конкретным значением\r\n")
		_T("    %%qw    - 8-байтовое целое в десятичной форме\r\n")
		_T("    %%dw.x  - 4-байтовое целое в десятичной форме, x={0,1}\r\n")
		_T("    %%w.x   - 2-байтовое целое в десятичной форме, x={0,1,2,3}\r\n")
		_T("    %%b.x   - 1-байтовое целое в десятичной форме, x={0,1,2,3,4,5,6,7})\r\n")
		_T("    x       - номер части счетчика\r\n")
		_T("    %%++qw\r\n")
		_T("    %%++dw.x\r\n")
		_T("    %%++w.x\r\n")
		_T("    %%++b.x - префиксное увеличение счетчика\r\n")
		_T("    %%qw++\r\n")
		_T("    %%dw.x++\r\n")
		_T("    %%w.x++\r\n")
		_T("    %%b.x++ - постфиксное увеличение счетчика\r\n")
		_T("    %%<<    - открывает режим echo \r\n")
		_T("    %%>>    - закрывает режим echo \r\n")
		_T("    %%<*    - открывает скрытый режим \r\n")
		_T("    %%*>    - закрывает скрытый режим \r\n")
		_T("    %%%%    - экранирование символа %% \r\n")
		_T("\r\nрежим echo:\r\n")
		_T("  в этом режиме, весь вывод начинает дублироваться в stdout,\r\n")
		_T("  что позволяет наблюдать за режимом трансляции,\r\n")
		_T("  выводить и логировать необходимые участки выходного файла,\r\n")
		_T("  отлаживать файл шаблона\r\n")
		_T("\r\nскрытый режим:\r\n")
		_T("  в этом режиме, вывод в файл блокируется,\r\n")
		_T("  но вся арифметическая часть продолжает функционировать,\r\n")
		_T("  кроме того, в пределах комментария, можно так же включать режим echo\r\n")
		_T("  \r\n")
		_T("\r\nпараметры:\r\n")
		_T("  имя файла-параметров, в котором хранится текущая версия\r\n")
		_T("  это обычный INI-файл следующего вида:\r\n\r\n")
		_T("    [general]\r\n")
		_T("      low=12345\r\n")
		_T("      high=67890\r\n")
		_T("  low - младшее двойное слово версии\r\n")
		_T("  high - старшее двойное слово версии\r\n")
	);
	return S_FALSE;
}
int out_of_range(LPCTSTR lpszFile,int line){
	lc_print(_T(".866"),_T("\r\n%s(%i): выход за границы диапазона\r\n"),lpszFile,line);
	return S_FALSE;
}
extern "C"{
	int __cdecl _winput_s_l(FILE*,const wchar_t*,localeinfo_struct*,va_list);
	int __cdecl _input_s_l(FILE*,const unsigned char*,localeinfo_struct*,char*);
}
bool translate(_TCHAR**lpszInput,size_t ilen,int*marker,LPCTSTR lpszFormat,...){
	FILE file;
	va_list args;
	//
	file._flag	= _IOREAD|_IOSTRG|_IOMYBUF;
	file._ptr	= file._base = (char*)(*lpszInput);
	file._cnt	= (int)ilen * sizeof(**lpszInput);
	//
	*marker		= -1;
	//
	va_start(args,lpszFormat);
#if defined(_UNICODE)
	if( (_winput_s_l(&file,lpszFormat,NULL,args) >= 0) && (*marker >= 0) ){
#else
	if( (_input_s_l(&file,(const unsigned char*)lpszFormat,NULL,args) >= 0) && (*marker >= 0) ){
#endif
		*lpszInput	= ( (_TCHAR*)file._ptr ) - 1;
		va_end(args);
		return true;
	}
	va_end(args);
	return false;
}
int _tmain(int argc, _TCHAR* argv[]){
	_TCHAR szINI[MAX_PATH];
	_TCHAR mode[64];
	LARGE_INTEGER version;
	LPTSTR lpszInput,lpszOutput,lpszINI,lpszBuffer,lpCh,lpCCs;
	FILE*output,*input;
	size_t i,len,ilen,std_out,hidden;
	errno_t err;
	long bomlen;
	int line,marker;
	unsigned int x;
	_TCHAR ccs;
	//
	if( argc < 4 ){
		return usage();
	}
	//
	lpszINI		= argv[argc - 3];
	lpszInput	= argv[argc - 2];
	lpszOutput	= argv[argc - 1];
	//
	if( ::GetFileAttributes(lpszInput) == INVALID_FILE_ATTRIBUTES ){
		return usage();
	}
	//
	for(ccs	= _T('a'),i = argc - 4;i--;){
		lpCh	= argv[i + 1];
		if( !_tcschr(_T("-/"),*lpCh) )continue;
		for(len = 0; *lpCh ;++len,++lpCh);
		if( !len )continue;
		for(--lpCh; len-- && !_tcschr(_T("aUue"),*lpCh) ; --lpCh);
		if( !++len )continue;
		ccs	= *lpCh;
		break;
	}
	switch( ccs ){
		case _T('a'):{
			lpCCs	= _T("");
			break;
		}
		case _T('U'):{
			lpCCs	= _T(", ccs=UNICODE");
			break;
		}
		case _T('u'):{
			lpCCs	= _T(", ccs=UTF-8");
			break;
		}
		case _T('e'):{
			lpCCs	= _T(", ccs=UTF-16LE");
			break;
		}
	}
	_stprintf_s(mode,_T("rt%s"),lpCCs);
	if( err = _tfopen_s(&input,lpszInput,mode) ){
		lc_print(_T(".866"),_T("\r\nошибка при открытии файла-шаблона\r\n"));
		return usage();
	}
	_stprintf_s(mode,_T("wt%s"),lpCCs);
	if( err = _tfopen_s(&output,lpszOutput,mode) ){
		lc_print(_T(".866"),_T("\r\nошибка при создании выходного файла\r\n"));
		fclose(input);
		return usage();
	}
	//
	bomlen	= ftell(input);
	for(len = 0; !feof(input) ;_fgettc(input),++len);
	lpszBuffer		= (LPTSTR)malloc( (len + 1) * sizeof(_TCHAR) );
	lpszBuffer[len]	= _T('\0');
	fseek(input,bomlen,SEEK_SET);
	for(lpCh = lpszBuffer; !feof(input) ;*(lpCh++) = _fgettc(input));
	fclose(input);
	//
	::GetFullPathName(lpszINI,sizeof(szINI) / sizeof(_TCHAR),szINI,NULL);
	version.LowPart		= ::GetPrivateProfileInt(_T("general"),_T("low"),0x00000000,szINI);
	version.HighPart	= ::GetPrivateProfileInt(_T("general"),_T("high"),0x00000000,szINI);
	//
	for(line = 1,hidden = 0,std_out = 0,lpCh = lpszBuffer; *lpCh ;++lpCh){
		switch( *lpCh ){
			case _T('\n'):{
				++line;
				break;
			}
			case _T('%'):{
				if( lpCh[1] == _T('%') ){
					++lpCh;
				}else{
					ilen	= len - (lpCh - lpszBuffer);
					if( translate(&lpCh,ilen,&marker,_T("%%<*%n"),&marker) ){
						++hidden;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%*>%n"),&marker) ){
						--hidden;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%<<%n"),&marker) ){
						++std_out;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%>>%n"),&marker) ){
						--std_out;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%qw++%n"),&marker) ){
						if( !hidden ){
							_ftprintf(output,_T("%I64u"),version.QuadPart);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I64u"),version.QuadPart);
						}
						++version.QuadPart;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%++qw%n"),&marker) ){
						++version.QuadPart;
						if( !hidden ){
							_ftprintf(output,_T("%I64u"),version.QuadPart);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I64u"),version.QuadPart);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%qw%n"),&marker) ){
						if( !hidden ){
							_ftprintf(output,_T("%I64u"),version.QuadPart);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I64u"),version.QuadPart);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%dw.%u++%n"),&x,&marker) ){
						if(x > 1){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						DWORD&value	= ((LPDWORD)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%I32u"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I32u"),value);
						}
						++value;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%++dw.%u%n"),&x,&marker) ){
						if(x > 1){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						DWORD&value	= ((LPDWORD)&version.LowPart)[x];
						++value;
						if( !hidden ){
							_ftprintf(output,_T("%I32u"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I32u"),value);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%dw.%u%n"),&x,&marker) ){
						if(x > 1){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						DWORD&value	= ((LPDWORD)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%I32u"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%I32u"),value);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%w.%u++%n"),&x,&marker) ){
						if(x > 3){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						WORD&value	= ((LPWORD)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%hu"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),value);
						}
						++value;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%++w.%u%n"),&x,&marker) ){
						if(x > 3){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						WORD&value	= ((LPWORD)&version.LowPart)[x];
						++value;
						if( !hidden ){
							_ftprintf(output,_T("%hu"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),value);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%w.%u%n"),&x,&marker) ){
						if(x > 3){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						WORD&value	= ((LPWORD)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%hu"),value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),value);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%b.%u++%n"),&x,&marker) ){
						if(x > 7){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						BYTE&value	= ((LPBYTE)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%hu"),(WORD)value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),(WORD)value);
						}
						++value;
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%++b.%u%n"),&x,&marker) ){
						if(x > 7){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						BYTE&value	= ((LPBYTE)&version.LowPart)[x];
						++value;
						if( !hidden ){
							_ftprintf(output,_T("%hu"),(WORD)value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),(WORD)value);
						}
						continue;
					}
					if( translate(&lpCh,ilen,&marker,_T("%%b.%u%n"),&x,&marker) ){
						if(x > 7){
							fclose(output);
							return out_of_range(lpszInput,line);
						}
						BYTE&value	= ((LPBYTE)&version.LowPart)[x];
						if( !hidden ){
							_ftprintf(output,_T("%hu"),(WORD)value);
						}
						if( std_out ){
							_ftprintf(stdout,_T("%hu"),(WORD)value);
						}
						continue;
					}
				}
			}
		}
		if( !hidden ){
			_fputtc(*lpCh,output);
		}
		if(std_out){
			if( _tcschr(_T("Uue"),ccs) ){
				lc_print(_T(".OCP"),_T("%c"),*lpCh);
			}else{
				oem_print(_T("%c"),*lpCh);
			}
		}
	}
	free(lpszBuffer);
	fclose(output);
	//
	for(i = argc - 4;i--;){
		lpCh	= argv[i + 1];
		if( _tcschr(_T("-/"),*lpCh) ){
			if( _tcschr(lpCh + 1,_T('r')) ){
				break;
			}
		}
	}
	if( !(i + 1) ){
		_stprintf_s(mode,_T("%#.8x"),version.LowPart);
		::WritePrivateProfileString(_T("general"),_T("low"),mode,szINI);
		_stprintf_s(mode,_T("%#.8x"),version.HighPart);
		::WritePrivateProfileString(_T("general"),_T("high"),mode,szINI);
	}
	//
	return S_OK;
}