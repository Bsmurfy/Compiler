#ifndef EofToken_H
#define EofToken_H

#include "eoftoken.h"

using namespace std;

EofToken::EofToken()
{
	this->set_token_type(TOKEN_EOF);
}

EofToken::~EofToken()
{

}

string* EofToken::to_string() const
{
	return new string("Eof");
}

#endif
