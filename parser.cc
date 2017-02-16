#include "parser.h"

using namespace std;

Parser::Parser(Scanner *the_scanner)
{
   // Initialize the parser.
   lex = the_scanner;
   word = lex->next_token();

   current_env = main_env = procedure_name = NULL;
   actual_parm_position = formal_parm_position = -1;
   parsing_formal_parm_list = false;

   e = new Emitter();
   allocator = new Register_Allocator();
}

Parser::~Parser()
{
   // Delete the parser. 
   if (word != NULL)
   {
      delete word;
   }
   if (e != NULL)
   {
      delete e;
   }
   if (allocator != NULL)
   {
      delete allocator;
   }
}

// If we have parsed the entire program, then word
// should be the EOF Token.  This function tests
// that condition.
bool Parser::done_with_input()
{
   return word->get_token_type() == TOKEN_EOF;
}

bool Parser::parse_program()
{
   // PROGRAM -> program identifier ; DECL_LIST BLOCK ;
   // Predict (program identifier ; DECL_LIST BLOCK ;) == {program}

   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROGRAM)
   {
      delete word;
      word = lex->next_token();
      if (word->get_token_type() == TOKEN_ID)
      {
         string *id_name = static_cast<IdToken *>(word)->get_attribute();
         string *global_env_name = new string ("_EXTERNAL");
         stab.install(id_name, global_env_name, PROGRAM_T);
         current_env = new string (*id_name);
         main_env = new string (*id_name);
         e->emit_label(id_name);
         delete global_env_name;
         delete id_name;
         delete word;
         word = lex->next_token();
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
         {
            delete word;
            word = lex->next_token();
            if (parse_decl_list())
            {
               if (parse_block())
               {
                  if (word->get_token_type() == TOKEN_PUNC
                     && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
                  {
                     delete word;
                     word = lex->next_token();
                     e->emit_halt();
                     for (unsigned int i = 0; i < labels.size(); ++i) {
                       const string &label = labels.at(i);
                       e->emit_data_directive(&label, 1);
                     }
                     return true;
                  }
                  else
                  {
                     string *expected = new string("';'");
                     parse_error(expected, word);
                     return false;
                  }
               }
               else
               {
                  return false;
               }
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("';'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         string *expected = new string("identifier");
         parse_error(expected, word);
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword program");
      parse_error(expected, word);
      return false;
   }
}


bool Parser::parse_decl_list()
{
   /* DECL_LIST -> VARIABLE_DECL_LIST PROCEDURE_DECL_LIST

      Predict(VARIABLE_DECL_LIST PROCEDURE_DECL_LIST)
        = First(VARIABLE_DECL_LIST)
       union First (PROCEDURE_DECL_LIST)
       union Follow (DECL_LIST) = {identifier, procedure, begin}

       Note that we don't actually need to check the predict set
       here.  The predict set is used to choose the correct
       production, but there isn't a choice here.

       In addition, we take advantage of C++'s short circuit
       evaluation of Boolean expressions. */

   if (parse_variable_decl_list() && parse_procedure_decl_list())
   {
     return true;
   } else {
     return false;
   }
}


bool Parser::parse_variable_decl_list()
{
   if (word->get_token_type() == TOKEN_ID)
   {

      if (parse_variable_decl())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
         {
            delete word;
            word = lex->next_token();
            if (parse_variable_decl_list())
            {
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("';'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else 
   {
    return true;
   }
}

bool Parser::parse_variable_decl()
{
   if (word->get_token_type() == TOKEN_ID){
      if (parse_identifier_list()){
        if (word->get_token_type() == TOKEN_PUNC
             && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COLON){
          delete word;
          word = lex->next_token();
          expr_type standard_type_type;
          if (parse_standard_type(standard_type_type)){
             stab.update_type(standard_type_type);
             return true;
          } else {
             return false;
          }
        } else {
          string *expected = new string("':'");
          parse_error(expected, word);
          return false;
        }
      } else {
        return false;
      }
   } else {
      string *expected = new string("identifier");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_procedure_decl_list()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE)
   {
      if (parse_procedure_decl())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
         {
            delete word;
            word = lex->next_token();
            if (parse_procedure_decl_list())
            {
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("';'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
  } else {
    return true;
  }
}

bool Parser::parse_identifier_list()
{
   if (word->get_token_type() == TOKEN_ID)
   {
      string *id_attribute   = static_cast<IdToken *>(word)->get_attribute();
      if (stab.is_decl(id_attribute, current_env)){
        multiply_defined_identifier(id_attribute);
        return false;
      } else {
        stab.install(id_attribute, current_env, UNKNOWN_T);
      }
      if (*current_env == *main_env) {
        labels.push_back(*id_attribute);
      }
      delete word;
      word = lex->next_token();
      if (parse_identifier_list_prm())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("identifier");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_identifier_list_prm()
{
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA)
   {
      delete word;
      word = lex->next_token();
      if (word->get_token_type() == TOKEN_ID)
      {
        string *id_attribute = static_cast<IdToken *>(word)->get_attribute();
        if (stab.is_decl(id_attribute, current_env)){
          multiply_defined_identifier(id_attribute);
          return false;
        } else if (parsing_formal_parm_list){
          stab.install(id_attribute, current_env, UNKNOWN_T, formal_parm_position);
          formal_parm_position++;
        } else {
          stab.install(id_attribute, current_env, UNKNOWN_T);
        }
        if (*current_env == *main_env) {
          labels.push_back(*id_attribute);
        }
        delete word;
        word = lex->next_token();
        if (parse_identifier_list_prm())
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
         string *expected = new string("identifier");
         parse_error(expected, word);
         return false;
      }
   }
   else
   {
     return true;
   }
}

bool Parser::parse_standard_type(expr_type &standard_type_type)
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_INT)
   {
      standard_type_type = INT_T;
      delete word;
      word = lex->next_token();
      return true;
   }
   else if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_BOOL)
   {
      standard_type_type = BOOL_T;
      delete word;
      word = lex->next_token();
      return true;
   }
   else
   {
      string *expected = new string("(keyword int, keyword bool)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_block()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_BEGIN)
   {
      delete word;
      word = lex->next_token();
      if (parse_stmt_list())
      {
         if (word->get_token_type() == TOKEN_KEYWORD
            && static_cast<KeywordToken *>(word)->get_attribute() == KW_END)
         {
            delete word;
            word = lex->next_token();
            return true;
         }
         else
         {
            string *expected = new string("keyword end");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword begin");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_procedure_decl()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PROCEDURE)
   {
      delete word;
      word = lex->next_token();
      if (word->get_token_type() == TOKEN_ID)
      {
         string *id_attribute = static_cast<IdToken *>(word)->get_attribute();
         if (stab.is_decl(id_attribute, current_env)){
           multiply_defined_identifier(id_attribute);
           return false;
         } else {
           stab.install(id_attribute, current_env, PROCEDURE_T);
           current_env = id_attribute;
           formal_parm_position = 0;
         }
         delete word;
         word = lex->next_token();
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
         {
            delete word;
            word = lex->next_token();
            if (parse_procedure_args())
            {
               if (word->get_token_type() == TOKEN_PUNC
                  && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE)
               {
                  delete word;
                  word = lex->next_token();
                  if (parse_variable_decl_list())
                  {
                     if (parse_block())
                     {
                        current_env = main_env;
                        return true;
                     }
                     else
                     {
                        return false;
                     }
                  }
                  else
                  {
                     return false;
                  }
               }
               else
               {
                  string *expected = new string("')'");
                  parse_error(expected, word);
                  return false;
               }
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("'('");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         string *expected = new string("identifier");
         parse_error(expected, word);
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword procedure");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_procedure_args()
{
   if (word->get_token_type() == TOKEN_ID)
   {
      parsing_formal_parm_list = true;
      if (parse_formal_parm_list())
      {
         parsing_formal_parm_list = false;
         return true;
      }
      else
      {
         return false;
      }
   }
   else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE)
   {
      delete word;
      word = lex->next_token();
      return true;
   }
   else
   {
      string *expected = new string("(identifier, ')')");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_formal_parm_list()
{
   if (word->get_token_type() == TOKEN_ID)
   {
      string *id_attribute = static_cast<IdToken *>(word)->get_attribute();
      if (stab.is_decl(id_attribute, current_env)){
        multiply_defined_identifier(id_attribute);
        return false;
      } else {
        stab.install(id_attribute, current_env, UNKNOWN_T, formal_parm_position);
        formal_parm_position++;
      }
      delete word;
      word = lex->next_token();
      if (parse_identifier_list_prm())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COLON)
         {
            delete word;
            word = lex->next_token();
            expr_type standard_type_type;
            if (parse_standard_type(standard_type_type))
            {
               stab.update_type(standard_type_type);
               if (parse_formal_parm_list_hat()){
                  return true;
               } else {
                  return false;
               }
            } else {
               return false;
            }
         } else {
            string *expected = new string("':'");
            parse_error(expected, word);
            return false;
         }
      } else {
         return false;
      }
   } else {
      string *expected = new string("identifier");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_formal_parm_list_hat()
{
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
   {
      delete word;
      word = lex->next_token();
      if (parse_formal_parm_list())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
     return true;
   }
}

bool Parser::parse_stmt_list()
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT))
   {
      if (parse_stmt())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
         {
            delete word;
            word = lex->next_token();
            if (parse_stmt_list_prm())
            {
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("';'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
   {
      delete word;
      word = lex->next_token();
      if (parse_stmt_list_prm())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(identifier, keyword if, keyword while, keyword print, ';')");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_stmt_list_prm()
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT))
   {
      if (parse_stmt())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_SEMI)
         {
            delete word;
            word = lex->next_token();
            if (parse_stmt_list_prm())
            {
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("';'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
        return false;
      }
   }
   else 
   {
     return true;
   }
}

bool Parser::parse_stmt()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
   {
      if (parse_if_stmt())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
   {
      if (parse_while_stmt())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT)
   {
      if (parse_print_stmt())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else if (word->get_token_type() == TOKEN_ID)
   {
      string *id_attribute = static_cast<IdToken *>(word)->get_attribute();
      if (!stab.is_decl(id_attribute, current_env)){
        undeclared_identifier(id_attribute);
        return false;
      } else {
        procedure_name = new string (*(static_cast<IdToken *>(word)->get_attribute()));
      }
      expr_type id_type = stab.get_type(id_attribute, current_env);
      delete word;
      word = lex->next_token();
      expr_type ad_hoc_as_pc_tail_type;
      Operand *op = new Operand(OPTYPE_MEMORY, id_attribute);
      if (parse_ad_hoc_as_pc_tail(ad_hoc_as_pc_tail_type, op))
      {
         if (ad_hoc_as_pc_tail_type != id_type){
           type_error(id_type, ad_hoc_as_pc_tail_type);
           return false;
         }
         if (id_type != PROCEDURE_T) {
           Register *op_reg = NULL;
           if (op->get_type() == OPTYPE_REGISTER) {
             op_reg = op->get_r_value();
           } else {
              if (!allocator->has_free_reg()) {
                string *spill_location = e->get_new_label("spill");
                e->emit_move(spill_location, (*last_reg_op)->get_r_value());
                allocator->deallocate_register((*last_reg_op)->get_r_value());
                labels.push_back(*spill_location);
                *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
             }

             op_reg = allocator->allocate_register();
             if (op->get_type() == OPTYPE_IMMEDIATE) {
               e->emit_move(op_reg, op->get_i_value());
             } else if (op->get_type() == OPTYPE_MEMORY) {
               e->emit_move(op_reg, op->get_m_value());
             }
           }
           e->emit_move(id_attribute, op_reg);
           allocator->deallocate_register(op_reg);
           delete op;
        }
        return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(keyword if, keyword while, keyword print, identifier)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_ad_hoc_as_pc_tail(expr_type &ad_hoc_as_pc_tail_type, Operand *&op)
{
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_ASSIGN)
   {
      delete word;
      word = lex->next_token();
      expr_type expr_type_result;
      if (parse_expr(expr_type_result, op))
      {
        ad_hoc_as_pc_tail_type = expr_type_result;
        return true;
      }
      else
      {
        return false;
      }
   }
   else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
   {  
      if (stab.get_type(procedure_name, main_env) != PROCEDURE_T){
        type_error(PROCEDURE_T, stab.get_type(procedure_name, main_env));
        actual_parm_position = 0;
      }
      delete word;
      word = lex->next_token();
      if (parse_expr_list())
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE)
         {
            ad_hoc_as_pc_tail_type = PROCEDURE_T;
            delete word;
            word = lex->next_token();
            return true;
         }
         else
         {
            string *expected = new string("')'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(':=', '(')");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_if_stmt()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_IF)
   {
      delete word;
      word = lex->next_token();
      expr_type expr_type_result = GARBAGE_T;
      Operand *expression = NULL;
      if (parse_expr(expr_type_result, expression))
      {
         if (expr_type_result != BOOL_T){
           type_error(BOOL_T, expr_type_result);
           return false;
         }
         Register *exp_reg = NULL;
         if (expression->get_type() == OPTYPE_REGISTER) {
            exp_reg = expression->get_r_value();
         } else {
            if (!allocator->has_free_reg()) {
              string *spill_location = e->get_new_label("spill");
              e->emit_move(spill_location, (*last_reg_op)->get_r_value());
              allocator->deallocate_register((*last_reg_op)->get_r_value());
              labels.push_back(*spill_location);
              *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
            }

            exp_reg = allocator->allocate_register();
            if (expression->get_type() == OPTYPE_IMMEDIATE) {
              e->emit_move(exp_reg, expression->get_i_value());
            } else if (expression->get_type() == OPTYPE_MEMORY) {
              e->emit_move(exp_reg, expression->get_m_value());
            }
         }

        string *else_part = e->get_new_label("else");
        string *if_done = e->get_new_label("if_done");
        e->emit_branch(INST_BREZ, exp_reg, else_part);

        allocator->deallocate_register(exp_reg);
        delete expression;

         if (word->get_token_type() == TOKEN_KEYWORD
            && static_cast<KeywordToken *>(word)->get_attribute() == KW_THEN)
         {
            delete word;
            word = lex->next_token();
            if (parse_block())
            {
               e->emit_branch(if_done);
               e->emit_label(else_part);
  
               if (parse_if_stmt_hat())
               {
                  e->emit_label(if_done);
                  return true;
               }
               else
               {
                  return false;
               }
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("keyword then");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword if");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_if_stmt_hat()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_ELSE)
   {
      delete word;
      word = lex->next_token();
      if (parse_block())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
    return true;
   }
}

bool Parser::parse_while_stmt()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_WHILE)
   {
      delete word;
      word = lex->next_token();
      expr_type expr_type_result = GARBAGE_T;
      Operand *expression = NULL;
      string *loop_begin = e->get_new_label("loop_begin");
      string *while_done = e->get_new_label("while_done");
      e->emit_label(loop_begin);
      if (parse_expr(expr_type_result, expression))
      {
         if (expr_type_result != BOOL_T){
           type_error(BOOL_T, expr_type_result);
           return false;
         }
         Register *exp_reg = NULL;
         if (expression->get_type() == OPTYPE_REGISTER) {
           exp_reg = expression->get_r_value();
         } else {
           exp_reg = allocator->allocate_register();
           if (!allocator->has_free_reg()) {
             string *spill_location = e->get_new_label("spill");
             e->emit_move(spill_location, (*last_reg_op)->get_r_value());
             allocator->deallocate_register((*last_reg_op)->get_r_value());
             labels.push_back(*spill_location);
             *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
           }
           if (expression->get_type() == OPTYPE_IMMEDIATE) {
             e->emit_move(exp_reg, expression->get_i_value());
           } else if (expression->get_type() == OPTYPE_MEMORY) {
             e->emit_move(exp_reg, expression->get_m_value());
           }
         }
         e->emit_branch(INST_BREZ, exp_reg, while_done);
         allocator->deallocate_register(exp_reg);
         delete expression;

         if (word->get_token_type() == TOKEN_KEYWORD
            && static_cast<KeywordToken *>(word)->get_attribute() == KW_LOOP)
         {
            delete word;
            word = lex->next_token();
            if (parse_block())
            {
               e->emit_branch(loop_begin);
               e->emit_label(while_done);
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            string *expected = new string("keyword loop");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword while");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_print_stmt()
{
   if (word->get_token_type() == TOKEN_KEYWORD
      && static_cast<KeywordToken *>(word)->get_attribute() == KW_PRINT)
   {
      delete word;
      word = lex->next_token();
      expr_type expr_type_result = GARBAGE_T;
      Operand *expression = NULL;
      Register *exp_reg = NULL;
      if (parse_expr(expr_type_result, expression))
      {
         if (expr_type_result != BOOL_T
             && expr_type_result != INT_T){
           type_error(BOOL_T, INT_T, expr_type_result);
           return false;
         }
         if (expression->get_type() == OPTYPE_REGISTER) {
           exp_reg = expression->get_r_value();
         } else {
           if (!allocator->has_free_reg()) {
             string *spill_location = e->get_new_label("spill");
             e->emit_move(spill_location, (*last_reg_op)->get_r_value());
             allocator->deallocate_register((*last_reg_op)->get_r_value());
             labels.push_back(*spill_location);
             *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
           }

           exp_reg = allocator->allocate_register();
           if (expression->get_type() == OPTYPE_IMMEDIATE) {
             e->emit_move(exp_reg, expression->get_i_value());
           } else if (expression->get_type() == OPTYPE_MEMORY) {
             e->emit_move(exp_reg, expression->get_m_value());
           }
         }
         e->emit_1addr(INST_OUTB, exp_reg);
         allocator->deallocate_register(exp_reg);
         delete expression;
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("keyword print");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_expr_list()
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_NUM)
      || (word->get_token_type() == TOKEN_PUNC
         && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT))
   {
      if (parse_actual_parm_list())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
    return true;
   }
}

bool Parser::parse_actual_parm_list()
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_NUM)
      || (word->get_token_type() == TOKEN_PUNC
         && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT))
   {
      expr_type expr_type_result;
      Operand *exp = NULL;
      if (parse_expr(expr_type_result, exp))
      {
         actual_parm_position++;
         if (parse_actual_parm_list_hat())
         {
            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(identifer, num, '(', '+', '-', keyword not)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_actual_parm_list_hat()
{
   if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_COMMA)
   {
      delete word;
      word = lex->next_token();
      if (parse_actual_parm_list())
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else 
   {
    return true;
   }
}

bool Parser::parse_expr(expr_type &expr_type_result, Operand *&op)
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_NUM)
      || (word->get_token_type() == TOKEN_PUNC
         && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT))
   {
      expr_type simple_expr_type;
      if (parse_simple_expr(simple_expr_type, op))
      {
         expr_type expr_hat_type;
         if (parse_expr_hat(expr_hat_type, op))
         {
            if (expr_hat_type == NO_T){
              expr_type_result = simple_expr_type;
              return true;
            } else if (simple_expr_type == INT_T && expr_hat_type == INT_T){
              expr_type_result = BOOL_T;
              return true;
            } else {
              type_error(INT_T, simple_expr_type, expr_hat_type);
              return false;
            }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(identifer, num, '(', '+', '-', keyword not)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_expr_hat(expr_type &expr_hat_type, Operand *&op)
{
   if (word->get_token_type() == TOKEN_RELOP)
   {
      relop_attr_type relop_attribute = static_cast<RelopToken *>(word)->get_attribute();
      delete word;
      word = lex->next_token();
      expr_type simple_expr_type = GARBAGE_T;
      Operand *op2 = NULL;
      if (parse_simple_expr(simple_expr_type, op2))
      {
        if (simple_expr_type == INT_T){
          expr_hat_type = INT_T; 
        } else {
          type_error(INT_T, simple_expr_type);
          return false;
        }
        Register *op_reg;
        if (op->get_type() == OPTYPE_REGISTER) {
          op_reg = op->get_r_value();
        } else {
          if (!allocator->has_free_reg()) {
            string *spill_location = e->get_new_label("spill");
            e->emit_move(spill_location, (*last_reg_op)->get_r_value());
            allocator->deallocate_register((*last_reg_op)->get_r_value());
            labels.push_back(*spill_location);
            *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
          }
          op_reg = allocator->allocate_register();
          if (op->get_type() == OPTYPE_IMMEDIATE) {
            e->emit_move(op_reg, op->get_i_value());
          } else if (op->get_type() == OPTYPE_MEMORY) {
            e->emit_move(op_reg, op->get_m_value());
          }
          delete op;
          op = new Operand(OPTYPE_REGISTER, op_reg);
        }
        last_reg_op = &op;
        if (op2->get_type() == OPTYPE_REGISTER) {
          e->emit_2addr(INST_SUB, op->get_r_value(), op2->get_r_value());
        } else if (op2->get_type() == OPTYPE_IMMEDIATE) {
          e->emit_2addr(INST_SUB, op->get_r_value(), op2->get_i_value());
        } else if (op2->get_type() == OPTYPE_MEMORY) {
          e->emit_2addr(INST_SUB, op->get_r_value(), op2->get_m_value());
        }
        string *if_false = e->get_new_label("false");
        string *next = e->get_new_label("next");

        if (relop_attribute == RELOP_EQ) {
          e->emit_branch(INST_BRNE, op->get_r_value(), if_false);
          e->emit_branch(INST_BRPO, op->get_r_value(), if_false);
        } else if (relop_attribute == RELOP_NE) {
          e->emit_branch(INST_BREZ, op->get_r_value(), if_false);
        } else if (relop_attribute == RELOP_GT) {
          e->emit_branch(INST_BREZ, op->get_r_value(), if_false);
          e->emit_branch(INST_BRNE, op->get_r_value(), if_false);
        } else if (relop_attribute == RELOP_GE) {
          e->emit_branch(INST_BRNE, op->get_r_value(), if_false);
        } else if (relop_attribute == RELOP_LT) {
          e->emit_branch(INST_BREZ, op->get_r_value(), if_false);
          e->emit_branch(INST_BRPO, op->get_r_value(), if_false);
        } else if (relop_attribute == RELOP_LE) {
          e->emit_branch(INST_BRPO, op->get_r_value(), if_false);
        }
        e->emit_move(op->get_r_value(), 1);
        e->emit_branch(next);
        e->emit_label(if_false);
        e->emit_move(op->get_r_value(), 0);
        e->emit_label(next);
        if (op2->get_type() == OPTYPE_REGISTER) {
          allocator->deallocate_register(op2->get_r_value());
        }
        delete op2;
        return true;
      } else {
        return false;
      }
   }
   else
   {
      expr_hat_type = NO_T;
      return true;
   }
}

bool Parser::parse_simple_expr(expr_type &simple_expr_type, Operand *&op)
{
   if ((word->get_token_type() == TOKEN_ID)
      || (word->get_token_type() == TOKEN_NUM)
      || (word->get_token_type() == TOKEN_PUNC
         && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT))
   {
      expr_type term_type;
      if (parse_term(term_type, op))
      {
         expr_type simple_expr_prm_type;
         if (parse_simple_expr_prm(simple_expr_prm_type, op))
         {
            if (simple_expr_prm_type == NO_T){
              simple_expr_type = term_type;
              return true;
            } else if (term_type == simple_expr_prm_type){
              simple_expr_type = term_type;
              return true;
            } else {
              type_error(term_type, simple_expr_prm_type);
              return false;
            }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(identifer, num, '(', '+', '-', keyword not)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_simple_expr_prm(expr_type &simple_expr_prm_type, Operand *&op)
{
   if (word->get_token_type() == TOKEN_ADDOP)
   {
      expr_type addop_type = GARBAGE_T;
      addop_attr_type addop_attribute = static_cast<AddopToken *>(word)->get_attribute();
      if (addop_attribute == ADDOP_ADD || addop_attribute == ADDOP_SUB) {
        addop_type = INT_T;
      } else {
        addop_type = BOOL_T;
      }
      delete word;
      word = lex->next_token();
      expr_type term_type = GARBAGE_T;
      Operand *op2 = NULL;
      if (parse_term(term_type, op2))
      {
         expr_type simple_expr_prm_type_1 = GARBAGE_T;
         Register *op_reg;
         if (op->get_type() == OPTYPE_REGISTER) {
           op_reg = op->get_r_value();
         } else {
           if (!allocator->has_free_reg()) {
             string *spill_location = e->get_new_label("spill");
             e->emit_move(spill_location, (*last_reg_op)->get_r_value());
             allocator->deallocate_register((*last_reg_op)->get_r_value());
             labels.push_back(*spill_location);
             *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
           }
           op_reg = allocator->allocate_register();
           if (op->get_type() == OPTYPE_IMMEDIATE) {
             e->emit_move(op_reg, op->get_i_value());
           } else if (op->get_type() == OPTYPE_MEMORY) {
             e->emit_move(op_reg, op->get_m_value());
           }
           delete op;
           op = new Operand(OPTYPE_REGISTER, op_reg);
         }
         last_reg_op = &op;
         inst_type instruction;
         if (addop_attribute == ADDOP_ADD || addop_attribute == ADDOP_OR) {
           instruction = INST_ADD;
         } else {
           instruction = INST_SUB;
         }
         if (op2->get_type() == OPTYPE_REGISTER) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_r_value());
         } else if (op2->get_type() == OPTYPE_IMMEDIATE) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_i_value());
         } else if (op2->get_type() == OPTYPE_MEMORY) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_m_value());
         }
         if (addop_attribute == ADDOP_OR) {
           string *if_false = e->get_new_label("false");
           e->emit_branch(INST_BREZ, op->get_r_value(), if_false);
           e->emit_move(op->get_r_value(), 1);
           e->emit_label(if_false);
         }
         if (op2->get_type() == OPTYPE_REGISTER) {
           allocator->deallocate_register(op2->get_r_value());
         }
         delete op2;

         if (parse_simple_expr_prm(simple_expr_prm_type_1, op))
         {
           if (simple_expr_prm_type_1 == NO_T){
              if (term_type == addop_type){
                simple_expr_prm_type = addop_type;
                return true;
              } else {
                type_error(addop_type, term_type);
                return false;
              }
           } else if (addop_type == term_type
                      && term_type == simple_expr_prm_type_1){
             simple_expr_prm_type = addop_type;
             return true;
           } else {
             type_error(addop_type, term_type, simple_expr_prm_type_1);
             return false;
           }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else 
   {
     simple_expr_prm_type = NO_T;
     return true;
   }
}

bool Parser::parse_term(expr_type &term_type, Operand *&op)
{
  expr_type factor_type;
  if (parse_factor(factor_type, op))
  {
     expr_type term_prm_type;
     if (parse_term_prm(term_prm_type, op))
     {
        if (term_prm_type == NO_T){
          term_type = factor_type;
          return true;
        } else if (factor_type == term_prm_type){
          term_type = factor_type;
          return true;
        } else {
          type_error(factor_type, term_prm_type);
          return false;
        }
     }
     else
     {
        return false;
     }
  }
  else
  {
     return false;
  }
}

bool Parser::parse_term_prm(expr_type &term_prm_type, Operand *&op)
{
   if (word->get_token_type() == TOKEN_MULOP)
   {
      expr_type mulop_type = GARBAGE_T;
      mulop_attr_type mulop_attribute = static_cast<MulopToken *>(word)->get_attribute();
      if (mulop_attribute == MULOP_MUL || mulop_attribute == MULOP_DIV) {
        mulop_type = INT_T;
      } else {
        mulop_type = BOOL_T;
      }
      delete word;
      word = lex->next_token();
      expr_type factor_type = GARBAGE_T;
      Operand *op2 = NULL;
      if (parse_factor(factor_type, op2))
      {
         expr_type term_prm_type_1 = GARBAGE_T;
         Register *op_reg;
         if (op->get_type() == OPTYPE_REGISTER) {
           op_reg = op->get_r_value();
         } else {
           if (!allocator->has_free_reg()) {
             string *spill_location = e->get_new_label("spill");
             e->emit_move(spill_location, (*last_reg_op)->get_r_value());
             allocator->deallocate_register((*last_reg_op)->get_r_value());
             labels.push_back(*spill_location);
             *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
           }
           op_reg = allocator->allocate_register();
           if (op->get_type() == OPTYPE_IMMEDIATE) {
             e->emit_move(op_reg, op->get_i_value());
           } else if (op->get_type() == OPTYPE_MEMORY){
             e->emit_move(op_reg, op->get_m_value());
           }
           delete op;
           op = new Operand(OPTYPE_REGISTER, op_reg);
         }
         last_reg_op = &op;
         inst_type instruction;
         if (mulop_attribute == MULOP_MUL || mulop_attribute == MULOP_AND) {
           instruction = INST_MUL;
         } else { 
           instruction = INST_DIV;
         }
         if (op2->get_type() == OPTYPE_REGISTER) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_r_value());
         } else if (op2->get_type() == OPTYPE_IMMEDIATE) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_i_value());
         } else if (op2->get_type() == OPTYPE_MEMORY) {
           e->emit_2addr(instruction, op->get_r_value(), op2->get_m_value());
         }
         if (op2->get_type()  == OPTYPE_REGISTER) {
           allocator->deallocate_register(op2->get_r_value());
         }
         delete op2;

         if (parse_term_prm(term_prm_type_1, op))
         {
            if (term_prm_type_1 == NO_T
                && mulop_type == factor_type){
              term_prm_type = mulop_type;
              return true;
            } else if (mulop_type == factor_type
                       && factor_type == term_prm_type_1){
              term_prm_type = mulop_type;
              return true;
            } else {
              type_error(mulop_type, factor_type, term_prm_type_1);
              return false;
            }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else 
   {
     term_prm_type = NO_T;
     return true;
   }
}

bool Parser::parse_factor(expr_type &factor_type, Operand *&op)
{
   if (word->get_token_type() == TOKEN_ID)
   {  
      string *identifier_attribute = static_cast<IdToken *>(word)->get_attribute();
      if (!stab.is_decl(identifier_attribute, current_env)){
        undeclared_identifier(identifier_attribute);
        return false;
      } else {
        factor_type = stab.get_type(identifier_attribute, current_env);
      }
      op = new Operand(OPTYPE_MEMORY, identifier_attribute);
      delete word;
      word = lex->next_token();
      return true;
   }
   else if (word->get_token_type() == TOKEN_NUM)
   {
      factor_type = INT_T;
      stringstream ss(*(static_cast<NumToken *>(word)->get_attribute()));
      int op_val;
      ss >> op_val;
      op = new Operand(OPTYPE_IMMEDIATE, op_val);
      delete word;
      word = lex->next_token();
      return true;
   }
   else if (word->get_token_type() == TOKEN_PUNC
      && static_cast<PuncToken *>(word)->get_attribute() == PUNC_OPEN)
   {
      delete word;
      word = lex->next_token();
      expr_type expr_type_result;
      if (parse_expr(expr_type_result, op))
      {
         if (word->get_token_type() == TOKEN_PUNC
            && static_cast<PuncToken *>(word)->get_attribute() == PUNC_CLOSE)
         {
            factor_type = expr_type_result;
            delete word;
            word = lex->next_token();
            return true;
         }
         else
         {
            string *expected = new string("')'");
            parse_error(expected, word);
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else if ((word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD)
      || (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB)
      || (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT))
   {
      delete word;
      word = lex->next_token();
      expr_type sign_type;
      int sign = -1;
      if (word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD) {
        sign = 0;
      } else if (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB) {
        sign = 1;
      } else if (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT) {
        sign = 2;
      }

      if (parse_sign(sign_type))
      {
         expr_type factor_type_1;
         if (parse_factor(factor_type_1, op))
         {
            if (sign_type != factor_type_1){
              type_error(sign_type, factor_type_1);
              return false; 
            } else {
              factor_type = factor_type_1;    
              if (sign != 0) {
                Register *op_reg;
                if (op->get_type() == OPTYPE_REGISTER) {
                  op_reg = op->get_r_value();
                } else {
                  if (!allocator->has_free_reg()) {
                    string *spill_location = e->get_new_label("spill");
                    e->emit_move(spill_location, (*last_reg_op)->get_r_value());
                    allocator->deallocate_register((*last_reg_op)->get_r_value());
                    labels.push_back(*spill_location);
                    *last_reg_op = new Operand(OPTYPE_MEMORY, spill_location);
                  }
                  op_reg = allocator->allocate_register();
                  if (op->get_type() == OPTYPE_IMMEDIATE) {
                    e->emit_move(op_reg, op->get_i_value());
                  } else if (op->get_type() == OPTYPE_MEMORY) {
                    e->emit_move(op_reg, op->get_m_value());
                  }
                  delete op;
                  op = new Operand(OPTYPE_REGISTER, op_reg);
                }
                last_reg_op = &op;
                if (sign == 1) {
                  e->emit_1addr(INST_NEG, op->get_r_value());
                } else if (sign == 2) {
                  e->emit_1addr(INST_NOT, op->get_r_value());
                }
              }

              return true;
            }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      string *expected = new string("(identifier, num, '(', '+', '-', keyword not)");
      parse_error(expected, word);
      return false;
   }
}

bool Parser::parse_sign(expr_type &sign_type)
{
   if (word->get_token_type() == TOKEN_ADDOP
      && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_ADD){
     sign_type = INT_T;
     delete word;
     word = lex->next_token();
     return true;
   } else if (word->get_token_type() == TOKEN_ADDOP
         && static_cast<AddopToken *>(word)->get_attribute() == ADDOP_SUB){
     sign_type = INT_T;
     delete word;
     word = lex->next_token();
     return true;
   } else if (word->get_token_type() == TOKEN_KEYWORD
         && static_cast<KeywordToken *>(word)->get_attribute() == KW_NOT){
     sign_type = BOOL_T;
     delete word;
     word = lex->next_token();
     return true;
   }
   else
   {
     sign_type = INT_T;
     return true;
   }
}

void Parser::parse_error(string *expected, Token *found)
{
   cout << "Parse error: Expected \"" << *expected << "\", found " << *(word->to_string()) << endl;
}

void Parser::multiply_defined_identifier (string *id) const
{
  cerr << "The identifier " << *id << " has already been declared. " << endl;
  exit(-1);
}

void Parser::undeclared_identifier (string *id) const
{
  cerr << "The identifier " << *id << " had not been declared. " << endl;
  exit(-1);
}

void Parser::type_error (const expr_type expected,
			 const expr_type found) const
{
  cerr << "Type error: expected " << *(stab.type_to_string (expected))
       << " found " << *(stab.type_to_string(found)) << "." << endl;
  exit (-1);
}

void Parser::type_error (const expr_type expected1,
			 const expr_type expected2,
			 const expr_type found) const
{
  cerr << "Type error: expected " << *(stab.type_to_string (expected1))
       << " or " << *(stab.type_to_string (expected2))
       << ", found " << *(stab.type_to_string(found)) << "." << endl;
  exit (-1);
}
