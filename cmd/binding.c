/*** binding.c *****************************************************************
** 
** This file is part of BibTool.
** It is distributed under the GNU General Public License.
** See the file COPYING for details.
** 
** (c) 2015-2016 Gerd Neugebauer
** 
** Net: gene@gerd-neugebauer.de
** 
******************************************************************************/

#include <bibtool/bibtool.h>
#include <bibtool/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "binding.h"
#include "lcore.h"

/*****************************************************************************/
/* Internal Programs                                                         */
/*===========================================================================*/

#ifdef __STDC__
#define _ARG(A) A
#else
#define _ARG(A) ()
#endif
 Binding binding _ARG((unsigned int size));
 Term eval_term _ARG((Binding binding, Term term));
 Term eval_self _ARG((Binding binding, Term term));

/*****************************************************************************/
/* External Programs                                                         */
/*===========================================================================*/


/*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
** Function:	binding()
** Type:	Binding
** Purpose:	Allocate a new instance of a Binding.
**		
** Arguments:
**	size	the number of junks contained
** Returns:	
**___________________________________________________			     */
Binding binding(size)			   	   /*                        */
  unsigned int size;				   /*                        */
{						   /*                        */
  Binding b = (Binding) malloc(sizeof(SBinding));  /*                        */
  if (b == BindingNULL) OUT_OF_MEMORY("binding");  /*                        */
 						   /*                        */
  BJunks(b) = (SymDef*) calloc(size, sizeof(SymDef));/*                      */
  if (BJunks(b) == NULL) OUT_OF_MEMORY("binding"); /*                        */
 						   /*                        */
  BSize(b) = size;				   /*                        */
  NextBinding(b) = BindingNULL;		   	   /*                        */
  return b;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	bind()
** Type:	void
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	sym	the symbol definition
** Returns:	nothing
**___________________________________________________			     */
void bind(binding, sym)		   		   /*                        */
  Binding binding;				   /*                        */
  SymDef sym;					   /*                        */
{ String key 	 = SymName(sym);		   /*                        */
  unsigned int h = hash(key) % BSize(binding);	   /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (junk = BJunks(binding)[h];		   /*                        */
       junk;					   /*                        */
       junk = NextJunk(junk))			   /*                        */
  { if (SymName(junk) == key)			   /*                        */
    { SymTerm(junk)  = SymTerm(sym);		   /*                        */
      SymValue(junk) = SymValue(sym);		   /*                        */
      SymGet(junk)   = SymGet(sym);		   /*                        */
      SymSet(junk)   = SymSet(sym);		   /*                        */
      return;					   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  NextJunk(sym) = BJunks(binding)[h];		   /*                        */
  BJunks(binding)[h]  = sym;			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	setq()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	b	the binding
**	key	the key
**	term	the term
** Returns:	
**___________________________________________________			     */
Term setq(b, key, term)		   		   /*                        */
  Binding b;				   	   /*                        */
  String key;					   /*                        */
  Term term;					   /*                        */
{ unsigned int h = hash(key) % BSize(b);	   /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (junk = BJunks(b)[h]; junk; junk = NextJunk(junk))/*                   */
  { if (SymName(junk) == key)			   /*                        */
    { UnlinkTerm(SymValue(junk));		   /*                        */
      SymValue(junk) = term;		   	   /*                        */
      LinkTerm(term);				   /*                        */
      return term;				   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  junk = symdef(key,			   	   /*                        */
		L_FIELD,		   	   /*                        */
		SYM_NONE,		   	   /*                        */
		g_field,		   	   /*                        */
		g_setq);  		   	   /*                        */
  SymValue(junk) = term;		   	   /*                        */
  NextJunk(junk) = BJunks(b)[h];		   /*                        */
  BJunks(b)[h]   = junk;			   /*                        */
  LinkTerm(term);				   /*                        */
  return term;					   /*                        */
}						   /*------------------------*/

#undef DEBUG_BIND

/*-----------------------------------------------------------------------------
** Function:	get_bind()
** Type:	SymDef
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	key	the key
** Returns:	
**___________________________________________________			     */
SymDef get_bind(binding, key)			   /*                        */
  Binding binding;				   /*                        */
  String key;					   /*                        */
{ SymDef s;					   /*                        */
  unsigned int h = hash(key) % BSize(binding);	   /*                        */
  						   /*                        */
#ifdef DEBUG_BIND
  puts("BINDING");				   /*                        */
  dump_binding(binding, stdout);		   /*                        */
  printf("--- lookup %s at %d\n", (char*)key, h);  /*                        */
#endif
  while (binding)				   /*                        */
  {						   /*                        */
    for (s = BJunks(binding)[h];		   /*                        */
	 s;					   /*                        */
	 s = NextJunk(s))			   /*                        */
    {						   /*                        */
#ifdef DEBUG_BIND
      printf("--- cmp %s\n",(char*)SymName(s));	   /*                        */
#endif
      if (cmp(SymName(s), key) == 0) {		   /*                        */
#ifdef DEBUG_BIND
	printf("--- found 0x%x\n",s ? SymOp(s):0); /*                        */
#endif
	return s; 	   			   /*                        */
      }						   /*                        */
    }						   /*                        */
    binding = NextBinding(binding);		   /*                        */
  }						   /*                        */
  return SymDefNULL; 	   			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	bool_s_rsc()
** Type:	static Term
** Purpose:	Setter for a boolean resource.
**		
** Arguments:
**	binding	the binding
**	name	the name
**	term	the term
**	rp	a pointer to the resource
** Returns:	the new value
**___________________________________________________			     */
static Term bool_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char * name;				   	   /*                        */
  Term term;					   /*                        */
  int * rp;					   /*                        */
{ term = eval_bool(binding, Cadr(term));	   /*                        */
  *rp  = (TermIsTrue(term) ? 1 : 0 );	   	   /*                        */
  UnlinkTerm(term);				   /*                        */
  return (*rp ? term_true : term_false);	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	num_rsc()
** Type:	static Term
** Purpose:	
**		
** Arguments:
**	binding	
**	name	
**	term	
**	rp	
** Returns:	
**___________________________________________________			     */
static Term num_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char * name;				   	   /*                        */
  Term term;					   /*                        */
  int * rp;					   /*                        */
{						   /*                        */
  term = eval_num(binding, Cadr(term));	   	   /*                        */
  *rp  = TNumber(term);	   		   	   /*                        */
  return term;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	str_rsc()
** Type:	static Term
** Purpose:	
**		
** Arguments:
**	binding	
**	name	
**	term	
**	rp	
** Returns:	
**___________________________________________________			     */
static Term str_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char *name;				   	   /*                        */
  Term term;					   /*                        */
  String *rp;					   /*                        */
{						   /*                        */
  term = eval_str(binding, Cadr(term));	   	   /*                        */
  *rp  = TString(term);	   		   	   /*                        */
  return term;				   	   /*                        */
}						   /*------------------------*/

#define Bind(NAME,GET,SET,FLAGS,OP)
#define BindSym(NAME,SYM)
#define BindBool(NAME,GETTER,SETTER,RSC)		\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return (RSC ? term_true : term_false); }		\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return bool_s_rsc(binding, NAME, term, &RSC); }
#define BindNum(NAME,GETTER,SETTER,RSC)			\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return NumberTerm(RSC); }				\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return num_s_rsc(binding, NAME, term, &RSC); }
#define BindStr(NAME,GETTER,SETTER,RSC)			\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern String RSC;					\
    return StringTerm(RSC ? RSC : (String)""); }	\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern String RSC;					\
    return str_s_rsc(binding, NAME, term, &RSC); }
#define BindFct(NAME,GETTER,SETTER,SETTER_FCT)		\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { ErrorNF(NAME, " is not accessible"); return NIL; }	\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { String val;						\
    term = eval_str(binding, Cadr(term));		\
    val  = TString(term);				\
    SETTER_FCT;						\
    return term; }
#define BindFunc(NAME,GETTER,GET_FCT,SETTER,SET_FCT)	\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { return GET_FCT; }					\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { String val;						\
    term = eval_str(binding, Cadr(term));		\
    val  = TString(term);				\
    SET_FCT;						\
    return GET_FCT; }
#include "builtin.h"

#undef Bind
#undef BindSym
#undef BindBool
#undef BindNum
#undef BindStr
#undef BindFct
#undef BindFunc

/*-----------------------------------------------------------------------------
** Function:	root_binding()
** Type:	Binding
** Purpose:	Create a new binding and initialize it with the BibTool
**		defaults.
**		
** Arguments:	none
** Returns:	the new binding
**___________________________________________________			     */
Binding root_binding()				   /*                        */
{ Binding b = binding(511);			   /*                        */
 						   /*                        */
#define BindBool(NAME,GET,SET,R)     Bind(NAME, GET, SET, SYM_BUILTIN, L_FIELD)
#define BindNum(NAME,GET,SET,R)	     Bind(NAME, GET, SET, SYM_BUILTIN, L_FIELD)
#define BindStr(NAME,GET,SET,R)	     Bind(NAME, GET, SET, SYM_BUILTIN, L_FIELD)
#define BindFct(NAME,GET,SET,EX)     Bind(NAME, GET, SET, SYM_BUILTIN, L_FIELD)
#define BindFunc(NAME,GET,G,SET,S)   Bind(NAME, GET, SET, SYM_BUILTIN, L_FIELD)
#define Bind(NAME,GET,SET,FLAGS,OP)  bind(b, symdef(symbol((String)NAME),     \
						    OP, FLAGS, GET, SET));
#define BindSym(NAME,SYM)	      bind(b, SYM);
 						   /*                        */
#include "builtin.h"
  return b;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	dump_binding()
** Type:	void
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	file	the target output file
** Returns:	nothing
**___________________________________________________			     */
void dump_binding(binding, file)		   /*                        */
  Binding binding;				   /*                        */
  FILE* file;					   /*                        */
{ int i;					   /*                        */
  SymDef s;					   /*                        */
 						   /*                        */
  for (; binding; binding = NextBinding(binding))  /*                        */
  {						   /*                        */
#ifdef DEBUG
    fprintf(file, "--- 0x%lx [%d] ---\n",	   /*                        */
	    (unsigned long)binding,		   /*                        */
	    BSize(binding));			   /*                        */
#endif
    for (i = 0; i < BSize(binding); i++)	   /*                        */
    { s = BJunks(binding)[i];			   /*                        */
      if (s) {					   /*                        */
	fprintf(file, "    #%d\n", i);		   /*                        */
	for (; s; s = NextJunk(s))		   /*                        */
	{ fprintf(file,				   /*                        */
		  "\t%s\n",			   /*                        */
		  SymName(s));			   /*                        */
	}					   /*                        */
      }						   /*                        */
    }						   /*                        */
  }						   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	eval_term()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	term	the term to evaluate
** Returns:	the resulting term
**___________________________________________________			     */
Term eval_term(binding, term)			   /*                        */
  Binding binding;				   /*                        */
  Term term;					   /*                        */
{ SymDef s;					   /*                        */
  String key = NULL;				   /*                        */
 						   /*                        */
  if (term == NIL) return NIL;			   /*                        */
 						   /*                        */
  switch (TType(term))				   /*                        */
  { case 0:					   /*                        */
    case EOF:					   /*                        */
      return term_eof;	   			   /*                        */
 						   /*                        */
    case L_STRING:				   /*                        */
    case L_NUMBER:				   /*                        */
    case L_TRUE:				   /*                        */
    case L_FALSE:				   /*                        */
    case L_CONS:				   /*                        */
      LinkTerm(term);				   /*                        */
      return term;				   /*                        */
 						   /*                        */
    case L_GROUP:				   /*                        */
      { Term tt, t = NIL;			   /*                        */
	for (tt = Cdr(term); tt; tt = Cdr(tt))     /*                        */
	{ t = eval_term(binding, Car(tt)); }	   /*                        */
	return t;				   /*                        */
      }						   /*                        */
 						   /*                        */
    case L_IF:				   	   /*                        */
      if (TermIsTrue(eval_bool(binding, Car(term))))/*                       */
      { return eval_term(binding, Cadr(term)); }   /*                        */
      						   /*                        */
      return (Cddr(term)			   /*                        */
	      ? eval_term(binding, Cddr(term))	   /*                        */
	      : NIL);				   /*                        */
 						   /*                        */
    case L_FUNCTION:				   /*                        */
      key = TString(term);			   /*                        */
      s	  = get_bind(binding, key);	   	   /*                        */
      if (s == NULL || SymGet(s) == NULL)	   /*                        */
	ErrorNF("Undefined function ", key);	   /*                        */
 						   /*                        */
      if (Cdr(term))				   /*                        */
      { if (SymSet(s) == NULL)	   		   /*                        */
	  ErrorNF(key, " is immutable");	   /*                        */
	return (*SymSet(s))(binding, term);	   /*                        */
      }						   /*                        */
      else					   /*                        */
      { if (SymGet(s) == NULL)	   		   /*                        */
	  ErrorNF(key, " is not readable");	   /*                        */
 						   /*                        */
	return (*SymGet(s))(binding, term);	   /*                        */
      }						   /*                        */
 						   /*                        */
    case L_FIELD:				   /*                        */
      s = get_bind(binding, TString(term));	   /*                        */
      return s ? SymValue(s) : NIL;		   /*                        */
 						   /*                        */
    case L_QUOTE:    key = (String)"'";	     break;/*                        */
    case L_MINUS:    key = (String)"-";	     break;/*                        */
    case L_PLUS:     key = (String)"+";	     break;/*                        */
    case L_TIMES:    key = (String)"*";	     break;/*                        */
    case L_DIV:      key = (String)"/";	     break;/*                        */
    case L_MOD:      key = (String)"mod";    break;/*                        */
    case L_SET:      key = (String)"=";	     break;/*                        */
    case L_LIKE:     key = (String)"like";   break;/*                        */
    case L_ILIKE:    key = (String)"ilike";  break;/*                        */
    case L_EQ:       key = (String)"==";     break;/*                        */
    case L_NE:       key = (String)"!=";     break;/*                        */
    case L_GT:       key = (String)">";	     break;/*                        */
    case L_GE:       key = (String)">=";     break;/*                        */
    case L_LT:       key = (String)"<";	     break;/*                        */
    case L_LE:       key = (String)"<=";     break;/*                        */
    case L_NOT:      key = (String)"!";	     break;/*                        */
    case L_AND:      key = (String)"&&";     break;/*                        */
    case L_OR:       key = (String)"||";     break;/*                        */
    default:					   /*                        */
      ErrorNF("Undefined tag ",		   	   /*                        */
	      tag_id(TType(term)));	   	   /*                        */
  }						   /*                        */
 						   /*                        */
   s = get_bind(binding, key);		   	   /*                        */
   if (s == SymDefNULL) 			   /*                        */
   { ErrorNF("Undefined function ", key); } 	   /*                        */

   return (*SymGet(s))(binding, term);		   /*                        */
}						   /*------------------------*/

/*---------------------------------------------------------------------------*/
