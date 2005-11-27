/**
 *
 * @file StringUtils.h
 * @brief header file for the StringUtils namespace
 * @author
 * @todo notcommented
 *
 */



#ifndef STRINGUTILS
#define STRINGUTILS

#ifndef SYMBIAN
	#include <string>
	#include <vector>
	#include <deque>
	#include "CommonIncludes.h"
	using namespace ::std;
#endif //ifndef SYMBIAN

#ifdef SYMBIAN
struct StringUtils
#else
namespace StringUtils
#endif //SYMBIAN
{
    /**
     * @brief breakes the input string into pieces sparated by sTokens; call it repeatedly to get all the pieces
     */
#ifdef SYMBIAN
	static
#endif
    string Tokenize( string &sInput, string sTokens, string::size_type &CurPos );

    /**
     * @brief converts input string to lowercase
     */
#ifdef SYMBIAN
	static
#endif
    string ToLower( string sInput );

    /**
     * @brief converts input string to uppercase
     */
#ifdef SYMBIAN
	static
#endif
    string ToUpper( string sInput );

    /**
     * @brief returns the string representing the integer
     */
#ifndef SYMBIAN
	string i64tos( u_int64_t iNum );
#endif

#ifdef SYMBIAN
	static
#endif
    string itos( int iNum );
    string ftos( double dNum );

	/**
     * @brief replaces the sSearch string with the sReplace string in the sInput string
     */
#ifdef SYMBIAN
	static
#endif
	// Replaces sSearch with sReplace.  The first one replaces the input string passed in, the 2nd does not
    string Replace( string sInput, const string &sSearch, const string &sReplace );
	string Replace( string *sInput, const string &sSearch, const string &sReplace );

#ifndef SYMBIAN

    /**
     * @brief replaces the sSearch string with the sReplace string in the specified file and returns false if errors
     */
    bool Replace( string sInputFile, string sOutputFile, string sSearch, string sReplace);

    /**
     * @brief breaks the Input string in tokens and putes them in the deque_strings parameter, or a vector
     */
    void Tokenize(string &Input, string Tokens, deque<string> &deque_strings, bool bPushToFront);
	void Tokenize(string &Input, string Tokens, vector<string> &vect_strings);

    /**
     * @brief retrns a sring containing the specified char repeated count times
     */
    string RepeatChar(char c,int count);

    /**
     * @brief returns the string representing the long
     */
    string ltos( long lNum );

    /**
     * @brief eliminates spaces from the begining and the ending of the input string
     */
    string TrimSpaces( string &sInput );

    /**
     * @brief compares strings ignoring the case
     */
    int CompareNoCase( string sFirst, string sSecond );

	/**
     * @brief returns true if sFirst starts with sSecond
     */
    bool StartsWith( string sFirst, string sSecond, bool bIgnoreCase=false );

	/**
     * @brief returns true if sFirst ends with sSecond
     */
    bool EndsWith( string sFirst, string sSecond, bool bIgnoreCase=false );

    /**
     * @brief returns the value folowing the parameter name in the parameter list tokenized using the delimiters
     */
    string ParseValueFromString( string sParameterList, string sParameterName, string sDefaultValue, const char *pcDelimiters = NULL );

    /**
     * @brief same, but it returns an integer (the parameter is assumed to be a number)
     */
    int ParseIntFromString( string sParameterList, string sParameterName, int iDefaultValue );


    /**
     * @brief converts a printf format string with the parameter list into a string, like this: "%d bla", 12 becomes "12 bla"
     */
    string Format( const char *pcFormat, ... );

    /**
     * @brief @todo ask
     */
    string EscapeChars( string sInput );


    /**
     * @brief Do a word wrap  on sInput, where the max line length is iNumChars, and the result is vectStrings lines.
     */
    void BreakIntoLines( string sInput, vector<string> *vectStrings, int iNumChars );

    /**
     * @brief converts hex number stored in the first two chars of the parameter to a long and returns it as a char
     */
    char HexByte2Num( char* pcPtr );


    /**
     * @brief assumes the input to have the form: ipaddress:port and does the parsing
     */
    void AddressAndPortFromString( string sInput, int iDefaultPort, string &sAddress, int &iPort );


    /**
     * @brief adds some URL compliancy
     * @todo extend
     */
    string URLEncode( string in );

    /**
     * @brief removes some of the URL special (like %20)
     * @todo extend
     */
    string URLDecode(string in);


    /**
     * @brief the checksum is calulated by adding the values in the char representation of the data
     */
    unsigned int CalcChecksum( unsigned int iSize, char *pData );

	/**
	 * @brief make a mac address from an integer. Usefull to simulate mac addresses from devices
	 */
	string makeUpPlayerAddressFromPlayerId(unsigned int playerId);

    time_t SQLDateTime( string sSQLDate ); /** < converts the sql formatted date time into a time_t structure */
    string SQLDateTime( time_t t=0 ); /** < converts a time_t structure into a sql formatted date time */
    string SQLEscape( string sInput ); /** < convert a escape sequence so that it can be understood by sql */

	string GetStringFromConsole(); /** < Let's the user enter a string terminated by a carriage return */

	/** < Convert space delimited string to arguments, like on a command line.  Use " to include spaces
		  There's a hardcoded limit of 500 arguments */
	char **ConvertStringToArgs(string sInput,int &iNumArgs,int *p_iPosNext=NULL);
	void FreeArgs(char **pArgs,int iNumArgs);

	time_t StringToDate(string Value);  /** < Converts a date/time as a string to a time_t */

	string GetDow( int iDow, bool bFull=false );  /** < Returns the name of the day of the week based on the integer.  If bFull, it is not abbreviated */

	// Convert Input to Upper and strip anything but A-Z and 0-9
	string UpperAZ09Only(string sInput);

	string SecondsAsTime(int iSeconds);
	string PrecisionTime();

	bool ContainsNonAscii(string &sInput);

#endif //#ifndef SYMBIAN
};

#endif //#ifndef _STRINGUTILS
