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

#include <string.h>
#include <stdlib.h>
#include <bibtool/symbols.h>
#include <bibtool/error.h>
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

#define Error(X,Y)					\
	error(ERR_ERROR|ERR_EXIT,(String)X,		\
	      (String)Y,(String)0,(String)0,(String)0,0,NULL)
/*-----------------------------------------------------------------------------
** Function:	binding()
** Type:	Binding
** Purpose:	
**		
** Arguments:
**	term	
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
**	b	
**	key	
**	term	
**	get	
**	set	
** Returns:	nothing
**___________________________________________________			     */
void bind(b, sym)		   		   /*                        */
  Binding b;				   	   /*                        */
  SymDef sym;					   /*                        */
{ String key 	 = SymName(sym);		   /*                        */
  unsigned int h = hash(key) % BSize(b);	   /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (junk = BJunks(b)[h]; junk; junk = NextJunk(junk))/*                   */
  { if (strcmp((char*)SymName(junk), (char*)key) == 0)/*                     */
    {						   /*                        */
      SymValue(junk) = SymValue(sym);		   /*                        */
      SymGet(junk)   = SymGet(sym);		   /*                        */
      SymSet(junk)   = SymSet(sym);		   /*                        */
      return;					   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  NextJunk(sym) = BJunks(b)[h];		   	   /*                        */
  BJunks(b)[h]  = sym;			   	   /*                        */
}						   /*------------------------*/

#undef DEBUG_BIND

/*-----------------------------------------------------------------------------
** Function:	get_bind()
** Type:	SymDef
** Purpose:	
**		
** Arguments:
**	b	
**	key	
** Returns:	
**___________________________________________________			     */
SymDef get_bind(b, key)			   	   /*                        */
  Binding b;				   	   /*                        */
  String key;					   /*                        */
{ SymDef s;					   /*                        */
  unsigned int h = hash(key) % BSize(b);	   /*                        */
  						   /*                        */

#ifdef DEBUG_BIND
  dump_binding(b, stdout);
  fprintf(stdout, "--- lookup %s at %d\n", (char*)key, h);
#endif
  while (b)					   /*                        */
  {						   /*                        */
    for (s = BJunks(b)[h];		   	   /*                        */
	 s;					   /*                        */
	 s = NextJunk(s))			   /*                        */
    {						   /*                        */
#ifdef DEBUG_BIND
      printf("--- cmp %s\n",(char*)SymName(s));	   /*                        */
#endif
      if (strcmp((char*)SymName(s),		   /*                        */
		 (char*)key) == 0) {		   /*                        */
#ifdef DEBUG_BIND
	puts(">>> found");			   /*                        */
#endif
	return s; 	   			   /*                        */
      }						   /*                        */
    }						   /*                        */
    b = NextBinding(b);		   		   /*                        */
  }						   /*                        */
  return SymDefNULL; 	   			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	g_print()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	binding	
**	 term	
** Returns:	
**___________________________________________________			     */
Term g_print(binding, term)			   /*                        */
  Binding binding;				   /*                        */
  Term term;					   /*                        */
{						   /*                        */
  print_term(stdout, Cdr(term));		   /*                        */
  return NIL;			   		   /*                        */
}						   /*------------------------*/

Term g_cons(binding, term)			   /*                        */
  Binding binding;				   /*                        */
  Term term;					   /*                        */
{						   /*                        */
  print_term(stdout, Cdr(term));		   /*                        */
  return NIL;			   		   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	def_binding()
** Type:	Binding
** Purpose:	
**		
** Arguments:
**		
** Returns:	
**___________________________________________________			     */
Binding def_binding()				   /*                        */
{ Binding b = binding(511);			   /*                        */

#define BIND(NAME)   	bind(b, symdef(symbol((String)NAME), L_FIELD, NIL, NULL))
#define Bind(NAME, SYM) bind(b, SYM)

  BIND("add.field"		);   		   /* RscByFct	             */
  BIND("apply.alias"		);   		   /* RscBoolean	     */
  BIND("apply.modify"		);   		   /* RscBoolean	     */
  BIND("apply.include"		);   		   /* RscBoolean	     */
  Bind("and"			, sym_and);   	   /* RscTerm	             */
  BIND("bibtex.env.name"	);   		   /* RscString	             */
  BIND("bibtex.search.path"	);   		   /* RscString	             */
  BIND("check.double"		);   		   /* RscBoolean	     */
  BIND("check.double.delete"	);   		   /* RscBoolean	     */
  BIND("check.rule"		);   		   /* RscByFct	             */
  BIND("check.case.sensitive"	);   		   /* RscBoolean	     */
  BIND("clear.ignored.words"	);   		   /* RscByFct	             */
  BIND("count.all"		);   		   /* RscBoolean	     */
  BIND("count.used"		);   		   /* RscBoolean	     */
  BIND("crossref.limit"		);   		   /* RscNumeric	     */
  BIND("default.key"		);   		   /* RscByFct	             */
  BIND("delete.field"		);   		   /* RscByFct	             */
  BIND("dir.file.separator"	);   		   /* RscString	             */
  BIND("dump.symbols"		);   		   /* RscBoolean	     */
  BIND("env.separator"		);   		   /* RscString	             */
  BIND("extract.file"		);   		   /* RscByFct	             */
  BIND("extract.regex"		);   		   /* RscByFct	             */
  BIND("expand.macros"		);   		   /* RscBoolean	     */
  BIND("expand.crossref"	);   		   /* RscBoolean	     */
  BIND("fmt.inter.name"		);   		   /* RscByFct	             */
  BIND("fmt.name.pre"		);   		   /* RscByFct	             */
  BIND("fmt.name.name"		);   		   /* RscByFct	             */
  BIND("fmt.name.title"		);   		   /* RscByFct	             */
  BIND("fmt.title.title"	);   		   /* RscByFct	             */
  BIND("fmt.et.al"		);   		   /* RscByFct	             */
  BIND("fmt.word.separator"	);   		   /* RscByFct	             */
  BIND("field.type"		);   		   /* RscByFct	             */
  Bind("false"			, sym_false);      /* RscTerm	             */
  BIND("input"			);   		   /* RscByFct	             */
  BIND("ignored.word"		);   		   /* RscByFct	             */
  Bind("ilike"			, sym_ilike);	   /* RscTerm	             */
  BIND("key.generation"		);   		   /* RscBoolean	     */
  BIND("key.base"		);   		   /* RscByFct	             */
  BIND("key.format"		);   		   /* RscByFct	             */
  BIND("key.make.alias"		);   		   /* RscBoolean	     */
  BIND("key.number.separator"	);   		   /* RscByFct	             */
  BIND("key.expand.macros"	);   		   /* RscBoolean	     */
  Bind("like"			, sym_like);	   /* RscTerm	             */
  BIND("macro.file"		);   		   /* RscByFct	             */
  Bind("mod"			, sym_mod);	   /* RscTerm	             */
  BIND("new.entry.type"		);   		   /* RscByFct	             */
  BIND("new.field.type"		);   		   /* RscByFct	             */
  BIND("new.format.type"	);   		   /* RscByFct	             */
  Bind("not"			,sym_not);   	   /* RscTerm	             */
  BIND("nil"			);   		   /* RscTerm	             */
  BIND("output.file"		);   		   /* RscByFct	             */
  Bind("or"			,sym_or);   	   /* RscTerm	             */
  BIND("pass.comments"		);   		   /* RscBoolean	     */
  BIND("preserve.key.case"	);   		   /* RscBoolean	     */
  BIND("preserve.keys"		);   		   /* RscBoolean	     */
  BIND("print"			);   		   /* RscByFct	             */
  BIND("print.align.string"	);   		   /* RscNumeric	     */
  BIND("print.align.comment"	);   		   /* RscNumeric	     */
  BIND("print.align.preamble"	);   		   /* RscNumeric	     */
  BIND("print.align.key"	);   		   /* RscNumeric	     */
  BIND("print.align"		);   		   /* RscNumeric	     */
  BIND("print.all.strings"	);   		   /* RscBoolean	     */
  BIND("print.entry.types"	);   		   /* RscString	             */
  BIND("print.equal.right"	);   		   /* RscBoolean	     */
  BIND("print.braces"		);   		   /* RscBoolean	     */
  BIND("print.comma.at.end"	);   		   /* RscBoolean	     */
  BIND("print.deleted.prefix"	);   		   /* RscString	             */
  BIND("print.deleted.entries"	);   		   /* RscBoolean	     */
  BIND("print.indent"		);   		   /* RscNumeric	     */
  BIND("print.line.length"	);   		   /* RscNumeric	     */
  BIND("print.newline"		);   		   /* RscNumeric	     */
  BIND("print.parentheses"	);   		   /* RscBoolean	     */
  BIND("print.terminal.comma"	);   		   /* RscBoolean	     */
  BIND("print.use.tab"		);   		   /* RscBoolean	     */
  BIND("print.wide.equal"	);   		   /* RscBoolean	     */
  Bind("quote"			, sym_quote);	   /*	                     */
  BIND("quiet"			);   		   /* RscBoolean	     */
  BIND("regexp.syntax"		);   		   /* RscByFct	             */
  BIND("rename.field"		);   		   /* RscByFct	             */
  BIND("resource"		);   		   /* RscByFct	             */
  BIND("resource.search.path"	);   		   /* RscByFct	             */
  BIND("rewrite.rule"		);   		   /* RscByFct	             */
  BIND("rewrite.case.sensitive"	);   		   /* RscBoolean	     */
  BIND("rewrite.limit"		);   		   /* RscNumeric	     */
  BIND("select"			);   		   /* RscByFct	             */
  BIND("select.by.string"	);   		   /* RscByFct	             */
  BIND("select.by.non.string"	);   		   /* RscByFct	             */
  BIND("select.by.string.ignored");   		   /* RscString	             */
  BIND("select.case.sensitive"	);   		   /* RscBoolean	     */
  BIND("select.fields"		);   		   /* RscString	             */
  BIND("select.non"		);   		   /* RscByFct	             */
  BIND("select.crossrefs"	);   		   /* RscBoolean	     */
  BIND("sort"			);   		   /* RscBoolean	     */
  BIND("sort.cased"		);   		   /* RscBoolean	     */
  BIND("sort.macros"		);   		   /* RscBoolean	     */
  BIND("sort.reverse"		);   		   /* RscBoolean	     */
  BIND("sort.order"		);   		   /* RscByFct	             */
  BIND("sort.format"		);   		   /* RscByFct	             */
  BIND("suppress.initial.newline");   		   /* RscBoolean	     */
  BIND("symbol.type"		);   		   /* RscByFct	             */
  BIND("tex.define"		);   		   /* RscByFct	             */
  Bind("true"			,sym_true);   	   /* RscTerm	             */
  BIND("verbose"		);   		   /* RscBoolean	     */
  BIND("version"		);   		   /* RscByFct	             */

  Bind("="		, sym_set);		   /*                        */
  Bind("=="		, sym_eq);		   /*                        */
  Bind("<="		, sym_le);		   /*                        */
  BIND("<"		);			   /*                        */
  BIND(">="		);			   /*                        */
  BIND(">"		);			   /*                        */
  BIND("!="		);			   /*                        */
  BIND("+"		);			   /*                        */
  BIND("-"		);			   /*                        */
  BIND("*"		);			   /*                        */
  Bind("&&"		, sym_and);		   /*                        */
  Bind("||"		, sym_or);		   /*                        */
  Bind("!"		, sym_not);		   /*                        */
  Bind("'"		, sym_quote);		   /*                        */
  BIND("`"		);			   /*                        */
  Bind("cons"		, sym_cons);		   /*                        */
  return b;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	dump_binding()
** Type:	void
** Purpose:	
**		
** Arguments:
**	b	
**	 file	
** Returns:	nothing
**___________________________________________________			     */
void dump_binding(b, file)			   /*                        */
  Binding b;					   /*                        */
  FILE* file;					   /*                        */
{ int i;					   /*                        */
  SymDef s;					   /*                        */
 						   /*                        */
  for (; b; b = NextBinding(b))			   /*                        */
  {						   /*                        */
    fprintf(file, "--- 0x%lx [%d] ---\n", (unsigned long)b, BSize(b));/*     */
    for (i = 0; i < BSize(b); i++)		   /*                        */
    { s = BJunks(b)[i];				   /*                        */
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
**	binding	
**	 term	
** Returns:	
**___________________________________________________			     */
Term eval_term(binding, term)			   /*                        */
  Binding binding;				   /*                        */
  Term term;					   /*                        */
{ SymDef junk;					   /*                        */
  String key;
 						   /*                        */
  if (term == NIL) return NIL;			   /*                        */

  if (TSym(term) == sym_cons)
  { Term t = Car(term);

    if (t == NIL || TSym(term) != sym_field)
    { Error("Undefined function ", TermName(term)); }/*                        */

    key = SymName(TSym(t));

  } else {
    key = TermName(term);
  }
 						   /*                        */
  junk = get_bind(binding, key);	   	   /*                        */
 						   /*                        */
  if (junk == NULL)	   			   /*                        */
  { Error("Undefined function ", key); }	   /*                        */
  if (SymGet(junk) == NULL)	   		   /*                        */
  { Error("Undefined getter for ", key); }	   /*                        */
 						   /*                        */
  return (*SymGet(junk))(binding, term);	   /*                        */
}						   /*------------------------*/

/*---------------------------------------------------------------------------*/