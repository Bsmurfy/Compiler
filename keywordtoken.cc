#ifndef KeywordToken_H
#define KeywordToken_H

#include "keywordtoken.h"

using namespace std;

KeywordToken::KeywordToken()
{
	this->set_token_type(TOKEN_KEYWORD);
}

KeywordToken::~KeywordToken()
{

}

KeywordToken::KeywordToken(keyword_attr_type   attr) : KeywordToken()
{
	this->set_attribute(attr);
}

void KeywordToken::set_attribute(keyword_attr_type   attr)
{
	this->attribute = attr;
}

keyword_attr_type  KeywordToken::get_attribute()
{
	return this->attribute;
}

string* KeywordToken::to_string() const
{
   string szKWWord = "";
   if (this->attribute == KW_BEGIN)
   {
      szKWWord = "KW_BEGIN";
   }
   else if (this->attribute == KW_BOOL)
   {
      szKWWord = "KW_BOOL";
   }
   else if (this->attribute == KW_ELSE)
   {
      szKWWord = "KW_ELSE";
   }
   else if (this->attribute == KW_END)
   {
      szKWWord = "KW_END";
   }
   else if (this->attribute == KW_IF)
   {
      szKWWord = "KW_IF";
   }
   else if (this->attribute == KW_INT)
   {
      szKWWord = "KW_INT";
   }
   else if (this->attribute == KW_LOOP)
   {
      szKWWord = "KW_LOOP";
   }
   else if (this->attribute == KW_NOT)
   {
      szKWWord = "KW_NOT";
   }
   else if (this->attribute == KW_NO_ATTR)
   {
      szKWWord = "KW_NO_ATTR";
   }
   else if (this->attribute == KW_PRINT)
   {
      szKWWord = "KW_PRINT";
   }
   else if (this->attribute == KW_PROCEDURE)
   {
      szKWWord = "KW_PROCEDURE";
   }
   else if (this->attribute == KW_PROGRAM)
   {
      szKWWord = "KW_PROGRAM";
   }
   else if (this->attribute == KW_THEN)
   {
      szKWWord = "KW_THEN";
   }
   else if (this->attribute == KW_WHILE)
   {
      szKWWord = "KW_WHILE";
   }
	return new string(szKWWord);
}

#endif
