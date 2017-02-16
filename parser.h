#ifndef PARSER_H
#define PARSER_H

// The parser needs to access token attributes.
#include "token.h"
#include "keywordtoken.h"
#include "punctoken.h"
#include "reloptoken.h"
#include "addoptoken.h"
#include "muloptoken.h"
#include "idtoken.h"
#include "numtoken.h"
#include "eoftoken.h"
#include "scanner.h"
#include "symbol_table.h"
#include "register.h"
#include "register_allocator.h"
#include "emitter.h"
#include "operand.h"

// To print error messages.
#include <iostream>
#include <vector>

class Parser {
   public:
      Parser (Scanner *the_scanner);
      ~Parser();
  
      bool parse_program();

      // Return true if current token is EOF
      bool done_with_input();
  
   private:

      bool parse_decl_list();
      
      bool parse_variable_decl_list();

      bool parse_variable_decl();

      bool parse_procedure_decl_list();

      bool parse_identifier_list();

      bool parse_identifier_list_prm();

      bool parse_standard_type(expr_type &standard_type_type);

      bool parse_block();

      bool parse_procedure_decl();

      bool parse_procedure_args();

      bool parse_formal_parm_list();

      bool parse_formal_parm_list_hat();

      bool parse_stmt_list();

      bool parse_stmt_list_prm();

      bool parse_stmt();

      bool parse_ad_hoc_as_pc_tail(expr_type &ad_hoc_as_pc_tail_type, Operand *&op);

      bool parse_if_stmt();

      bool parse_if_stmt_hat();

      bool parse_while_stmt();

      bool parse_print_stmt();

      bool parse_expr_list();

      bool parse_actual_parm_list();

      bool parse_actual_parm_list_hat();

      bool parse_expr(expr_type &expr_type_result, Operand *&op);

      bool parse_expr_hat(expr_type &expr_hat_type, Operand *&op);

      bool parse_simple_expr(expr_type &simple_expr_type, Operand *&op);

      bool parse_simple_expr_prm(expr_type &simple_expr_prm_type, Operand *&op);

      bool parse_term(expr_type &term_type, Operand *&op);

      bool parse_term_prm(expr_type &term_prm_type, Operand *&op);

      bool parse_factor(expr_type &factor_type, Operand *&op);

      bool parse_sign(expr_type &sign_type);
  
      // The lexical analyzer
      Scanner *lex;
      // The current token the parser is looking at
      Token *word;
  
      /* Print out a parse error message:
	 
	 "Parse error: Expected *expected*, found *found*."

	 This method should delete the string after it has printed the
	 error message. */
      void parse_error (string *expected, Token *found);

      // Semantic Analysis Stuff
      string *current_env, *main_env, *procedure_name;
      int actual_parm_position, formal_parm_position;
      bool parsing_formal_parm_list;

      Symbol_Table stab;

      Register_Allocator *allocator;
      Emitter *e;

      vector<string> labels;
      Operand **last_reg_op;

      /* These functions are for signalling semantic errors.  None of
	 them return - they exit and terminate the compilation.
      */

	    // Identifier has been defined twice.
      void multiply_defined_identifier (string *id) const;
								
      // Identifier is undeclared.
      void undeclared_identifier (string *id) const;

      // Type error:  a single type was expected.
      void type_error (const expr_type expected, 
		       const expr_type found) const;

      // Type error: one of two types was expected.
      void type_error (const expr_type expected1,
			const expr_type expected2,
			const expr_type found) const;
  
};

#endif

