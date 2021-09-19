#pragma once
//#define DOTA_HELPER_LOG




#define MY_HEADER_FILE_
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
//#define PSAPI_VERSION 1



#pragma region Includes
#include <cstdio>
#include <cstring>
#include <csetjmp>
#include <ctime>
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream> 
#include <iostream>
#include <sstream>
#include <algorithm>
#include <MinHook.h>
#include <thread>
#include <chrono>
#include <map>

//#include "PolyHook.hpp"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

int GameDll = ( int )( GetModuleHandle( "Game.dll" ) );
using namespace std;
#pragma comment(lib, "libMinHook.x86.lib")
//
//#include "PolyHook.hpp"

struct xxxx
{
	int funcaddr;
	int callcount;
	int functime;
};


UINT TicksCount;
UINT TicksCountSecond;


struct FuncHeader
{
	int funcaddr;
	UINT calltime;
	UINT callcount;
	UINT currenttime;
	int starttimer;
	int stoptimer;
	int realfuncaddr;
};


std::vector<FuncHeader *> FuncList;
//std::vector<std::shared_ptr<PLH::Detour>> Detour_Ex_list;


bool compareByLength( const FuncHeader * a, const FuncHeader * b )
{
	if ( a->callcount > 0 && b->callcount > 0 )
	{
		return ( a->calltime / a->callcount ) > ( b->calltime / b->callcount );
	}

	return a->calltime > b->calltime;
}


vector<FuncHeader *> Result;


void __stdcall StartTimer( FuncHeader * memaddr )
{
	if ( memaddr->currenttime < TicksCount )
	{
		memaddr->currenttime = TicksCount;
	}
}


void __stdcall StopTimer( FuncHeader * memaddr )
{
	if ( memaddr->currenttime < TicksCount )
	{
		memaddr->calltime += TicksCount - memaddr->currenttime;
	}
}


DWORD WINAPI UpdateTimer( LPVOID )
{
	while ( true )
	{
		std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
		TicksCount++;
		if ( TicksCountSecond + 1000 < TicksCount )
		{
			TicksCountSecond = TicksCount;
			TicksCount--;
			for ( auto a : FuncList )
			{
				FuncHeader * tempstruct = a;
				if ( a->callcount > 0 )
				{
					Result.push_back( tempstruct );
				}
			}

			if ( Result.size( ) )
			{
				std::sort( Result.begin( ), Result.end( ), compareByLength );

				FILE * file;
				fopen_s( &file, "Dump.txt", "w" );
				if ( file )
				{
					for ( auto f : Result )
					{
						if ( f )
						{
							if ( f->callcount > 0 )
							{
								fprintf_s( file, "Func addr Game.dll+%X, call time: %.2f ms, call count: %u\r\n", f->realfuncaddr, ( f->calltime / 100.0f ), f->callcount );
							}
							f->callcount = 0;
						}
					}

					Result.clear( );
					fclose( file );
				}

			}
			TicksCount++;
		}

	}


	return 0;
}


/*
int funcaddr
int calltime
int callcount
int currenttime
int starttimer
int stoptimer
//
//
__pushad
mov ecx, calltime
call StartTimer
callcount++;
__popad
call funcaddr
mov ecx, calltime
call StopTimer
__popad
ret
*/

//
//std::vector<std::shared_ptr<PLH::Detour> > Detour_Ex_list;

void CreateNewFunc( int funcaddr_ )
{
	/*	std::shared_ptr<PLH::Detour> Detour_Ex( new PLH::Detour );
		Detour_Ex_list.push_back( Detour_Ex );
		Detour_Ex = Detour_Ex_list[ Detour_Ex_list.size( ) - 1 ];*/

	vector<unsigned char> NewFunc;
	LPVOID newmemory = VirtualAlloc( 0, 0x100, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	typedef void( *FuncVoid )( );
	FuncVoid FuncVoid_org;
	FuncVoid FuncVoid_ptr;
	FuncVoid FuncVoid_my;

	int funcaddr = ( int )newmemory + 0;

	int calltime = ( int )newmemory + 4;

	int callcount = ( int )newmemory + 8;

	int currenttime = ( int )newmemory + 12;

	int starttimer = ( int )newmemory + 16;

	int stoptimer = ( int )newmemory + 20;

	int realfuncaddr = ( int )newmemory + 24;

	int funcorgaddr = ( int )newmemory + 28;

	int mh_disable = ( int )newmemory + 32;

	int mh_enable = ( int )newmemory + 36;



	FuncVoid_my = ( FuncVoid )( ( int )newmemory + 40 );
	FuncVoid_org = ( FuncVoid )( funcaddr_ );

	*( int* )( funcorgaddr ) = funcaddr_;

	for ( int i = 0; i < 40; i++ )
		NewFunc.push_back( 0 );

	//pushad
	NewFunc.push_back( 0x60 );

	//inc callcout
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x05 );
	NewFunc.push_back( ( ( unsigned char * )&callcount )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&callcount )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&callcount )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&callcount )[ 3 ] );
	//push memaddr
	NewFunc.push_back( 0x68 );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 3 ] );
	//call StartTimer
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x15 );
	NewFunc.push_back( ( ( unsigned char * )&starttimer )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&starttimer )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&starttimer )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&starttimer )[ 3 ] );
	//push memaddr
	NewFunc.push_back( 0x68 );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 3 ] );
	//call StartTimer
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x15 );
	NewFunc.push_back( ( ( unsigned char * )&mh_disable )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_disable )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_disable )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_disable )[ 3 ] );
	//popad
	NewFunc.push_back( 0x61 );
	//call FuncPtr
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x15 );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 3 ] );
	//pushad
	NewFunc.push_back( 0x60 );
	//push memaddr
	NewFunc.push_back( 0x68 );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcaddr )[ 3 ] );
	//call StopTimer
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x15 );
	NewFunc.push_back( ( ( unsigned char * )&stoptimer )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&stoptimer )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&stoptimer )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&stoptimer )[ 3 ] );
	//push memaddr
	NewFunc.push_back( 0x68 );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&funcorgaddr )[ 3 ] );
	//call StartTimer
	NewFunc.push_back( 0xFF );
	NewFunc.push_back( 0x15 );
	NewFunc.push_back( ( ( unsigned char * )&mh_enable )[ 0 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_enable )[ 1 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_enable )[ 2 ] );
	NewFunc.push_back( ( ( unsigned char * )&mh_enable )[ 3 ] );
	//popad
	NewFunc.push_back( 0x61 );
	//ret
	NewFunc.push_back( 0xC3 );
	CopyMemory( newmemory, &NewFunc[ 0 ], NewFunc.size( ) );

	*( int* )realfuncaddr = funcaddr_ - GameDll;

	//Detour_Ex->SetupHook( FuncVoid_org, FuncVoid_my );
	//Detour_Ex->Hook( );
	//FuncVoid_ptr = Detour_Ex->GetOriginal<FuncVoid>( );

	MH_CreateHook( FuncVoid_org, FuncVoid_my, reinterpret_cast< void** >( &FuncVoid_ptr ) );
	//MH_EnableHook( FuncVoid_org );

	*( int* )funcaddr = ( int )FuncVoid_ptr;
	*( int* )starttimer = ( int )StartTimer;
	*( int* )stoptimer = ( int )StopTimer;

	*( int* )mh_disable = ( int )MH_DisableHook;
	*( int* )mh_enable = ( int )MH_EnableHook;

	//*( int* )( ( int )newmemory + 0 ) = funcaddr;
	//*( int* )( ( int )newmemory + 16 ) = starttimer;
	//*( int* )( ( int )newmemory + 20 ) = stoptimer;
	//*( int* )( ( int )newmemory + 16 ) = starttimer;
	//*( int* )( ( int )newmemory + 20 ) = stoptimer;

	FlushInstructionCache( GetCurrentProcess( ), newmemory, 0x100 );
	//FuncList.push_back( ( FuncHeader * )newmemory );
	/*
		if ( Detour_Ex->GetLastError( ).GetSeverity( ) == PLH::RuntimeError::Severity::Critical )
		{
			char test[ 200 ];

			sprintf_s( test, "Critical! %X->%X->%s", newmemory, funcaddr_, Detour_Ex->GetLastError( ).GetString( ).c_str( ) );
			MessageBox( 0, test, test, 0 );
		}

		if ( Detour_Ex->GetLastError( ).GetSeverity( ) == PLH::RuntimeError::Severity::UnRecoverable )
		{
			char test[ 200 ];

			sprintf_s( test, "UnRecoverable! %X->%X->%s", newmemory, funcaddr_, Detour_Ex->GetLastError( ).GetString( ).c_str( ) );
			MessageBox( 0, test, test, 0 );
		}
		*/
		//char test[ 200 ];

		//sprintf_s( test, "Warning! %X->%X", newmemory, funcaddr_ );
		//MessageBox( 0, test, test, 0 );
}


DWORD WINAPI InitAll( LPVOID )
{
	FILE  *f;
	fopen_s( &f, "functions.dat", "rb" );
	fseek( f, 0, SEEK_END );
	int addrcount = ftell( f ) / 4;
	fseek( f, 0, SEEK_SET );
	int funcaddr = 0;

	//int maxx = 20000;

	while ( fread( &funcaddr, 4, 1, f ) )
	{
		if ( funcaddr != 0x5FD050 )
			//maxx--;
			CreateNewFunc( GameDll + funcaddr );
	}
	MH_EnableHook( MH_ALL_HOOKS );
	MessageBox( 0, "End inject", " Injected ", 0 );
	return 0;
}

BOOL __stdcall DllMain( HINSTANCE Module, unsigned int reason, LPVOID )
{

	if ( reason == DLL_PROCESS_ATTACH )
	{
		MH_Initialize( );

		CreateThread( 0, 0, InitAll, 0, 0, 0 );
		CreateThread( 0, 0, UpdateTimer, 0, 0, 0 );

		//CreateNewFunc( GameDll + 0x5FD050 );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		//MH_Uninitialize( );
	}

	return TRUE;
}