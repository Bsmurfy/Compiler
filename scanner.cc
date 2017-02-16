#include "scanner.h"


Scanner::Scanner (char *filename)
{
  // Create and fill the buffer.
  this->buf = new Buffer (filename);
}

Scanner::~Scanner ()
{
  delete buf;
}

Token *Scanner::next_token()
{
  Token *lexeme = NULL;

  // lexemes will be separated by a space, tab, or endline
  string szRetVal = "";

  bool isNum = false;
  bool isCharS = false;

  char next_char;
  next_char = this->next_char();

  // If we are on a blank/new line, skip it
  while (is_space(next_char))
  {
	  next_char = this->next_char();
  }

  if (next_char == ':')
  {
	  szRetVal = ":";
	  if ((next_char = this->next_char()) == '=')
	  {
		  szRetVal += "=";

		  lexeme = new PuncToken(PUNC_ASSIGN);
	  }
	  else
	  {
		  this->buf->unread_char(next_char);

		  lexeme = new PuncToken(PUNC_COLON);
	  }

  }
  else if (next_char == ';')
  {
	  szRetVal = ";";

	  lexeme = new PuncToken(PUNC_SEMI);
  }
  else if (next_char == ',')
  {
	  szRetVal = ",";

	  lexeme = new PuncToken(PUNC_COMMA);
  }
  else if (next_char == '(')
  {
	  szRetVal = "(";

	  lexeme = new PuncToken(PUNC_OPEN);
  }
  else if (next_char == ')')
  {
	  szRetVal = ")";

	  lexeme = new PuncToken(PUNC_CLOSE);
  }
  else if (next_char == '=')
  {
	  szRetVal = "=";

	  lexeme = new RelopToken(RELOP_EQ);
  }
  else if (next_char == '>')
  {
	  szRetVal = ">";
	  if ((next_char = this->next_char()) == '=')
	  {
		  szRetVal += "=";

		  lexeme = new RelopToken(RELOP_GE);
	  }
	  else
	  {
		  this->buf->unread_char(next_char);

		  lexeme = new RelopToken(RELOP_GT);
	  }
  }
  else if (next_char == '<')
  {
	  szRetVal = "<";
	  if ((next_char = this->next_char()) == '>')
	  {
		  szRetVal += ">";

		  lexeme = new RelopToken(RELOP_NE);
	  }
	  else if (next_char == '=')
	  {
		  szRetVal += "=";

		  lexeme = new RelopToken(RELOP_LE);
	  }
	  else
	  {
		  this->buf->unread_char(next_char);

		  lexeme = new RelopToken(RELOP_LT);
	  }
  }
  else if (next_char == '+')
  {
	  szRetVal = "+";

	  lexeme = new AddopToken(ADDOP_ADD);
  }
  else if (next_char == '-')
  {
	  szRetVal = "-";

	  lexeme = new AddopToken(ADDOP_SUB);
  }
  else if (next_char == '*')
  {
	  szRetVal = "*";

	  lexeme = new MulopToken(MULOP_MUL);
  }
  else if (next_char == '/')
  {
	  szRetVal = "/";

	  lexeme = new MulopToken(MULOP_DIV);
  }
  else if (next_char == '$')
  {
	  szRetVal = "$";

	  lexeme = new EofToken();
  }
  else if (is_alpha(next_char))
  {
	  szRetVal += next_char;
	  next_char = this->next_char();
	  while (is_alphanum(next_char))
	  {
		  szRetVal += next_char;
		  next_char = this->next_char();
	  }
	  if (next_char != ' ')
	  {
		  this->buf->unread_char(next_char);
	  }

	  isCharS = true;
  }
  else if (is_digit(next_char))
  {
	  szRetVal += next_char;
	  next_char = this->next_char();
	  while (is_digit(next_char))
	  {
		  szRetVal += next_char;
		  next_char = this->next_char();
	  }
	  if (next_char != ' ')
	  {
		  this->buf->unread_char(next_char);
	  }
	  
	  isNum = true;
  }
  else
  {
	  lexeme = new Token();
  }

  if (lexeme == NULL)
  {
	  if (szRetVal == "or")
	  {
		  lexeme = new AddopToken(ADDOP_OR);
	  }
	  else if (szRetVal == "and")
	  {
		  lexeme = new MulopToken(MULOP_AND);
	  }
	  else if (szRetVal == "program")
	  {
		  lexeme = new KeywordToken(KW_PROGRAM);
	  }
	  else if (szRetVal == "procedure")
	  {
		  lexeme = new KeywordToken(KW_PROCEDURE);
	  }
	  else if (szRetVal == "int")
	  {
		  lexeme = new KeywordToken(KW_INT);
	  }
	  else if (szRetVal == "bool")
	  {
		  lexeme = new KeywordToken(KW_BOOL);
	  }
	  else if (szRetVal == "begin")
	  {
		  lexeme = new KeywordToken(KW_BEGIN);
	  }
	  else if (szRetVal == "end")
	  {
		  lexeme = new KeywordToken(KW_END);
	  }
	  else if (szRetVal == "if")
	  {
		  lexeme = new KeywordToken(KW_IF);
	  }
	  else if (szRetVal == "then")
	  {
		  lexeme = new KeywordToken(KW_THEN);
	  }
	  else if (szRetVal == "else")
	  {
		  lexeme = new KeywordToken(KW_ELSE);
	  }
	  else if (szRetVal == "while")
	  {
		  lexeme = new KeywordToken(KW_WHILE);
	  }
	  else if (szRetVal == "loop")
	  {
		  lexeme = new KeywordToken(KW_LOOP);
	  }
	  else if (szRetVal == "print")
	  {
		  lexeme = new KeywordToken(KW_PRINT);
	  }
	  else if (szRetVal == "not")
	  {
		  lexeme = new KeywordToken(KW_NOT);
	  }
	  else if (isNum)
	  {
		  lexeme = new NumToken(szRetVal);
	  }
	  else if (isCharS)
	  {
		  lexeme = new IdToken(szRetVal);
	  }
  }

  return lexeme;
}

char Scanner::next_char() const
{
	char next_char = buf->current_char();

	// Treat comments as a single space.
	if (next_char == '#')
	{

		while ( next_char != '\n' )
		{
			buf->pop_char();
         next_char = buf->current_char();
		}

		next_char = ' ';
	}
	// Skip Passed empty space.
	else if ( ( next_char == ' '    ) ||
		       ( next_char == '\t'   ) ||
		       ( next_char == '\n'   ) )
	{

		while ( ( next_char == ' '    ) ||
			     ( next_char == '\t'   ) ||
			     ( next_char == '\n'   ) )
		{
			buf->pop_char();
         next_char = buf->current_char();
		}

		next_char = ' ';
	}
	// Regular character
	else
	{
		next_char = buf->pop_char();;
	}

	return next_char;
	
}

void Scanner::scanner_fatal_error(const string& message)
{
  cerr << "Exiting on Scanner Fatal Error: " << message << endl;
  exit (-1);
}




