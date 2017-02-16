#ifndef PUNCTOKEN_H
#define PUNCTOKEN_H

#include "muloptoken.h"

using namespace std;

MulopToken::MulopToken()
{
	this->set_token_type(TOKEN_MULOP);
}

MulopToken::~MulopToken()
{

}

MulopToken::MulopToken(mulop_attr_type attr) : MulopToken()
{
	this->set_attribute(attr);
}

void MulopToken::set_attribute(mulop_attr_type attr)
{
	this->attribute = attr;
}

mulop_attr_type MulopToken::get_attribute()
{
	return this->attribute;
}

string* MulopToken::to_string() const
{
   string szKWMulop = "";
   if (this->attribute == MULOP_AND)
   {
      szKWMulop = "MULOP_AND";
   }
   else if (this->attribute == MULOP_DIV)
   {
      szKWMulop = "MULOP_DIV";
   }
   else if (this->attribute == MULOP_MUL)
   {
      szKWMulop = "MULOP_MUL";
   }
	return new string(szKWMulop);
}

#endif
